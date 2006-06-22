# login.py
# part of logintools
# A CGI Authentication and user account system
# Copyright (C) 2004/2005 Michael Foord
# E-mail: fuzzyman AT voidspace DOT org DOT uk

# Released subject to the BSD License
# Please see http://www.voidspace.org.uk/documents/BSD-LICENSE.txt

# Scripts maintained at http://www.voidspace.org.uk/python/index.shtml
# Comments, suggestions and bug reports welcome.

# For information about bugfixes, updates and support, please join the Pythonutils mailing list.
# http://voidspace.org.uk/mailman/listinfo/pythonutils_voidspace.org.uk
# Comments, suggestions and bug reports welcome.

import sys
import cgi
import os
import sha
from time import time

# FIXME: cgitb is insecure and should be removed from production code.
import cgitb
cgitb.enable()


sys.path.append('../modules')
from configobj import ConfigObj
from dataenc import pass_enc, pass_dec, unexpired, table_enc, table_dec
from Cookie import SimpleCookie
from pathutils import *
from cgiutils import *
from loginutils import *

# a default max age of 15 days. This ensures encoded cookies don't have
# unlimited lifetimes.
AGETEST = (15,0,0)
THISSCRIPT = os.environ['SCRIPT_NAME']

####################################################
# The names for the various template files used by login_utils
# These must be stored in templatedir
# Which must be given in 'config.ini' in userdir

logintemplate = 'login_page.html'
form_nojs = 'login_nojs.txt'

#####################################################

"""
Cookie string format is : ::

    username||password SHA||random string``

which is dataenc'ed with the date-time the cookie was generated.

The cookie string must successfully de-enc and the username - pass hash match
for the cookie to be a valid login. The password is hashed along with the
random string, so that the hash is different every time.

.. warning::

    We don't log failed login attempts. We could return ``None`` for no cookie
    and ``False`` for a failed login or 'bad' cookie.

The function called is usually determined by user choices. E.g.

    * Login
    * perform admin functions
    * create new login
    
The actual action is saved in the form field 'login'.

The function login determines which function to call. This means most scripts
only need directly import and call this function.

If there is no 'login' form field then the login function assumes that you
want to check if the user already has a valid login. This is the 'checklogin'
function.

Many operations of these scripts will generate new pages asking for further
intervention from the user. For example - login generates a login page which
waits for the user to enter a password. This means if the action interrupts a
script (e.g. before performing an action the script must verify user identity)
information about what the script was about to do may be lost. The login
function allows you to pass in an 'action' parameter. This must be a string
escaped to be URL safe. You can use this value to store information about what
was being done before the interruption. (A simple way of saving state).
"""
######################################################################
# login is the function most frequently imported
# and called directly by external scripts

def login(theform, userdir, thisscript=None, action=None):
    """From the form decide which function to call."""
    # FIXME: this function got a bit more complicated than intended
    # it also handles checking logins when editaccount or admin
    # functions are being called.
    br = False
    loginaction = ''
    if thisscript is None:
        thisscript = THISSCRIPT
    #
    # used so that we can tell if we have a genuinely empty action
    # or not later on....
    action = sortaction(theform.getfirst('action') or action)
    #
    # let's work out what we're supposed - if nothing is specified we'll just
    # check if we have a valid login and then return
    try:
        loginaction = theform['login'].value
    except KeyError:
        # this function displays login and exits if there is no valid login
        action, userconfig, newcookie = checklogin(userdir, thisscript, action)
        # break - means go straight to the end
        br = True
    #
    # dologin is special
    # it can be called by functions here - they may require a user to login before continuing.
    # So if we are returning from a login - we may need to go onto another function
    if loginaction.startswith('login'):
        action, userconfig, newcookie = dologin(theform, userdir, thisscript,
                                                    action)       # this function displays login and exits if username doesn't exist or password doesn't match
# if this function returns, then the login attempt was succesful
        userconfig['numlogins'] = str(int(userconfig['numlogins']) + 1)     # increment the number of logins in the user config
        userconfig.write()
        if action[:11] == 'edacc-mjf||':
            loginaction = 'editaccount'
        elif action[:11] == 'admin-mjf||':
            loginaction = 'admin'
        else:
            br = True
        
# this is the list of possible actions
# they must either call sys.exit() or return action, userconfig, newcookie
# functions which can be called without an explicit checklogin must do their own first (to get the new cookie header) - e.g. admin and edit account (otherwise you could access them without being logged in)
# if we have already done all this, then br will be set to True
    if br:
        pass
    elif loginaction == 'showlogin':
        displaylogin(userdir, thisscript)
    elif loginaction == 'logout':
# this means a logout link can be implemented as myscript.py?login=logout
        print logout(userdir)   # this prints the empty cookie
        displaylogin(userdir, thisscript)       # we don't use 'action' in this case because user has explicitly logged out, outstanding action is lost.
    elif loginaction == 'newlogin':
        from newlogin import newlogin
        newlogin(userdir, thisscript, action)           # this script always exits - so no return, preserves action though if needed
    elif loginaction.startswith('donewlogin'):
        from newlogin import donewlogin
        donewlogin(theform, userdir, thisscript, action)           # this script always exits - so no return, preserves action though if needed
    elif loginaction == 'confirm':
        from newlogin import confirm
        action, userconfig, newcookie = confirm(theform, userdir, thisscript)           # no action sent in, because the action is stored in the temporary user store
        userconfig['created'] = str(time())
    elif loginaction.startswith('editaccount'):
        from newlogin import editaccount
        if action[:11] != 'edacc-mjf||':
            action, userconfig, newcookie = checklogin(userdir, thisscript, 'edacc-mjf||' + action)
        action = action[11:]
        if not istrue(userconfig['editable']):
            error('Sorry, this account cannot be edited.')
        editaccount(userdir, thisscript, userconfig, sortaction(action), newcookie)           # we have checked that we have a valid login. This script prints a page, so no return
    elif loginaction.startswith('doeditaccount'):
        from newlogin import doeditaccount
        action, userconfig, newcookie = checklogin(userdir, thisscript, action)
        if not istrue(userconfig['editable']):
            error('Sorry, this account cannot be edited.')
        action, userconfig, newcookie = doeditaccount(theform, userconfig, userdir, thisscript, sortaction(action), newcookie)           # we have checked that we have a valid login
    elif loginaction == 'admin':
        from admin import admin
        if action[:11] != 'admin-mjf||':
            action, userconfig, newcookie = checklogin(userdir, thisscript, 'admin-mjf||' + action)
        action = action[11:]
        adminlevel = int(userconfig['admin'])
        if not adminlevel:
            error("You're not authorised to do that. Sorry.")
        action, userconfig, newcookie = admin(theform, userdir, thisscript, userconfig, sortaction(action), newcookie)           # we have checked that we have a valid login. 

    else:
        displaylogin(userdir, thisscript, action) # we haven't understood - just display the login screen XXXX error message instead ?          
    
    print newcookie     # XXXX ought to be a way of returning the cookie object instead...
    if action == 'EMPTY_VAL_MJF':           # the programmer ought never to 'see' this value, although it might get passed to his script by the login code a few times....
        action = None
    userconfig['lastused'] = str(time())
    userconfig['numused'] = str(int(userconfig['numused']) + 1)
    userconfig.write()
    return action, userconfig
    
############################################################
# Action functions - check a login etc

def checklogin(userdir, thisscript=None, action=None):
    """Check if a user has a valid cookie and return an updated one
    or display the default login screen and exit."""
    if thisscript == None:
        thisscript = THISSCRIPT 
    test = isloggedin(userdir)
    if test:
        return action, test[0], test[1]
    else:
        displaylogin(userdir, thisscript, action)

def dologin(theform, userdir, thisscript=None, action=None):
    """Ths is the function called by login when the results of a login form are posted.
    I.e. this actually does the login
    If any errors occur (like wrong/missing password) it calls displaylogin and exits.
    """
    loginaction = theform['login'].value                # used when we have js and nojs versions of dologin
    theform = getform(['username', 'pass'], theform)
    if thisscript == None:
        thisscript = THISSCRIPT 
    if loginaction == 'loginnojs':
        username = theform['username']
        password  = theform['pass']
        test = checkpass(username, password, userdir, thisscript, action)
        return test or displaylogin(userdir, thisscript, action)
    displaylogin(userdir, thisscript, action)                       # XXXX should display error message
    
def displaylogin(userdir, thisscript=None, action=None):
    """This function will display the login page and then exit.
    Usually called if the user has no cookie or an expired/forged cookie.
    """
    if thisscript == None:
        thisscript = THISSCRIPT
    config = ConfigObj(userdir + 'config.ini')
    templatedir = config['templatedir']

    loginform = readfile(templatedir+form_nojs)
    loginform = loginform.replace('**script**', thisscript)

    loginpage = readfile(templatedir+logintemplate)  
    loginpage = loginpage.replace('**login form**', loginform)

    if istrue(config['newloginlink']):
        loginpage = loginpage.replace('<!-- **commstart**', '')
        loginpage = loginpage.replace('**commend** -->', '')
        loginpage = loginpage.replace('**new login link**', thisscript+'?login=newlogin')

    if action:
        loginpage = loginpage.replace('<!-- **action** -->', actionline % action)
    print serverline
    print '\r'
    print loginpage
    
    sys.exit()
    
def logout(userdir):
    """Returns the cookie header in the case of an explicit logout."""
    config = ConfigObj(userdir + 'config.ini')
    cookiepath =  config['cookiepath']
    return emptycookie(cookiepath)
    
############################################################################

def isloggedin(userdir):
    """If user has sent us an in date, valid cookie then return updated cookie header,
    otherwise return False."""
    try:
        rawcookie = os.environ['HTTP_COOKIE']
    except KeyError:
        return False
    thecookie = SimpleCookie(rawcookie)
    try:
        cookiestring = thecookie['userid'].value
    except KeyError:
        return False
    test = decodestring(cookiestring, userdir)
    if not test:
        return False
    user, password, cookiepath = test
    thecookie = makecookie(user, password, cookiepath)
    return user, thecookie
    
def decodestring(cookiestring, userdir):
    """Given a username/password encoded into a string - decode it and check it's validity.
    It checks the username against the one stored in the user file..
    """
# try decoding the string, if it's badly formed then it may raise an excpetion - in which case we just return False
    try:
        instring, daynumber, timestamp = pass_dec(cookiestring)
    except:
        return False
# check it's not a really old (or copied) cookie
    if not unexpired(daynumber, timestamp, AGETEST):
        return False
# we've extracted the timestamped string from the cookie string.
# Let's pull out the username and password hash
    try:
        username, passhash, ranstring = instring.split('||')
    except ValueError:
        return False
    if not len(ranstring) == 10:
        return False
# Now we need to check it's a valid username and check the password
    if username in RESERVEDNAMES or not os.path.isfile(userdir+username+'.ini'):
        return False
    user = ConfigObj(userdir+username+'.ini')
    stampedpass = user['password']
    maxage = user['max-age']
    cookiepath = ConfigObj(userdir+'config.ini')['cookiepath']
# the password is time stamped - so we need to decode it 
    try:
        password, daynumber, timestamp = pass_dec(stampedpass)
    except:
        return False
    thishash = sha.new(password+ranstring).hexdigest()
    if thishash != passhash:
        return False
    return user, password, cookiepath

def encodestring(username, password):
    """Given a username and password return a new encoded string for use by decodecookie."""  
    ranstring = randomstring(10)
    thishash = sha.new(password + ranstring).hexdigest()
    return pass_enc('||'.join([username, thishash, ranstring]), daynumber=True, timestamp=True)

def checkpass(username, password, userdir, thisscript, action):
    """Check the password from a new login."""
# XXXX log failed login attempts
    if username in RESERVEDNAMES:
        return False
    if not os.path.isfile(userdir+username+'.ini'):
        return False
    user = ConfigObj(userdir+username+'.ini')
    stampedpass = user['password']
    cookiepath = ConfigObj(userdir+'config.ini')['cookiepath']
# we need to un-time stamp the password
    realpass, daynumber, timestamp = pass_dec(stampedpass)
    if realpass != password:
        return False
    open('test.txt', 'w').write(str(user))
# if we've got this far then the login was successful and we need to return a cookie
    thecookie = makecookie(user, password, cookiepath)
    return action, user, thecookie

"""
CHANGELOG
=========

2005/09/09
----------

Changes to work with pythonutils 0.2.2

"""
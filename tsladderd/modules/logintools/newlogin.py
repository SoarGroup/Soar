# newlogin.py
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
import os
sys.path.append('../modules')
from configobj import ConfigObj
from pathutils import *
from cgiutils import *
from loginutils import *

####################################################
# Various default values etc

newlogintemplate = 'newlogin_page.html'
newform_nojs = 'newlogin_nojs.txt'
logindone = 'login_done.html'
editform_nojs = 'editform_nojs.txt'
edacctemplate = 'edacc_page.html'
newloginlist = ['email', 'username', 'realname', ]
newloginformkeys = {'username' : 'Login Name', 'pass1' : 'Password', 'pass2' : 'Confirmation Password',
         'email' : 'Email Address', 'realname' : 'Real Name'}
edacckeys = ['realname', 'username', 'pass0', 'pass1', 'pass2', 'email']

# FIXME: do we trust this in all cases ? (is it part of the CGI spec or optional)
# can't we just use SCRIPT_NAME ?
SCRIPTLOC = 'http://' + os.environ.get('HTTP_HOST', '')

##########################################################
# functions for handling new logins

def newlogin(userdir, thisscript, action=None):
    """Display the newlogin form."""
    config = ConfigObj(userdir + 'config.ini')
    templatedir = config['templatedir']
    # check that new logins are enabled on this setup
    checknewlogin(userdir)
    #
    newloginform = readfile(templatedir+newform_nojs)
    newloginform = newloginform.replace('**script**', thisscript)
    #
    newloginpage = readfile(templatedir+newlogintemplate)  
    newloginpage = newloginpage.replace('**new login form**', newloginform)
    newloginpage = filltemplate(newloginpage)
    #
    if action:
        newloginpage = newloginpage.replace('<!-- **action** -->', actionline % action)
    print serverline
    print '\r'
    print newloginpage
    #
    sys.exit()

def donewlogin(theform, userdir, thisscript, action=None):
    """Process the results from new login form submissions."""
    loginaction = theform['login'].value
    if not loginaction == 'donewloginnojs':
        # only type of newlogin supported so far
        sys.exit()
    # check that new logins are enabled on this setup
    checknewlogin(userdir)
    allentries = theform.keys()
    vallist = allentries + [entry for entry in newloginformkeys if entry not in allentries]  
    formdict = getform(vallist, theform, nolist=True)
    for entry in newloginformkeys:
        if not formdict[entry]:
            invalidentry('Required field missing : "%s"' % newloginformkeys[entry], formdict, userdir, thisscript, action)
    # missingentry should redisplay the form, with values filled in, and then exit
    email = validateemail(formdict)
    if not email:
        invalidentry('Email address appears to be invalid.', formdict, userdir, thisscript, action)
    # if the address is invalid it should behave like missingentry
    # also max one login per email address ?
    login = validatelogin(formdict, userdir, thisscript, action)
    # if login name already exists or is invalid
    password = validatepass(formdict, userdir, thisscript, action)
    # if passwords don't match or are too short 
    realname = formdict['realname']
    # If we've got this far we've got a valid new login and need to save the
    # details, send the email and print a mesage
    #
    config = ConfigObj(userdir + 'config.ini') 
    link_url = savedetails(userdir, formdict, action)     # this includes any 'extra' keys, not just the ones we require      
    msg = config['email_message'] + '\n'
    msg = msg + SCRIPTLOC + thisscript + '?login=confirm&id=' + link_url
    writefile('log.txt', msg)
    sendmailme(email, msg, config['email_subject'], config['adminmail'], html=False)
    #
    templatedir = config['templatedir']
    logindonepage = readfile(templatedir+logindone)
    logindonepage = logindonepage.replace('**this script**', thisscript)
    #
    print serverline
    print '\r'
    print logindonepage
    #
    sys.exit()
    
def confirm(theform, userdir, thisscript):
    """Confirm a login.
    Either from an invite or from a user who has registered."""
    from dataenc import pass_dec, pass_enc
    from login import encodestring
    fail = False
    try:
        theval, daynumber, timestamp = pass_dec(theform['id'].value)
    except:
        # FIXME: bare except....
        newloginfail()
    tempstore = ConfigObj(userdir + 'temp.ini')
    if not tempstore.has_key(theval):
        newloginfail()
    uservals = tempstore[theval]
    del tempstore[theval]
    username = uservals['username']
    if username in tempstore['pending']:
        tempstore['pending'].remove(username)
    tempstore.write()
    #
    newconfig = ConfigObj(userdir + 'default.ini')
    newpath = userdir + username + '.ini'
    if os.path.isfile(newpath):
        newloginfail()
    newconfig.filename = newpath
    # FIXME: should this be '' ?
    action = None
    for entry in uservals:
        if entry == 'action':
            action = uservals[entry]
        elif entry == 'password':
            password = uservals[entry]
            newconfig[entry] = pass_enc(password, timestamp=True, daynumber=True)
        else:
            newconfig[entry] = uservals[entry]
    newconfig.write()
    #
    # next we need to create the cookie header to return it 
    from Cookie import SimpleCookie
    thecookie = SimpleCookie()
    thecookie['userid'] = encodestring(newconfig['username'], password)
    config = ConfigObj(userdir + 'config.ini')
    maxage = newconfig['max-age'] 
    cookiepath = config['cookiepath']
    if maxage and int(maxage):            # possible cause of error here if the maxage value in a users file isn't an integer !!
        thecookie['userid']['max-age'] = int(maxage) 
    if cookiepath:
        thecookie['userid']['path'] = cookiepath 
    if config['adminmail']:
        msg = 'A new user has created a login - "%s".\n\n' % thisscript
        for entry in newconfig:
            if entry != 'password':
                msg += entry + '   :   ' + newconfig[entry] + '\n'
        # FIXME: should be mailme
        sendmailme(config['adminmail'], msg, config['email_subject'],
                config['adminmail'], html=False)
    return action, newconfig, thecookie.output()
        
def editaccount(userdir, thisscript, userconfig, action, newcookie):
    """Display the form for the user to edit their account details."""
    msg = ''
    display_edit(userconfig, userdir, thisscript, msg, action, newcookie)
   
def doeditaccount(theform, userconfig, userdir, thisscript, action, newcookie):
    """Process the results from edit account form submissions."""
    from dataenc import pass_enc, pass_dec
    loginaction = theform['login'].value
    if not loginaction == 'doeditaccountnojs':                      # only type of newlogin supported so far
        sys.exit()
    allentries = theform.keys()
    vallist = allentries + [entry for entry in edacckeys if entry not in allentries]
    formdict = getform(vallist, theform, nolist=True)
    #
    oldpass = formdict['pass0']
    storedpass = pass_dec(userconfig['password'])[0] 
    pass1 = formdict['pass1']
    pass2 = formdict['pass2']
    #
    email = validateemail(formdict)
    oldemail = userconfig['email']
    if not email:
        msg = 'The email address you supplied appears to be invalid.'
        display_edit(formdict, userdir, thisscript, msg, action, newcookie, userconfig)
    if email != oldemail and (not oldpass or oldpass != storedpass):
        msg = 'You must correctly enter your password to change your email address.'
        display_edit(formdict, userdir, thisscript, msg, action, newcookie, userconfig)
    userconfig['email'] = email
    if not formdict['realname']:
        msg = 'You need to enter a name for us to use.'
        display_edit(formdict, userdir, thisscript, msg, action, newcookie, userconfig)
    userconfig['realname'] = formdict['realname']
    if pass1 or pass2:
        if pass1 != pass2: 
            msg = "The two passwords don't match."
            display_edit(formdict, userdir, thisscript, msg, action, newcookie, userconfig)
        if len(pass1) < 5:
            msg = "The password must be longer than 5 characters."
            display_edit(formdict, userdir, thisscript, msg, action, newcookie, userconfig)
        if not oldpass or oldpass != storedpass:
            msg = 'You must correctly enter your current password to change it.'
            display_edit(formdict, userdir, thisscript, msg, action, newcookie, userconfig)
        userconfig['password'] = pass_enc(pass1, daynumber=True, timestamp=True)
        newcookie = makecookie(userconfig, pass1, ConfigObj(userdir+'config.ini')['cookiepath'])
    for entry in formdict:
        if entry not in edacckeys:
            userconfig[entry] = formdict[entry]
    userconfig.write()
    return action, userconfig, newcookie                # XXXXX display values changed page
                                                        # XXXXX must change cookie if password changed

#################################################
# helper functions

# in the new login and edit account templates you need value="**keyname1**" in the relevant input fields
# plus a comment like this somewhere in the document:
# <!-- **keynamelist** keyname1, keyname2, keyname3 -->
# When a validation error occurs - the template is reprinted with the existing values filled into the template
# XXXX There's no way (yet) of making any of these new keys required keys though.
# XXXX the first time this happens it ought to fill in the values from 'default.ini'
def filltemplate(template, formdict=None, vallist=None):
    """Fill in the blanks in a new login form."""
    if not formdict:
        formdict = {}
    if not vallist:
        vallist = newloginlist
    index = template.find('**keynamelist**')
    if index == -1:
        keynames = []
    else:
        start = index
        while index < len(template)-1:
           index += 1
           if template[index] in '"\'><':
               break
        keynames = [entry.strip() for entry in template[start:index].split(',') if entry not in vallist]
    for entry in keynames+vallist:
        if entry in ['pass1', 'pass2']:
           continue
        template = template.replace('**%s**' % entry, formdict.get(entry, ''))
    return template


def validateemail(formdict):
    email = formdict['email'].strip()
    return validemail(email)
  
def validatelogin(formdict, userdir, thisscript, action):
    name = formdict['username']
    tempstore = ConfigObj(userdir + 'temp.ini')
    pendinglist = tempstore.get('pending', [])
    if os.path.isfile(userdir+name+'.ini') or name in pendinglist or name.lower() in RESERVEDNAMES:
        invalidentry('Username already exists.', formdict, userdir, thisscript, action)
    for char in name.lower():
        if not char in validchars:
            invalidentry('Username contains invalid characters.', formdict, userdir, thisscript, action)
    return name

def validatepass(formdict, userdir, thisscript, action):
    pass1 = formdict['pass1']
    pass2 = formdict['pass2']
    if pass1 != pass2:
        invalidentry('The two passwords don\'t match.', formdict, userdir, thisscript, action)
    if len(pass1) < 5:
        invalidentry('The password must be at least 5 characters long.', formdict, userdir, thisscript, action)
    return pass1
    
def invalidentry(msg, formdict, userdir, thisscript, action=None):
    """Display the newlogin form."""
    config = ConfigObj(userdir + 'config.ini')
    templatedir = config['templatedir']
    #
    newloginform = readfile(templatedir+newform_nojs)
    newloginform = newloginform.replace('**script**', thisscript)
    #
    newloginpage = readfile(templatedir+newlogintemplate)  
    newloginpage = newloginpage.replace('**new login form**', newloginform)
    newloginpage = filltemplate(newloginpage, formdict)
    newloginpage = newloginpage.replace('<!-- **message** -->', '<h2>'+msg+'</h2><br>')
    #
    if action:
        newloginpage = newloginpage.replace('<!-- **action** -->', actionline % action)
    print serverline
    print '\r'
    print newloginpage
    #
    sys.exit()

def savedetails(userdir, formdict, action=None):
    """
    Given the form from a validated new login, it saves the details to the 
    temporary store.
    
    It also cleans up any out of date ones that haven't been used.
    """
    from dateutils import returndate, daycount
    from dataenc import pass_enc
    #
    tempstore = ConfigObj(userdir + 'temp.ini')
    if action: 
        formdict['action'] = action
    year, month, day = returndate()
    today = daycount(year, month, day)
    #
    for section in tempstore:
        if section[4:].isdigit():
            if int(section[4:]) > today + 30:
                name = tempstore[section]['username']
                tempstore['pending'].remove(name)
                del tempstore[section]
    #
    ran = randomstring(4)
    while tempstore.has_key(ran+str(today)):
        ran = randomstring(4)
    key = ran+str(today)
    tempstore[key] = {}
    store = tempstore[key]
    for entry in formdict:
        if entry == 'pass1' or entry == 'pass2':
            store['password'] = formdict[entry]
        elif entry == 'login':
            pass
        else:
            store[entry] = formdict[entry]
    if not tempstore.has_key('pending'):
        tempstore['pending'] = []
    tempstore['pending'].append(formdict['username'])
    tempstore.write()
    return pass_enc(key, timestamp=True, daynumber=True)

def checknewlogin(userdir):
    """Check that new logins are enabled on this setup.
    Return or display the error message and quit."""
    config = ConfigObj(userdir + 'config.ini')
    if istrue(config['newloginlink']):
        return
    error('Sorry, New Logins are Disabled on this System.')

def newloginfail():
    # FIXME: this could be more useful
    error('Sorry, you have either already confirmed your login, '
            'or it is out of date :-)')

def display_edit(formdict, userdir, thisscript, msg, action, newcookie,
                                                            userconfig=None):
    """
    Display the form for editing account details.
    This is a different form to creating a new account as it requires
    confirmation of the old password before doing certain things, like 
    changing email address and password.
    """
    from time import ctime
    print newcookie
    config = ConfigObj(userdir + 'config.ini')
    templatedir = config['templatedir']
    # if we are sending the userconfig as the formdict, we needn't explicitly
    # pass it in
    if not userconfig:
        userconfig = formdict
    #
    edaccform = unicode(readfile(templatedir+editform_nojs))
    edaccform = edaccform.replace('**script**', thisscript)
    #
    edaccpage = readfile(templatedir+edacctemplate)  
    edaccpage = edaccpage.replace('**edit account form**', edaccform)
    edaccpage = filltemplate(edaccpage, formdict)
    if msg:
        edaccpage = edaccpage.replace('<!-- **message** -->', '<h2>'+msg+'</h2><br>')
    edaccpage = edaccpage.replace('**created on**', ctime(float(userconfig['created'])))
    edaccpage = edaccpage.replace('**num logins**', userconfig['numlogins'])
    edaccpage = edaccpage.replace('**last used**', ctime(float(userconfig['lastused'])))
    edaccpage = edaccpage.replace('**num used**', userconfig['numused'])
    edaccpage = edaccpage.replace('**this script**', thisscript + '?action=' + action)
    #
    if action:
        edaccpage = edaccpage.replace('<!-- **action** -->', actionline % action)
    print serverline
    print '\r'
    print edaccpage
    #
    sys.exit()


"""
CHANGELOG
=========

2005/10/30
----------

Fixed the email function... oops.

2005/10/11
----------

Bugfix in 

2005/09/09
----------

Changes to work with pythonutils 0.2.1

"""
# admin.py
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

###################################################

def cSort(inlist, minisort=True):
    """A case insensitive sort. If minisort is True then elements of the list for which element1.lower() == element2.lower() will also be sorted.
    (See the examples/test stuff."""
    sortlist = []
    newlist = []
    sortdict = {}
    for entry in inlist:
        try:
            lentry = entry.lower()
        except AttributeError:
            sortlist.append(lentry)
        else:
            try:
                sortdict[lentry].append(entry)
            except KeyError:
                sortdict[lentry] = [entry]
                sortlist.append(lentry)

    sortlist.sort()
    for entry in sortlist:
        try:
            thislist = sortdict[entry]
            if minisort: thislist.sort()
            newlist = newlist + thislist
        except KeyError:
            newlist.append(entry)
    return newlist


####################################################
# Various default values etc

adminpage_file = 'admin_page.html'
adminmenu_file = 'admin_menu.txt'
adminconfig_file = 'admin_config.txt'       # the template used for the 'edit config' option.
admininvite_file = 'admin_invite.txt'       # the template to invite/create new users
adminuser_file = 'admin_eduser.txt'           # template for edit/delete user

MAXADMINLEV = 3                             # the maximum admin level it's possible for a user to have             
MINMAXAGE = 600                             # the minimum value for cookie max-age

pass_msg = '\nYour login name is "%s", your password is "%s".\nYou can change this once you have logged in.\n'
SCRIPTLOC = 'http://' + os.environ.get('HTTP_HOST', '')             # XXXX do we trust this in all cases ? (i.e. not always http - https)
numonpage = 5                                 # number of users shown on a page at a time

# table elements used to display the accounts in edit users
edit_table_s = '<table width="90%" cellspacing="15" bgcolor="#3377bb" class="table">'
table_e = '</table>'
elem_h = '<tr><td align="center"><table border="4" width="100%" bgcolor="#dddddd">'
elem_f = '</table></td></tr>'
form_s = '''<form method="post" action="%s"><input type="hidden" name="login" value="admin">
<input type="hidden" name="action" value="%s"><input type="hidden" name="admin" value="%s">
<input type="hidden" name="start" value="%s"><input type="hidden" name="username" value="%s">
'''
form_e = '</form>'

account_table = form_s + '''<tr>
<td align="center"><strong>Login Name : </strong></td><td align="center"><input type="text" name="loginname" value="%s"></td>
<td align="center"><strong>Real Name : </strong></td><td align="center"><input type="text" name="realname" value="%s"></td></tr><tr>
<td align="center"><strong>Email : </strong></td><td align="center"><input type="text" name="email" value="%s"></td>
<td align="center"><strong>Admin Level</strong></td><td align="center"><input type="text" name="adminlev" value="%s"></td></tr><tr>
<td align="center"><strong>New Password : </strong></td><td align="center"><input type="text" name="pass1"></td>
<td align="center"><strong>Cookie max-age : </strong></td><td align="center"><input type="text" name="maxage" value="%s"></td></tr><tr>
<td align="center"><strong>Confirm Password : </strong></td><td align="center"><input type="text" name="pass2"></td>
<td align="center"><strong>Editable : </strong></td><td align="center"><input type="checkbox" name="editable" %s ></td></tr><tr><td align="center">
<input type="reset"></td><td>&nbsp;</td><td>&nbsp;</td><td align="center"><input type="submit" value="Submit Changes"></td></tr><tr>''' + form_e + form_s + '''
<td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr><tr>
<td>&nbsp;</td><td align="center"><input type="checkbox" name="confirm">Confirm Delete</td><td align="center"><input type="submit" value="Delete User">
</td><td>&nbsp;</td></tr>''' + form_e 

####################################################

# main menu - offering
# edit config
# edit users (including delete)
# invite/create new users

# display edit config  (including values from the default user)                
# edit config

# display invite/create
# invite new users
# create new user file

# edit user                      - can't edit or delete yourself or the 'main admin' (saves having to change newcookie)
# choose a user to edit or delete

# display edit user for that account
# or delete user (confirm ?)

# edit user -
# change password
# rename (change login name - ?)
# change display name
# change email address


def admin(theform, userdir, thisscript, userconfig, action, newcookie):
    """Decide what admin action to perform. """
    adminaction = theform.getfirst('admin', '')
    if adminaction.startswith('editconfig'):
        editconfig(theform, userdir, thisscript, userconfig, action, newcookie)
    elif adminaction.startswith('invite'):
        invite(theform, userdir, thisscript, userconfig, action, newcookie)
    elif adminaction.startswith('edituser'):
        edituser(theform, userdir, thisscript, userconfig, action, newcookie)
    elif adminaction.startswith('doeditconfig'):
        doeditconfig(theform, userdir, thisscript, userconfig, action, newcookie)
    elif adminaction.startswith('doinvite'):
        doinvite(theform, userdir, thisscript, userconfig, action, newcookie)
    elif adminaction.startswith('edituser'):
        edituser(theform, userdir, thisscript, userconfig, action, newcookie)
    elif adminaction.startswith('doedituser'):
        doedituser(theform, userdir, thisscript, userconfig, action, newcookie)
    elif adminaction.startswith('deluser'):
        deluser(theform, userdir, thisscript, userconfig, action, newcookie)
        
    else:
        displaymenu(theform, userdir, thisscript, userconfig, action, newcookie)

def displaymenu(theform, userdir, thisscript, userconfig, action, newcookie):
    """Display the admin menu page."""
    config = ConfigObj(userdir + 'config.ini')
    templatedir = config['templatedir']
    
    adminpage = readfile(templatedir+adminpage_file)
    adminpage = adminpage.replace('**this script**', thisscript + '?action=' + action)
    
    url = '?login=admin&admin=%s&action=' + action 
    adminmenu = readfile(templatedir+adminmenu_file)
    adminmenu = adminmenu.replace('**edit config**', thisscript+url % 'editconfig')
    adminmenu = adminmenu.replace('**edit users**', thisscript+url % 'edituser')
    adminmenu = adminmenu.replace('**invite**', thisscript+url % 'invite')

    adminpage = adminpage.replace('**admin**', adminmenu)
    adminpage = adminpage.replace('**admin menu**', thisscript+'?login=admin'+'&action='+action)
        
    print newcookie
    print serverline
    print
    print adminpage
    sys.exit()


def editconfig(theform, userdir, thisscript, userconfig, action, newcookie, msg=None, success=None):
    """Display the screen to edit the main config file.
    This includes the default user."""
    config = ConfigObj(userdir + 'config.ini')
    default = ConfigObj(userdir + 'default.ini')
    templatedir = config['templatedir']
    
    adminpage = readfile(templatedir+adminpage_file)
    adminpage = adminpage.replace('**this script**', thisscript + '?action=' + action)
    adminpage = adminpage.replace('**admin menu**', thisscript+'?login=admin'+'&action='+action)
# The values of this that are editable from config.ini are :
# newloginlink, adminmail, email_subject, email_message
#
# The values of this that are editable from default.ini are :
# max-age, editable

    if msg:
        adminpage = adminpage.replace('<br><!- message --> ', '<h2>'+msg+'</h2>')
    if msg and not success:
        loginlink = theform.getfirst('loginlink', '')
        if loginlink:
            loginlink = 'checked'
        adminmail = theform.getfirst('adminmail', '')
        emailsubj = theform.getfirst('emailsubject', '')
        emailmsg = theform.getfirst('emailmsg', '')
        maxage = theform.getfirst('maxage', '')
        editable = theform.getfirst('editable', '')
        if editable:
            editable = 'checked'
    else:
        loginlink = config['newloginlink'].lower()
        if loginlink == 'yes':
            loginlink = 'checked'
        else:
            loginlink = ''
        adminmail = config['adminmail']
        emailsubj = config['email_subject']
        emailmsg = config['email_message']
        maxage = default['max-age']
        editable = default['editable'].lower()
        if editable == 'yes':
            editable = 'checked'
        else:
            editable = ''
            
    configmenu = readfile(templatedir+adminconfig_file)
    configmenu = configmenu.replace('**loginlink**', loginlink)
    configmenu = configmenu.replace('**adminmail**', adminmail)
    configmenu = configmenu.replace('**email subject**', emailsubj)
    configmenu = configmenu.replace('**email message**',emailmsg)
    configmenu = configmenu.replace('**maxage**', maxage)
    configmenu = configmenu.replace('**editable**', editable)
    configmenu = configmenu.replace('**thisscript**', thisscript)
    configmenu = configmenu.replace('**action**', action)

    adminpage = adminpage.replace('**admin**', configmenu)
    print newcookie
    print serverline
    print
    print adminpage
    sys.exit()
    
def invite(theform, userdir, thisscript, userconfig, action, newcookie, msg=None, success=None):
    """Display the screen to create or invite a new user."""
    config = ConfigObj(userdir + 'config.ini')
    templatedir = config['templatedir']
    
    adminpage = readfile(templatedir+adminpage_file)
    adminpage = adminpage.replace('**this script**', thisscript + '?action=' + action)
    adminpage = adminpage.replace('**admin menu**', thisscript+'?login=admin'+'&action='+action)

# Values to be filled in are :
# **create1** and **create2** - the one switched on should be 'checked', the other should be ''

    if msg:
        adminpage = adminpage.replace('<br><!- message --> ', '<h2>'+msg+'</h2>')
    if msg and not success:
        create = theform.getfirst('create', '')
        if create == 'create':
            create1 = 'checked'
            create2 = ''
        else:
            create2 = 'checked'
            create1 = ''
        realname = theform.getfirst('realname', '')
        username = theform.getfirst('username', '')
        email = theform.getfirst('email', '')
        pass1 = theform.getfirst('pass1', '')
        pass2 = theform.getfirst('pass2', '')
        adminlev = theform.getfirst('adminlev', '')
    else:
        create2 = 'checked'
        create1 = ''
        realname = ''
        username = ''
        email = ''
        pass1 = randomstring(8)
        pass2 = pass1
        adminlev = '0'
        
    invitemenu = readfile(templatedir+admininvite_file)
    invitemenu = invitemenu.replace('**create1**', create1)
    invitemenu = invitemenu.replace('**create2**', create2)
    invitemenu = invitemenu.replace('**realname**', realname)
    invitemenu = invitemenu.replace('**username**', username)
    invitemenu = invitemenu.replace('**email**', email)
    invitemenu = invitemenu.replace('**pass1**', pass1)
    invitemenu = invitemenu.replace('**pass2**', pass2)
    invitemenu = invitemenu.replace('**adminlev**', adminlev)

    invitemenu = invitemenu.replace('**thisscript**', thisscript)
    invitemenu = invitemenu.replace('**action**', action)

    adminpage = adminpage.replace('**admin**', invitemenu)
    print newcookie
    print serverline
    print
    print adminpage
    sys.exit()
  
def edituser(theform, userdir, thisscript, userconfig, action, newcookie, msg=None, success=None):
    """Display the screen to edit or delete users.."""
    config = ConfigObj(userdir + 'config.ini')
    templatedir = config['templatedir']
    realadminlev = int(userconfig['admin'])
    
    adminpage = readfile(templatedir+adminpage_file)
    adminpage = adminpage.replace('**this script**', thisscript + '?action=' + action)
    adminpage = adminpage.replace('**admin menu**', thisscript+'?login=admin'+'&action='+action)
  
    userlist = [entry[:-4] for entry in os.listdir(userdir) if os.path.isfile(userdir+entry) and entry[:-4] not in RESERVEDNAMES ]
    mainadmin = config['adminuser']
    username = userconfig['username']
    if mainadmin in userlist:
        userlist.remove(mainadmin)
    if username in userlist:
        userlist.remove(username)
    userlist = cSort(userlist)
    
    start = int(theform.getfirst('start', '1'))
    length = len(userlist)
    if start*numonpage > length:
        start = length//numonpage + 1
    url = '<a href="' + thisscript + '?start=%s&login=admin&admin=edituser&action=' + action + '">%s</a>'
    indexline = '<div style="text-align:center;">%s</div>' % makeindexline(url, start, length, numonpage)

# need to be able to edit -
# username, realname, new password, confirm password, adminlev, email, max-age, editable
    index = (start-1)*numonpage + 1 
    last = min(length+1, index+numonpage)
    usermenu = indexline + '<br>' + edit_table_s
    while  index < last:                                # go through all the users
        thisuser = userlist[index-1]
        index += 1
        thisuserc = ConfigObj(userdir+thisuser+'.ini')
        adminlev = thisuserc['admin']
        if realadminlev <= int(adminlev):
            continue
        loginname = thisuser
        realname = thisuserc['realname']
        email = thisuserc['email']
        maxage = thisuserc['max-age']
        editable = ''
        if istrue(thisuserc['editable']):
            editable = 'checked'
        if theform.getfirst('username')==loginname and msg and not success:
            realname = theform.getfirst('realname', '')
            realname = theform.getfirst('realname', '')
            email = theform.getfirst('email', '')
            adminlev = theform.getfirst('adminlev', '')
            maxage = theform.getfirst('maxage', '')
            editable = theform.getfirst('editable', '')
            if editable:
                editable = 'checked'
            
        thevals = (thisscript, action, 'doedituser', start, loginname,
                   loginname, realname, email, adminlev, maxage, editable,
                   thisscript, action, 'deluser', start, loginname)
        
        usermenu += elem_h + (account_table % thevals) + elem_f
        
    usermenu += table_e + '<br>' + indexline

    eduserpage = readfile(templatedir+adminuser_file)
    eduserpage = eduserpage.replace('**account table**', usermenu)
    if msg:
        adminpage =  adminpage.replace('<br><!- message --> ', '<h2>%s</h2>' % msg) 

    adminpage = adminpage.replace('**admin**', eduserpage)
    print newcookie
    print serverline
    print
    print adminpage
    sys.exit()
    
##########################################################

def doeditconfig(theform, userdir, thisscript, userconfig, action, newcookie):
    """Receives the submission from the edit config page."""
    config = ConfigObj(userdir + 'config.ini')
    default = ConfigObj(userdir + 'default.ini')

    loginlink = theform.getfirst('loginlink', '')
    adminmail = theform.getfirst('adminmail', '')
    emailsubj = theform.getfirst('emailsubject', '')
    emailmsg = theform.getfirst('emailmsg', '')
    maxage = theform.getfirst('maxage', '')
    editable = theform.getfirst('editable', '')

    if adminmail and not validemail(adminmail):
        editconfig(theform, userdir, thisscript, userconfig, action, newcookie, "The adminmail doesn't appear to be a valid address.")
    if not maxage.isdigit():
        editconfig(theform, userdir, thisscript, userconfig, action, newcookie, "maxage must be a number.")
    if int(maxage) and int(maxage) < MINMAXAGE:
        editconfig(theform, userdir, thisscript, userconfig, action, newcookie, "maxage must be greater than %s." % MINMAXAGE)

    if loginlink:
        config['newloginlink'] = 'Yes'
    else:
        config['newloginlink'] = 'No'
    config['adminmail'] = adminmail
    config['email_subject'] = emailsubj
    config['email_message'] = emailmsg
    config.write()
    default['max-age'] = maxage
    if editable:
        default['editable'] = 'Yes'
    else:
        default['editable'] = 'No'
    default.write()

    displaymenu(theform, userdir, thisscript, userconfig, action, newcookie)            # XXXX should we send a msg here 'COnfig File Edited' ?
    
#####################################################################

def doinvite(theform, userdir, thisscript, userconfig, action, newcookie):
    """Receives the submission from the invite/create new user page."""
    config = ConfigObj(userdir + 'config.ini')
    default = ConfigObj(userdir + 'default.ini')

    create = theform.getfirst('create', '')
    realname = theform.getfirst('realname', '')
    username = theform.getfirst('username', '')
    email = validemail(theform.getfirst('email', ''))
    pass1 = theform.getfirst('pass1', '')
    pass2 = theform.getfirst('pass2', '')
    adminlev = theform.getfirst('adminlev', '')
    
    maxadminlev = min(int(userconfig['admin']), MAXADMINLEV)
    
    if not email:
        invite(theform, userdir, thisscript, userconfig, action, newcookie, 'The email address appears to be invalid.')
    if pass1 != pass2:
        invite(theform, userdir, thisscript, userconfig, action, newcookie, "The two passwords don't match.")
    if len(pass1) < 5:    
        invite(theform, userdir, thisscript, userconfig, action, newcookie, "The password must be at least five characters long.")
    if not realname:
        invite(theform, userdir, thisscript, userconfig, action, newcookie, 'You must supply a realname.')
    if not username:
        invite(theform, userdir, thisscript, userconfig, action, newcookie, 'You must supply a username.')
    if not adminlev.isdigit():
        invite(theform, userdir, thisscript, userconfig, action, newcookie, 'Admin level must be a number.')
    if int(adminlev) > maxadminlev:
        invite(theform, userdir, thisscript, userconfig, action, newcookie, 'Admin level is greater than the maximum allowed (%s).' % maxadminlev)

# now we need to check if the username already exists
    tempstore = ConfigObj(userdir + 'temp.ini')
    pendinglist = tempstore.get('pending', [])
    if os.path.isfile(userdir+username+'.ini') or username in pendinglist or username.lower() in RESERVEDNAMES:
        invite(theform, userdir, thisscript, userconfig, action, newcookie, 'username already exists.')
    for char in username.lower():
        if not char in validchars:
            invite(theform, userdir, thisscript, userconfig, action, newcookie, 'username contains invalid characters.')
# now we have verified the values - we *either* need to create a new username *or* send an invitiation
    
    if create == 'create':
        createuser(userdir, realname, username, email, pass1, adminlev)
        msg = 'New User Created'
    else:
        inviteuser(userdir, realname, username, email, pass1, thisscript, adminlev)
        msg = 'New User Invited'
    invite(theform, userdir, thisscript, userconfig, action, newcookie, msg, True)
    
    
####################################################################

def doedituser(theform, userdir, thisscript, userconfig, action, newcookie):
    """Receives form submissions from the 'edit user' page."""
# parameters to get :
# username, realname, email, adminlev, pass1, pass2
    username = theform.getfirst('username')         # the user we are editing
    loginname = theform.getfirst('loginname')       # the new user name (won't usually change I guess)
    realname = theform.getfirst('realname')
    email = theform.getfirst('email')
    adminlev = theform.getfirst('adminlev')
    pass1 = theform.getfirst('pass1')
    pass2 = theform.getfirst('pass2')
    maxage = theform.getfirst('maxage')
    editable = theform.getfirst('editable')
    
    maxadminlev = min(int(userconfig['admin']), MAXADMINLEV)

# check all the account values
# this could be turned into a generic 'account checker' function if we wanted.
    email = validemail(email)
    if not email:
        edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'The Email Address Appears to Be Invalid.')
    if not loginname:
        edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'You Must Supply a Login Name.')
    for char in loginname.lower():
        if not char in validchars:
            edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'Login Name Contains Invalid Characters')
    if not realname:
        edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'You Must Supply a Real Name')
    if (pass1 or pass2) and not (pass1 and pass2):
        edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'To Change the Password - Enter it Twice')
    if pass1 != pass2:
        edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'The Two Passwords Are Different')
    if pass1 and len(pass1) < 5:
        edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'Password Must Be at Least Five Characters')
    if not adminlev.isdigit():
        edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'The Admin Level Must Be a Number')
    if int(adminlev) > maxadminlev:
        edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'Admin Level is Higher than the Max (%s).' % maxadminlev)
    if not maxage.isdigit():
        edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'Cookie "max-age" Must Be a Number')
    if int(maxage) and int(maxage) < MINMAXAGE:
        edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'Cookie "max-age" Must Be Greater Than %s' % MINMAXAGE)
    if editable:
        editable = 'Yes'
    else:
        editable = 'No'
    # let's just check if the username has changed
    thisuser = ConfigObj(userdir+username+'.ini') 
    if loginname != username:
        pendinglist = ConfigObj(userdir + 'temp.ini').get('pending', [])
        if os.path.isfile(userdir+loginname+'.ini') or loginname in pendinglist or loginname.lower() in RESERVEDNAMES:
            edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'Login Name Chosen Already Exists')    
        thisuser.filename = userdir+loginname+'.ini'                # change to new name
        os.remove(userdir+username+'.ini')                          # free up the old name
    if pass1:
        from dataenc import pass_enc
        thisuser['password'] = pass_enc(pass1, daynumber=True, timestamp=True)
    #
    thisuser['realname'] = realname
    thisuser['email'] = email 
    thisuser['admin'] = adminlev 
    thisuser['max-age'] = maxage 
    thisuser['editable'] = editable
    thisuser.write()
    
#    edituser(theform, userdir, thisscript, userconfig, action, newcookie, '')
    edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'Changes Made Successfully', True)

def deluser(theform, userdir, thisscript, userconfig, action, newcookie):
    """Receives form submissions from when 'delete user' is hit."""
# parameters to get :
# username, realname, email, adminlev, pass1, pass2
    username = theform.getfirst('username')
    confirm = theform.getfirst('confirm')
    if not confirm:
        edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'Confirm Was Not Selected - Delete Not Done', True)
# XXXX we ought to check that the user being deleted isn't the main admin user and hasn't got a higher admin level
    os.remove(userdir+username+'.ini')      # is it really that easy    
    edituser(theform, userdir, thisscript, userconfig, action, newcookie, 'Delete Done Successfully', True)
    
###############################################
# createuser is in loginutils - but this uses a couple of values defined in this module

def inviteuser(userdir, realname, username, email, password, thisscript, adminlev):
    """Invite a new user."""
    from newlogin import savedetails
    from configobj import ConfigObj
    formdict = {'username' : username, 'pass1' : password, 'admin' : adminlev,
                'realname' : realname, 'email' : email, 'action' : '' }
    thekey = savedetails(userdir, formdict)
    config = ConfigObj(userdir + 'config.ini') 
    msg = config['email_message'] + '\n'
    msg = msg + SCRIPTLOC + thisscript + '?login=confirm&id=' + thekey + (pass_msg % (username, password))
    writefile('log.txt', msg)
    sendmailme(email, msg, config['email_subject'], email, html=False)

"""
CHANGELOG
=========

2005/10/30
----------

Fixed the email function... oops.

2005/09/16
----------

Removed dependency on caseless module (added ``cSort``).

2005/09/09
----------

Changes to work with pythonutils 0.2.1

"""

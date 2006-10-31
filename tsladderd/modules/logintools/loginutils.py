# loginutils.py
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
from cgiutils import *

actionline = '<input type="hidden" name="action" value="%s" />'
# a list of reserved names for users
RESERVEDNAMES = ['config', 'default', 'temp', 'emails', 'pending']

# Helper functions for the login scripts and tools
  
def makecookie(userconfig, password, cookiepath):
    """
    Return the current valid cookie heaader for the values supplied in the
    userconfig, the straight password and the cookiepath.
    """
    from login import encodestring
    from Cookie import SimpleCookie
    thecookie = SimpleCookie()
    cookiestring = encodestring(userconfig['username'],password)
    maxage = userconfig['max-age']
    thecookie['userid'] = cookiestring
    if maxage and int(maxage):            # possible cause of error here if the maxage value in a users file isn't an integer !!
        thecookie['userid']['max-age'] = int(maxage) 
    if cookiepath:
        thecookie['userid']['path'] = cookiepath
    return thecookie.output()                         # XXXX may need to be able to return the cookie object

def emptycookie(cookiepath=None):
    """Return an empty cookie with max-age 0.
     Used for logout features.
     """
    from Cookie import SimpleCookie
    thecookie = SimpleCookie()
    thecookie['userid'] = ''
    thecookie['userid']['max-age'] = 0 
    if cookiepath:
        thecookie['userid']['path'] = cookiepath 
    return thecookie.output()                       # XXXX may need to be able to return the cookie object

def sortaction(action):
    return action or 'EMPTY_VAL_MJF'

def createuser(userdir, realname, username, email, password, adminlev):
    """Create a new user."""
    from time import time
    from dataenc import pass_enc
    from configobj import ConfigObj
    
    user = ConfigObj(userdir+'default.ini')
    user.filename = userdir + username + '.ini'         # XXXX  this does no checkign htat the name is valid and doesn't already exist !!
    user['username'] = username
    user['realname'] = realname
    user['email'] = email
    user['admin'] = adminlev
    user['password'] = pass_enc(password, timestamp=True, daynumber=True)
    user['created'] = str(time())
    user.write()

"""
CHANGELOG
=========

2005/09/09
----------

Changes to work with pythonutils 0.2.1

"""

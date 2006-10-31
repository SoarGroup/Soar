# __init__.py
# A CGI Authentication and user account system
# Copyright (C) 2004/2005 Michael Foord
# E-mail: fuzzyman AT voidspace DOT org DOT uk

# logintools

# Released subject to the BSD License
# Please see http://www.voidspace.org.uk/python/license.shtml

# Scripts maintained at http://www.voidspace.org.uk/python/index.shtml
# Comments, suggestions and bug reports welcome.

from login import *

__all__ = (
    'login',
    'isloggedin',
    'logout',
    'createuser',
    'checklogin',
    'displaylogin'
    )

__version__ = '0.6.2'

"""
CHANGELOG
=========

2005/09/09
----------

Changed module name to logintools.

Changed license to the BSD license.

Code cleanup - added ``__version__``.

"""

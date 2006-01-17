###
# Copyright 1995-2004 Soar Technology, Inc., University of Michigan. All 
# rights reserved.
# 
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are 
# met:
# 
#    1.	Redistributions of source code must retain the above copyright 
#       notice, this list of conditions and the following disclaimer. 
# 
#    2.	Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in 
#       the documentation and/or other materials provided with the 
#       distribution. 
# 
# THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND 
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SOAR 
# CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
# DAMAGE.
# 
# The views and conclusions contained in the software and documentation 
# are those of the authors and should not be interpreted as representing 
# official policies, either expressed or implied, of Soar Technology, Inc., 
# the University of Michigan, or the Soar consortium.
### 

##
# Some simple utility functions.

import os.path

##
# Compare two objects alphabetically using their string (__str__)
# representations.
#
# a < b -> -1
# a = b -> 0
# a > b -> 1
def ObjectStrCmp(a, b):
   a = str(a)
   b = str(b)
   if a < b: return -1
   elif a > b: return 1
   return 0

##
# Compare two objects alphabetically using their string (__str__)
# representations. The comparison is case-insensitive
#
# a < b -> -1
# a = b -> 0
# a > b -> 1
def ObjectStrICmp(a, b):
   a = str(a).lower()
   b = str(b).lower()
   if a < b: return -1
   elif a > b: return 1
   return 0

##
# Returns a sorted COPY of a list
#
# @param a List to sort
# @param func Comparison (-1, 0, 1) function.
# @returns A copy of 'a', sorted by 'func'
def SortList(a, func = None):
   b = a[:]
   b.sort(func)
   return b

##
# Uniquifies a list of strings.
def UniqueStringList(l):
   t = {}
   for a in l:
      t[a] = 1
   return t.keys()[:]

##
# Splits a path into all path components unlike os.path.split()
# which only pulls off the last component of the path.
def SplitPath(p):
   if not p: return []
   o = []
   func = os.path.split # store the func
   h, t = func(p)
   if t: o.append(t)
   while h:
      h1, t = func(h)   # split path into head and tail
      if h1 == h: break # This can happen if h is '/' or '\'
      if t: o.append(t) # If path ends in '/' or '\' then tail may be empty
      h = h1
   o.reverse()
   return o

# taken from Python Cookbook at:
# http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/82465
def MkDir(newdir):
   """works the way a good mkdir should :)
      - already exists, silently complete
      - regular file in the way, raise an exception
      - parent directory(ies) does not exist, make them as well
   """
   if os.path.isdir(newdir):
      pass
   elif os.path.isfile(newdir):
      raise Exception("a file with the same name as the desired " \
                      "dir, '%s', already exists." % newdir)
   else:
      head, tail = os.path.split(newdir)
      if head and not os.path.isdir(head):
         MkDir(head)
      #print "_mkdir %s" % repr(newdir)
      if tail:
         os.mkdir(newdir)

##
# Stores the value of the path seperator used by os.path functions
SystemPathSeperator = os.path.join('a', 'b')[1:-1]

##
# Same as os.path.join(), but the seperator is enforced to 'sep'.
# This is useful for things like generating hrefs in HTML and making sure
# that you always use '/' as the path seperator.
#
# @param a First component
# @param b Second component
# @param sep Seperator to use (default to system default)
# @returns a and b appropriately joined by seperator
def JoinPath(a, b, sep=SystemPathSeperator):
   # by using os.path.join here, we're basically limiting sep
   # to being '/' or '\\'. Oh well.
   p = os.path.join(a, b)
   return sep.join(p.split(SystemPathSeperator))

##
# Tries to determine the current user and return their name.
# Returns empty if unknown.
def GetCurrentUser():
   if os.environ.has_key('USER'):
      return os.environ['USER']
   elif os.environ.has_key('USERNAME'):
      return os.environ['USERNAME']
   return ''

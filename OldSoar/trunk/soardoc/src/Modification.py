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
# Code for @modification commands 
import re
import time

DateFormats = ['%Y%m%d', '%Y-%m-%d', '%Y/%m/%d']

def getStrptimeFunc():
   try:
      f = time.strptime
   except AttributeError:      
      import strptime
      f = strptime.strptime
   return f

myStrptime = getStrptimeFunc()



# \[(\d+)\]       An integer in brackets
# \s+             Some whitespace
# (\w+)           Username
# \s+             Some whitespace
# ([\d\.\,/-]+)   Date
# \s+             Some whitespace
# (\w+)           Project

# Match @modified [1] user date project Comments...
anchorRegEx = re.compile('\[(\d+)\]\s+(\w+)\s+([\d\.\,/-]+)\s+(\w+)')

# Match @modified [1] Comments ...
refRegEx = re.compile('\[(\d+)\]')

# Match @modified user date project Comments...
noRefRegEx = re.compile('(\w+)\s+([\d\.\,/-]+)\s+(\w+)')

class Modification:
   def __init__(self, cmd, cmdArgs):
      self.valid = 0
      self.isCreated = cmd == 'created'
      self.user = None
      self.date = None
      self.project = None
      self.comments = ''
      self.isRefAnchor = 0

      m = anchorRegEx.match(cmdArgs)
      if m:
         self.ref = m.groups()[0]
         self.initFromMatch(cmdArgs, m, 1)
         self.valid = 1
         self.isRefAnchor = 1
      else:
         m = refRegEx.match(cmdArgs)
         if m:
            self.ref = m.groups()[0]
            self.comments = cmdArgs[m.end():]
            self.valid = 1
         else:
            self.ref = None
            m = noRefRegEx.match(cmdArgs)
            if m:
               self.initFromMatch(cmdArgs, m, 0)
               self.valid = 1
            else:
               self.comments = cmdArgs

   def IsValid(self): return self.valid   
      
   def IsCreated(self): return self.isCreated

   def IsRefAnchor(self): return self.isRefAnchor
   
   def GetRef(self): return self.ref

   def GetUser(self): return self.user

   def GetDate(self): return self.date

   def GetProject(self): return self.project

   def GetComments(self): return self.comments

   def initFromMatch(self, cmdArgs, m, start):
      groups = m.groups()[start:]
      self.user = groups[0]
      self.date = self.parseDate(groups[1])
      self.project = groups[2]
      self.comments = cmdArgs[m.end():]

   def parseDate(self, s):
      d = None
      for f in DateFormats:
         try:
            d = myStrptime(s, f)
            break
         except ValueError:
            d = None
      return d
   
   def __str__(self):
      pre = ''
      if self.ref:
         pre = '[%s] ' % self.ref
      return '%sBy %s on %s for %s: %s' % (pre, self.user, self.date,
                                           self.project, self.comments)

if __name__ == '__main__':
   m = Modification('modified', 'ray 20030127 myProject this is a \ntest.')
   print str(m)

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
# Code for @production documentation blocks 
import string
import DocBlock

_TypeGuessTable = {
   'elaborate': 'operator-elaboration',
   'elaborate*state': 'state-elaboration',
   'elaborate*top-state': 'state-elaboration',
   'apply': 'application',
   'propose' : 'proposal',
   'compare' : 'selection',
}

class ProductionBlock(DocBlock.DocBlock):
   def __init__(self, sourceFileName, lineNo, isTemp, block):
      DocBlock.DocBlock.__init__(self, sourceFileName, lineNo, isTemp)
      self.ProblemSpaces = []
      self.Operators = []
      self.Type = None
      self.Source = ''
      for cmd, args in block:
         if self.handleBlockCommand(cmd, args):
            continue
         elif cmd == 'problem-space':
            self.ProblemSpaces.extend(map(string.strip, args.split()))
         elif cmd == 'operator':
            self.Operators.extend(map(string.strip, args.split()))
         elif cmd == 'type':
            if len(args) > 0:
               self.Type = args.split()[0].strip()
         else:
            pass

   def GuessType(self):
      name = self.GetName().lower()
      for k, v in _TypeGuessTable.items():
         if name.find(k) != -1:
            return v
      return None
      
   def Accept(self, visitor):
      return visitor.VisitProductionBlock(self)

   def GetNameCommand(self): return 'production'

   ##
   # Override CopyTempInfo because, besides the source
   # location info that everyone wants, we have to copy
   # the production source code as well.
   def CopyTempInfo(self, temp):
      self.SetSourceFile(temp.GetSourceFile())
      self.SetSourceLineNo(temp.GetSourceLineNo())
      if not self.Source and temp.Source:
         self.Source = temp.Source

##   def __str__(self):
##      return '%s\nCreated=%s\nMods=%s' % (DocBlock.DocBlock.__str__(self),
##                                          self.GetCreated(),
##                                          self.GetMods())

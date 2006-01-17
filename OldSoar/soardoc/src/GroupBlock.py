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
# Code for @group documentation blocks 
import string
import SoarDocUtil
import DocBlock

class GroupBlock(DocBlock.DocBlock, DocBlock.DocBlockVisitor):
   def __init__(self, sourceFileName, lineNo, isTemp, block):
      DocBlock.DocBlock.__init__(self, sourceFileName, lineNo, isTemp)
      self.ProblemSpaces = []
      self.Operators = []
      self.Groups = []
      self.Files = []
      self.Productions = []
      for cmd, args in block:
         if self.handleBlockCommand(cmd, args):
            continue
         else:
            pass

   def AddBlock(self, b):
      if not b is self:
         b.Accept(self)      
      
   def Accept(self, visitor):
      return visitor.VisitGroupBlock(self)
      
   def GetNameCommand(self): return 'group'
   
   def assign(self, b, l):
      l.append(b)
      l.sort(SoarDocUtil.ObjectStrCmp)
      
   def VisitFileBlock(self, b):
      self.assign(b, self.Files)
   def VisitProductionBlock(self, b):
      self.assign(b, self.Productions)
   def VisitProblemSpaceBlock(self, b):
      self.assign(b, self.ProblemSpaces)
   def VisitOperatorBlock(self, b):
      self.assign(b, self.Operators)
   def VisitGroupBlock(self, b):
      self.assign(b, self.Groups)
   def VisitMainPageBlock(self, b):
      assert(0)

##   def __str__(self):
##      return '%s\nProjects=%s\nCreated=%s\nMods=%s' % (DocBlock.DocBlock.__str__(self),
##                                          self.GetProjects(),
##                                          self.GetCreated(),
##                                          self.GetMods())

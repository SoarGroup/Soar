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
# Documentation block base class
import string
import re
import Modification

kernelRegEx = re.compile(r'(\S+)')

class DocBlock:
   def __init__(self, docFileName, docLineNo, isTemp):
      self.SetSourceFile(docFileName)
      self.__lineNo = docLineNo
      self.__docFile = docFileName
      self.__docLineNo = docLineNo
      
      self.__isTemp = isTemp
      
      self.__name = ''
      self.__brief = ''
      self.__desc = ''
      self.Groups = []
      self.__created = ''
      self.__mods = []
      self.__kernels = []
      self.__devNotes = []
      self.__todoList = []
      
   def SetSourceFile(self, f):
      self.__file = f
   def SetSourceLineNo(self, f):
      self.__lineNo = f

   def GetSourceFile(self): return self.__file

   def GetSourceLineNo(self): return self.__lineNo
   
   def GetDocFile(self): return self.__docFile

   def GetDocLineNo(self): return self.__docLineNo

   def IsTemp(self): return self.__isTemp
   def CopyTempInfo(self, temp):
      self.SetSourceFile(temp.GetSourceFile())
      self.SetSourceLineNo(temp.GetSourceLineNo())

   def GetName(self): return self.__name
   def SetName(self, name): self.__name = name

   def GetBrief(self): return self.__brief
   def SetBrief(self, b): self.__brief = b
   
   def GetDesc(self): return self.__desc
   def SetDesc(self, b): self.__desc = b

   def GetDevNotes(self): return self.__devNotes
   def SetDevNotes(self, b): self.__devNotes = b

   def GetTodoList(self): return self.__todoList
   def SetTodoList(self, tl): self.__todoList = tl
   
   def Accept(self, visitor):
      assert(0) # Must implement Accept!

   def GetNameCommand(self):
      assert(0) #Must implement!
      
   def GetCreated(self): return self.__created
   def GetMods(self): return self.__mods
   def GetKernels(self): return self.__kernels

   def __str__(self): return self.GetName()
##      return 'Source file=%s:%d, Is Temp=%d, Name=%s, Brief=%s\nDesc=%s\nGroups=%s' % \
##            (self.GetSourceFile(), self.GetSourceLineNo(),
##             self.IsTemp(), self.GetName(),
##             self.GetBrief(), self.GetDesc(), self.Groups)

   def handleBlockCommand(self, cmd, args):
      if cmd == self.GetNameCommand():
         self.SetName(args)
      elif cmd == 'brief': self.SetBrief(args)
      elif cmd == 'desc':
         d = self.GetDesc()
         if d:
            self.SetDesc(d + '\n\n' + args) # make sure blank line is preserved.
         else:
            self.SetDesc(args)
      elif cmd == 'devnote': self.__devNotes.append(args)
      elif cmd == 'ingroup':
         self.Groups.extend(map(string.strip, args.split()))
      elif cmd == 'created':
         self.__created = Modification.Modification(cmd, args)
      elif cmd == 'modified':
         self.__mods.append(Modification.Modification(cmd, args))
      elif cmd == 'kernel':
         m = kernelRegEx.search(args)
         if m:
            self.__kernels.append((m.groups()[0], args[m.end():].strip()))
      elif cmd == 'todo':
         self.__todoList.append(args);
      else:
         return 0
      return 1

class DocBlockVisitor:

   def VisitDefault(self, b): pass
   
   def VisitFileBlock(self, b): self.VisitDefault(b)
   def VisitProductionBlock(self, b): self.VisitDefault(b)
   def VisitProblemSpaceBlock(self, b): self.VisitDefault(b)
   def VisitOperatorBlock(self, b): self.VisitDefault(b)
   def VisitGroupBlock(self, b): self.VisitDefault(b)
   def VisitMainPageBlock(self, b): self.VisitDefault(b)

      

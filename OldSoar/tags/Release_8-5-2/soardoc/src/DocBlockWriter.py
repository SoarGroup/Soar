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
# Supprt for writing out doc blocks as SoarDoc comments
import time
import DocBlock
import Modification

##
# An object that can write out a documentation block
# from a DocBlock derived object.
class Writer(DocBlock.DocBlockVisitor):
   def __init__(self):
      pass

   ##
   # Write a docblock to file f.
   #
   # @param f An open soar file to write to.
   # @param b A DocBlock object to write to f (as comments)
   def __call__(self, f, b):
      self.__f = f
      self.writeHead(b)
      self.writeBase(b)
      b.Accept(self)
      
      self.__f = None
      return self
   
   def writeHead(self, b):
      self.__f.write('##!\n')
      self.__f.write('# @%s %s\n' % (b.GetNameCommand(), b.GetName()))
      
   def formatMultiLineValue(self, v):
      lines = v.split('\n')
      if not lines: return ''
      return '\n'.join([lines[0]] + ['#   %s' % s for s in lines[1:]])

   def writeMod(self, m):
      if not m: return
      f = self.__f
      if m.IsCreated():
         f.write('# @created ')
      else:
         f.write('# @modified ')
      if m.GetRef():
         f.write('[%s] ' % m.GetRef())
         if m.IsRefAnchor():
            self.writeModArgs(m)
         else:
            f.write(self.formatMultiLineValue(m.GetComments()))
      else:
         self.writeModArgs(m)
      
   def writeModArgs(self, m):
      f = self.__f
      f.write('%s %s %s %s\n' % (m.GetUser(),
                                 time.strftime(Modification.DateFormats[0],
                                               m.GetDate()),
                                 m.GetProject(),
                                 self.formatMultiLineValue(m.GetComments())))
      
   def writeBase(self, b):
      f = self.__f
      f.write('# @brief %s\n#\n' % self.formatMultiLineValue(b.GetBrief()))
      f.write('# @desc %s \n' % self.formatMultiLineValue(b.GetDesc()))
      if b.Groups:
         f.write('# @ingroup %s\n' % ' '.join(b.Groups))
      for k, c in b.GetKernels():
         f.write('# @kernel %s %s\n' % (k, self.formatMultiLineValue(c)))
      for d in b.GetDevNotes():
         f.write('# @devnote %s\n' %  self.formatMultiLineValue(d))
      self.writeMod(b.GetCreated())
      for m in b.GetMods():
         self.writeMod(m)
   def writeListCmd(self, f, cmd, argList):
      if not argList: return
      f.write('# @%s %s\n' % (cmd, ' '.join(argList)))
      
   def VisitDefault(self, b): assert(0) # unhandled block type.
   
   def VisitFileBlock(self, b):
      f = self.__f
      self.writeListCmd(f, 'project', b.GetProjects())
      self.writeListCmd(f, 'problem-space', b.ProblemSpaces)
      self.writeListCmd(f, 'operator', b.Operators)
      if b.ProductionType:
         f.write('# @type %s\n' % b.ProductionType)
      
   def VisitProductionBlock(self, b):
      f = self.__f
      self.writeListCmd(f, 'problem-space', b.ProblemSpaces)
      self.writeListCmd(f, 'operator', b.Operators)
      if b.Type:
         f.write('# @type %s\n' % b.Type)
   def VisitProblemSpaceBlock(self, b):
      f = self.__f
      self.writeListCmd(f, 'operator', b.Operators)
   def VisitOperatorBlock(self, b):
      f = self.__f
      self.writeListCmd(f, 'problem-space', b.ProblemSpaces)
   def VisitGroupBlock(self, b):
      pass
   def VisitMainPageBlock(self, b):
      pass


if __name__ == '__main__':
   import StringIO
   import ProductionBlock
   io = StringIO.StringIO()
   w = Writer()
   w(io, ProductionBlock.ProductionBlock('foo', 10, 0, []))
   print io.getvalue()

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
# Code for generating SoarDoc as XML.
#
# Note: Read code is incomplete at this point.
import xml.dom.minidom # for writing
import types
import os.path
import time

import Config
import SoarDocOutput
import BlockCollector
import DocBlock
import FileBlock
import ProductionBlock
import ProblemSpaceBlock
import OperatorBlock
import MainPageBlock
import GroupBlock

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
   def __call__(self, collector):
      self.__doc = xml.dom.minidom.Document()

      e = self.__doc.createElement('soardoc')
      e.appendChild(self.__doc.createComment('Generated on %s by SoarDoc' %
                                             time.ctime()))
      e.attributes['project'] = Config.Instance().ProjectName
      if collector.MainPage:
         e.appendChild(collector.MainPage.Accept(self))
      for b in collector.All:   
         e.appendChild(b.Accept(self))
      self.__doc.appendChild(e)
      
      return self.__doc
   
   def makeTextElement(self, tag, txt):
      e = self.__doc.createElement(tag)
      e.appendChild(self.__doc.createTextNode(txt))
      return e
   
   def createMod(self, m):
      if not m: return
      if m.IsCreated():
         tag = 'created'
      else:
         tag = 'modified'
      e = self.__doc.createElement(tag)
      if m.GetRef():
         e.attributes['ref'] = str(m.GetRef())
         if m.IsAnchorRef():
            self.writeModArgs(e, m)
         else:
            e.appendChild(self.makeTextElement('comments', m.GetComments()))
      else:
         self.writeModArgs(e, m)
      return e
   
   def writeModArgs(self, e, m):
      mte = self.makeTextElement
      e.appendChild(mte('user', m.GetUser()))
      e.appendChild(mte('date',
                        time.strftime(Modification.DateFormats[0],
                                      m.GetDate())))
      e.appendChild(mte('project', m.GetProject()))
      e.appendChild(mte('comments', m.GetComments()))
      return e
                                         
   def createBaseElement(self, b, tag):
      doc = self.__doc
      mte = self.makeTextElement
      e = doc.createElement(tag)
      
      e.appendChild(mte('name', b.GetName()))

      de = doc.createElement('defined')
      de.appendChild(mte('file', b.GetSourceFile()))
      de.appendChild(mte('lineno', str(b.GetSourceLineNo())))
      e.appendChild(de)
      
      de = doc.createElement('documented')
      de.appendChild(mte('file', b.GetDocFile()))
      de.appendChild(mte('lineno', str(b.GetDocLineNo())))
      e.appendChild(de)
      
      e.appendChild(mte('brief', b.GetBrief()))
      e.appendChild(mte('desc', b.GetDesc()))
      for g in b.Groups:
         e.appendChild(mte('ingroup', g))
      for k, c in b.GetKernels():
         ke = doc.createElement('kernel')
         ke.appendChild(mte('version', k))
         ke.appendChild(mte('comment', c))
         e.appendChild(ke)

      for d in b.GetDevNotes():
         e.appendChild(mte('devnote', d))
      for t in b.GetTodoList():
         e.appendChild(mte('todo', t))
         
      if b.GetCreated():
         e.appendChild(self.createMod(b.GetCreated()))
      for m in b.GetMods():
         e.appendChild(self.createMod(m))
      return e

   def createListCmd(self, e, cmd, argList):
      if not argList: return
      for a in argList:
         e.appendChild(self.makeTextElement(cmd, a))
      return e
   
   def VisitDefault(self, b): assert(0) # unhandled block type.
   
   def VisitFileBlock(self, b):
      e = self.createBaseElement(b, 'file')
      self.createListCmd(e, 'project', b.GetProjects())
      self.createListCmd(e, 'problem-space', b.ProblemSpaces)
      self.createListCmd(e, 'operator', b.Operators)
      if b.ProductionType:
         e.appendChild(self.makeTextElement('type', b.ProductionType))
      return e
      
   def VisitProductionBlock(self, b):
      e = self.createBaseElement(b, 'production')
      self.createListCmd(e, 'problem-space', b.ProblemSpaces)
      self.createListCmd(e, 'operator', b.Operators)
      if b.Type:
         e.appendChild(self.makeTextElement('type', b.Type))
      return e

   def VisitProblemSpaceBlock(self, b):
      e = self.createBaseElement(b, 'problem-space')
      self.createListCmd(e, 'operator', b.Operators)
      return e
   def VisitOperatorBlock(self, b):
      e = self.createBaseElement(b, 'operator')
      self.createListCmd(e, 'problem-space', b.ProblemSpaces)
      return e
   def VisitGroupBlock(self, b):
      e = self.createBaseElement(b, 'group')
      return e
   def VisitMainPageBlock(self, b):
      e = self.createBaseElement(b, 'mainpage')
      return e

class Reader:
   def __init__(self):
      self.__funcs = {
         'file' : self.handleFile,
         'production': self.handleProduction,
         'problem-space' : self.handleProblemSpace,
         'operator' : self.handleOperator,
         'group' : self.handleGroup,
         'mainpage' : self.handleMainPage,
      }

   ##
   # Read a log from an XML DOM tree
   #
   # @param root Root of DOM tree (soarlog element)
   # @returns tuple (name of log, root state)
   def __call__(self, root):
      self.__root = root
      if root.nodeName != 'soardoc':
         raise Exception('soardoc root node not found')
      project = str(root.getAttribute('project'))
      blocks = []
      
      for k in root.childNodes:
         blocks.append(self.__funcs[k.nodeName](k))
      return project, BlockCollector.Collector(blocks, None)

   def handleTextNode(self, e):
      # Walk over any text nodes in the current node.
      text = []
      for child in e.childNodes:
         if child.nodeType == xml.dom.Node.TEXT_NODE:
            text.append(child.nodeValue)
      return str(''.join(text)).strip()
   def handleBase(self, e, b):

      return b      
   def handleFile(self, e):
      b = self.handleBase(e, FileBlock.FileBlock())

      return b
   def handleProduction(self, e):
      b = self.handleBase(e, ProductionBlock.ProductionBlock())

      return b
   def handleProblemSpace(self, e):
      b = self.handleBase(e, ProblemSpaceBlock.ProblemSpaceBlock())

      return b
   def handleOperator(self, e):
      b = self.handleBase(e, OperatorBlock.OperatorBlock())

      return b
   def handleGroup(self, e):
      b = self.handleBase(e, GroupBlock.GroupBlock())

      return b
   def handleMainPage(self, e):
      b = self.handleBase(e, MainPageBlock.MainPageBlock())

      return b
   


if __name__ == '__main__':
   w = Writer()
   c = BlockCollector.Collector([], None)
   c.All.append(ProductionBlock.ProductionBlock('foo', 10, 0, []))
   print w(c).toprettyxml(' ' * 3)

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
# Code for writing soar log XML
import xml.dom.minidom # for writing
import types
import os.path
import SoarLog
import SoarDocOutput
import Config

class Writer(SoarLog.BaseVisitor):
   def __init__(self):
      pass

   ##
   # Write the log in XML format to a DOM tree
   #
   # @param name Name of log
   # @param state Root state of log
   # @returns An xml.dom.minidom.Document() object.
   def __call__(self, name, state):
      self.__doc = xml.dom.minidom.Document()

      e = self.__doc.createElement('soarlog')
      e.appendChild(self.makeTextElement('name', name))
      e.appendChild(state.Accept(self))
      self.__doc.appendChild(e)
      
      return self.__doc

   def setBaseAttributes(self, b, e):
      e.attributes['cycle'] = str(b.GetCycle())
      e.attributes['lineno'] = str(b.GetLineNo())
      
   def setDummy(self, b, e):
      if b.IsDummy():
         e.appendChild(self.__doc.createElement('dummy'))
   def makeTextElement(self, tag, txt):
      e = self.__doc.createElement(tag)
      e.appendChild(self.__doc.createTextNode(txt))
      return e
   
   def VisitDefault(self, b): assert(0)
   
   def VisitState(self, b):
      e = self.__doc.createElement('state')
      self.setBaseAttributes(b, e)
      e.attributes['id'] = b.GetId()
      self.setDummy(b, e)
      e.appendChild(self.makeTextElement('reason', b.GetReason()))

      for k in b.GetChildren():
         e.appendChild(k.Accept(self))
      return e
      
   def VisitOperator(self, b):
      e = self.__doc.createElement('operator')
      self.setBaseAttributes(b, e)
      e.attributes['id'] = b.GetId()
      self.setDummy(b, e)
      e.appendChild(self.makeTextElement('name', b.GetName()))

      for k in b.GetChildren():
         e.appendChild(k.Accept(self))
      return e
   
   def VisitProduction(self, b):
      e = self.__doc.createElement('production')
      self.setBaseAttributes(b, e)
      if b.WasFired():
         e.attributes['type'] = 'firing'
      else:
         e.attributes['type'] = 'retraction'
         
      self.setDummy(b, e)
      e.appendChild(self.makeTextElement('name', b.GetName()))
      for o in b.GetOutput():
         e.appendChild(self.makeTextElement('output', o))
      return e


class Reader:
   def __init__(self):
      self.__funcs = {
         'state' : self.handleState,
         'operator' : self.handleOperator,
         'production': self.handleProduction,
         'name' : self.handleTextNode,
         'reason' : self.handleTextNode,
         'output' : self.handleTextNode,
      }

   ##
   # Read a log from an XML DOM tree
   #
   # @param root Root of DOM tree (soarlog element)
   # @returns tuple (name of log, root state)
   def __call__(self, root):
      self.__root = root
      if root.nodeName != 'soarlog':
         raise Exception('soarlog root node not found')
      name = ''
      state = None

      for k in root.childNodes:
         x = self.__funcs[k.nodeName](k)
         if type(x) == types.StringType:
            if k.nodeName == 'name':
               name = x
         else:
            assert(k.nodeName == 'state')
            state = x
      return name, state
   
   def handleState(self, e):
      cycle = int(e.getAttribute('cycle'))
      lineno = int(e.getAttribute('lineno'))
      stateId = e.getAttribute('id')
      reason = ''
      children = []

      for k in e.childNodes:
         x = self.__funcs[k.nodeName](k)
         if type(x) == types.StringType:
            if k.nodeName == 'reason':
               reason = x
         else:
            children.append(x)
      s = SoarLog.State(cycle, lineno, stateId, reason)
      for c in children: s.AddChild(c) 
      return s
   
   def handleOperator(self, e):
      cycle = int(e.getAttribute('cycle'))
      lineno = int(e.getAttribute('lineno'))
      opId = e.getAttribute('id')
      name = ''
      children = []

      for k in e.childNodes:
         x = self.__funcs[k.nodeName](k)
         if type(x) == types.StringType:
            if k.nodeName == 'name':
               name = x
         else:
            children.append(x)
      o = SoarLog.Operator(cycle, lineno, opId, name)
      for c in children: o.AddChild(c) 
      return o
   
   def handleProduction(self, e):
      cycle = int(e.getAttribute('cycle'))
      lineno = int(e.getAttribute('lineno'))
      prodType = e.getAttribute('type')
      outputs = []
      name = ''

      for k in e.childNodes:
         x = self.__funcs[k.nodeName](k)
         if type(x) == types.StringType:
            if k.nodeName == 'name':
               name = x
            elif k.nodeName == 'output':
               outputs.append(x)
         else:
            assert(0)
      p = SoarLog.Production(cycle, lineno, name, prodType=='firing')
      for o in outputs: p.AddOutput(o) 
      return p
   
   def handleTextNode(self, e):
      # Walk over any text nodes in the current node.
      text = []
      for child in e.childNodes:
         if child.nodeType == xml.dom.Node.TEXT_NODE:
            text.append(child.nodeValue)
      return str(''.join(text)).strip()

def GenerateXmlLog(logFileName, state):
   name, ext = os.path.splitext(logFileName)    # Pull off the extension
   SoarDocOutput.CreateOutputDirectory()

   doc = Writer()(name, state)
   f = open(os.path.join(Config.Instance().OutputDirectory,
                         name + '.xml'),
            'w')
   f.write(doc.toprettyxml(' ' * 3))
   
if __name__ == '__main__':
   s = SoarLog.State(10, 15, 'S1', 'HOWDY')
   o = SoarLog.Operator(11, 16, 'O2', 'attack')
   s.AddChild(o)
   p = SoarLog.Production(12, 17, 'propose*attack', 1)
   o.AddChild(p)
   p.AddOutput('output 1')
   p.AddOutput('output 2')
   doc = Writer()('log', s)
   print doc.toprettyxml(' ' * 3)

   n, s = Reader()(doc.documentElement)
   print n, s
   

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


from xml.sax import make_parser
from xml.sax.handler import feature_namespaces
from xml.sax import ContentHandler
import string
import sys
import os.path

class Datamap:
   def __init__(self):
      self.__name = ''
      self.__problemSpaces = {}
      self.__operators = {}

   def GetName(self): return self.__name
   
   def GetProblemSpaces(self): return self.__problemSpaces.values()
   def GetProblemSpace(self, name):
      if self.__problemSpaces.has_key(name):
         return self.__problemSpaces[name]
      return None
   def GetOperators(self): return self.__operators.values()
   def GetOperator(self, name):
      if self.__operators.has_key(name):
         return self.__operators[name]
      return None
   def GetAll(self):
      return self.__problemSpaces.values() + self.__operators.values()

   def AddProblemSpace(self, ps):
      self.__problemSpaces[ps.GetName()] = ps

   def AddOperator(self, op):
      self.__operators[op.GetName()] = op

   def AddPsOrOp(self, psOrOp):
      if psOrOp.IsProblemSpace():
         self.AddProblemSpace(psOrOp)
      else:
         self.AddOperator(psOrOp)

##
# base class for a problem-space or operator
class Base:
   def __init__(self, prods = []):
      self.__dm = None        # parent datamap
      self.__name = ''        # name of ps or op
      self.__startNode = None # first attribute node (state or operator)
      self.__prods = prods    # list of productions
      self.ProblemSpaces = [] # problem-spaces in links
      self.Operators = []     # operators in links
      self.ProposedOperators = []     # operators in links on RHS only

   def GetDatamap(self): return self.__dm
   def SetDatamap(self, dm): self.__dm = dm
   def GetName(self): return self.__name
   def SetName(self, n): self.__name = n
   def GetStartNode(self): return self.__startNode
   def SetStartNode(self, n): self.__startNode = n
   def GetProductions(self): return self.__prods
   def SetProductions(self, p): self.__prods = p[:]
   
   def IsProblemSpace(self): return 0
   def IsOperator(self): return 0
   
class ProblemSpace(Base):
   def __init__(self, prods = []):
      Base.__init__(self, prods)

   def IsProblemSpace(self): return 1
   
class Operator(Base):
   def __init__(self, prods = []):
      Base.__init__(self, prods)

   def IsOperator(self): return 1   

##
# attribute node in ps or op graph...
class Node:
   def __init__(self):
      self.Name = ''
      self.Values = []  # values tested
      self.Productions = [] # names of source productions
      self.Type = ''
      self.Special = ''
      self.Links = [] # List of tuples (type, name, path)
      self.__adjacencies = []

   def AddAdjacency(self, n):
      if not n in self.__adjacencies:
         self.__adjacencies.append(n)

   def GetAdjacencies(self): return self.__adjacencies
   
class NodePathBuilder:
   def __init__(self):
      self.NodePaths = {} # Table of Node to Path (doubles as DFS closed list)

   def __call__(self, n): return self.GetNodePaths(n)
   
   def GetNodePaths(self, n, p = []):
      if not p: self.NodePaths = {}

      np = p[:]
      np.append(n.Name)
      self.NodePaths[n] = np
      for k in n.GetAdjacencies():
         if not self.NodePaths.has_key(k):
            self.GetNodePaths(k, np)
      return self.NodePaths

# Note: in the following code, all the calls to str() are there to convert
# from unicode to ascii. This is a little kludgy, but sometimes python
# cares and sometimes it doesn't, so let's just always use ascii.

class PsOrOpXmlLoader(ContentHandler):
   def __init__(self):
      self.tagStack = []   # stack of tag names
      
      self.vertref = ''    # cumulative vertref element text
      self.prod = ''       # cumulative prod element text
      self.val = ''        # cumulative val element text
      self.path = []       # Current vertex name path (for vertref resolution)

      self.currentTag = ''
      self.root = None
      self.currentNode = None
      self.nodeStack = []
      self.nodeTable = {} # nodes indexed by path
      self.prods = {} # table of all productions found (keys)
     
      # A list of (node, vertref) tuples used to delay vertref
      # resolution until after all the nodes have been parsed. Otherwise
      # cycles could cause a problem.
      self.vertrefs = []

      self.startFuncs = {
         'O': self.startRoot,
         'S': self.startRoot,
         'vert': self.startVert,
         'vertref': self.startVertRef,
         'name': self.startName,
         'prod': self.startProd,
         'val': self.startVal,
         'link': self.startLink
      }
      self.endFuncs = {
         'O': self.endRoot,
         'S': self.endRoot,
         'vert': self.endVert,
         'vertref': self.endVertRef,
         'name': self.endName,
         'prod': self.endProd,
         'val': self.endVal,
         'link': self.endLink
      }
      self.charFuncs = {
         'vertref': self.vertRefChars,
         'name': self.nameChars,
         'prod': self.prodChars,
         'val': self.valChars
      }

   def Load(self, fileName):
      self.dm = None
      self.prods = {}
      self.vertrefs = []

      parser = make_parser()
      parser.setFeature(feature_namespaces, 0)
      parser.setContentHandler(self)
      oldcwd = os.getcwd() # Remember current working directory
      dirName, baseName = os.path.split(fileName)
      if not dirName: dirName = '.' # Use current directory
      
      os.chdir(dirName) # Change to files directory so we can easily resolve hrefs
      parser.parse(baseName)
      os.chdir(oldcwd) # Restore original working directory

      # resolve vertrefs
      for n, ref in self.vertrefs:
         nref = self.nodeTable[ref]
         n.AddAdjacency(nref)

      return self.root
   
   def GetResult(self): return self.root
 
   def startRoot(self, name, attrs):
      self.prods = {}
      if name == 'O':
         self.root = Operator()
      else:
         self.root = ProblemSpace()

      self.root.SetName(str(attrs.get('name', None)))

   def endRoot(self, name):
      self.root.SetProductions(self.prods.keys())
   
   def startVert(self, name, attrs):
      if self.currentNode:
         self.nodeStack.append(self.currentNode)
      n = Node()
      n.Type = str(attrs.get('type', 'unknown'))
      n.Special = str(attrs.get('spec', ''))
      n.Side = str(attrs.get('side', 'B'))
      self.currentNode = n

   def endVert(self, name):
      n = self.currentNode
      if self.nodeStack:
         self.currentNode = self.nodeStack.pop()
         self.path.pop()
         self.currentNode.AddAdjacency(n)
      else:
         self.currentNode = None
         self.root.SetStartNode(n)
   
   def startVertRef(self, name, attrs):  self.vertref = ''
   def endVertRef(self, name):
      self.vertrefs.append((self.currentNode, self.vertref))
      self.vertref = ''
   def vertRefChars(self, ch): self.vertref += str(ch)
   
   def startName(self, name, attrs): self.currentNode.Name = ''
   def endName(self, name):
      self.path.append(self.currentNode.Name) # extend the path
      self.nodeTable['.'.join(self.path)] = self.currentNode # register vertex id
   def nameChars(self, ch): self.currentNode.Name += str(ch)
      
   def startProd(self, name, attrs): self.prod = ''
   def endProd(self, name):
      self.currentNode.Productions.append(self.prod)
      self.prods[self.prod] = 1 # Add production
      self.prod = ''
   def prodChars(self, ch): self.prod += str(ch)
   
   def startVal(self, name, attrs): self.val = ''
   def endVal(self, name):
      self.currentNode.Values.append(self.val)
      self.val = ''
   def valChars(self, ch): self.val += str(ch)
   
   def startLink(self, name, attrs):
      name = str(attrs.get('name', None))
      type = str(attrs.get('type', None))
      path = str(attrs.get('path', None))
      if name and type:
         self.currentNode.Links.append((type, name, path))
         if type == 'S' and not name in self.root.ProblemSpaces:
            self.root.ProblemSpaces.append(name)
         elif type == 'O' and not name in self.root.Operators:
            # only add operators found at state.operator (1 level in the stack)
            # so if there's something like state.topstate.operator, it won't be
            # in this list
            if len(self.nodeStack) == 1 and self.currentNode.Special == 'Operator':
               self.root.Operators.append(name)
               # If it appears on the RHS, it's probably a proposal.
               # TODO: is this the correct way to do this?
               if self.currentNode.Side == 'B' or self.currentNode.Side == 'R':
                  self.root.ProposedOperators.append(name)
         
   def endLink(self, name): pass
   
   def startElement(self, name, attrs):
      name = str(name)
      self.tagStack.append(name)

      if self.startFuncs.has_key(name):
         self.startFuncs[name](name, attrs)
         
   def characters(self, ch):
      name = self.tagStack[-1]
      if self.charFuncs.has_key(name):
         self.charFuncs[name](str(ch))

   def endElement(self, name):
      name = str(name)
      if self.endFuncs.has_key(name):
         self.endFuncs[name](name)
      self.tagStack.pop()

PROBLEM_SPACE = 'S'
OPERATOR = 'O'

class DatamapXmlLoader(ContentHandler):
   def __init__(self):
      self.tagStack = []   # stack of tag names
      
      # Create a parser
      self.parser = make_parser()
      # Tell the parser we are not interested in XML namespaces
      self.parser.setFeature(feature_namespaces, 0)
      self.dm = None
      
      self.startFuncs = {
         'dm': self.startDatamap,
         'S': self.startPsOrOp,
         'O': self.startPsOrOp,
      }
      self.endFuncs = {
         'dm': self.endDatamap,
         'S': self.endPsOrOp,
         'O': self.endPsOrOp,
      }
      self.charFuncs = {
      }
      
      self.filterFunc = None

   ##
   # Set a filter function that filters loading of particular
   # problem-spaces and operators. It has the following signature:
   #
   #  f(loader, name, type)
   #
   # where:
   #  loader = this loader object
   #  name = name of problem-space or operator about to be loaded
   #  type = PROBLEM_SPACE or OPERATOR constants
   #
   # f should return 1 if the ps or op shoudl be loaded, or 0 to skip it.
   def SetFilterFunc(self, f):
      self.filterFunc = f
   def GetFilterFunc(self): return self.filterFunc
   
   def Load(self, fileName):
      self.dm = None
      parser = make_parser()
      parser.setFeature(feature_namespaces, 0)
      parser.setContentHandler(self)
      oldcwd = os.getcwd() # Remember current working directory
      dirName, baseName = os.path.split(fileName)
      if not dirName: dirName = '.' # Use current directory
      
      os.chdir(dirName) # Change to files directory so we can easily resolve hrefs
      try:
         parser.parse(baseName)
      finally:
         os.chdir(oldcwd) # Restore original working directory
      return self.dm
   
   def GetResult(self): return self.dm
   
   def startDatamap(self, name, attrs):
      self.dm = Datamap()
   def endDatamap(self, name): pass
   
   def startPsOrOp(self, tag, attrs):
      name = attrs.get('name', None)
      href = attrs.get('href', None)
      
      if not name and href: return
      name = str(name)
      href = str(href)

      assert(tag == PROBLEM_SPACE or tag == OPERATOR)
      f = self.filterFunc
      if f and not f(self, name, tag): return
      
      loader = PsOrOpXmlLoader()

      # Tell the parser to use our handler
      self.parser.setContentHandler(loader)

      # Parse the input
      self.parser.parse(href)
      self.dm.AddPsOrOp(loader.GetResult())

   def endPsOrOp(self, name):
      pass
      
   def startElement(self, name, attrs):
      name = str(name)
      self.tagStack.append(name)

      if self.startFuncs.has_key(name):
         self.startFuncs[name](name, attrs)
         
   def characters(self, ch):
      name = self.tagStack[-1]
      if self.charFuncs.has_key(name):
         self.charFuncs[name](str(ch))

   def endElement(self, name):
      name = str(name)
      if self.endFuncs.has_key(name):
         self.endFuncs[name](name)
      self.tagStack.pop()

##if __name__ == '__main__':
##   dmLoader = DatamapXmlLoader()
##   dm = dmLoader.Load('C:\\home\\dray\\dev\\st\\cvs1\\dmgen\\xml\\index.xml')
##   sys.exit(0)

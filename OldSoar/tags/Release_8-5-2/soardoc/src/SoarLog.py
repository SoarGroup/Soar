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
# Datastructures and parser for Soar log files...

import re

##
# Base class for State, Operator, and Production
class Base:
   ##
   #
   # @param cycle Cycle where the object was found
   # @param lineNo Log line number
   def __init__(self, cycle, lineNo):
      self.__cycle = cycle
      self.__lineNo = lineNo
      self.__kids = []
   
   def GetCycle(self): return self.__cycle
   def GetLineNo(self): return self.__lineNo
   def GetChildren(self): return self.__kids
   def AddChild(self, k): self.__kids.append(k)

   def IsDummy(self): return 0
   
   ##
   # Get a list of children of a particular type
   #
   # @param t A type, i.e. State
   def GetKidsByType(self, t):
      return [c for c in self.GetChildren() if isinstance(c, t)]

   ##
   # Visitor pattern acceptor
   def Accept(self, visitor): assert(0) # must implement accept

##
# Basic visitor interface
class BaseVisitor:

   def VisitDefault(self, b): pass
   
   def VisitState(self, b): self.VisitDefault(b)
   def VisitOperator(self, b): self.VisitDefault(b)
   def VisitProduction(self, b): self.VisitDefault(b)

class State(Base):
   def __init__(self, cycle, lineNo, id, reason):
      Base.__init__(self, cycle, lineNo)
#      self.__parent = None
      self.__cycle = cycle
      self.__id = id
      self.__reason = reason

#   def GetParentOperator(self): return self.__parent
   def GetId(self): return self.__id
   def GetReason(self): return self.__reason
   def IsStateNoChange(self): return self.__reason == '(state no-change)'
   def Accept(self, visitor):
      return visitor.VisitState(self)
   
class Operator(Base):
   def __init__(self, cycle, lineNo, id, name):
      Base.__init__(self, cycle, lineNo)
#      self.__parent = None
      self.__id = id
      self.__name = name
      self.__subState = None

#   def GetParentState(self): return self.__parent
   def GetId(self): return self.__id
   def GetName(self): return self.__name
   def Accept(self, visitor):
      return visitor.VisitOperator(self)

class Production(Base):
   def __init__(self, cycle, lineNo, name, fired):
      Base.__init__(self, cycle, lineNo)
      self.__name = name
      self.__fired = fired
      self.__output = []
      
   def GetName(self): return self.__name
   def WasFired(self): return self.__fired
   def WasRetracted(self): return not self.__fired
   def GetOutput(self): return self.__output
   def AddOutput(self, o): self.__output.append(o)
   
   def Accept(self, visitor):
      return visitor.VisitProduction(self)

##
# A class that parses a Soar log file into a hierarchy of the objects
# above.
class Parser:
   def __init__(self):
      # compile some regexs for extracting data from lines...
      
      # (\d+|\B)     Optional cycle number
      # :            A colon
      # ' '          A space
      # ((   )*)     0 or more instances of 3 spaces
      # ==>S:        ==>S:
      # \s+          Some whitespace
      # (S\d+)       The state id
      # (.*)         Optional state comment to end of line
      self.__stateRegEx = re.compile(r'(\d+|\B): ((   )*)==>S:\s+(S\d+)(.*)')
      
      # (\d+)     The cycle number
      # :         A colon
      # ' '       A space
      # ((   )*)  0 or more instances of 3 spaces
      # O         The letter O
      # :         A colon
      # \s+       Some whitespace
      # (O\d+)    The operator id
      # \s+       Some whitespace
      # \(\S+)\)  Name of op in parens
      self.__opRegEx = re.compile(r'(\d+): ((   )*)O:\s+(O\d+)\s+\((\S+)\)')

      # (Firing|Retracting)   One of 'Firing' or 'Retracting'
      # \s+                   Some whitespace
      # (\S+)                 The name of the production
      self.__prodRegEx = re.compile(r'^(Firing|Retracting)\s+(\S+)')

      self.__phaseRegEx = re.compile(r'\s*--(.+)--')

      self.__wmeRegEx = re.compile(r'(<=|=>)WM: \((\d+): (.*)\)')
      
      self.__lineNo = 0    # current parse line number
      self.__cycle = 0     # current decision cycle

      self.__curState = None  # Current State object
      self.__curObj = None    # Current object (State, Prod, etc)
      self.__lastOp = None    # Last Operator object encountered
      self.__lastProd = None  # Last Production object encountered
      self.__goalStack = []   # Stack of States as indentation increases

   ##
   # Parses a soar log file and returns a State object
   #
   def Parse(self, filename):
      f = open(filename, 'r')
      
      self.__curState = None      
      self.__curObj = None
      self.__lastOp = None
      self.__lastProd = None
      self.__goalStack = []
      self.__maxDepth = 0
      self.__lineNo = 0
      
      for line in f.xreadlines():
         self.__lineNo += 1
         self.parseLine(line.strip())
      if self.__goalStack:
         return self.__goalStack[0]
      else:
         return self.__curObj
#      return self.__curState
      
   # Some helper functions for interpretting results of RegExs.
   
   # returns cycle, indentation, id, comments
   def extractStateMatches(self, m):
      g = m.groups()
      if g[0] != '':
         c = int(g[0])
      else:
         c = self.__cycle
      return (c, len(g[1]) / 3, g[3], g[4].strip())
   
   # returns cycle, indentation, id, name
   def extractOperatorMatches(self, m):
      g = m.groups()
      return (int(g[0]), len(g[1]) / 3, g[3], g[4])
   
   # returns type, name
   def extractProductionMatches(self, m):
      g = m.groups()
      return (g[0], g[1])

   def parseLine(self, line):
      if not line: return

      m = self.__stateRegEx.search(line)      
      if m:  # it's a state
         self.__cycle, ind, id, comment = self.extractStateMatches(m)
         self.handleState(ind, id, comment)
         return

      m = self.__opRegEx.search(line)   # it's an operator
      if m:
         self.__cycle, ind, id, name = self.extractOperatorMatches(m)
         self.handleOp(ind, id, name)
         return

      m = self.__prodRegEx.search(line) # it's a production
      if m:
         pt, name = self.extractProductionMatches(m)
         self.handleProd(pt, name)
         return
      
      m = self.__phaseRegEx.search(line) # It's a phase
      if m:
         return # ignore phases for now
      
      m = self.__wmeRegEx.search(line) # Adding/Removing a WME
      if m:
         return # ignore WMEs for now

      # if we're here, it's probably agent output from productions...   
      self.handleOutput(line)

   ##
   # If we get a partial log file (perhaps with the beginning chopped off),
   # then we probably missed the first few 'levels' of states and operators.
   # So this function attempts to fill in those missing levels of states
   # with dummies so that the hierarchy can be built.
   #
   # NOTE: This will probably only handle the case where the beginning of
   # the log has been lost! This needs way more testing.
   #
   # @param ind Indention level that is being parsed.
   def insertDummyStates(self, ind):
      # calculate current 'depth' (goal stack + 1 if there's a current state)
      gsl = len(self.__goalStack) + (not self.__curState is None)
#      print 'ind=%d, gs=%d' % (ind, gsl)
      # if ind is less than the depth of the goal stack, everything is fine.
      # otherwise, the log file is missing some of the initial states.
      if ind <= gsl: return

      # Let's insert fake states just to balance everything out.
      for i in range(gsl, ind-1):
         s = State(self.__cycle, self.__lineNo, '-', 'Dummy state inserted by SoarLog')
         if self.__goalStack: # Make dummy a child of next state up
            self.__goalStack[-1].AddChild(s)
         # add state to end of goal stack
         self.__goalStack.append(s)
         
      s = State(self.__cycle, self.__lineNo, '-', 'Dummy state inserted by SoarLog')
      if self.__goalStack:
         self.__goalStack[-1].AddChild(s)
      self.__curState = s
      self.__curObj = self.__curState
      pass
   
   def handleState(self, ind, id, comment):
      self.insertDummyStates(ind)
#      print '%d %s STATE %s  -->%s<--' % (self.__cycle, '-' * ind, id, comment)
      s = State(self.__cycle, self.__lineNo, id, comment)
      if self.__curObj:
         self.__curObj.AddChild(s)
         self.__goalStack.append(self.__curState)
      self.__curState = s
      self.__curObj = s
      
   def handleOp(self, ind, id, name):
      self.insertDummyStates(ind)
#      print '%d %s OP %s %s' % (self.__cycle, '-' * ind, id, name)
      op = Operator(self.__cycle, self.__lineNo, id, name)
      if ind <= len(self.__goalStack):
         while ind < len(self.__goalStack):
            self.__goalStack.pop()
         self.__curState = self.__goalStack.pop()
      self.__curState.AddChild(op)
      self.__curObj = op
      self.__lastOp = op
      
   def handleProd(self, type, name):
      if not self.__curObj: return
      
      assert(type=='Firing' or type=='Retracting')
#      print '%d %s PROD %s' % (self.__cycle, type , name)
      prod = Production(self.__cycle, self.__lineNo, name, type == 'Firing')
      self.__curObj.AddChild(prod)
      self.__lastProd = prod
      
   def handleOutput(self, o):
      if not self.__lastProd: return
#      print '%d OUTPUT %s' % (self.__cycle, o)
      self.__lastProd.AddOutput(o)

##
# Find the maximum depth of a log tree
class DepthCalculator(BaseVisitor):
   def __init__(self):
      self.__depth = 0
      self.__maxDepth = -1

   ##
   # Function call syntax. Returns anchor name for b
   #
   # @param b State, Operator or Production object
   def __call__(self, b):
      self.__depth = 0
      self.__maxDepth = -1
      b.Accept(self)
      return self.__maxDepth

   def updateMax(self):
      self.__maxDepth = max((self.__depth, self.__maxDepth))
         
   def VisitDefault(self, b):
      self.__depth += 1
      self.updateMax()
      for k in b.GetChildren():
         k.Accept(self)
      self.__depth -= 1

   def VisitProduction(self, b):
      delta = 1 + (len(b.GetOutput()) > 0)
      self.__depth += delta
      self.updateMax()
      self.__depth -= delta


if __name__ == '__main__':
   p = Parser()
   b = p.Parse('E:/dev/sca/logs/logfile1.txt')
   print b
   
   

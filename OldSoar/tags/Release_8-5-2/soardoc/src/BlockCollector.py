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
# Collector object
import os.path
import SoarDocUtil
import DocBlock

##
# A class that collects together, prunes, and sorts a set of
# docblock objects for generation of docs...
class Collector(DocBlock.DocBlockVisitor):
   def __init__(self, blocks, dm):
      self.MainPage = None       # The mainpage object
      self.FileTab = {}          # Files by name
      self.ProdTab = {}          # Productions by name
      self.ProblemSpaceTab = {}  # Problem spaces by name
      self.OperatorTab = {}      # Operators by name
      self.GroupTab = {}         # Groups by name

      # Operator children (names) of ProblemSpaces by PS name
      self.PsChildOps = {}
      # ProblemSpace children (names) of Operators by Op name
      self.OpChildPss = {}

      self.Datamap = dm # The datamap, if any

      self.FileLineRefs = {} # indexed by filename (list of relevant lines)

      # Go through all the objects, putting them in their places :)
      for b in blocks: b.Accept(self)

      # Assign productions to their source files...      
      for p in self.ProdTab.values():
         if self.FileTab.has_key(p.GetSourceFile()):
            self.FileTab[p.GetSourceFile()].Prods.append(p)

      # Let's collect everybody together into lists sorted by name
      sort = SoarDocUtil.SortList
      cmp = SoarDocUtil.ObjectStrICmp
      self.Files = sort(self.FileTab.values(), cmp)
      self.Prods = sort(self.ProdTab.values(), cmp)
      self.ProblemSpaces = sort(self.ProblemSpaceTab.values(), cmp)
      self.Operators = sort(self.OperatorTab.values(), cmp)
      self.Groups = sort(self.GroupTab.values(), cmp)

      # Ok, one giant list of everybody.
      self.All = sort(self.Files + self.Prods +
                      self.ProblemSpaces + self.Operators +
                      self.Groups, cmp)
      
      # Assign everybody to their groups...
      gtab = self.GroupTab
      for b in self.All:
         for gname in b.Groups:
            if gtab.has_key(gname):
               gtab[gname].AddBlock(b)

      self.buildGoalHierarchy()
      
      # Collect into useful categories:
      # Productions by problem space/operator
      self.ProdsByPsTab = self.buildProductionsByX(self.GetProdProblemSpaces)
      self.ProdsByOpTab = self.buildProductionsByX(self.GetProdOperators)
      # Production by type
      self.ProdsByTypeTab = self.buildProductionsByX(lambda p: [self.GetProdType(p, 'unknown')])
      # ?
      
      # collect together files by base name (without path)
      ft = {}
      for f in self.Files:
         bname = os.path.basename(f.GetName())
         if not ft.has_key(bname):
            ft[bname] = []
         ft[bname].append(f)
      self.FilesByBaseTab = ft
      
   ##
   # Convenience function to avoid lots of has_key checks in client code.
   #
   # @param table The table to reference (ProdTab, FileTab, etc)
   # @param name  Name of the object to lookup
   # @returns The object, or None if it's not there.
   def GetObject(self, table, name):
      if table.has_key(name):
         return table[name]
      return None

   ##
   # Get a list of problem space names for a production, taking inheritence
   # from filescope into account.
   #
   # @param prod ProductionBlock object
   # @returns List of ProblemSpace names
   def GetProdProblemSpaces(self, prod):
      r = None
      if not prod.ProblemSpaces:
         r = self.FileTab[prod.GetSourceFile()].ProblemSpaces[:]
      else:
         r = prod.ProblemSpaces[:]
      r.sort()
      return r
   
   ##
   # Get a list of operator names for a production, taking inheritence
   # from filescope into account.
   #
   # @param prod ProductionBlock object
   # @returns List of Operator names
   def GetProdOperators(self, prod):
      r = None
      if not prod.Operators:
         r = self.FileTab[prod.GetSourceFile()].Operators[:]
      else:
         r = prod.Operators[:]
      r.sort()
      return r
   
   ##
   # Get a production's type, taking inheritence
   # from filescope into account.
   #
   # @param prod ProductionBlock object
   # @returns Type of 'prod'
   def GetProdType(self, prod, default=None):
      if not prod.Type:
         t = self.FileTab[prod.GetSourceFile()].ProductionType
         if t:
            return t
         else:
            return default
      return prod.Type
   
   ##
   # Get a list of kernels for a production, taking inheritence
   # from filescope into account.
   #
   # @param prod ProductionBlock object
   # @returns List of kernels
   def GetProdKernels(self, prod):
      r = None
      if not prod.GetKernels():
         r = self.FileTab[prod.GetSourceFile()].GetKernels()[:]
      else:
         r = prod.GetKernels()[:]
      r.sort()
      return r

   def assignLineRefs(self, b):
      t = self.FileLineRefs
      pairs = [(b.GetSourceFile(), b.GetSourceLineNo()),
               (b.GetDocFile(), b.GetDocLineNo())]
      for f, l in pairs:
         if not t.has_key(f):
            t[f] = [l]
         else:
            t[f].append(l)
            
   def assign(self, b, table):
      # If an object already exists for this name, it may
      # be a temporary that should be replaced by b.
      if table.has_key(b.GetName()):
         f = table[b.GetName()]
         if f.IsTemp() and not b.IsTemp():
            b.CopyTempInfo(f)
            table[b.GetName()] = b
         elif not f.IsTemp() and b.IsTemp():
            f.CopyTempInfo(b)
      else:
         table[b.GetName()] = b
      self.assignLineRefs(b)

   def buildGoalHierarchy(self):
      for ps in self.ProblemSpaces:
         psname = ps.GetName()
         for oname in ps.Operators:
            if self.OpChildPss.has_key(oname):
               self.OpChildPss[oname].append(psname)
            else:
               self.OpChildPss[oname] = [psname]
      for o in self.Operators:
         oname = o.GetName()
         for psname in o.ProblemSpaces:
            if self.PsChildOps.has_key(psname):
               self.PsChildOps[psname].append(oname)
            else:
               self.PsChildOps[psname] = [oname]
      
   def buildProductionsByX(self, func):
      r = {}
      for p in self.Prods:
         for x in func(p):
            if not x is None:
               if r.has_key(x):
                  r[x].append(p)
               else:
                  r[x] = [p]
      return r
   
   def VisitFileBlock(self, b):
      self.assign(b, self.FileTab)
   def VisitProductionBlock(self, b):
      self.assign(b, self.ProdTab)
   def VisitProblemSpaceBlock(self, b):
      self.assign(b, self.ProblemSpaceTab)
   def VisitOperatorBlock(self, b):
      self.assign(b, self.OperatorTab)
   def VisitGroupBlock(self, b):
      self.assign(b, self.GroupTab)
   def VisitMainPageBlock(self, b):
      if self.MainPage:
         print 'Warning: Multiple mainpage blocks found in %s:%d and %s:%d' % (
               self.MainPage.GetSourceFile(), self.MainPage.GetSourceLineNo(),
               b.GetSourceFile(), b.GetSourceLineNo())
      self.MainPage = b
      self.assignLineRefs(b)

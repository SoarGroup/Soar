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
# SoarDoc main program file.

import re
import types
import time
import os
import sys
import os.path
import fnmatch
import string
import getopt

import Config
import Version

import DocBlock
import FileBlock
import ProductionBlock
import ProblemSpaceBlock
import OperatorBlock
import MainPageBlock
import GroupBlock
import BlockCollector

import SoarDocOutput
import SoarDocXml
import DocBlockWriter
import SoarLog
import SoarLogOutput
import SoarLogXml

import DmgenDatamap

from HTMLgen import HTMLgen

__prodRegEx = re.compile(r'sp\s+([{"])\s*(\S+)')
__cmdRegEx = re.compile(r'@(\S*)(.*$)')
__otherCommentRegEx = re.compile(r'^\s*#+\s*(.*)')

def parseSingleLineCommand(line):
   if line[0:3] != '##!':
      return (0, None, None)

   line = line[3:].strip()
   if not line or line[0] != '@':
      return (0, None, None)
   
   y = __cmdRegEx.match(line)
   command = y.groups()[0].strip()
   cmdArgs = y.groups()[1].strip()

   return (1, command, cmdArgs)   


def parseDocBlock(f, lineno):
   rawline = ''
   command = ''
   inArgs = 0
   cmdArgs = ''

   commands = []   
   while 1:
      rawline = f.readline()
      lineno += 1
      if not rawline: break # Reach EOF
      line = rawline.strip()
      if len(line) == 0: # Empty line, end of doc block
         break
      if line[0] == '#': # a commented line
         line = line[1:].strip() # pull off the comment character (what about more than 1?)
         # If it's a blank comment line, then the previous command is done.
         if not line: 
            if inArgs:
               commands.append((command, cmdArgs))
               inArgs = 0
         # A new command...
         elif line[0] == '@':
            if inArgs: # finish previous command
               commands.append((command, cmdArgs))
            # seperate the command from its args...
            y = __cmdRegEx.match(line)
            command = y.groups()[0].strip()
            cmdArgs = y.groups()[1].strip()
            # If this is the first command in the block and it is a blocktype
            # identifier, then it can't have multi-line arguments, cut it off here.
            if not commands and command in __blockTypes:
               commands.append((command, cmdArgs))
            else:
               inArgs = 1 # we're now collecting args for this command
         else: # detailed description, or multi-line command args
            if inArgs:
               cmdArgs = cmdArgs + '\n' + line
            else: # it's just detailed description, start a desc command.
               command = 'desc'
               cmdArgs = line
               inArgs = 1
      else: # a non-commented line, end of doc block
         break
   if inArgs: # finish up the last command if any.
      commands.append((command, cmdArgs))
   # return the list of commands, our final line number and last line we read.
   return (commands, lineno, rawline)

__blockTypes = ('mainpage', 'file', 'production',
                'problem-space', 'operator',
                'group')

##
# Returns 1 if the block is a block that should be followed by a production
# definition from which we can extract the name.
def blockIsUnnamedProduction(block):
   if len(block) > 0:
      if not block[0][0] in __blockTypes:
         return 1
      elif block[0][0] == 'production' and len(block[0][1]) == 0:
         return 1
      else:
         return 0
   else:
      return 0

##
# A cheap way of extracting a production.
#
# Performs a very simple "parse" of a production, counting
# opening and closing braces. It returns the sub string of
# rawSource that is from the start of the string to the
# closing brace of the production.
#
# TODO: this code won't work for productions enclosed in
# quotes rather than braces!
#
# @param rawSource Some Soar code that starts with sp
def extractProductionSource(rawSource, delim):
   if delim == '{':
      pushChar = '{'
      popChar = '}'
   elif delim == '"':
      pushChar = '"'
      popChar = '"'
   else:
      print 'Unexpected production delimiter:', delim
      return ''
   
   s = rawSource.find(delim)  
   if s == -1:
      return ''
   
   stack = 1
   for i in range(s + 1, len(rawSource)):
      c = rawSource[i]
      if c == popChar:
         stack -= 1
      elif c == pushChar:
         stack += 1
      if stack == 0:
         return rawSource[0:i+1]
   return ''
   

##
# Parses a list of command blocks out of the given file.
# Returns a new list. If @blocks is not None, the new blocks
# are appended to it.
def parseSoarFileDocs(fileName, blocks = None):
   cfg = Config.Instance()
   if blocks is None:
      blocks = []
   count = len(blocks)
   f = open(fileName, 'r')
   lineno = 0
   rawline = f.readline()
   ignore = 0
   blockLineNo = 0 # line number of start of current block
   block = None # current block

   curProdSrc = ''
   curProdDelim = ''
   curProdBlock = None

   otherComments = []   
   while rawline:
      if curProdSrc:
         curProdSrc += rawline
      lineno += 1
      line = rawline.strip()
      if line[0:3] == '##!': # soardoc block marker
         single, cmd, cmdArgs = parseSingleLineCommand(line)
         if single:
            if cmd == 'start-no-soardoc' or cmd == 'end-no-soardoc':
               ignore = (cmd == 'start-no-soardoc')
         blockLineNo = lineno
         block, lineno, rawline = parseDocBlock(f, lineno)
         lineno -= 1    # to compoensate for lineno += 1 above
         if ignore:
            block = None
            continue
         # if it's not an unnamed production, create an object for
         # it. Otherwise, we'll have to wait until we extract a
         # production name from the file (sp {name-of-prod )
         if block and not blockIsUnnamedProduction(block):
            o = createBlock(fileName, blockLineNo, block)
            if o: blocks.append(o)
#            print 'BLOCK:', block
            block = None
      elif not ignore: # let's see if we can find a production
         y = __prodRegEx.match(line)
         if y: # found a production
            prodName = y.groups()[1]

            if curProdSrc:
               curProdBlock.Source = extractProductionSource(curProdSrc, curProdDelim)

            curProdDelim = y.groups()[0]   
            if block: # there was a preceding, unnamed doc block
               if block[0][0] == 'production':
                  block.pop(0) # get rid of empty @production command
               # insert a new production command with the name
               block.insert(0, ('production', prodName))
               # create the object
               o = createBlock(fileName, blockLineNo, block)
               if o:
                  o.SetSourceLineNo(lineno)
                  blocks.append(o)
#               print 'BLOCK2:', block
               block = None
            else: # it's an undocumented production. just create a temporary
                  # object for it.
               o = ProductionBlock.ProductionBlock(fileName, lineno, 1, [])
               o.SetName(prodName)
               blocks.append(o)
               if cfg.UseExistingComments and otherComments:
                  o.SetDesc('<blockquote><i>%s</i></blockquote>' %
                                '<br>\n'.join(otherComments))
            curProdBlock = o
            if o:
               curProdSrc = rawline
            else:
               curProdSrc = ''
               
            otherComments = []
         elif cfg.UseExistingComments:
            m = __otherCommentRegEx.search(line)
            if m:
               otherComments.append(str(HTMLgen.Text(m.groups()[0])))
            elif line: # a non-empty, non-comment line, clear otherComments
               otherComments = []
            elif otherComments:
               otherComments.append('') # maintain whitespace between comment
                                        # blocks
            
         # this is in the else because parseDocBlock gets the next
         # rawline for us in the if part.
         rawline = f.readline()
      else:         
         rawline = f.readline()
   # don't forget the last production in the file!
   if curProdSrc:
      curProdBlock.Source = extractProductionSource(curProdSrc, curProdDelim)

   # If we haven't encountered anything, ignore this file.
   if len(blocks) - count != 0:
      # Append a fake file block marked as temporary
      blocks.append(FileBlock.FileBlock(fileName, 1, 1, []))
   return blocks

##
# A table of 'factories' for creating objects based on command names.
__blockFactories = {
   'file' : FileBlock.FileBlock,
   'production' : ProductionBlock.ProductionBlock,
   'problem-space' : ProblemSpaceBlock.ProblemSpaceBlock,
   'operator' : OperatorBlock.OperatorBlock,
   'mainpage' : MainPageBlock.MainPageBlock,
   'group' : GroupBlock.GroupBlock
}

##
# Given a doc block from the parser (list of commands/args)
# create a block object using the appropriate factory.
#
# @param fileName Name of source file
# @param lineNo   Line block was found
# @param block    List of (command type, args) tuples
# @returns A new object or None if the command is unrecognized
def createBlock(fileName, lineno, block):
   type = block[0][0] # Get the type (command name)
   # Do we recognize this command?   
   if not __blockFactories.has_key(type):
      return None

   # Get the factory and create the object.
   fact = __blockFactories[type]
   return fact(fileName, lineno, 0, block)

##
# An object that walks a soar source tree, parsing files of SoarDoc
# content.
class DirTreeWalker:
   def __init__(self):
      self.__result = []
      self.SetFilePatterns(Config.Instance().FilePatterns)
      nc = os.path.normcase
      np = os.path.normpath
      # normalize exlude directory names so matches work
      self.excludeDirs = [nc(np(x)) for x in Config.Instance().ExcludeDirectories]
      pass

   ##
   # Recursively parse all files under 'dir' that match patterns given
   # in the config file.
   #
   # @param dir Directory to parse
   # @returns A list of block objects...
   def __call__(self, dir):
      olddir = os.getcwd()
      os.chdir(dir)
      self.__result = []
      os.path.walk('.', self.Visit, None)
      os.chdir(olddir)
      return self.__result

   ##
   # Get the list of file glob patterns being used.
   def GetFilePatterns(self): return self.__filePatterns

   ##
   # Set the list of file glob patterns being used.
   def SetFilePatterns(self, fp):
      # Ensure that all the patterns are unique
      assert type(fp) == types.ListType or type(fp) == types.TupleType
      tab = {}
      for p in fp: tab[p] = 1
      self.__filePatterns = tab.keys()[:]
      return self

   ##
   # Get a list of names that match one of our file patterns
   def getFileMatches(self, names):
      matches = []
      for pat in self.__filePatterns:
         matches.extend(fnmatch.filter(names, pat))
      return matches

   ##
   # Check for directory pattern exclusion
   def isDirectoryExcluded(self, dirname):
      for ex in self.excludeDirs:
         if dirname.find(ex) != -1:
            return 1
      return 0

   ##
   # Called for each directory by os.path.walk.
   #
   # @param arg None
   # @param dirname Name of the directory (relative to start directory)
   # @param names List of filenames in this directory
   def Visit(self, arg, dirname, names):
      # Is the end of this directory in the exclude path?
      if self.isDirectoryExcluded(dirname):
#         print 'Excluding directory \'%s\'' % dirname
         return
      
      matches = self.getFileMatches(names)
      for m in matches:
         filename = os.path.normpath(os.path.join(dirname, m))
         print filename
         # add to results list...
         self.__result = parseSoarFileDocs(filename, self.__result)

##
# callback function that implements XML datmap exclude filtering
def xmlDatamapLoaderFilter(loader, name, type):
   cfg = Config.Instance()
   if type == DmgenDatamap.PROBLEM_SPACE:
      exList = cfg.XmlDmProbSpaceExcludes
   elif type == DmgenDatamap.OPERATOR:
      exList = cfg.XmlDmOperatorExcludes
   else:
      assert(false)
   return not (name in exList)

##
# If XmlDatamap is specified, loads the datamap and returns it, taking
# problem-space and operator exclusions into account.
# If XmlDatamap is None, returns None
def LoadXmlDatamap():
   cfg = Config.Instance()
   if cfg.XmlDatamap:
      loader = DmgenDatamap.DatamapXmlLoader()
      if cfg.XmlDmProbSpaceExcludes or cfg.XmlDmOperatorExcludes:
         loader.SetFilterFunc(xmlDatamapLoaderFilter)
      dm = loader.Load(cfg.XmlDatamap)
   else:
      dm = None
   return dm

#
# Code for OutputFormat='autodoc'
def WriteAutoDocFile(collector, dm):
   cfg = Config.Instance()
   SoarDocOutput.CreateOutputDirectory()
   w = DocBlockWriter.Writer()

   # we append here since we may have read documentation from
   # an existing autodoc file and we don't want to blow it away.
   f = open(os.path.join(cfg.OutputDirectory,
                         cfg.AutoDocFileName),
            'a')

   # todo, write this as a 'diff/patch' file?
   
   # if there's no mainpage write a stub...
   if not collector.MainPage:
      f.write('##!\n')
      f.write('# @mainpage %s\n' % cfg.ProjectName)
      f.write('#\n')
      f.write('# <h1>Documentation for %s</h1>\n' % cfg.ProjectName)
      f.write('\n')
  
   for p in collector.Prods:
      if p.IsTemp():
         p.Type = p.GuessType()
      
   if dm:
      # for all undocumented productions, see if we can augment their
      # @problem-space and @operator properties with info from the
      # datamap
      # TODO - how can we handle this for documented productions?
      for ps in dm.GetProblemSpaces():
         psname = ps.GetName()
         for pname in ps.GetProductions():
            p = collector.GetObject(collector.ProdTab, pname)
            if p and p.IsTemp() and not psname in p.ProblemSpaces:
               p.ProblemSpaces.append(psname)
      for op in dm.GetOperators():
         opname = op.GetName()
         for pname in op.GetProductions():
            p = collector.GetObject(collector.ProdTab, pname)
            if p and p.IsTemp() and not opname in p.Operators:
               p.Operators.append(opname)
      
   # Now write everybody else that isn't documented.
   for b in collector.All:
      if b.IsTemp():
         f.write('# %s:%d\n' % (b.GetSourceFile(), b.GetSourceLineNo()))
         w(f, b)
         f.write('\n')
   # If there's a datamap, let's try to generate some problem-space and
   # operators...
   if dm:
      opParentMap = {}
      # look for undocumented problem spaces
      for ps in dm.GetProblemSpaces():
         if not collector.GetObject(collector.ProblemSpaceTab, ps.GetName()):
            f.write('##!\n')
            f.write('# @problem-space %s\n' % ps.GetName())
            f.write('# @brief Generated by SoarDoc from dmgen\n')
            # if there's an operator with the same name, we'll assume this
            # problem-space is a child of that operator
            if dm.GetOperator(ps.GetName()):
               f.write('#\n')
               f.write('# @operator %s\n' % ps.GetName())
            f.write('\n')
         for opName in ps.ProposedOperators:
            if not opParentMap.has_key(opName):
               opParentMap[opName] = []
            opParentMap[opName].append(ps.GetName())
            
      # look for undocumented operators
      for op in dm.GetOperators():
         if not collector.GetObject(collector.OperatorTab, op.GetName()):
            f.write('##!\n')
            f.write('# @operator %s\n' % op.GetName())
            f.write('# @brief Generated by SoarDoc from dmgen\n')
            if opParentMap.has_key(op.GetName()):
               opParents = opParentMap[op.GetName()]
               f.write('#\n# @problem-space %s\n' % ' '.join(opParents))
            f.write('\n')
      
#
# Code for OutputFormat='xml'
def WriteXmlFile(collector):
   cfg = Config.Instance()
   SoarDocOutput.CreateOutputDirectory()
   w = SoarDocXml.Writer()
   doc = w(collector)
   f = open(os.path.join(cfg.OutputDirectory,
                         cfg.XmlFileName),
            'w')
   f.write(doc.toprettyxml(' ' * 3))

def printUsage():
   print 'SoarDoc - A Soar documentation generator by Soar Technology, Inc.'
   print 'Usage: python soardoc.py [options]'
   print '  -g : Prints a default config file to stdout.'
   print '  -f configFile : Name of configuration file. Defaults to "soardocfile".'
   print '  -h : Prints this message.'
   print '  -v : Prints SoarDoc version information.'
   print '  -L logfile : Generate documentation for a Soar log file'
   print '  -C param=value : Set a configuration parameter, overriding config file'

def printVersion():
   print 'SoarDoc version %s : %s : %s' % (Version.Version, 
                                         Version.VersionDate, 
                                         Version.VersionTag)
   
OptionDefs = 'gf:hvL:C:'
ConfigFile = 'soardocfile' # Default config file
LogFile = ''
GenerateConfig = 0
ConfigOverrides = []

def processArgs():
   global ConfigFile, LogFile, ConfigOverrides, GenerateConfig
   try:
      opts, pargs = getopt.getopt(sys.argv[1:], OptionDefs)
   except getopt.GetoptError, e:
      print 'Error: %s' % e
      sys.exit(1)

   for o, v in opts:
      if o == '-g':
         GenerateConfig = 1
      elif o == '-f':
         ConfigFile = v
      elif o == '-h':
         printUsage()
         sys.exit(0)
      elif o == '-v':
         printVersion()
         sys.exit(0)
      elif o == '-L':
         LogFile = v
      elif o == '-C':
         ConfigOverrides.append(v)
   return pargs


def main():
   pargs = processArgs()
   cfg = Config.Instance()
   if GenerateConfig:
      # Add config overrides to config object...   
      for override in ConfigOverrides:
         cfg.EvaluateOverride(override)
      print cfg
      sys.exit(0)
      
   print 'SoarDoc - A Soar documentation generator by Soar Technology, Inc.'

   try:
      cfg.LoadFile(ConfigFile)
   except IOError, e:
      # We only care if we can't load a non-default config file.
      if ConfigFile != 'soardocfile':
         print 'Error: Failed to load config file: ', e
         sys.exit(1)
      else:
         print 'Warning: Failed to load \'soardocfile\', using config defaults: ', e
   except Exception, e:
      print 'Error while loading config file "%s": %s' % (filename, e)
      sys.exit(1);
         

   # Add config overrides to config object...   
   for override in ConfigOverrides:
      cfg.EvaluateOverride(override)

   try:
      dm = LoadXmlDatamap() # load datamap if specified in config file
   except Exception, e:
      print 'Error: While loading XML datamap: ', e
      sys.exit(1)
      
   if not LogFile:
      # Recursively walk the input directory tree, creating docblock objects
      blocks = DirTreeWalker()(cfg.InputDirectory)
      # Collect blocks, removing duplicates, etc.
      c = BlockCollector.Collector(blocks, dm)
      if cfg.OutputFormat == 'html':
         # Write the documentation
         w = SoarDocOutput.Writer(c)
      elif cfg.OutputFormat == 'xml':
         WriteXmlFile(c)         
      elif cfg.OutputFormat == 'autodoc':
         WriteAutoDocFile(c, dm)
      else:
         print 'Error: Unknown OutputFormat value "%s"' % cfg.OutputFormat
         sys.exit(1)
   else:
      p = SoarLog.Parser()
      try:
         state = p.Parse(LogFile)
      except IOError, e:
         print 'Error: While loading log file "%s": %s' % (LogFile, e)
      baseName = os.path.basename(LogFile)  # File name without path
      if cfg.LogOutputFormat == 'html':
         if cfg.LogUseSoarDocs:
            blocks = DirTreeWalker()(cfg.InputDirectory)
            c = BlockCollector.Collector(blocks, dm)
         else:
            c = BlockCollector.Collector([], dm) # just use an empty collector then
         SoarLogOutput.GenerateLogDoc(c, baseName, state)
      elif cfg.LogOutputFormat == 'xml':
         SoarLogXml.GenerateXmlLog(baseName, state)
      
if __name__== '__main__':
   main()

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
# Code for generating SoarDoc HTML output.

import time
import types
import os.path
import tempfile
import re
import StringIO

from HTMLgen import HTMLgen
import Config
import Version
import DocBlock
import SoarToHtml
import SoarDocUtil
import ProblemSpaceBlock
import DmgenDatamap
import DmgenXmlToDot

##
# If the output directory in the config file doesn't exist, create it.
def CreateOutputDirectory():
   outDir = Config.Instance().OutputDirectory
   SoarDocUtil.MkDir(outDir)

def CreateGeneratorTags():
   return '<META NAME="GENERATOR" CONTENT="SoarDoc %s : %s : %s by Soar Technology, Inc.">\n' % \
            (Version.Version, Version.VersionDate, Version.VersionTag)

def WriteHeader(html):
   html.append(HTMLgen.RawText(Config.Instance().MainFrameHeader))
   p = HTMLgen.Paragraph(align='center')
   space = HTMLgen.RawText(' &nbsp; ')
   p.append(HTMLgen.Href('mainpage.html', 'Main Page'), space,
            HTMLgen.Href('fileindex.html', 'Files'), space,
            HTMLgen.Href('prodindex.html', 'Productions'), space,
            HTMLgen.Href('psindex.html', 'Problem Spaces'), space,
            HTMLgen.Href('opindex.html', 'Operators'), space,
            HTMLgen.Href('ghindex.html', 'Goal Hierarchy'), space,
            HTMLgen.Href('groupindex.html', 'Groups'), space,
            HTMLgen.Href('pbytypeindex.html', 'Prod. By Type'), space)
   if Config.Instance().ShowTodoLists:
      p.append(HTMLgen.Href('todoindex.html', 'Todo'), space)
   p.append(HTMLgen.Href('allindex.html', 'All'), space,
            HTMLgen.HR())
   html.append(p)


##
# Writes the footer for a SoarDoc generated page.
#
# @param html HTMLgen doc to write to.
def WriteFooter(html):
   user = SoarDocUtil.GetCurrentUser()
   if user:
      user = ' by ' + user
   html.append(HTMLgen.HR(),
               HTMLgen.Address(
                  HTMLgen.Small('Generated with SoarDoc on %s%s' % (time.ctime(), user))),
               HTMLgen.RawText(Config.Instance().MainFrameFooter))


##
# regex for finding @ref <type>:<target> in text...
#
# [\s$]*    Some whitespace including linefeeds
# @ref      The string @ref
# [\s$]+    More whitespace and lines.
# (file|prod|ps|op|group)     The ref target type
# :         Colon between type and target name
# (\S+)     The ref target
objRefRegEx = re.compile(r'[\s$]*@ref[\s$]+(file|prod|ps|op|group):(\S+)',
                         re.MULTILINE)

##
# regex for finding a @ref <target>.
#
# Just like objRefRegEx, but without the type.
anchorRefRegEx = re.compile(r'[\s$]*@ref[\s$]+(\S+)',
                            re.MULTILINE)

##
# When generating filenames for problem-spaces, files, productions,
# etc we have to make sure they don't have any illegal file name
# characters (*, %, /, etc).  This is a little table that maps
# these characters to different, legal values. not the perfect way to
# do it, but...
__escapeTable = {
   '*' : '_a',
   '%' : '_b',
   '.' : '_c',
   '\\': '_d',
   '/': '_e',
   ':': '_f',
   '?': '_g',
   '"': '_h',
   '<': '_i',
   '>': '_j',
   '|': '_k',
   ' ': '_l'
}
##
# Use __escapeTable to escape 'n'. Returns the escaped string.
def escapeHrefName(n):
   newn = ''
   for c in n:
      if __escapeTable.has_key(c):
         c = __escapeTable[c]
      newn += c
   return newn

##
# Iteratively build up a 'graph' from a series of file paths...
# returns a table that is where keys are filenames and values
# are lists of adjacent filenames...
def buildFileTreeTable(table, path, idx, arg = None):
   p = path[idx]
   if idx == len(path) - 1:
      table[p] = arg
   elif not p: # it's just a file with no parent path.
      buildFileTreeTable(table, path, idx + 1, arg)
   else:
      if not table.has_key(p):
         table[p] = {}
      next = table[p]
      buildFileTreeTable(next, path, idx + 1, arg)
   return table

##
# A callable object that generates SoarDoc hrefs for Block objects.
class HrefGen(DocBlock.DocBlockVisitor):
   ##
   # Constructor
   #
   # @param collector A BlockCollector.Collector object
   def __init__(self, collector):
      self.__result = ''
      self.__col = collector

   ##
   # Returns an href for block object 'b'
   #
   # @param b A DocBlock object.
   def __call__(self, b):
      b.Accept(self)
      return self.__result

   ##
   # Returns a user email address, or none if hostnames aren't
   # defined in the config file.
   #
   # @param user User name from modified command, etc
   # @returns user@hostname or None if hostname isn't defined
   def GetUserHref(self, user):
      cfg = Config.Instance()
      if cfg.CustomMailHosts.has_key(user):
         return '%s@%s' % (user, cfg.CustomMailHosts[user])
      elif cfg.DefaultMailHost:
         return '%s@%s' % (user, cfg.DefaultMailHost)
      return None

   ##
   # Get an href for a line of a source file.
   #
   # @param fileName Name of source file.
   # @param lineNo Line number
   def GetSourceLineHref(self, fileName, lineNo):
      col = self.__col
      if not col.FileTab.has_key(fileName): return None
      f = col.FileTab[fileName]
      return self.genName(f, 'src', 'l%05d' % lineNo)

   ##
   # Get an href for a source file.
   #
   # @param fileName Name of source file.
   def GetSourceFileHref(self, fileName):
      return escapeHrefName('src_%s' % fileName) + '.html'
      
   def genName(self, b, typeName, anchor = ''):
      href = escapeHrefName('%s_%s' % (typeName, b.GetName())) + '.html'
      if anchor:
         href = href + '#' + anchor
      self.__result = href
      return self.__result

   def VisitFileBlock(self, b): self.genName(b, 'f')
   def VisitProductionBlock(self, b):
      # Generate productions as links to anchors on the file page...
      fhref = escapeHrefName('%s_%s' % ('f', b.GetSourceFile())) + '.html'
      self.__result = '%s#%s' % (fhref, b.GetName())
   def VisitProblemSpaceBlock(self, b): self.genName(b, 'ps')
   def VisitOperatorBlock(self, b): self.genName(b, 'o')
   def VisitGroupBlock(self, b): self.genName(b, 'g')


class Writer(DocBlock.DocBlockVisitor):
   def __init__(self, collector):
      self.Collector = collector

      # a table of @ref type names for resolving @refs.
      self.refTypes = {
         'file':self.Collector.FileTab,
         'prod':self.Collector.ProdTab,
         'ps':self.Collector.ProblemSpaceTab,
         'op':self.Collector.OperatorTab,
         'group':self.Collector.GroupTab
      }
      # a local href generator
      self.hg = HrefGen(collector)

      # cache the output directory      
      self.outDir = Config.Instance().OutputDirectory

      CreateOutputDirectory()

      self.writeFrameSet()
      self.writeIndexFrame()
      self.writeMainPage()

      # Write the indices...
      j = os.path.join # less typing :)
      self.writeAlphabeticalIndex(self.Collector.Prods,
                                  'Alphabetical Production Index',
                                  j(self.outDir,'prodindex.html'))
      self.writeAlphaFileIndex(self.Collector.Files,
                               j(self.outDir,'fileindex.html'))
      
      self.writeAlphabeticalIndex(self.Collector.ProblemSpaces,
                                  'Alphabetical Problem Space Index',
                                  j(self.outDir,'psindex.html'))
      self.writeAlphabeticalIndex(self.Collector.Operators,
                                  'Alphabetical Operator Index',
                                  j(self.outDir,'opindex.html'))
      if Config.Instance().ShowTodoLists:
         TodoIndexWriter()(self, j(self.outDir,'todoindex.html'))
      self.writeAlphabeticalIndex(self.Collector.Groups,
                                  'Alphabetical Group Index',
                                  j(self.outDir,'groupindex.html'))
      AllIndexWriter()(self, j(self.outDir,'allindex.html'))
      self.writeGoalHierarchyIndex(j(self.outDir,'ghindex.html'))
      self.writeProdsByTypeIndex(j(self.outDir, 'pbytypeindex.html'))

      # Build docs for all the objects...   
      for b in self.Collector.All: b.Accept(self)

      # Generate source code   
      if Config.Instance().GenerateSource:
         for fname in self.Collector.FileLineRefs.keys():
            f = self.Collector.GetObject(self.Collector.FileTab, fname)
            if f:
               self.writeSourceFileAsHtml(f)

   def GetHrefGen(self): return self.hg
   
   ##
   # Creates a new HTML doc with given title.
   def CreateDoc(self, title):
      doc = HTMLgen.SimpleDocument(meta=CreateGeneratorTags())
      doc.title = title
      doc.script = [] # so we can easily append later without None-checks.
#      doc.stylesheet = 'doxygen.css'
      return doc

   ##
   # Parse 's', finding any instances of '@ref [target]' and replacing them
   # with an appropriate <a href="">target</a>
   # If the reference is badly formed, then red text is inserted with
   # a warning.
   def resolveRefs(self, s):
      m = objRefRegEx.search(s)
      while m:
         type = m.groups()[0]
         name = m.groups()[1]
         a = ''
         if self.refTypes.has_key(type):
            o = self.Collector.GetObject(self.refTypes[type], name)
            if o:
               a = '<a href="%s">%s</a>' % (self.hg(o), name)
            else:
               a = '<font color="red">!%s [bad reference]!</font>' % name
         else:
            a = '<font color="red">!%s [bad ref type:%s]!</font>' % (name,
                                                                     type)
         s = s[:m.start()+1] + a + s[m.end():]
         m = objRefRegEx.search(s)
      return s
   
   def writeFrameSet(self):
      cfg = Config.Instance()
      f1 = HTMLgen.Frame(name=cfg.IndexFrameName, src='index_frm.html')
      f2 = HTMLgen.Frame(name=cfg.MainFrameName, src ='mainpage.html')
      fs = HTMLgen.Frameset(f1, f2, cols='25%,75%')
      fdoc = HTMLgen.FramesetDocument(title=cfg.ProjectName)
      fdoc.append(fs)
      fdoc.write(os.path.join(self.outDir, 'index.html'))

##   def buildFileHierarchyList(self, table, lst = None):
##      if lst is None: lst = HTMLgen.List()
##      
##      keys = table.keys()
##      keys.sort()
##      showSrc = Config.Instance().GenerateSource
##      # do files first...
##      for k in keys:
##         f = table[k]
##         if type(f) != types.DictType:
##            c = HTMLgen.Container(HTMLgen.Href(self.hg(f), k, target='main'))
##            if showSrc:
##               c.append(' ',
##                        HTMLgen.Small(
##                        HTMLgen.Href(self.hg.GetSourceFileHref(f.GetSourceFile()),
##                                    '[src]', target='main')))
##            lst.append(c)
##      for k in keys:
##         d = table[k]
##         if type(d) == types.DictType:
##            sublist = HTMLgen.List()
##            self.buildFileHierarchyList(d, sublist)
##            lst.append(HTMLgen.Bold(k + '/'))
##            lst.append(sublist)
##      return lst
   def buildFileHierarchyList(self, table, depth = 0, cont = None):
      if not cont: cont = HTMLgen.Container()
      keys = table.keys()
      keys.sort()
      showSrc = Config.Instance().GenerateSource
      # do files first...
      spaces = HTMLgen.RawText('&nbsp;' * (4 * depth));
      for k in keys:
         f = table[k]
         if type(f) != types.DictType:
            c = HTMLgen.Container(HTMLgen.Href(self.hg(f), k, target='main'))
            if showSrc:
               c.append(' ',
                        HTMLgen.Small(
                        HTMLgen.Href(self.hg.GetSourceFileHref(f.GetSourceFile()),
                                    '[src]', target='main')))
            cont.append(spaces, c, HTMLgen.BR())
      for k in keys:
         d = table[k]
         if type(d) == types.DictType:
            cont.append(spaces, HTMLgen.Bold(k + '/'), HTMLgen.BR())
            self.buildFileHierarchyList(d, depth + 1, cont)
      return cont
            
   def writeFileHierarchy(self, doc, files):
      if not files: return
      t = {}
      for f in files:
         n = f.GetName()
         buildFileTreeTable(t, SoarDocUtil.SplitPath(n), 0, f)

      doc.append(self.buildFileHierarchyList(t))
      
   def writeIndexFrameSection(self, doc, title, blocks):
      if not blocks: return
      
      doc.append(HTMLgen.Heading(2, title))
      for b in blocks:
         doc.append(
            HTMLgen.Href(self.hg(b), b.GetName(), target='main'),
            HTMLgen.BR())

   def writeIndexFrame(self):
      doc = HTMLgen.SimpleDocument(meta=CreateGeneratorTags())
      doc.append(HTMLgen.RawText(Config.Instance().IndexFrameHeader))
      self.writeIndexFrameSection(doc, 'Groups', self.Collector.Groups)
      self.writeIndexFrameSection(doc, 'Problem Spaces', self.Collector.ProblemSpaces)
      self.writeIndexFrameSection(doc, 'Operators', self.Collector.Operators)
      doc.append(HTMLgen.Heading(2, 'Files'))
      self.writeFileHierarchy(doc, self.Collector.Files)
#      self.writeIndexFrameSection(doc, 'Productions', self.Collector.Prods)
                 
      doc.append(HTMLgen.RawText(Config.Instance().IndexFrameFooter))
      
      doc.write(os.path.join(self.outDir, 'index_frm.html'))
      
   def writeMainPage(self):
      mp = self.Collector.MainPage
      if mp:
         doc = self.CreateDoc(mp.GetName())
         self.writeHeader(doc)
         desc = HTMLgen.Paragraph(mp.GetDesc())
         desc.html_escape = 'OFF'
         doc.append(desc, HTMLgen.P())
         self.writeDocFileRef(mp, doc)
      else: # no main page, just make something up...
         doc = self.CreateDoc('Default Main Page')
         self.writeHeader(doc)
         doc.append(HTMLgen.Center(
                     HTMLgen.Header(1,
                                    Config.Instance().ProjectName,
                                    ' Documentation')))
      self.writeFooter(doc)
      doc.write(os.path.join(self.outDir, 'mainpage.html'))

   def writeSourceFileAsHtml(self, f):
      name = f.GetName()
      shortname = os.path.basename(name)
      html = self.CreateDoc(shortname + ' source')
      self.writeHeader(html)
      
      t = {}
      buildFileTreeTable(t, SoarDocUtil.SplitPath(name), 0, f)
      
      html.append(HTMLgen.Heading(1, shortname),
                  HTMLgen.P(),
                  self.buildFileHierarchyList(t),
                  HTMLgen.P(),                  
                  HTMLgen.Href(self.hg(f),
                               'Go to the documentation for this file...'))
      
      s2h = SoarToHtml.SoarToHtml(self.Collector)
      html.append(s2h(Config.Instance().InputDirectory, f.GetName()))
      self.writeFooter(html)
      html.write(os.path.join(self.outDir,
                              self.hg.GetSourceFileHref(f.GetName())))
      
   def writeBrief(self, b, html):
      if not b.GetBrief(): return
      
      html.append(HTMLgen.Paragraph(b.GetBrief()))
      
   def writeDesc(self, b, html, hweight = 2):
      desc = b.GetDesc()
      if not desc: return
      
      html.append(HTMLgen.Heading(hweight, 'Detailed Description'),
                  HTMLgen.Paragraph(self.resolveRefs(desc), html_escape='OFF'))
      
   def writeDevNotes(self, b, html, hweight = 2):
      if not Config.Instance().ShowDevNotes: return
      notes = b.GetDevNotes()
      if not notes: return
      
      html.append(HTMLgen.Heading(hweight, 'Developer Notes'))
      for n in notes:
         html.append(HTMLgen.Paragraph(self.resolveRefs(n),
                                       html_escape='OFF'))
         
   def writeTodoList(self, b, html, hweight = 2):
      if not Config.Instance().ShowTodoLists: return
      tl = b.GetTodoList()
      if not tl: return
      
      html.append(HTMLgen.Heading(hweight, 'Todo'))
      lst = HTMLgen.List()
      for t in tl:
         lst.append(HTMLgen.RawText(self.resolveRefs(t)))
      html.append(lst)

   ##
   # Builds an entry in the change history list and returns it.
   #
   # @param b Block object...
   # @param m Modification object to render
   def buildMod(self, b, m):
      cfg = Config.Instance()
      
      c = HTMLgen.Container()

      if m.IsValid():
         u = m.GetUser()
         r = m.GetRef()
         mail = self.hg.GetUserHref(u) # should be make a mailto: link?
         if mail: # yes
            mail = HTMLgen.MailTo(mail, u)
         else:    # no, just use the raw username
            mail = u
         # If it's a referencable mod (in @file scope)
         if r and m.IsRefAnchor():
            c.append(HTMLgen.Name(r),
                     '[%s] by ' % r,
                     mail,
                     ' on %s for %s: ' % (time.strftime('%b %d, %Y', m.GetDate()),
                                          m.GetProject()))
         elif r: # It references a mod at @file scope
            f = self.Collector.GetObject(self.Collector.FileTab,
                                         b.GetSourceFile())
            if f: 
               c.append(HTMLgen.Href('%s#%s' % (self.hg(f), r) , '[%s]' % r))
            else: # couldn't find the file, just mention it then.
               c.append(HTMLgen.Font('[%s]' % r, color='red'))
            c.append(': ')
         else: # it's just a plain or mod
            c.append('by ',
                     mail,
                     ' on %s for %s: ' % (time.strftime('%b %d, %Y', m.GetDate()),
                                          m.GetProject()))
      # write out the comments (rawtext since it may have HTML), resolving
      # any @refs as necessary.
      c.append(HTMLgen.RawText(self.resolveRefs(m.GetComments())))
      return c

   ##
   # Write a change history section for the given object.
   #
   # @param b The block object (file, production, etc)
   # @param html HTMLgen object to append to.
   # @param hweight Weight of the 'Change History' title
   def writeChangeHistory(self, b, html, hweight = 2):
      if not (b.GetCreated() or b.GetMods()): return
      
      html.append(HTMLgen.Heading(hweight, 'Change History'))
      m = b.GetCreated()
      if m:
         html.append(HTMLgen.P(),
                     HTMLgen.Bold('Created '),
                     self.buildMod(b, m))
         
      for m in b.GetMods():
         html.append(HTMLgen.P(),
                     HTMLgen.Bold('Modified '),
                     self.buildMod(b, m))
         
   def writeGroupMembership(self, b, html):
      if not b.Groups: return

      n = 'group'
      if len(b.Groups) > 1: n = 'groups'
      
      html.append(HTMLgen.P(), HTMLgen.Bold('Member of %s: ' % n))
      gtab = self.Collector.GroupTab
      for gname in b.Groups[:-1]:
         if gtab.has_key(gname):
            html.append(HTMLgen.Href(self.hg(gtab[gname]), gname), ', ')
         else:
            html.append('%s, ' % gname)
      gname = b.Groups[-1]
      if gtab.has_key(gname):
         html.append(HTMLgen.Href(self.hg(gtab[gname]), gname), '.')
      else:
         html.append('%s.' % gname)
      
   def writeObjectList(self, title, objects, html):
      if not objects: return
      
      html.append(HTMLgen.Bold(title), ': ')
      for o in objects[0:-1]:
         if type(o) != types.StringType:
            html.append(HTMLgen.Href(self.hg(o), o.GetName()),
                        ', ')
         else:
            html.append(o, ', ')
      o = objects[-1]
      if type(o) != types.StringType:
         html.append(HTMLgen.Href(self.hg(o), o.GetName()))
      else:
         html.append(o)
         
   def writeObjectBriefList(self, title, objects, html, hweight = 2):
      if not objects: return

      html.append(HTMLgen.Heading(hweight, title))
      t = HTMLgen.TableLite(width='100%')
      c = 1
      twoSpaces = HTMLgen.RawText('&nbsp;&nbsp;')
      for o in objects:
         td0 = HTMLgen.TD()
         td1 = HTMLgen.TD()
         if type(o) != types.StringType:
            td0 = HTMLgen.TD(HTMLgen.Href(self.hg(o), o.GetName()))
            td1 = HTMLgen.TD(twoSpaces, o.GetBrief())
         else:
            td0 = HTMLgen.TD(o)
            td1 = HTMLgen.TD(twoSpaces)
         # Set the width so that brief lists 'line up' with each other
         # Only problem with this is that some production names are
         # really long, so setting the width here will probably force
         # wrapping. hmmm..
         #td0.width = '35%'
         if c:
            t.append(HTMLgen.TR(td0, td1, bgcolor='#f2f2ff'))
         else:
            t.append(HTMLgen.TR(td0, td1))
         c = not c
      html.append(HTMLgen.P(), t)
      
   def buildKernelList(self, kernels):
      if not kernels: return

      t = HTMLgen.TableLite(width='100%')
      c = 1
      twoSpaces = HTMLgen.RawText('&nbsp;&nbsp;')
      for k, comments in kernels: # list of 2-tuples
         td0 = HTMLgen.TD(HTMLgen.Bold(k), width='10%')
         td1 = HTMLgen.TD(twoSpaces, comments)
         if c:
            t.append(HTMLgen.TR(td0, td1, bgcolor='#f2f2ff'))
         else:
            t.append(HTMLgen.TR(td0, td1))
         c = not c
      return t
      
   def writeFileLineRef(self, b, html, s, fname, lineno, short=1):
      f = self.Collector.FileTab[fname]
      lineHref = self.hg.GetSourceLineHref(fname, lineno)
      if Config.Instance().GenerateSource and lineHref:
         lineHref = HTMLgen.Href(lineHref, str(lineno))
      else:
         lineHref = str(lineno)
      # write full name with path? or just the file name itself?
      if short:
         name = os.path.basename(fname)
      else:
         name = fname
      html.append('%s at line ' % s, lineHref, ' of file ',
                  HTMLgen.Href(self.hg(f), name))
      
   def writeSourceFileRef(self, b, html):
      self.writeFileLineRef(b, html, 'Defined',
                            b.GetSourceFile(), b.GetSourceLineNo())
      
   def writeDocFileRef(self, b, html):
      if not b.IsTemp():
         self.writeFileLineRef(b, html, 'Documented',
                               b.GetDocFile(), b.GetDocLineNo())

   def writeHeader(self, html):
      WriteHeader(html)
      
   def writeFooter(self, html):
      WriteFooter(html)
            
   def buildObjectList(self, objNames, objTable):
      objs = []
      for n in objNames:
         if objTable.has_key(n):
            objs.append(objTable[n])
         else:
            objs.append(n)
      return objs

   ## Some alphabetical index helper functions...

   def StartAlphabeticalIndex(self, title):
      html = self.CreateDoc(title)
      self.writeHeader(html)

      html.append(HTMLgen.Heading(1, title))      
      alphaIdx = HTMLgen.Paragraph(align='center')
      html.append(alphaIdx)
      return html, alphaIdx
   
   def EndAlphabeticalIndex(self, html, alphas, alphaIdx, filename):
      # Now go back and build a list of links to letters...      
      space = HTMLgen.RawText(' &nbsp; ')
      for a in alphas:
         alphaIdx.append(HTMLgen.Href('#%s' % a, a), space)
         
      self.writeFooter(html)
      html.write(filename)
      
   def UpdateAlphas(self, prev, next, html, alphas):
      if next[0].upper() == prev: return prev
      prev = next[0].upper()
      html.append(HTMLgen.P(),
                  HTMLgen.Name(prev),
                  # Make a block box with a bold, white letter in it.
                  HTMLgen.TableLite(
                     HTMLgen.TD(
                        HTMLgen.Font(
                           HTMLgen.Bold(
                              HTMLgen.RawText('&nbsp;'),
                              prev,
                              HTMLgen.RawText('&nbsp;')),
                           color='white'),
                        bgcolor='black')),
                  HTMLgen.P())
      alphas.append(prev)
      return prev
   
   def WriteAlphaIndexEntry(self, html, o, post = ''):
      b = ''
      if o.GetBrief(): b = ' - %s' % o.GetBrief()
      html.append(HTMLgen.Href(self.hg(o), o.GetName()),
                  post,
                  b,
                  HTMLgen.BR())
      
   ##
   # Writes an alphabetical index of objects sorted by GetName().
   def writeAlphabeticalIndex(self, objects, title, filename):
      html, alphaIdx = self.StartAlphabeticalIndex(title)
      
      alphas = []
      prev = ''
      for o in objects:
         next = o.GetName()
         prev = self.UpdateAlphas(prev, next, html, alphas)
         self.WriteAlphaIndexEntry(html, o)
         
      self.EndAlphabeticalIndex(html, alphas, alphaIdx, filename)

   def WriteFileIndexEntry(self, html, baseName, files):
      # write the base file name
      html.append(HTMLgen.P(), HTMLgen.Bold(baseName), HTMLgen.BR())
      # for all files with this base name, write their full paths
      # indented a bit and @brief comments if any
      for f in files:
         b = ''
         if f.GetBrief(): b = ' - %s' % f.GetBrief()
         html.append(HTMLgen.RawText('&nbsp;' * 8),
                     HTMLgen.Href(self.hg(f), f.GetName()),
                     b,
                     HTMLgen.BR())
      
   ##
   # Writes an alphebetical index of files sorted by basename rather
   # than entire path name.
   def writeAlphaFileIndex(self, files, filename):
      html, alphaIdx = self.StartAlphabeticalIndex('Alphabetical File Index')

      fileTab = self.Collector.FilesByBaseTab
      names = fileTab.keys()
      names.sort()
      
      alphas = [] # letters encountered
      prev = ''   # last letter encountered
      tab = HTMLgen.RawText('&nbsp;' * 8)
      for n in names:
         prev = self.UpdateAlphas(prev, n, html, alphas)
         self.WriteFileIndexEntry(html, n, fileTab[n])
         
      self.EndAlphabeticalIndex(html, alphas, alphaIdx, filename)
      
   def writeGoalHierarchyIndex(self, filename):
      html = self.CreateDoc('Goal Hierarchy')

      self.writeHeader(html)
      html.append(HTMLgen.Heading(1, html.title))
      cfg = Config.Instance()
      hasPs = self.Collector.ProblemSpaces
      if hasPs and cfg.GraphicalGoalHierarchy:
         dotDef = GoalHierarchyGraphBuilder(self.Collector, self.hg).Build()
         dot = '"%s"' % os.path.join(cfg.DotPath, cfg.DotExeName)
         dotTmp = tempfile.mktemp()
         dotCmd = '%s -Tcmap -o %s' % (dot, dotTmp)
         # First generate a client-side image map with dot         
         try:
            dotStdIn = os.popen(dotCmd, 'w')
            dotStdIn.write(dotDef)
            dotStdIn.close()
            dotStdIn = None
         except:
            pass
         if not os.path.exists(dotTmp):
            msg = 'Error executing dot [ %s ]' % dotCmd
            print msg
            html.append(HTMLgen.P(), HTMLgen.Font(msg, color='red'))
            return
         dotOutFile = open(dotTmp)
         dotOut = dotOutFile.read()
         dotOutFile.close()
         dotOutFile = None
         os.remove(dotTmp)

         if dotOut:
            cmap = '<map name="goals">%s</map>' % dotOut
         else:
            msg = 'Error executing dot [ %s ]' % dotCmd
            print msg
            html.append(HTMLgen.P(), HTMLgen.Font(msg, color='red'))
            self.writeFooter(html)
            html.write(filename)
            return

         imgFileName = 'goals.%s' % cfg.DotImageFormat
         imgFilePath = os.path.join(self.outDir, imgFileName)
         dotCmd = '%s -T%s -o %s' % (dot, cfg.DotImageFormat, imgFilePath)
         try:
            dotStdIn = os.popen(dotCmd, 'w')
            dotStdIn.write(dotDef)
            dotStdIn.close()
            dotStdIn = None
         except:
            pass

         if os.path.exists(imgFilePath):
            html.append(HTMLgen.RawText(cmap),
                        HTMLgen.Image(imgFileName, usemap='#goals', border=0))
         else:
            msg = 'Error executing dot [ %s ]' % dotCmd
            print msg
            html.append(HTMLgen.P(), HTMLgen.Font(msg, color='red'))
            self.writeFooter(html)
            html.write(filename)
            return      
      elif hasPs:         
         html.append(GoalHierarchyListBuilder(self.Collector, self.hg).Build() )
      else:
         html.append(HTMLgen.P(),
                     HTMLgen.Font('No problem-spaces documented.', color='red'))

      self.writeFooter(html)
      html.write(filename)

   def writeProdsByTypeIndex(self, filename):
      html = self.CreateDoc('Productions By Type')
      
      self.writeHeader(html)
      html.append(HTMLgen.Heading(1, html.title))
      
      types = self.Collector.ProdsByTypeTab.keys()
      types.sort()
      
      alphaIdx = HTMLgen.Paragraph(align='center')
      html.append(alphaIdx)
      space = HTMLgen.RawText(' &nbsp; ')
      for a in types:
         alphaIdx.append(HTMLgen.Href('#%s' % a, a), space)

      for t in types:
         html.append(HTMLgen.P(), HTMLgen.Name(t), HTMLgen.Heading(2, t), HTMLgen.P())
         for p in self.Collector.ProdsByTypeTab[t]:
            html.append(HTMLgen.Href(self.hg(p), p.GetName()),
                        ' - %s' % p.GetBrief(),
                        HTMLgen.BR())
            
      self.writeFooter(html)
      html.write(filename)
      
   def VisitFileBlock(self, b):
      name = b.GetName()
      h = self.CreateDoc(os.path.basename(name))
      self.writeHeader(h)
      h.append(HTMLgen.Heading(1, h.title))

      t = {}
      buildFileTreeTable(t, SoarDocUtil.SplitPath(name), 0, b)
      h.append(HTMLgen.P(),
               'Path to this file:', HTMLgen.BR(),
               self.buildFileHierarchyList(t, 1))
      
      if Config.Instance().GenerateSource:      
         h.append(HTMLgen.P(),
                  HTMLgen.Href(self.hg.GetSourceFileHref(b),
                               'Go to the source for this file...'))

      self.writeBrief(b, h)
      self.writeGroupMembership(b, h)
      h.append(HTMLgen.P())
      self.writeDesc(b, h)
      self.writeDevNotes(b, h)
      self.writeTodoList(b, h)
      
      k = b.GetKernels()
      if k:
         h.append(HTMLgen.Bold('Kernel Compatibility'), HTMLgen.BR(),
                  self.buildKernelList(k))
         
      self.writeObjectBriefList('Productions', b.Prods, h)
      col = self.Collector      
      self.writeObjectBriefList(
         'Problem Spaces',
         self.buildObjectList(b.ProblemSpaces, col.ProblemSpaceTab),
         h)      
      self.writeObjectBriefList(
         'Operators',
         self.buildObjectList(b.Operators, col.OperatorTab),
         h)      
   
      self.writeChangeHistory(b, h)

      if b.Prods:
         h.append(HTMLgen.HR())
         h.append(HTMLgen.P(), HTMLgen.Heading(2, 'Production Documentation'))
         for p in b.Prods:
            self.writeProductionBlock(p,h)

      self.writeFooter(h)
      
      h.write(os.path.join(self.outDir, self.hg(b)))
      
   def writeProductionBlock(self, b, h):
      h.append(HTMLgen.P())
      t = HTMLgen.TableLite(width='100%', cellpadding='0',
                            cellspacing='0', border='0')
      tr = HTMLgen.TR()
      t.append(tr)
      td = HTMLgen.TD(bgcolor='#f2f2ff', valign='middle')
      tr.append(td)
      td.append(HTMLgen.Name(b.GetName()),
                HTMLgen.Bold(HTMLgen.Heading(2, b.GetName())))
      h.append(t)
      
      t = HTMLgen.TableLite(cellpadding='0', cellspacing='5', border='0')      
      tr = HTMLgen.TR()
      t.append(tr)
      td = HTMLgen.TD()
      tr.append(HTMLgen.TD(' '), td)
      if not b.IsTemp():
         self.writeDocFileRef(b, td)
         td.append(HTMLgen.BR())
      self.writeSourceFileRef(b, td)
      self.writeBrief(b, td)

      ptype = self.Collector.GetProdType(b)
      if ptype:
         td.append(HTMLgen.P(), HTMLgen.Bold('Type: '), ptype)
      
      td.append(HTMLgen.P())
      self.writeDesc(b, td, 3)
      self.writeDevNotes(b, td, 3)
      self.writeTodoList(b, td, 3)
      td.append(HTMLgen.P())
      
      k = b.GetKernels()
      if k:
         td.append(HTMLgen.Bold('Kernel Compatibility'), HTMLgen.BR(),
                   self.buildKernelList(k))

      col = self.Collector      
      td.append(HTMLgen.P())
      self.writeObjectList(
         'Problem Spaces',
         self.buildObjectList(
            self.Collector.GetProdProblemSpaces(b),
            col.ProblemSpaceTab),
            td)      
      td.append(HTMLgen.P())
      self.writeObjectList(
         'Operators',
         self.buildObjectList(
            self.Collector.GetProdOperators(b),
            col.OperatorTab),
            td)
      
      self.writeGroupMembership(b, td)
      self.writeChangeHistory(b, td, 3)
      self.writeProductionSource(b, td)
      h.append(t)

   ##
   # If ShowProdSourceInline is set, this writes the
   # production source out, limiting the text if
   # MaxProdSourceLines is set.
   def writeProductionSource(self, b, td):
      cfg = Config.Instance()
      if not (cfg.ShowProdSourceInline and b.Source):
         return

      # count lines and determine if we need a "more" link   
      lines = b.Source.split('\n')
      more = 0 
      if cfg.GenerateSource and cfg.MaxProdSourceLines > 0:
         more = cfg.MaxProdSourceLines < len(lines)
         if more:
            lines = lines[0:cfg.MaxProdSourceLines]

      # generate the more link if necessary
      moreLine = ''
      if more and cfg.GenerateSource:
         lineHref = self.hg.GetSourceLineHref(b.GetSourceFile(),
                                              b.GetSourceLineNo())
         moreLine = HTMLgen.Small(HTMLgen.Href(lineHref, 'more...'))

      # Stick a heading, the source in <pre> tags, and the more link
      # into a <div> section with a nice grey background like the source
      # code pages.
      td.append(HTMLgen.P(),
                HTMLgen.Div(HTMLgen.Heading(3, 'Source'),
                            HTMLgen.Pre(HTMLgen.Text('\n'.join(lines))),
                            moreLine,
                            style='width:100%;background-color: #eeeeee'))

   # This is empty because productions are written within VisitFileBlock   
   def VisitProductionBlock(self, b): pass

   def writeDatamapGraphLegend(self, html):
      # For now, if the nodes are not color coded, then the legend
      # is pointless...
      if not Config.Instance().EnableDatamapNodeColoring:
         return

      code = """
      <table bgcolor="#f2f2ff" bordercolor="#f2f2ff" cellspacing=2 border=1>
         <tr>
            <td colspan=1><small><b>- Legend -&nbsp;&nbsp;</b><small></td>
            <td bordercolor="black" bgcolor="white">
               <code>&nbsp;&nbsp;&nbsp;</code>
            </td>
            <td><small> - Left side</small></td>
            <td bordercolor="black" bgcolor="lightgrey">
               <code>&nbsp;&nbsp;&nbsp;</code>
            </td>
            <td><small> - Both sides</small></td>
            <td bgcolor="black">
               <font color="white"><code>&nbsp;&nbsp;&nbsp;</code></font>
            </td>
            <td><small> - Right side</small></td>
         </tr>
      </table>"""
      html.append(HTMLgen.Paragraph(HTMLgen.RawText(code), align='left'))

   def writePsOrOpDatamap(self, isPs, b, html):
      cfg = Config.Instance()
      if not cfg.ShowDatamaps: return

      if not self.Collector.Datamap: return
      dm = self.Collector.Datamap
      if isPs:
         dmPsOrOp = dm.GetProblemSpace(b.GetName())
      else:
         dmPsOrOp = dm.GetOperator(b.GetName())
      if not dmPsOrOp: return

      builder = DatamapGraphBuilder(self.Collector, self.hg,
                                    'popupAttrDetails')
      dotDef = builder.Build(dmPsOrOp)
      detScriptBuilder = DatamapNodeDetailsScriptBuilder(self.Collector, b,
                                                         builder.Nodes,
                                                         builder.NodePaths,
                                                         'popupAttrDetails')
      html.script.append(detScriptBuilder.Build())

      # construct Dot command...         
      dot = '"%s"' % os.path.join(cfg.DotPath, cfg.DotExeName)
      dotTmp = tempfile.mktemp()
      dotCmd = '%s -Tcmap -o %s' % (dot, dotTmp)

      # First generate a client-side image map with dot         
      try:
         dotStdIn = os.popen(dotCmd, 'w')
         dotStdIn.write(dotDef)
         dotStdIn.close()
         dotStdIn = None
      except:
         pass
      
      if not os.path.exists(dotTmp):
         msg = 'Error executing dot [ %s ]' % dotCmd
         print msg
         html.append(HTMLgen.P(), HTMLgen.Font(msg, color='red'))
         return

      dotOutFile = open(dotTmp)
      dotOut = dotOutFile.read()
      dotOutFile.close()
      dotOutFile = None
      os.remove(dotTmp)
      if dotOut:
         cmap = '<map name="datamap">%s</map>' % dotOut
      else:
         msg = 'Error executing dot [ %s ]' % dotCmd
         print msg
         html.append(HTMLgen.P(), HTMLgen.Font(msg, color='red'))
         return

      # Now generate the actual image...
      imgFileName = os.path.splitext(self.hg(b))[0] + '.' + cfg.DotImageFormat
      imgFilePath = os.path.join(self.outDir, imgFileName)
      dotCmd = '%s -T%s -o %s' % (dot, cfg.DotImageFormat, imgFilePath)
      try:
         dotStdIn = os.popen(dotCmd, 'w')
         dotStdIn.write(dotDef)
         dotStdIn.close()
         dotStdIn = None
      except:
         pass
      
      if not os.path.exists(imgFilePath):
         msg = 'Error executing dot [ %s ]' % (dotCmd)
         print msg
         html.append(HTMLgen.P(), HTMLgen.Font(msg, color='red'))
         return

      html.append(HTMLgen.P(),
                  HTMLgen.RawText(cmap),
                  HTMLgen.Image(imgFileName, usemap='#datamap', border=0))
      self.writeDatamapGraphLegend(html)

   def writeGoalHierarchyNavBar(self, html, parents, kids, colTable):
      if not Config.Instance().ShowGoalHierarchyNavBar: return
      
      html.append(HTMLgen.P(),
                  HTMLgen.RawText('<small>'))
      self.writeObjectList('up',
                           self.buildObjectList(parents, colTable),
                           html)
      if parents:
         html.append(HTMLgen.RawText('&nbsp;' * 5))
      self.writeObjectList('down',
                           self.buildObjectList(kids, colTable),
                           html)
      html.append(HTMLgen.RawText('</small>'))
      
   def VisitProblemSpaceBlock(self, b):
      col = self.Collector
      name = b.GetName()
      h = self.CreateDoc('Problem-space: ' + name)

      kids = []
      if col.PsChildOps.has_key(name):
         kids = col.PsChildOps[name]
         
      self.writeHeader(h)
      self.writeGoalHierarchyNavBar(h, b.Operators, kids, col.OperatorTab)
      
      h.append(HTMLgen.Heading(1, h.title))
      self.writeBrief(b, h)
      self.writeDocFileRef(b, h)
      self.writePsOrOpDatamap(1, b, h)
      self.writeGroupMembership(b, h)
      
      h.append(HTMLgen.P())
      self.writeDesc(b, h)
      self.writeDevNotes(b, h)
      self.writeTodoList(b, h)
      h.append(HTMLgen.P())

      
      self.writeObjectBriefList('Parent Operator',
                                self.buildObjectList(b.Operators,
                                                     col.OperatorTab),
                                h)

      if kids:      
         self.writeObjectBriefList('Operators',
                                   self.buildObjectList(kids,
                                                        col.OperatorTab),
                                   h)      

      self.writeChangeHistory(b, h)

      prods = self.Collector.ProdsByPsTab
      if prods.has_key(name):
         self.writeObjectBriefList('Productions',
                                   self.buildObjectList(prods[name],
                                                        col.ProdTab),
                                   h)
      
      self.writeFooter(h)
      h.write(os.path.join(self.outDir, self.hg(b)))

   def VisitOperatorBlock(self, b):
      col = self.Collector
      name = b.GetName()
      h = self.CreateDoc('Operator: ' + name)

      self.writeHeader(h)
      
      kids = []
      if col.OpChildPss.has_key(name):
         kids = col.OpChildPss[name]
      self.writeGoalHierarchyNavBar(h, b.ProblemSpaces, kids,
                                    col.ProblemSpaceTab)
         
      h.append(HTMLgen.Heading(1, h.title))
      self.writeBrief(b, h)
      self.writeDocFileRef(b, h)
      self.writePsOrOpDatamap(0, b, h)
      self.writeGroupMembership(b, h)
      
      h.append(HTMLgen.P())
      self.writeDesc(b, h)
      self.writeDevNotes(b, h)
      self.writeTodoList(b, h)
      h.append(HTMLgen.P())
      
      self.writeObjectBriefList('Problem-space',
                                self.buildObjectList(b.ProblemSpaces,
                                                     col.ProblemSpaceTab), h)
      
      if kids:      
         self.writeObjectBriefList('Child Problem-space',
                                   self.buildObjectList(kids,
                                                        col.ProblemSpaceTab),
                                   h)
      
      self.writeChangeHistory(b, h)
      
      prods = self.Collector.ProdsByOpTab
      if prods.has_key(name):
         self.writeObjectBriefList('Productions',
                                   self.buildObjectList(prods[name],
                                                        col.ProdTab),
                                   h)
         
      self.writeFooter(h)
      h.write(os.path.join(self.outDir, self.hg(b)))

   def VisitGroupBlock(self, b):
      h = self.CreateDoc('Group: ' + b.GetName())

      self.writeHeader(h)
      h.append(HTMLgen.Heading(1, h.title))
      self.writeBrief(b, h)
      self.writeDocFileRef(b, h)
      self.writeGroupMembership(b, h)
      
      h.append(HTMLgen.P())
      self.writeDesc(b, h)
      self.writeDevNotes(b, h)
      self.writeTodoList(b, h)
      h.append(HTMLgen.P())
      
      col = self.Collector
      self.writeObjectBriefList(
         'Files',
         self.buildObjectList(b.Files, col.FileTab),
         h)
      self.writeObjectBriefList('Productions', b.Productions, h)
      self.writeObjectBriefList(
         'Problem-spaces',
         self.buildObjectList(b.ProblemSpaces, col.ProblemSpaceTab),
         h)
      self.writeObjectBriefList(
         'Operators',
         self.buildObjectList(b.Operators, col.OperatorTab),
         h)
      self.writeObjectBriefList(
         'Groups',
         self.buildObjectList(b.Groups, col.GroupTab),
         h)
      
      self.writeChangeHistory(b, h)
      
      self.writeFooter(h)
      h.write(os.path.join(self.outDir, (self.hg(b))))

##
# Writes a complete alphabetical list of all objects.
# Files are handled specially in that they are sorted by
# base name rather than full path name. They are also grouped
# together by base name.
class AllIndexWriter(DocBlock.DocBlockVisitor):
   def __init__(self):
      self.__writer = None
      self.__html = None
      self.__hg = None
      pass

   def __call__(self, writer, fileName):
      self.__writer = writer
      self.__html, self.__alphaIdx = writer.StartAlphabeticalIndex(
                                       'Complete Alphabetical Index')
      self.__alphas = []
      self.__next = ''
      self.__prev = ''
      
      col = self.__writer.Collector
      objs = col.Prods + col.ProblemSpaces + col.Operators + col.Groups
      ft = col.FilesByBaseTab
      for k, v in ft.items():
         objs.append((k, v))
      objs.sort(self.__comp)
      for o in objs:
         if type(o) == types.TupleType:
            base, files = o
            self.__updateAlphas(base)
            self.__writer.WriteFileIndexEntry(self.__html, base, files)
            self.__html.append(HTMLgen.BR())
         else:
            self.__updateAlphas(o.GetName())
            o.Accept(self)
      self.__writer.EndAlphabeticalIndex(self.__html, self.__alphas,
                                        self.__alphaIdx, fileName)

   def __updateAlphas(self, n):
      self.__next = n
      self.__prev = self.__writer.UpdateAlphas(self.__prev, self.__next,
                                               self.__html, self.__alphas)

   def __getCompVal(self, a):
      if type(a) == types.TupleType:
         return a[0]
      else:
         return a.GetName()

   def __comp(self, a, b):
      return cmp(self.__getCompVal(a).upper(), self.__getCompVal(b).upper())

   def __visitBlock(self, b, post):
      self.__writer.WriteAlphaIndexEntry(self.__html, b,
                                         HTMLgen.Small(' [', post, '] '))
      
   def VisitDefault(self, b): assert(0)
   
   def VisitProductionBlock(self, b): self.__visitBlock(b, 'prod')
   def VisitProblemSpaceBlock(self, b): self.__visitBlock(b, 'ps')
   def VisitOperatorBlock(self, b): self.__visitBlock(b, 'op')
   def VisitGroupBlock(self, b): self.__visitBlock(b, 'group')
   
##
# Writes a complete todo list, sorted by object name.
class TodoIndexWriter(DocBlock.DocBlockVisitor):
   def __init__(self):
      self.__writer = None
      self.__html = None
      self.__list = None
      self.__hg = None
      pass

   def __call__(self, writer, fileName):
      self.__writer = writer
      self.__hg = writer.GetHrefGen()
      self.__html = writer.CreateDoc("Complete Todo List")
      self.__writer.writeHeader(self.__html)
      
      self.__html.append(HTMLgen.Heading(1, "Complete Todo List"))
      
      col = self.__writer.Collector
      objs = col.Prods + col.ProblemSpaces + col.Operators + col.Groups
      ft = col.FilesByBaseTab
      for k, v in ft.items():
         objs.append((k, v))
      objs.sort(self.__comp)
      for o in objs:
         if type(o) == types.TupleType:
            base, files = o
            for f in files: f.Accept(self)
         else:
            o.Accept(self)
      self.__writer.writeFooter(self.__html)
      self.__html.write(fileName)

   def __getCompVal(self, a):
      if type(a) == types.TupleType:
         return a[0]
      else:
         return a.GetName()
   def __comp(self, a, b):
      return cmp(self.__getCompVal(a).upper(), self.__getCompVal(b).upper())

   def __visitBlock(self, b, post):
      if not b.GetTodoList(): return

      self.__html.append(HTMLgen.Container(HTMLgen.Href(self.__hg(b), b.GetName()),
                                           HTMLgen.Small(' [', post, ']')))
      # build a sub list...
      lst = HTMLgen.List()
      for t in b.GetTodoList():
         lst.append(HTMLgen.RawText(self.__writer.resolveRefs(t)))
      self.__html.append(lst, HTMLgen.BR())
      
   def VisitDefault(self, b): assert(0)
   
   def VisitFileBlock(self, b): self.__visitBlock(b, 'file')
   def VisitProductionBlock(self, b): self.__visitBlock(b, 'prod')
   def VisitProblemSpaceBlock(self, b): self.__visitBlock(b, 'ps')
   def VisitOperatorBlock(self, b): self.__visitBlock(b, 'op')
   def VisitGroupBlock(self, b): self.__visitBlock(b, 'group')

##
# Builds an HTML list to represent the goal hierarchy.
# Used when DOT is not available.
class GoalHierarchyListBuilder:
   def __init__(self, collector, hg):
      self.Collector = collector # Collector
      self.hg = hg               # Href generator
      # Since processing either ops or ps is so similar, we store the
      # differences in a look up table and use the generic buildX method below.
      self.lookup = [
         (self.Collector.OpChildPss, self.Collector.OperatorTab, 'OP -'),     # isPs = 0
         (self.Collector.PsChildOps, self.Collector.ProblemSpaceTab, 'PS -')  # isPs = 1
      ]
         
   def Build(self):
      l = HTMLgen.List()
      for ps in self.Collector.ProblemSpaces:
         if not ps.Operators: # no parent
            l.append(self.buildX(ps.GetName(), 1))
      return l

   def buildX(self, name, isPs):
      hierTable, objTable, label = self.lookup[isPs]
      
      ps = self.Collector.GetObject(objTable, name)
      c = HTMLgen.Container(label)
      if ps:
         c.append(HTMLgen.Href(self.hg(ps), name))
      else:      
         c.append(name)
      if hierTable.has_key(name): # psname has children then
         l = HTMLgen.List()
         for oname in hierTable[name]:
            # pass not isPs here since the hierarchy alternates
            l.append(self.buildX(oname, not isPs))
         c.append(l)
      return c
   
##
# Builds an DOT graph to represent the goal hierarchy.
# Used when DOT is available.
# Only builds the DOT definition and returns it. Doesn't actually
# call DOT.
class GoalHierarchyGraphBuilder:
   def __init__(self, collector, hg):
      self.Collector = collector # Collector
      self.hg = hg               # Href generator
      self.f = StringIO.StringIO()
      self.visited = {}
      self.nextid = 0
      # Since processing either ops or ps is so similar, we store the
      # differences in a look up table and use the generic buildX method below.
      self.lookup = [
         (self.Collector.OpChildPss, self.Collector.OperatorTab, {}),     # isPs = 0
         (self.Collector.PsChildOps, self.Collector.ProblemSpaceTab, {})  # isPs = 1
      ]
      
   def Build(self):
      self.f.write('digraph G {\n')
      self.f.write('rankdir=%s\n' % Config.Instance().GoalHierarchyRankDir)
      for ps in self.Collector.ProblemSpaces:
         self.buildX(ps.GetName(), 1)
      for op in self.Collector.Operators:
         self.buildX(op.GetName(), 0)
      self.f.write('}\n')
      return self.f.getvalue()
   
   def getId(self, name, isPs, idTable):
      if idTable.has_key(name):
         return idTable[name]
      else:
         newId = 'x%dx%d' % (isPs, self.nextid)
         idTable[name] = newId
         self.nextid += 1
         return newId
   def getNodeAttrs(self, name, obj, isPs):
      attrs = ['width=0.3,height=0.2,label="%s",fontname="Helvetica",fontsize="8"' % name]
      if isPs:
         attrs.append('shape=box')
      if obj:
         attrs.append('URL="%s"' % self.hg(obj))
         attrs.append('fontcolor=blue')
      return attrs
   def buildX(self, name, isPs):
      hierTable, objTable, idTable = self.lookup[isPs]

      psId = self.getId(name, isPs, idTable)      
      if self.visited.has_key(psId): return
      
      ps = self.Collector.GetObject(objTable, name)
      self.f.write('%s [%s];\n' % (psId, ','.join(self.getNodeAttrs(name, ps, isPs))))
      self.visited[psId] = 1
      if hierTable.has_key(name): # psname has children then
         cIdTable = self.lookup[not isPs][2]
         kids = hierTable[name]
         for oname in kids:
            # write edge to child
            self.f.write('%s -> %s;\n' % (psId, self.getId(oname, not isPs, cIdTable)))
            # pass not isPs here since the hierarchy alternates
            self.buildX(oname, not isPs)

class DatamapGraphBuilder(DmgenXmlToDot.PsOrOpToDot):
   def __init__(self, collector, hg, nodeDetailsJsFunc):
      self.Collector = collector
      self.hg = hg
      self.nodeDetailsJsFunc = nodeDetailsJsFunc
      self.NodePaths = {}
      self.DotFile = ''

   def Build(self, dmPsOrOp):
      self.NodePaths = DmgenDatamap.NodePathBuilder()(dmPsOrOp.GetStartNode())
      self.Nodes = self.NodePaths.keys()[:]
      self.Nodes.sort()
      
      self.DotFile = ''
      
      io = StringIO.StringIO()
      io.write('digraph G {\n')
      io.write('rankdir=%s\n' % Config.Instance().DatamapRankDir)
      self.Write(io, dmPsOrOp)
      io.write('}\n')

      self.DotFile = io.getvalue()
      return io.getvalue()
   
   # Overload node attributes...   
   def GetNodeAttributes(self, n):
      attrs = ['fontname="Helvetica"',
               'fontsize=8',
               'shape=box',
               'width=0.3',
               'height=0.2',
               'label="%s"'% n.Name,
               'color=blue',
               'URL="javascript:%s(%d)"' % (self.nodeDetailsJsFunc,
                                            self.Nodes.index(n))]
      # Color code the node based on LHS vs. RHS.
      if not Config.Instance().EnableDatamapNodeColoring:
         pass
      elif n.Side == 'L':
         pass
      elif n.Side == 'R':
         attrs.extend(['style=filled', 'fillcolor=black', 'fontcolor=white'])
      else:
         attrs.extend(['style=filled', 'fillcolor=lightgrey'])
         
      return attrs

##
# Writes the javascript code for opening a detail windows for
# datamap nodes. Returns an HTMLgen.Script() object.
class DatamapNodeDetailsScriptBuilder:
   def __init__(self, collector, psOrOp, nodes, nodePaths, nodeDetailsJsFunc):
      self.collector = collector
      self.psOrOp = psOrOp
      self.isPs = isinstance(psOrOp, ProblemSpaceBlock.ProblemSpaceBlock)
      self.nodes = nodes
      self.nodePaths = nodePaths
      self.nodeDetailsJsFunc = nodeDetailsJsFunc
      self.hg = HrefGen(collector)
      
   def Build(self):
      s = HTMLgen.Script()
      s.code = self.writeScriptContent()
      return s
   
   def writeScriptContent(self):
      io = StringIO.StringIO()
      # Keep the markup for each nodes in an array
      io.write('var dmAttrDetails = new Array(%d);' % len(self.nodes))
      # Define the popup function that takes an index into the markup array
      io.write('function %s(idx){' % self.nodeDetailsJsFunc)
      # Open a new blank window
      io.write('pu = window.open("about:blank", "", "scrollbars,resizable,height=300,width=400");')
      # Write the node's markup to the window
      io.write('pu.document.write(dmAttrDetails[idx]);}\n')
      # Define the markup for each node
      for n, i in zip(self.nodes, range(0, len(self.nodes))):
         io.write('dmAttrDetails[%d]="%s"\n' % (i, self.getNodeDetailsMarkup(n)))
      return io.getvalue()
         
   def getNodeDetailsMarkup(self, n):
      foundLabels = ['Operator', 'Problem Space']
      sideLabels = { 'B' : 'Found on both sides of production',
                     'R' : 'Found on right side of production',
                     'L' : 'Found on left side of production'
                   }
      html = HTMLgen.SimpleDocument(meta=CreateGeneratorTags())
      p = '.'.join(self.nodePaths[n])
      html.title = HTMLgen.Text('Attribute: %s' % p)
      html.append(HTMLgen.Heading(3, HTMLgen.Code('%s' % p, html_escape='ON')),
                  HTMLgen.Bold('%s: ' % foundLabels[self.isPs]),
                  HTMLgen.Href(self.hg(self.psOrOp), self.psOrOp.GetName(), target='main'),
                  HTMLgen.BR(),
                  HTMLgen.Bold('Type: '), n.Type,
                  HTMLgen.BR(),
                  HTMLgen.Bold('Side: '), sideLabels[n.Side],
                  HTMLgen.P())
      self.writeNodeValues(n, html)
      self.writeNodeLinks(n, html)
      self.writeNodeProds(n, html)
      s = str(html) # render page
      
      # Since this is being stored in a javascript array, we have to remove
      # newlines and escape quotes...
      s = ' '.join(s.split('\n'))   # replace newlines with spaces
      s = '\\"'.join(s.split('"'))  # escape quotes
      return s
   
   def writeNodeLinks(self, n, html):
      if not n.Links: return
      typeMap = { 'S':self.collector.ProblemSpaceTab,
                  'O':self.collector.OperatorTab
                }
      typeLabelMap = { 'S':'Prob. Space',
                       'O':'Operator'
                     }
      table = HTMLgen.TableLite(width='100%')
      table.append(HTMLgen.TR(HTMLgen.TH('References', align='left')))
      c = 1
      for t, name, path in n.Links:
         o = self.collector.GetObject(typeMap[t], name)
         if o:
            td0 = HTMLgen.TD(typeLabelMap[t], ': ',
                             HTMLgen.Href(self.hg(o), o.GetName(), target='main'))
         else:
            td0 = HTMLgen.TD(typeLabelMap[t], ': ',name)
         if path:
            td0.append('.', path)
         if c:
            table.append(HTMLgen.TR(td0, bgcolor='#f2f2ff'))
         else:
            table.append(HTMLgen.TR(td0))
         c = not c
      html.append(table)
      
   def writeNodeProds(self, n, html):
      if not n.Productions: return
      table = HTMLgen.TableLite(width='100%')
      table.append(HTMLgen.TR(HTMLgen.TH('Productions', align='left')))
      c = 1
      for name in n.Productions:
         o = self.collector.GetObject(self.collector.ProdTab, name)
         if o:
            td0 = HTMLgen.TD(HTMLgen.Href(self.hg(o), o.GetName(), target='main'))
         else:
            td0 = HTMLgen.TD(name)
         if c:
            table.append(HTMLgen.TR(td0, bgcolor='#f2f2ff'))
         else:
            table.append(HTMLgen.TR(td0))
         c = not c
      html.append(table)

   ##   
   def writeNodeValues(self, n, html):
      if not n.Values: return
      table = HTMLgen.TableLite(width='100%', cellspacing='5')
      table.append(HTMLgen.TR(HTMLgen.TH('Values', align='left', colspan='3')))
      tr = HTMLgen.TR()
      cols = Config.Instance().AttributeDetailValueColumns
      for v in n.Values:
         if len(tr) == cols:
            table.append(tr)
            tr = HTMLgen.TR()
         td = HTMLgen.TD(HTMLgen.Code(v, html_escape='ON'),
                         bgcolor='#f2f2ff',
                         nowrap='1')
         tr.append(td)
      table.append(tr)
      html.append(table)
      
      

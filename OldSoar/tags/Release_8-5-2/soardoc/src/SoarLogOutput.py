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
# Code for generating log HTML
import os
import os.path
import re
from HTMLgen import HTMLgen

import SoarLog          # for parser, etc.
import SoarDocOutput    # for WriteFooter and HrefGen
import SoarDocUtil
import BlockCollector   # for linking to doc objects
import Config

INDEX_FRAME_NAME = 'logindex'
LOG_FRAME_NAME = 'log'

def GetBaseFmtTab(b, extend = {}):
   t = { 'IndexFrame' : INDEX_FRAME_NAME,
         'LogFrame' : LOG_FRAME_NAME,
         'Cycle': b.GetCycle(),
         'LineNo': b.GetLineNo() }
   t.update(extend)
   return t

def GetSoarDocFmtTab(b, hg, extend = {}):
   href, srcHref, target = GetSoarDocHrefs(hg, b)
   t = { 'Href': href,
        'SrcHref': srcHref,
        'Brief': b.GetBrief(),
        'Desc': b.GetDesc() }
   if not b.GetBrief():
      t['Brief'] = b.GetName()
   t.update(extend)
   return t

##
# For a given SoarLog production returns the type filter
# specified in the config file.
def GetProductionTypeFilter(p):
   if p.WasFired():
      typeFilter = Config.Instance().LogProdFiredTypeFilter
   else:
      typeFilter = Config.Instance().LogProdRetractedTypeFilter
   return typeFilter
   
##
# Generate anchors for objects in the html log.
class AnchorGen(SoarLog.BaseVisitor):
   def __init__(self):
      pass

   ##
   # Function call syntax. Returns anchor name for b
   #
   # @param b State, Operator or Production object
   def __call__(self, b):
      return b.Accept(self)

   ##
   # Get an anchor for the start of a particular cycle
   #
   # @param c Integer cycle number
   def GetCycle(self, c): return str(c)
   
   def VisitDefault(self, b): assert(0) # if new objects are added we'll know

   def VisitState(self, b):
      return '%d%s' % (b.GetCycle(), b.GetId())
   def VisitOperator(self, b):
      return '%d%s' % (b.GetCycle(), b.GetId())
   def VisitProduction(self, b):
      return '%s%d_%d' % (b.GetName(), b.WasFired(), b.GetCycle())

def GetSoarDocNavFrameScript():
   cfg = Config.Instance()
   idxHref = SoarDocUtil.JoinPath(cfg.LogPathToSoarDocs, 'index.html')
   
   s = """
      var soarDocWin = null
      function openSoarDocHref(h){
         if (soarDocWin == null || soarDocWin.closed){
            soarDocWin = window.open("%s", "soarDocWin");
         }         
         soarDocWin.frames["%s"].location = h;
      }
      """ % (idxHref, cfg.MainFrameName)
   
   return s
   
def GetSoarDocNavScript():
   s = """
      function openSoarDocHref(h) {
         if(window.parent != window){ // we have the parent frame..
            window.parent.openSoarDocHref(h);
         } else {
            window.location = h // just open it in the current window
         }
      }
   """
   return s

def GetSoarDocIndexHref():
   cfg = Config.Instance()
   href = SoarDocUtil.JoinPath(cfg.LogPathToSoarDocs, 'mainpage.html', '/')
   if cfg.LogOpenSoarDocsInPopup:
      href = 'javascript:openSoarDocHref(\'%s\')' % href
   target = None
   return href, target

def GetSoarDocHrefs(hg, sdObj):
   cfg = Config.Instance()
   href = SoarDocUtil.JoinPath(cfg.LogPathToSoarDocs, hg(sdObj), '/')
   srcHref = SoarDocUtil.JoinPath(cfg.LogPathToSoarDocs,
                                  hg.GetSourceLineHref(sdObj.GetSourceFile(),
                                                       sdObj.GetSourceLineNo()), '/')
   if cfg.LogOpenSoarDocsInPopup:
      href = 'javascript:openSoarDocHref(\'%s\')' % href
      srcHref = 'javascript:openSoarDocHref(\'%s\')' % srcHref

   target = cfg.MainFrameName
   target = None
   return href, srcHref, target

def GetSoarDocHtmlHref(hg, sdObj):
   href, srcHref, tgt = GetSoarDocHrefs(hg, sdObj)
   nameHtmlHref = HTMLgen.Href(href, sdObj.GetName())
   srcHtmlHref = HTMLgen.Href(srcHref, '[src]')
   if tgt:
      nameHtmlHref.target = tgt
      srcHtmlHref.target = tgt
   return HTMLgen.Container(nameHtmlHref, HTMLgen.Small(srcHtmlHref))
      

##
# Writes operator and production index files for a log file
#
class IndexWriter:
   ##
   #
   # @param logName Name of the log (file name without extension)
   # @param logState Root state of log file
   # @param logUrl   Path to html log file (Writer)
   # @param logTgt   Name of target window for html log
   def __init__(self, collector, logName, logState, logUrl, logTgt):
      self.__col = collector
      self.__logName = logName
      self.__logState = logState
      self.__logUrl = logUrl
      self.__logTgt = logTgt
      self.__sdhg = SoarDocOutput.HrefGen(collector)
      self.__ag = AnchorGen()
      self.__stateTab = {}
      self.__opTab = {}
      self.__prodTab = {}
      self.collectObject(None, logState)

      self.OpIdxFile = logName + 'OpIdx.html'
      self.ProdIdxFile = logName + 'PrIdx.html'

      self.__logNavScript = """
         function logNav(f, s) {
            window.open(window.document.forms[f][s].value, "%s")
         }
      """ % (self.__logTgt,)

   ##
   # Write the indices. After returning, the names of the files are
   # stored in OpIdxFile, ProdIdxFile, etc attributes.
   def Write(self):
      self.writeOpIndex()
      self.writeProdIndex()

   def createDoc(self):
      html = HTMLgen.SimpleDocument(meta=SoarDocOutput.CreateGeneratorTags(),
                                    script=[HTMLgen.Script(code=self.__logNavScript),
                                            HTMLgen.Script(code=GetSoarDocNavScript())])
      return html
   
   def writeHeader(self, html):
      p = HTMLgen.Paragraph(align='center')
      space = HTMLgen.RawText(' &nbsp; ')
      cfg = Config.Instance()
      docHref, docTgt = GetSoarDocIndexHref()
      p.append(HTMLgen.Href(docHref,
                            'Docs',
                            target=docTgt), space,
               HTMLgen.Href(self.OpIdxFile, 'Operators'), space)
      if cfg.LogShowProductions:
         p.append(HTMLgen.Href(self.ProdIdxFile, 'Productions'), space)
      p.append(HTMLgen.HR())   
      html.append(p)
   
   def writeOpIndex(self):
      html = self.createDoc()
      self.writeHeader(html)
      html.append(HTMLgen.Header(2, 'Operator Summary for %s' % self.__logName))
      opNames = self.__opTab.keys()[:]
      opNames.sort()
      t = HTMLgen.TableLite(width='100%')
      html.append(t)
      t.append(HTMLgen.TR(
                  HTMLgen.TH('Operator'),
                  HTMLgen.TH('Description'),
                  HTMLgen.TH('Cycles'), align='left'))
      trBgColors = ['white', '#f2f2ff']
      c = 1
      for opName in opNames:
         tr = HTMLgen.TR(bgcolor=trBgColors[c],
                         valign='top')
         self.writeOpEntry(tr, opName, self.__opTab[opName])
         t.append(tr)
         c = not c
      SoarDocOutput.WriteFooter(html)
      html.write(os.path.join(Config.Instance().OutputDirectory,self.OpIdxFile))
      
   
   def writeOpEntry(self, tr, name, instances):
      cfg = Config.Instance()
      sdop = self.__col.GetObject(self.__col.OperatorTab, name)
      if sdop:
         href = GetSoarDocHtmlHref(self.__sdhg, sdop)
         brief = sdop.GetBrief()
      else:
         href = name
         brief = HTMLgen.Italic('No documentation found.')
      tr.append(HTMLgen.TD(href, nowrap='1'),
                HTMLgen.TD(brief))
      if cfg.LogUseCycleDropdowns:
         td = HTMLgen.TD(self.buildCycleDropdown(name, instances))
      else:
         td = HTMLgen.TD(self.buildCycleTable(instances,
                                              cfg.LogOperatorCycleColumns))
      tr.append(td)
      
   def writeProdIndex(self):
      cfg = Config.Instance()
      if not cfg.LogShowProductions: return
      
      html = self.createDoc()
      self.writeHeader(html)
      html.append(HTMLgen.Header(2, 'Production Summary for %s' % self.__logName))
      prodNames = self.__prodTab.keys()[:]
      prodNames.sort()
      t = HTMLgen.TableLite(width='100%')
      html.append(t)
      tr = HTMLgen.TR(HTMLgen.TH('Production'),
                      HTMLgen.TH('Description'),
                      align='left')
      if cfg.LogShowProdFirings: tr.append(HTMLgen.TH('Firings'))
      if cfg.LogShowProdRetractions: tr.append(HTMLgen.TH('Retractions'))
      t.append(tr)
      trBgColors = ['white', '#f2f2ff']
      c = 1
      for prodName in prodNames:
         sdpr = self.__col.GetObject(self.__col.ProdTab, prodName)
         tr = self.buildProdEntry(prodName, self.__prodTab[prodName], sdpr)
         if tr:
            tr.bgcolor = trBgColors[c]
            t.append(tr)
            c = not c # alternate colors
            
      SoarDocOutput.WriteFooter(html)
      html.write(os.path.join(Config.Instance().OutputDirectory, self.ProdIdxFile))
      
   def buildProdEntry(self, name, instances, sdpr):
      cfg = Config.Instance()

      # TODO - if this prod is filtered out completely by the
      # various firing and retraction filters, then maybe
      # we should return None here and leave it out of the
      # index...
      
      if sdpr:
         href = GetSoarDocHtmlHref(self.__sdhg, sdpr)
         brief = sdpr.GetBrief()
      else:
         href = name
         brief = HTMLgen.Italic('No documentation found.')
         
      tr = HTMLgen.TR(HTMLgen.TD(href, nowrap='1'),
                      HTMLgen.TD(brief),
                      valign='top')
      
      bct = self.buildCycleTable
      bcd = self.buildCycleDropdown
      cols = cfg.LogProductionCycleColumns

      # write a table/dropdownlist of cycle where this production was
      # fired.
      fireFilter = cfg.LogProdFiredTypeFilter
      showFirings = (cfg.LogShowProdFirings and
                     not (sdpr and        # soardoc'd
                          fireFilter and  # non-empty filter
                          not sdpr.Type in fireFilter)) # type in filter
      if showFirings: # TD for Firings
         firings = filter(lambda p: p[1].WasFired(), instances)
         if cfg.LogUseCycleDropdowns:
            td = HTMLgen.TD(bcd(name, firings))
         else:
            td = HTMLgen.TD(HTMLgen.Italic('Firings'),
                            HTMLgen.BR(),
                            bct(firings, cols),
                            HTMLgen.BR())
         tr.append(td)
      elif cfg.LogShowProdFirings:
         tr.append(HTMLgen.TD(HTMLgen.RawText('&nbsp;')))

      # write a table/dropdownlist of cycle where this production was
      # retracted.
      retractFilter = cfg.LogProdRetractedTypeFilter
      showRetractions = (cfg.LogShowProdRetractions and
                         not (sdpr and          # soardoc'd
                              retractFilter and # non-empty filter
                              not sdpr.Type in retractFilter)) # in filter
      if showRetractions: # TD for Retractions
         retractions = filter(lambda p: p[1].WasRetracted(), instances)
         if cfg.LogUseCycleDropdowns:
            td = HTMLgen.TD(bcd(name, retractions))
         else:
            td = HTMLgen.TD(HTMLgen.Italic('Retractions'),
                            HTMLgen.BR(),
                            bct(retractions, cols),
                            HTMLgen.BR())
         tr.append(td)
      elif cfg.LogShowProdRetractions:
         tr.append(HTMLgen.TD(HTMLgen.RawText('&nbsp;')))
         
      return tr
                      
   def buildCycleTable(self, instances, cols):
      ct = HTMLgen.TableLite()
      ctr = HTMLgen.TR()
      for parent, prod in instances:
         if len(ctr) == cols:
            ct.append(ctr)
            ctr = HTMLgen.TR()
         href = HTMLgen.Href('%s#%s' % (self.__logUrl, self.__ag(prod)),
                             str(prod.GetCycle()),
                             target=self.__logTgt)
         ctr.append(HTMLgen.TD(HTMLgen.Small(href)))
      ct.append(ctr)
      return ct
   
   def buildCycleDropdown(self, formName, instances):
      opts = [(str(x.GetCycle()), '%s#%s' % (self.__logUrl, self.__ag(x)))
                          for p, x in instances]
      if not opts:
         return HTMLgen.Container()
      
      s = HTMLgen.Select(opts,
                         name='cycle')
      href = HTMLgen.Href("""javascript:logNav('%s','%s')""" % (formName,
                                                                s.name),
                          'go')
      return HTMLgen.Container(HTMLgen.RawText('<form name="%s">' % formName),
                               s,
                               href,
                               HTMLgen.RawText('</form>'))
      
   def collectObject(self, parent, obj):
      if isinstance(obj, SoarLog.State):
         if not self.__stateTab.has_key(obj.GetId()):
            self.__stateTab[obj.GetId()] = []
         self.__stateTab[obj.GetId()].append((parent, obj))
      elif isinstance(obj, SoarLog.Operator):
         if not self.__opTab.has_key(obj.GetName()):
            self.__opTab[obj.GetName()] = []
         self.__opTab[obj.GetName()].append((parent, obj))
      elif isinstance(obj, SoarLog.Production):
         if not self.__prodTab.has_key(obj.GetName()):
            self.__prodTab[obj.GetName()] = []
         self.__prodTab[obj.GetName()].append((parent, obj))
      else:
         assert(0)
      for k in obj.GetChildren():
         self.collectObject(obj, k)

##
# Writes a pretty tree-like html page of a parsed log file
class LogWriter:
   ##
   #
   # @param collector   A filled BlockCollector object
   # @param logFileName Name of the log file
   # @param logState    Root state object returned by Parser
   def __init__(self, collector, logFileName, logState):
      self.__col = collector
      self.__logFileName = logFileName
      self.__logState = logState
      self.__maxDepth = SoarLog.DepthCalculator()(logState)
      self.__sdhg = SoarDocOutput.HrefGen(collector)
      self.__ag = AnchorGen()
      self.__colors = ['#f2f2ff',
                       '#e2e2ff',
                       '#d2d2ff',
                       '#c2c2ff',
                       '#b2b2ff',
                       '#a2a2ff',
                       '#b2b2ff',
                       '#c2c2ff',
                       '#d2d2ff',
                       '#e2e2ff']
      self.__lastCycle = -1
      self.__wroteCycle = 0
      
   def Write(self, fileName):
      self.setupAgentOutputFilters()
      self.__html = HTMLgen.SimpleDocument()
      self.__html.script = HTMLgen.Script(code=GetSoarDocNavScript())
      self.__html.append(HTMLgen.Heading(2,'Soar Log File: %s' % self.__logFileName))
      self.__table = HTMLgen.TableLite(border=0, cellspacing=0, width='100%')
      self.__html.append(self.__table)
      self.writeState(self.__logState, 0)
      SoarDocOutput.WriteFooter(self.__html)
      self.__html.write(fileName)

   def setupAgentOutputFilters(self):
      cfg = Config.Instance()
      if cfg.LogAgentOutputIncludeFilter:
         self.__aoInclude = re.compile(cfg.LogAgentOutputIncludeFilter)
      else:
         self.__aoInclude = re.compile('.') # Match anything
      if cfg.LogAgentOutputRejectFilter:
         self.__aoReject = re.compile(cfg.LogAgentOutputRejectFilter)
      else:
         self.__aoReject = re.compile('^$') # don't match anything (except empty string)
         
   def getDepthColor(self, d):
#      return 'white'
      return self.__colors[d % len(self.__colors)]
   
   def buildCycleCell(self, b):
      c = b.GetCycle()
      self.__wroteCycle = self.__lastCycle != c
      if not self.__wroteCycle:
         return HTMLgen.TD()
      self.__lastCycle = c
      cs = str(c)
      return HTMLgen.TD(HTMLgen.Name(self.__ag.GetCycle(c)),
                        HTMLgen.Code(HTMLgen.Bold(cs),
                                     '.' * (7 - len(cs))),
                        nowrap='1', width='1')
   
   def addFillCells(self, tr, depth):
      tabSize = Config.Instance().LogTabSize
      if depth == 0: return
      if self.__wroteCycle:
         txt = HTMLgen.Code('.' * tabSize)
      else:
         txt = HTMLgen.Code('&nbsp;' * tabSize)
      for i in range(0, depth):
         tr.append(HTMLgen.TD(txt, html_escape='OFF',
                              nowrap='1', width='1',
                              bgcolor=self.getDepthColor(i)))

   def writeChildren(self, b, depth):
      for k in b.GetChildren():
         if isinstance(k, SoarLog.State):
            self.writeState(k, depth)
         elif isinstance(k, SoarLog.Operator):
            self.writeOperator(k, depth)
         elif isinstance(k, SoarLog.Production):
            self.writeProduction(k, depth)
      
   def writeState(self, s, depth):
      cfg = Config.Instance()
      
      if not cfg.LogShowStates:
         self.writeChildren(s, depth)
         return

      if cfg.LogSuppressStateNoChanges and s.IsStateNoChange():
         print 'SoarLog: suppressing (state no-change) on state %s' % s.GetId()
         return
      
      tr = HTMLgen.TR()
      tr.append(self.buildCycleCell(s))
      self.addFillCells(tr, depth)
      c = self.getDepthColor(depth)

      try:
         txt = (cfg.LogStateFormat %
                GetBaseFmtTab(s,
                              {'Id': s.GetId(), 'Reason': s.GetReason() }))
         txt = HTMLgen.RawText(txt)
      except KeyError, e:
         print 'SoarLog: Warning, invalid key in state format: ', e
         txt = HTMLgen.Italic(
                  HTMLgen.Font('State %s (%s)' % (s.GetId(), o.GetReason()),
                               color='red'))
      
      tr.append(HTMLgen.TD(HTMLgen.Name(self.__ag(s)),
                           txt,
                           colspan=str(self.__maxDepth - depth),
                           bgcolor=c,
                           width='100%'))
      self.__table.append(tr)
      self.writeChildren(s, depth + 1)
            
   def writeOperator(self, o, depth):
      cfg = Config.Instance()
      tr = HTMLgen.TR(self.buildCycleCell(o))
      self.addFillCells(tr, depth)
      c = self.getDepthColor(depth)
      
      sdop = self.__col.GetObject(self.__col.OperatorTab, o.GetName())
      try:
         if sdop:
            txt = (cfg.LogOpWithDocFormat %
                   GetBaseFmtTab(o,
                                 GetSoarDocFmtTab(sdop,
                                                  self.__sdhg,
                                                  { 'Name': o.GetName(),
                                                    'Id': o.GetId() })))
         else:
            txt = (cfg.LogOpFormat %
                   GetBaseFmtTab(o,
                                 { 'Name': o.GetName(), 'Id': o.GetId() }))
         txt = HTMLgen.RawText(txt)
      except KeyError, e:
         print 'SoarLog: Warning, invalid key in operator format: ', e
         txt = HTMLgen.Italic(
                  HTMLgen.Font('%s (%s)' % (o.GetName(), o.GetId()),
                               color='red'))

      tr.append(HTMLgen.TD(HTMLgen.Name(self.__ag(o)),
                           txt,
                           colspan=str(self.__maxDepth - depth),
                           bgcolor=c,
                           width='100%'))
      self.__table.append(tr)
      self.writeChildren(o, depth + 1)

   def writeProduction(self, p, depth):
      cfg = Config.Instance()
      if not cfg.LogShowProductions:
         self.writeAgentOutput(p.GetOutput(), depth) # Still have to write agent outputs!
         return         
      if ((not cfg.LogShowProdFirings and p.WasFired()) or
          (not cfg.LogShowProdRetractions and p.WasRetracted())):
         self.writeAgentOutput(p.GetOutput(), depth) # Still have to write agent outputs!
         return

      # Get the SoarDoc object associated with ths production, if any      
      sdpr = self.__col.GetObject(self.__col.ProdTab, p.GetName())

      if sdpr:
         # Let's filter the production by type...
         typeFilter = GetProductionTypeFilter(p)
         if typeFilter and not sdpr.Type in typeFilter:
            return # nope, don't show this production
                     
      tr = HTMLgen.TR(self.buildCycleCell(p))
      self.addFillCells(tr, depth)
      
      fmts = ((cfg.LogProdFiredFormat, cfg.LogProdFiredWithDocFormat),
              (cfg.LogProdRetractedFormat, cfg.LogProdRetractedWithDocFormat))
      fmt, sdFmt = fmts[p.WasRetracted()] # :)
      
      try:
         if sdpr:
            txt = (sdFmt %
                   GetBaseFmtTab(p,
                                 GetSoarDocFmtTab(sdpr,
                                                  self.__sdhg,
                                                  { 'Name': p.GetName(),
                                                    'Type': sdpr.Type })))
         else:
            txt = (fmt % GetBaseFmtTab(p, { 'Name': p.GetName() }))
         txt = HTMLgen.RawText(txt)
      except KeyError, e:
         print 'SoarLog: Warning, invalid key in production format: ', e
         txt = HTMLgen.Italic(
                  HTMLgen.Font('%s (%s)' % (o.GetName(), o.GetId()),
                               color='red'))
         
      td = HTMLgen.TD(HTMLgen.Name(self.__ag(p)),
                      txt,
                      colspan=str(self.__maxDepth - depth),
                      bgcolor=self.getDepthColor(depth),
                      width='100%')
      tr.append(td)
      self.__table.append(tr)
      self.writeAgentOutput(p.GetOutput(), depth)
      
   def writeAgentOutput(self, outputs, depth):
      if not Config.Instance().LogShowAgentOutput: return

      # first pass everybody through the include filter, followed by the
      # reject filter. collect eveybody that passes both.
      outputs = [o for o in outputs if (self.__aoInclude.match(o) and
                                        not self.__aoReject.match(o))]
      if not outputs: return
      
      tr = HTMLgen.TR(valign='top')
      td = HTMLgen.TD(HTMLgen.Bold(HTMLgen.Code('agent..')),
                      width=1, bgcolor='#ffffd2')
      tr.append(td)
      
      td = HTMLgen.TD(colspan=str(self.__maxDepth), bgcolor='#ffffd2')
      for o in outputs[:-1]:
         td.append(o, HTMLgen.BR())
      o = outputs[-1]
      td.append(o)
      
      tr.append(td)
      self.__table.append(tr)
      
def GenerateLogDoc(collector, logFileName, state):
   name, ext = os.path.splitext(logFileName)    # Pull off the extension
   SoarDocOutput.CreateOutputDirectory()

   w = LogWriter(collector,
                 logFileName, state) # Write a pretty version of the log file
   logUrl = name + '_log.html'
   w.Write(os.path.join(Config.Instance().OutputDirectory, logUrl))
   iw = IndexWriter(collector,
                    name,
                    state, logUrl, 'log')  # Write indices for the log file
   iw.Write()

   # Build a frame document to hold indices and pretty log   
   f1 = HTMLgen.Frame(name=INDEX_FRAME_NAME, src=iw.OpIdxFile)
   f2 = HTMLgen.Frame(name=LOG_FRAME_NAME, src =logUrl)
   fs = HTMLgen.Frameset(f1, f2, rows='50%,50%')
   fdoc = HTMLgen.FramesetDocument(title='SoarLog: %s' % name)
   fdoc.script = HTMLgen.Script(code=GetSoarDocNavFrameScript())
   fdoc.append(fs)
   fsUrl = name + '.html'
   
   # This is a bit of hack to work around a bug in HTMLgen.
   # The FramesetDocument class doesn't properly write out its
   # script property because it's derived from BasicDocument.
   f = open(os.path.join(Config.Instance().OutputDirectory, fsUrl), 'w')
   f.write(HTMLgen.DOCTYPE)
   f.write('<head><title>%s</title>%s</head>' % (fdoc.title, str(fdoc.script)))
   f.write(str(fs))
   f.write('</html>')

      

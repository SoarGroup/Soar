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
# Config file support
import types
import StringIO
import sys

# Default Configuration Options. Used for initializing
# Config object as well as generating a commented default
# config file.
_params = (
   # Each tuple is (Name, default value, description)
   # If Name and default value are None, then it's just a section comment
   # for the generated config file.
   # Note: If you get a 'tuple' object is not callable error, then
   # you probably forget to put a comma after a closing )

#--------------------------------------------------------   
   (None, None,
    """
SoarDoc configuration file.
Various configuration pararmeters can be set with this file.
Parameters are set with this syntax:
   ParameterName = ParameterValue
   
   If ParameterValue is a string, enclose it in ''s (single quotes),
""'s (double quotes), or triple double quotes for multi-line values.
In strings '\' is an escape character in the usual C way. For file
paths use forward slashes ('/').

   For integers, just the number will suffice.

Comments are started with the # symbol."""),

#--------------------------------------------------------   
   ('ProjectName', '',
    'Name of the project, used as window title.'),

#--------------------------------------------------------   
   ('InputDirectory', '.',
    'Root directory where .soar files are stored.'),

#--------------------------------------------------------   
   ('FilePatterns', ['*.soar'],
    """List of file glob patterns of files to process.
   example: FilePatterns = ['*.soar', '*.tcl']"""),

#--------------------------------------------------------   
   ('ExcludeDirectories', [],
    """List of directory name substrings which, if present, indicate that
the directory should be ignored.
   example: ExcludeDirectories=['bin', 'log']"""),
   
#--------------------------------------------------------   
   ('OutputFormat', 'html',
    'Output format for SoarDoc. Acceptable values are:\n' +
    '    \'html\'    - Writes html files to OutputDirectory\n' +
    '    \'xml\'     - Writes xml file to OutputDirectory\n' +
    '    \'autodoc\' - Generates a file of stub SoarDoc comment blocks for\n' +
    '                  the processed source files.\n' +
    '                  filename is OutputDirectory/AutoDocFileName\n'),
   
#--------------------------------------------------------   
   ('OutputDirectory', 'html',
    'Directory to which generated documentation is written. If it does not\n' +
    'exist then it is created when SoarDoc is executed.'),
   
#--------------------------------------------------------   
   ('XmlFileName', 'soardoc.xml',
    'File name used for xml output.'),
   
#--------------------------------------------------------   
   ('AutoDocFileName', 'soardoc.soar',
    'File name used for autodoc output.'),

#--------------------------------------------------------   
   ('GenerateSource', 1,
    'If 1, then browsable source code is generated. If 0, then no source is\n' +
    'generated.'),
   
#--------------------------------------------------------   
   ('ShowProdSourceInline', 1,
    'If 1, then the source code of small productions is listed inline with the\n' +
    'documentation of the production.'),
   
#--------------------------------------------------------   
   ('MaxProdSourceLines', 50,
    'The maximum number of source lines to show when ShowProdSourceInline is\n' +
    'enabled. Setting this value to 0 will always show the entire production.\n' + 
    'This value is ignored (assumed to be 0) if GenerateSource is 0.'),

#--------------------------------------------------------   
   ('UseExistingComments', 0,
    'If 1, then comments found before productions that haven\'t been\n' +
    'documented with SoarDoc will be used as the default detailed\n' +
    'description of those productions.'),

#--------------------------------------------------------   
   ('ShowDevNotes', 1,
    'If 1, then @devnote commands are included in output.'),
   
#--------------------------------------------------------   
   ('ShowTodoLists', 1,
    'If 1, then @todo commands are included in output.'),
   
#--------------------------------------------------------   
   ('ShowGoalHierarchyNavBar', 1,
    'If 1, the small controls for moving up and down the goal hierarchy\n' +
    'are added to the top of each problem-space and operator name'),

#--------------------------------------------------------   
   ('DefaultMailHost', None,
    'Default email host appended to user names (e.g., in @modified commands).\n' +
    'If not None, then mailto: links will be generated in the documentation.\n' +
    '   example: DefaultMailHost = \'soartech.com\''),

#--------------------------------------------------------   
   ('CustomMailHosts', {},
    'Table of custom email host names, indexed by username that override\n' +
    'the host specified by DefaultMailHost\n' +
    '   example: CustomMailHosts = { \'gates\':\'microsoft.com\',\n' +
    '                               \'jobs\':\'apple.com\' }'),
   
#--------------------------------------------------------   
   ('IndexFrameName', 'index',
    'HTML window name given to navigation index frame.'),

#--------------------------------------------------------   
   ('IndexFrameHeader', '',
    'Custom HTML code inserted at the top of the navigation index frame.\n' +
    '  example: IndexFrameHeader = \'<p><img src="mylogo.gif"></p>\''),

#--------------------------------------------------------   
   ('IndexFrameFooter', '',
    'Custom HTML code appended to the end of the navigation index frame.\n' +
    '  example: IndexFrameHeader = \'<p><img src="mylogo.gif"></p>\''),

#--------------------------------------------------------   
   ('MainFrameName', 'main',
    'HTML window name given to main documentation frame.'),

#--------------------------------------------------------   
   ('MainFrameHeader', '',
    'Custom HTML code inserted at the top of the main documentation frame.\n' +
    '  example: IndexFrameHeader = \'<p><img src="mylogo.gif"></p>\''),

#--------------------------------------------------------   
   ('MainFrameFooter', '',
    'Custom HTML code appended to the end of the main documentation frame.\n' +
    '  example: IndexFrameHeader = \'<p>Copyright (c) 2003 Foo, Inc.</p>\''),
   
#--------------------------------------------------------   
   (None, None, 'dmgen XML datamap related configuration options...'),
   
#--------------------------------------------------------   
   ('XmlDatamap', None,
    'If not None, specifies the path to an XML datamap file generated\n' +
    'by dmgen.\n' +
    '   example: XmlDatamap = \'xml/index.xml\''),

#--------------------------------------------------------   
   ('XmlDmProbSpaceExcludes', [],
    'A list of problem-space names which should be excluded when reading\n' +
    'the XML datamap. This can greatly reduce execution time by leaving out\n' +
    'large problem-spaces whose graph would be unintelligable anyway.\n' +
    '   example: XmlDmProbSpaceExcludes = [\'top-ps\', \'any-ps\']'),
   
#--------------------------------------------------------   
   ('XmlDmOperatorExcludes', [],
    'Same as XmlDmProbSpaceExcludes, but for operators.'),   

#--------------------------------------------------------   
   (None, None, 'GraphViz/DOT related configuration options...'),
#--------------------------------------------------------   
   ('DotPath', '',
    'Path to GraphViz DOT executable.\n' +
    '   example: DotPath = \'C:/Program Files/Graphviz/bin\''),

#--------------------------------------------------------   
   ('DotExeName', 'dot',
    'Name of the GraphViz DOT executable.\n'),

#--------------------------------------------------------   
   ('DotImageFormat', 'png',
    'Output format of DOT images. Possible values are:\n' +
    '   gif, jpeg, jpg, png, ps\n' +
    'Obviously, only those supported by your browser, will be visible\n'
    'on the generated HTML pages.'),
   
#--------------------------------------------------------   
   ('GraphicalGoalHierarchy', 0,
    'If non-zero, then a graphical goal hierarchy is generated for\n' +
    'the goal hierarhy page. Otherwise, an indented list is used.\n' +
    'This capability requires that DotPath and DotExeName be set.'),
   
#--------------------------------------------------------   
   ('GoalHierarchyRankDir', 'LR',
    'Specifies the rank direction of graphical datamaps. Possible values:\n' +
    '   \'LR\' = Left to right ranks\n' +
    '   \'TB\' = Top to bottom ranks\n' +
    'This option is only used if GraphicalGoalHierarchy is 1.'),
   
#--------------------------------------------------------   
   ('ShowDatamaps', 0,
    'If non-zero, then a graphical datamap is generated for each\n' +
    'problem-space and operator page.\n' +
    'This capability requires that DotPath and DotExeName be set.'),

#--------------------------------------------------------   
   ('EnableDatamapNodeColoring', 1,
    'If non-zero and ShowDatamaps is enabled, then the nodes of the\n' +
    'datamap graphs are color coded by the LHS/RHS info provided by\n' +
    'dmgen. If 0, then all nodes are white and no legend is generated.'),
   
#--------------------------------------------------------   
   ('DatamapRankDir', 'LR',
    'Specifies the rank direction of graphical datamaps. Possible values:\n' +
    '   \'LR\' = Left to right ranks\n' +
    '   \'TB\' = Top to bottom ranks\n' +
    'This option is only used if ShowDatamaps is 1.'),

#--------------------------------------------------------   
   ('AttributeDetailValueColumns', 3,
    'When graphical datamaps are enabled (ShowDatamaps=1), the nodes can\n' +
    'be clicked to get more detailed information. This parameter controls\n' +
    'the number of columns used to display the detected values of the clicked\n' +
    'atribute.'),

#--------------------------------------------------------   
   (None, None, 'SoarLog (-L) related configuration options...'),
   
#--------------------------------------------------------   
   ('LogOutputFormat', 'html',
    'Output format when processing log files. Acceptable values are:\n' +
    '    \'html\' - Writes html files to OutputDirectory\n' +
    '    \'xml\'  - Writes an XML file to OutputDirectory\n' +
    '               filename is \'baseNameOfLogFile.xml\'\n'),
   
#--------------------------------------------------------   
   ('LogUseSoarDocs', 0,
    'When 1, source files in InputDirectory are parsed for SoarDoc comments\n' +
    'and links from the log output to SoarDoc files are created when possible.'),
   
#--------------------------------------------------------   
   ('LogPathToSoarDocs', '',
    'If the log output directory (OutputDirectory) is different from the\n' +
    'location of the SoarDoc files it links to, this specifies a relative\n' +
    'path to them.\n' +
    '  example: LogPathToSoarDocs=\'../html\''),
   
#--------------------------------------------------------   
   ('LogShowStates', 0,
    'If 1, then States are shown in the log.'),
   
#--------------------------------------------------------   
   ('LogSuppressStateNoChanges', 1,
    'If 1, then States with (state no-change) and all children will be suppressed.'),
   
#--------------------------------------------------------   
   ('LogShowProductions', 1,
    'If 1, then an index of productions is created.'),
   
#--------------------------------------------------------   
   ('LogShowProdFirings', 1,
    'If 1, then firings of productions are displayed.'),
   
#--------------------------------------------------------   
   ('LogShowProdRetractions', 0,
    'If 1, then retractions of productions are displayed.'),
   
#--------------------------------------------------------   
   ('LogProdFiredTypeFilter', [],
    'List of SoarDoc production @type values that are allowed to be \n' +
    'displayed.\n' +
    '   example: LogProdFiredTypeFilter = [\'\', \'proposal\', \'application\']\n' +
    'This will allow productions with no type (\'\'), proposal and application\n' +
    'types to be displayed.'),
   
#--------------------------------------------------------   
   ('LogProdRetractedTypeFilter', [],
    'List of SoarDoc production @type values that are allowed to be \n' +
    'displayed. See LogProdFiredTypeFilter.'),
   
#--------------------------------------------------------   
   ('LogShowAgentOutput', 1,
    'If 1, then any detected agent output is included in the formatted log.'),
   
#--------------------------------------------------------   
   ('LogAgentOutputIncludeFilter',
    None,
    'A regular expression (Python) which specifies what agent output should be\n' +
    'shown in the log. If None, then all output that passes\n' +
    'LogAgentOutputRejectFilter will be shown'),
   
#--------------------------------------------------------   
   ('LogAgentOutputRejectFilter',
    None,
    'A regular expression (Python) which specifies what agent output should be\n' +
    'omitted from the log. If None, then all output that has passed \n' +
    'LogAgentOutputIncludeFilter will be shown'),
   
#--------------------------------------------------------   
   ('LogOpenSoarDocsInPopup', 1,
    'If 1, then links to SoarDoc documentation will open in a popup\n' +
    'window. Otherwise, links are opened in the current frame.\n' +
    'Using popup windows requires Javascript.'),
   
#--------------------------------------------------------   
   ('LogUseCycleDropdowns', 1,
    'If 1, then dropdown boxes are used to list cycle occurances of a\n' +
    'production or operator. Otherwise, a grid of links is used.'),
   
#--------------------------------------------------------   
   ('LogProductionCycleColumns', 5,
    'Number of columns used to display cycles in which productions occur.'),
   
#--------------------------------------------------------   
   ('LogOperatorCycleColumns', 10,
    'Number of columns used to display cycles in which operators occur.'),
   
#--------------------------------------------------------   
   ('LogTabSize', 4,
    'Number of spaces used to indent log levels.'),

#--------------------------------------------------------   
   ('LogStateFormat',
    """<b>State (%(Id)s)</b> <i>(%(Reason)s)</i>""",
    """A format string for displaying states in the log.
       Strings of the form %(Prop)F are replaced with various properties of the
       state. 'Prop' is the name of the property, while F is a type specifier.
       For states, the valid properties and their types are:
           %(Cycle)s       - The current cycle
           %(LineNo)d      - The logfile line number
           %(Id)s          - The ID of the state
           %(Reason)s      - The text accompanying the state in the log
           %(IndexFrame)s  - Name of the top frame for target= in hrefs
           %(LogFrame)s    - Name of the log frame
       This string can be arbitrary HTML."""),
   
#--------------------------------------------------------   
   ('LogOpFormat',
    """<b>%(Name)s</b>""",
    """A format string for displaying an operator in the log.
Usage is the same as for LogStateFormat. In addition to Cycle,
LineNo and Id (see above), Operators have the following properties:
   %(Name)s   - Name of the operator"""),
   
#--------------------------------------------------------   
   ('LogOpWithDocFormat',
    """<b>%(Brief)s</b>
       <a href="%(Href)s">
          <small>[more]</small>
       </a>""",
    """A format string for displaying an operator in the log which has
SoarDoc data associated with it.
Usage is the same as for LogOpFormat. In addition to Cycle,
LineNo, Id and Name (see above), Operators with docs have the
following properties:
   %(Href)s      - href path to the related documentation
   %(SrcHref)s   - href path to the source code
   %(Brief)s     - Brief description (@brief)
   %(Desc)s      - Detailed description (@desc)
          """),
   
#--------------------------------------------------------   
   ('LogProdFiredFormat',
    """<i>Fired</i> <b>%(Name)s</b>""",
    """A format string for displaying a fired production in the log.
Usage is the same as for LogStateFormat. In addition to Cycle and
LineNo (see above), productions have the following properties:
   %(Name)s   - Name of the production"""),
   
#--------------------------------------------------------   
   ('LogProdFiredWithDocFormat',
    """<i>Fired:</i> <b>%(Brief)s</b>
       <a href="%(Href)s">
          <small>[more]</small>
       </a>""",
    """A format string for displaying a fired production in the log which has
SoarDoc data associated with it.
Usage is the same as for LogOpFormat. In addition to Cycle,
LineNo, and Name (see above), productions with docs have the
following properties:
   %(Href)s      - href path to the related documentation
   %(SrcHref)s   - href path to the source code
   %(Brief)s     - Brief description (@brief)
   %(Desc)s      - Detailed description (@desc)
   %(Type)s      - Production type (@type)
          """),
   
#--------------------------------------------------------   
   ('LogProdRetractedFormat',
    """<i>Retracted</i> <b>%(Name)s</b>""",
    """A format string for displaying a retracted production in the log.
Otherwise identical to LogProdFiredFormat."""),
   
#--------------------------------------------------------   
   ('LogProdRetractedWithDocFormat',
    """<i>Retracted:</i> <b>%(Brief)s</b>
       <a href="%(Href)s">
          <small>[more]</small>
       </a>""",
    """A format string for displaying a retracted production in the log
which has SoarDoc data associated with it. Otherwise identical to
LogProdFiredWithDocFormat."""),
   
)   

class Config:

   def __init__(self):
      # initialize ourself with default param values      
      for name, v, desc in _params:
         self.__dict__[name] = v # __dict__ is table of member variables :)
         
   ##
   # Calling code must catch all exceptions that execfile
   # might throw!
   def LoadFile(self, filename):
      # pass self.__dict__ as the global namespace so that when
      # the config file does:
      #     foo=1
      # it ends up being self.foo = 1
      execfile(filename, self.__dict__)

   def EvaluateOverride(self, override):
      # execute the string in the object's namespace as with LoadFile.
      try:
         exec override in self.__dict__
      except Exception, e:
         print 'Error in command-line config parameter "%s": %s' % (override, e)
         sys.exit(1);
      
   def writeParamDesc(self, f, name, desc):
      if name:
         h = '\n##\n# -%s-\n# ' % name
      else:
         h = '\n# '
      f.write(h + '\n# '.join(desc.split('\n')))
      f.write('\n')
      
   def writeStringParam(self, f, name, value):
      # if the values is multi-line, then we have
      # to put it in triple-quotes.
      if value.find('\n') >= 0:
         f.write('%s = """%s"""\n' % (name, value))
      else:
         f.write('%s = \'%s\'\n' % (name, value))
         
   def __str__(self):
      io = StringIO.StringIO()
      for name, v, desc in _params:
         v = self.__dict__[name] # in case the value has been changed
         if name:
            self.writeParamDesc(io, name, desc)
            if type(v) != types.StringType:
               io.write('%s = %s\n' % (name, str(v)))
            else:
               self.writeStringParam(io, name, v)
         else:
            io.write('\n\n' + '#' * 79)
            self.writeParamDesc(io, name, desc)
         
      return io.getvalue()

__cfg = None

##
# Returns singleton instance of Config object.
def Instance():
   global __cfg
   if not __cfg:
      __cfg = Config()
   return __cfg


if __name__ == '__main__':
   Instance().LoadFile('soardocfile')
   print Instance()

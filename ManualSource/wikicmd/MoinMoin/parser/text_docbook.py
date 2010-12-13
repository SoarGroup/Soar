# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - DocBook-XML Parser

    This code was tested with 4Suite 1.0a4 and 1.0b1

    @copyright: 2005 Henry Ho <henryho167 AT hotmail DOT com>,
                2005 MoinMoin:AlexanderSchremmer
    @license: GNU GPL, see COPYING for details.

    DOCBOOK Parser:

    Features:
    - image support through Attachment
    - internal Wikilinks if a word is a strict wikiname
    - image alt is perserved
    - works with compiled xslt stylesheet for optimized performance

    Configuration:
    - make sure you have installed the DocBook XSLT files
    - set the path to the html directory of the DocBook XSLT files in your
      wiki or farm configuration:
      docbook_html_dir = r"/usr/share/xml/docbook/stylesheet/nwalsh/html/"
      Note that this directory needs to be writable because a cache file will
      be created there.

    >How can I use Ft API for DTD validation?
    If you have PyXMl installed, you can use ValidatingReader rather than
    NonvalidatingReader.  See:
    http://uche.ogbuji.net/tech/akara/nodes/2003-01-01/domlettes
"""

import os.path
import cPickle
import re

from MoinMoin import  Page
from MoinMoin.parser.text_xslt import Parser as XsltParser
from MoinMoin.parser.text_moin_wiki import Parser as WikiParser

Dependencies = []

class Parser(XsltParser):
    """
        Send XML file formatted via XSLT.
    """

    caching = 1
    Dependencies = Dependencies

    def __init__(self, raw, request, **kw):
        XsltParser.__init__(self, raw, request)

        # relative path to docbook.xsl and compiled_xsl
        docbook_html_directory = request.cfg.docbook_html_dir
        self.db_xsl = os.path.join(docbook_html_directory, 'docbook.xsl')
        self.db_compiled_xsl = os.path.join(docbook_html_directory, 'db_compiled.dat')

        self.wikiParser = WikiParser(raw=self.raw, request=self.request, pretty_url=1)
        self.key = 'docbook'

    def format(self, formatter):
        self.wikiParser.formatter = formatter
        XsltParser.format(self, formatter)

    def append_stylesheet(self):
        """"
            virtual function, for docbook parser
        """
        abs_db_xsl = os.path.abspath(self.db_xsl)
        abs_db_compiled_xsl = os.path.abspath(self.db_compiled_xsl)

        # same as path.exists, but also test if it is a file
        if not os.path.isfile(abs_db_compiled_xsl):
            _compile_xsl(abs_db_xsl, abs_db_compiled_xsl)

        assert os.path.isfile(abs_db_compiled_xsl)

        self.processor.appendStylesheetInstance(cPickle.load(file(abs_db_compiled_xsl, 'rb')))

    def parse_result(self, result):
        """
        additional parsing to the resulting XSLT'ed result (resultString) before saving

        will do:
            BASIC CLEAN UP   : remove unnecessary HTML tags
            RESOLVE IMG SRC  : fix src to find attachment
            RESOLVE WikiNames: if a word is a valid wikiname & a valid wikipage,
                               replace word with hyperlink
        """

        # BASIC CLEAN UP
        # remove from beginning until end of body tag
        found = re.search('<body.*?>', result)
        if found:
            result = result[found.end():]

        # remove everything after & including </body>
        found = result.rfind('</body>')
        if found != -1:
            result = result[:found]

        # RESOLVE IMG SRC
        found = re.finditer('<img.*?>', result)
        if found:
            splitResult = _splitResult(found, result)
            for index in range(len(splitResult)):
                if splitResult[index].startswith('<img'):
                    found = re.search('src="(?P<source>.*?)"', splitResult[index])
                    imageSrc = found.group('source')
                    imageAlt = None # save alt
                    found = re.search('alt="(?P<alt>.*?)"', splitResult[index])
                    if found:
                        imageAlt = found.group('alt')
                    splitResult[index] = self.wikiParser.attachment(('attachment:' + imageSrc, ""))
                    if imageAlt: # restore alt
                        splitResult[index] = re.sub('alt=".*?"', 'alt="%s"' % imageAlt, splitResult[index])

            result = ''.join(splitResult)


        # RESOLVE WikiNames
        #    if a word is a valid wikiname & a valid wikipage,
        #    replace word with hyperlink

        found = re.finditer(self.wikiParser.word_rule, result, re.UNICODE|re.VERBOSE)
        if found:
            splitResult = _splitResult(found, result)

            for index in range(len(splitResult)):
                if (re.match(self.wikiParser.word_rule, splitResult[index], re.UNICODE|re.VERBOSE)
                    and Page.Page(self.request, splitResult[index]).exists()):
                    splitResult[index] = self.wikiParser._word_repl(splitResult[index])
            result = ''.join(splitResult)

        # remove stuff that fail HTML 4.01 Strict verification

        # remove unsupported attributes
        result = re.sub(' target=".*?"| type=".*?"', '', result)
        result = re.sub('<hr .*?>', '<hr>', result)

        # remove <p>...</p> inside <a>...</a> or <caption>...</caption>
        found = re.finditer('<a href=".*?</a>|<caption>.*?</caption>', result) # XXX re.DOTALL)
        if found:
            splitResult = _splitResult(found, result)
            for index in range(len(splitResult)):
                if (splitResult[index].startswith('<a href="')
                    or splitResult[index].startswith('<caption>')):
                    splitResult[index] = splitResult[index].replace('<p>', '').replace('</p>', '')
            result = ''.join(splitResult)

        return result



def _compile_xsl(XSLT_FILE, XSLT_COMPILED_FILE):
    """
        compiling docbook stylesheet

        reference: http://155.210.85.193:8010/ccia/nodes/2005-03-18/compileXslt?xslt=/akara/akara.xslt
    """
    from Ft.Xml.Xslt.Processor import Processor
    from Ft.Xml.Xslt import Stylesheet
    from Ft.Xml import InputSource
    from Ft.Lib import Uri

    # New docbook processor
    db_processor = Processor()

    # Docbook Stylesheet
    my_sheet_uri = Uri.OsPathToUri(XSLT_FILE, 1)
    sty_isrc = InputSource.DefaultFactory.fromUri(my_sheet_uri)

    # Append Stylesheet
    db_processor.appendStylesheet(sty_isrc)

    # Pickled stylesheet will be self.abs_db_compiled_xsl file
    db_root = db_processor.stylesheet.root
    fw = file(XSLT_COMPILED_FILE, 'wb')
    cPickle.dump(db_root, fw) # , protocol=2)
    fw.close()


def _splitResult(iterator, result):
    startpos = 0
    splitResult = []

    for f in iterator:
        start, end = f.span()
        splitResult.append(result[startpos:start])
        splitResult.append(result[start:end])
        startpos = end
    splitResult.append(result[startpos:])

    return splitResult


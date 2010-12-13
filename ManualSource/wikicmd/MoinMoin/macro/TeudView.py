# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Teud Macro

    This integrates the "Teud" documentation system into
    MoinMoin. Besides Teud, you need 4XSLT.

    Teud: http://purl.net/wiki/python/TeudProject
    4XSLT: http://4suite.org/

    @copyright: 2001 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

_imperr = None
try:
    from teud import xmldoc, pydoc
except ImportError, _imperr:
    pass
try:
    from xml.xslt.Processor import Processor
except ImportError, _imperr:
    pass

from MoinMoin import config, wikiutil

Dependencies = ["time"]

def macro_TeudView(macro):
    if _imperr: return "Error in TeudView macro: " + str(_imperr)

    #dtdfile = xmldoc.getDTDPath()
    xslfile = xmldoc.getDataPath('webde.xsl')
    pagename = macro.formatter.page.page_name

    if 'module' in macro.request.args:
        modname = macro.request.args["module"]
        try:
            obj = pydoc.locate(modname)
        except pydoc.ErrorDuringImport, value:
            return "Error while loading module %s: %s" % (modname, value)
        else:
            xmlstr = xmldoc.xml.document(obj, encoding=config.charset)

        navigation = '<a href="%s">Index</a>' % pagename
        pathlen = modname.count('.')
        if pathlen:
            navigation = navigation + ' | '
            modparts = modname.split('.')
            for pathidx in range(pathlen):
                path = '.'.join(modparts[:pathidx+1])
                navigation = navigation + '<a href="%s?module=%s">%s</a>' % (
                    pagename, path, modparts[pathidx])
                if pathidx < pathlen:
                    navigation = navigation + '.'
        navigation = navigation + '<hr size="1">'
    else:
        # generate index
        xmlstr = xmldoc.xml.document(None, encoding=config.charset)
        navigation = ''

    processor = Processor()
    processor.appendStylesheetFile(xslfile)
    try:
        result = processor.runString(xmlstr,
            topLevelParams={
                'uri-prefix': pagename + "?module=",
                'uri-suffix': "",
            }
        )
    except:
        print wikiutil.escape(xmlstr)
        raise

    return navigation + result


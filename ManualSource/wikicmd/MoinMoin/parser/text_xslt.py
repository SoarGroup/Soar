# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - XML Parser

    This parser was tested with 4Suite 1.0a4 and 1.0b1.

    What's new:
    * much cleaner code
    * stylesheet can be extended to support other format:
        e.g. Docbook parser using docbook->html .xsl stylesheet

    @copyright: 2001-2003 Juergen Hermann <jh@web.de>,
                2005 Henry Ho <henryho167@hotmail.com>,
                2005 by MoinMoin:AlexanderSchremmer
    @license: GNU GPL, see COPYING for details.
"""

# cStringIO cannot be used because it doesn't handle Unicode.
import StringIO

from MoinMoin import caching, config, wikiutil, Page

Dependencies = []

class Parser:
    """ Send XML file formatted via XSLT. """
    caching = 1
    Dependencies = Dependencies

    def __init__(self, raw, request, **kw):
        self.raw = raw
        self.request = request
        self.form = request.form
        self._ = request.getText
        self.base_scheme = 'wiki'
        self.base_uri = 'wiki://Self/'
        self.key = 'xslt'

    def format(self, formatter):
        """ Send the text. """
        _ = self._

        if not self.request.cfg.allow_xslt:
            # use plain text parser if XSLT is not allowed
            # can be activated in wikiconfig.py
            from MoinMoin.parser.text import Parser as TextParser
            self.request.write(formatter.sysmsg(1) +
                               formatter.rawHTML(_('XSLT option disabled, please look at HelpOnConfiguration.', wiki=True)) +
                               formatter.sysmsg(0))
            TextParser(self.raw, self.request).format(formatter)
            return

        try:
            # try importing Ft from 4suite
            # if import fails or its version is not 1.x, error msg
            from Ft.Xml import __version__ as ft_version
            assert ft_version.startswith('1.')
        except (ImportError, AssertionError):
            self.request.write(self.request.formatter.sysmsg(1) +
                               self.request.formatter.text(_('XSLT processing is not available, please install 4suite 1.x.')) +
                               self.request.formatter.sysmsg(0))
        else:
            from Ft.Lib import Uri
            from Ft.Xml import InputSource
            from Ft.Xml.Xslt.Processor import Processor
            from Ft import FtException

            msg = None

            try:
                # location of SchemeRegisteryResolver has changed since 1.0a4
                if ft_version >= "1.0a4" or ft_version == "1.0" or ("1.0.1" <= ft_version <= "1.0.9"):
                    # such version numbers suck!
                    import Ft.Lib.Resolvers # Do not remove! it looks unused, but breaks when removed!!!

                class MoinResolver(Uri.SchemeRegistryResolver):
                    """ supports resolving self.base_uri for actual pages in MoinMoin """
                    def __init__(self, handlers, base_scheme):
                        Uri.SchemeRegistryResolver.__init__(self, handlers)
                        self.supportedSchemes.append(base_scheme)

                # setting up vars for xslt Processor
                out_file = StringIO.StringIO()
                wiki_resolver = MoinResolver(
                                    handlers={self.base_scheme: self._resolve_page, },
                                    base_scheme=self.base_scheme)
                input_factory = InputSource.InputSourceFactory(resolver=wiki_resolver)

                page_uri = self.base_uri + wikiutil.url_quote(formatter.page.page_name)
                # 4Suite needs an utf-8 encoded byte string instead of an unicode object
                raw = self.raw.strip().encode('utf-8')
                self.processor = Processor()
                self.append_stylesheet() # hook, for extending this parser
                self.processor.run(
                    input_factory.fromString(raw, uri=page_uri),
                    outputStream=out_file)
                # Convert utf-8 encoded byte string into unicode
                result = out_file.getvalue().decode('utf-8')
                result = self.parse_result(result) # hook, for extending this parser

            except FtException, msg:
                etype = "XSLT"
            except Uri.UriException, msg:
                etype = "XSLT"
            except IOError, msg:
                etype = "I/O"

            if msg:
                text = wikiutil.escape(self.raw)
                text = text.expandtabs()
                text = text.replace('\n', '<br>\n')
                text = text.replace(' ', '&nbsp;')
                before = _('%(errortype)s processing error') % {'errortype': etype, }
                title = u"<strong>%s: %s</strong><p>" % (before, msg)
                self.request.write(title)
                self.request.write(text.decode(config.charset))
            else:
                self.request.write(result)
                cache = caching.CacheEntry(self.request, formatter.page, self.key, scope='item')
                cache.update(result)

    def _resolve_page(self, uri, base):
        """ URI will be resolved into StringIO with actual page content """
        from Ft.Lib import Uri
        base_uri = self.base_uri

        if uri.startswith(base_uri):
            pagename = uri[len(base_uri):]
            page = Page.Page(self.request, pagename)
            if page.exists():
                result = StringIO.StringIO(page.getPageText().encode(config.charset))
            else:
                raise Uri.UriException(Uri.UriException.RESOURCE_ERROR, loc=uri,
                                       msg='Page does not exist')
        else:
            result = Uri.UriResolverBase.resolve(self, uri, base)

        return result

    def append_stylesheet(self):
        """ for other parsers based on xslt (i.e. docbook-xml) """
        pass

    def parse_result(self, result):
        """ additional parsing to the resulting XSLT'ed result before saving """
        return result


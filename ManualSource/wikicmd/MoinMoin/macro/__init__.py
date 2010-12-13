# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Macro Implementation

    These macros are used by the wiki parser module to implement complex
    and/or dynamic page content.

    The canonical interface to plugin macros is their execute() function,
    which gets passed an instance of the Macro class.

    @copyright: 2000-2004 Juergen Hermann <jh@web.de>,
                2006-2009 MoinMoin:ThomasWaldmann,
                2007 MoinMoin:JohannesBerg
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.util import pysupport
modules = pysupport.getPackageModules(__file__)

from MoinMoin import log
logging = log.getLogger(__name__)

import re, time, os
from MoinMoin import action, config, util
from MoinMoin import wikiutil, i18n
from MoinMoin.Page import Page
from MoinMoin.datastruct.backends.wiki_dicts import WikiDict


names = ["TitleSearch", "WordIndex", "TitleIndex", "GoTo",
         # Macros with arguments
         "Icon", "Date", "DateTime", "Anchor", "MailTo", "GetVal", "TemplateList",
]

#############################################################################
### Helpers
#############################################################################

def getNames(cfg):
    if not hasattr(cfg.cache, 'macro_names'):
        lnames = names[:]
        lnames.extend(i18n.wikiLanguages().keys())
        lnames.extend(wikiutil.getPlugins('macro', cfg))
        cfg.cache.macro_names = lnames # remember it
    return cfg.cache.macro_names


#############################################################################
### Macros - Handlers for <<macroname>> markup
#############################################################################

class Macro:
    """ Macro handler

    There are three kinds of macros:
     * Builtin Macros - implemented in this file and named macro_[name]
     * Language Pseudo Macros - any lang the wiki knows can be use as
       macro and is implemented here by _m_lang()
     * External macros - implemented in either MoinMoin.macro package, or
       in the specific wiki instance in the plugin/macro directory
    """
    defaultDependency = ["time"]

    Dependencies = {
        "TitleSearch": ["namespace"],
        "TemplateList": ["namespace"],
        "WordIndex": ["namespace"],
        "TitleIndex": ["namespace"],
        "Goto": [],
        "Icon": ["user"], # users have different themes and user prefs
        "Date": ["time"],
        "DateTime": ["time"],
        "Anchor": [],
        "Mailto": ["user"],
        "GetVal": ["pages"],
        }


    def __init__(self, parser):
        self.parser = parser
        #self.form --> gone, please use self.request.{form,args,values}
        self.request = self.parser.request
        self.formatter = self.request.formatter
        self._ = self.request.getText
        self.cfg = self.request.cfg

        # Initialized on execute
        self.name = None

        # we need the lang macros to execute when html is generated,
        # to have correct dir and lang html attributes
        # note: i18n needs to be initialized first before .wikiLanguages() will work
        for lang in i18n.wikiLanguages():
            self.Dependencies[lang] = []

    def execute(self, macro_name, args):
        """ Get and execute a macro

        Try to get a plugin macro, or a builtin macro or a language
        macro, or just raise ImportError.
        """
        self.name = macro_name
        try:
            call = wikiutil.importPlugin(self.cfg, 'macro', macro_name,
                                         function='macro_%s' % macro_name)
            execute = lambda _self, _args: wikiutil.invoke_extension_function(
                                               _self.request, call, _args, [_self])
        except wikiutil.PluginAttributeError:
            # fall back to old execute() method, no longer recommended
            execute = wikiutil.importPlugin(self.cfg, 'macro', macro_name)
        except wikiutil.PluginMissingError:
            try:
                call = getattr(self, 'macro_%s' % macro_name)
                execute = lambda _self, _args: wikiutil.invoke_extension_function(
                                                   _self.request, call, _args, [])
            except AttributeError:
                if macro_name in i18n.wikiLanguages():
                    execute = self.__class__._m_lang
                else:
                    raise ImportError("Cannot load macro %s" % macro_name)
        try:
            return execute(self, args)
        except Exception, err:
            # we do not want that a faulty macro aborts rendering of the page
            # and makes the wiki UI unusable (by emitting a Server Error),
            # thus, in case of exceptions, we just log the problem and return
            # some standard text.
            try:
                page_spec = " (page: '%s')" % self.formatter.page.page_name
            except:
                page_spec = ""
            logging.exception("Macro %s%s raised an exception:" % (self.name, page_spec))
            _ = self.request.getText
            return self.formatter.text(_('<<%(macro_name)s: execution failed [%(error_msg)s] (see also the log)>>') % {
                   'macro_name': self.name,
                   'error_msg': err.args[0], # note: str(err) or unicode(err) does not work for py2.4/5/6
                 })
            import traceback
            logging.info("Stack:\n" + traceback.format_stack())

    def _m_lang(self, text):
        """ Set the current language for page content.

            Language macro are used in two ways:
             * [lang] - set the current language until next lang macro
             * [lang(text)] - insert text with specific lang inside page
        """
        if text:
            return (self.formatter.lang(1, self.name) +
                    self.formatter.text(text) +
                    self.formatter.lang(0, self.name))

        self.request.current_lang = self.name
        return ''

    def get_dependencies(self, macro_name):
        if macro_name in self.Dependencies:
            return self.Dependencies[macro_name]
        try:
            return wikiutil.importPlugin(self.request.cfg, 'macro',
                                         macro_name, 'Dependencies')
        except wikiutil.PluginError:
            return self.defaultDependency

    def macro_TitleSearch(self):
        from MoinMoin.macro.FullSearch import search_box
        return search_box("titlesearch", self)

    def macro_TemplateList(self, needle=u'.+'):
        # TODO: this should be renamed (RegExPageNameList?), it does not list only Templates...
        _ = self._
        try:
            needle_re = re.compile(needle, re.IGNORECASE)
        except re.error, err:
            raise ValueError("Error in regex %r: %s" % (needle, err))

        # Get page list readable by current user, filtered by needle
        hits = self.request.rootpage.getPageList(filter=needle_re.search)
        hits.sort()

        result = []
        result.append(self.formatter.bullet_list(1))
        for pagename in hits:
            result.append(self.formatter.listitem(1))
            result.append(self.formatter.pagelink(1, pagename, generated=1))
            result.append(self.formatter.text(pagename))
            result.append(self.formatter.pagelink(0, pagename))
            result.append(self.formatter.listitem(0))
        result.append(self.formatter.bullet_list(0))
        return ''.join(result)

    def _make_index(self, word_re=u'.+'):
        """ make an index page (used for TitleIndex and WordIndex macro)

            word_re is a regex used for splitting a pagename into fragments
            matched by it (used for WordIndex). For TitleIndex, we just match
            the whole page name, so we only get one fragment that is the same
            as the pagename.

            TODO: _make_index could get a macro on its own, more powerful / less special than WordIndex and TitleIndex.
                  It should be able to filter for specific mimetypes, maybe match pagenames by regex (replace PageList?), etc.
        """
        _ = self._
        request = self.request
        fmt = self.formatter
        allpages = int(request.values.get('allpages', 0)) != 0
        # Get page list readable by current user, filter by isSystemPage if needed
        if allpages:
            pages = request.rootpage.getPageList()
        else:
            def nosyspage(name):
                return not wikiutil.isSystemPage(request, name)
            pages = request.rootpage.getPageList(filter=nosyspage)

        word_re = re.compile(word_re, re.UNICODE)
        wordmap = {}
        for name in pages:
            for word in word_re.findall(name):
                try:
                    if not wordmap[word].count(name):
                        wordmap[word].append(name)
                except KeyError:
                    wordmap[word] = [name]

        # Sort ignoring case
        tmp = [(word.upper(), word) for word in wordmap]
        tmp.sort()
        all_words = [item[1] for item in tmp]

        index_letters = []
        current_letter = None
        output = []
        for word in all_words:
            letter = wikiutil.getUnicodeIndexGroup(word)
            if letter != current_letter:
                anchor = "idx-%s" % letter
                output.append(fmt.anchordef(anchor))
                output.append(fmt.heading(1, 2))
                output.append(fmt.text(letter.replace('~', 'Others')))
                output.append(fmt.heading(0, 2))
                current_letter = letter
            if letter not in index_letters:
                index_letters.append(letter)
            links = wordmap[word]
            if len(links) and links[0] != word: # show word fragment as on WordIndex
                output.append(fmt.strong(1))
                output.append(word)
                output.append(fmt.strong(0))

            output.append(fmt.bullet_list(1))
            links.sort()
            last_page = None
            for name in links:
                if name == last_page:
                    continue
                output.append(fmt.listitem(1))
                output.append(Page(request, name).link_to(request, attachment_indicator=1))
                output.append(fmt.listitem(0))
            output.append(fmt.bullet_list(0))

        def _make_index_key(index_letters):
            index_letters.sort()
            def letter_link(ch):
                anchor = "idx-%s" % ch
                return fmt.anchorlink(1, anchor) + fmt.text(ch.replace('~', 'Others')) + fmt.anchorlink(0)
            links = [letter_link(letter) for letter in index_letters]
            return ' | '.join(links)

        page = fmt.page
        allpages_txt = (_('Include system pages'), _('Exclude system pages'))[allpages]
        allpages_url = page.url(request, querystr={'allpages': allpages and '0' or '1'})

        output = [fmt.paragraph(1), _make_index_key(index_letters), fmt.linebreak(0),
                  fmt.url(1, allpages_url), fmt.text(allpages_txt), fmt.url(0), fmt.paragraph(0)] + output
        return u''.join(output)


    def macro_TitleIndex(self):
        return self._make_index()

    def macro_WordIndex(self):
        if self.request.isSpiderAgent: # reduce bot cpu usage
            return ''
        word_re = u'[%s][%s]+' % (config.chars_upper, config.chars_lower)
        return self._make_index(word_re=word_re)

    def macro_GoTo(self):
        """ Make a goto box

        @rtype: unicode
        @return: goto box html fragment
        """
        _ = self._
        html = [
            u'<form method="get" action="%s"><div>' % self.request.href(self.formatter.page.page_name),
            u'<div>',
            u'<input type="hidden" name="action" value="goto">',
            u'<input type="text" name="target" size="30">',
            u'<input type="submit" value="%s">' % _("Go To Page"),
            u'</div>',
            u'</form>',
            ]
        html = u'\n'.join(html)
        return self.formatter.rawHTML(html)

    def macro_Icon(self, icon=u''):
        # empty icon name isn't valid either
        if not icon:
            raise ValueError("You need to give a non-empty icon name")
        return self.formatter.icon(icon.lower())

    def __get_Date(self, args, format_date):
        _ = self._
        if args is None:
            tm = time.time() # always UTC
        elif len(args) >= 19 and args[4] == '-' and args[7] == '-' \
                and args[10] == 'T' and args[13] == ':' and args[16] == ':':
            # we ignore any time zone offsets here, assume UTC,
            # and accept (and ignore) any trailing stuff
            try:
                year, month, day = int(args[0:4]), int(args[5:7]), int(args[8:10])
                hour, minute, second = int(args[11:13]), int(args[14:16]), int(args[17:19])
                tz = args[19:] # +HHMM, -HHMM or Z or nothing (then we assume Z)
                tzoffset = 0 # we assume UTC no matter if there is a Z
                if tz:
                    sign = tz[0]
                    if sign in '+-':
                        tzh, tzm = int(tz[1:3]), int(tz[3:])
                        tzoffset = (tzh*60+tzm)*60
                        if sign == '-':
                            tzoffset = -tzoffset
                tm = (year, month, day, hour, minute, second, 0, 0, 0)
            except ValueError, err:
                raise ValueError("Bad timestamp %r: %s" % (args, err))
            # as mktime wants a localtime argument (but we only have UTC),
            # we adjust by our local timezone's offset
            try:
                tm = time.mktime(tm) - time.timezone - tzoffset
            except (OverflowError, ValueError):
                tm = 0 # incorrect, but we avoid an ugly backtrace
        else:
            # try raw seconds since epoch in UTC
            try:
                tm = float(args)
            except ValueError, err:
                raise ValueError("Bad timestamp %r: %s" % (args, err))
        return format_date(tm)

    def macro_Date(self, stamp=None):
        return self.__get_Date(stamp, self.request.user.getFormattedDate)

    def macro_DateTime(self, stamp=None):
        return self.__get_Date(stamp, self.request.user.getFormattedDateTime)

    def macro_Anchor(self, anchor=None):
        anchor = wikiutil.get_unicode(self.request, anchor, 'anchor', u'anchor')
        return self.formatter.anchordef(anchor)

    def macro_MailTo(self, email=unicode, text=u''):
        if not email:
            raise ValueError("You need to give an (obfuscated) email address")

        from MoinMoin.mail.sendmail import decodeSpamSafeEmail

        if self.request.user.valid:
            # decode address and generate mailto: link
            email = decodeSpamSafeEmail(email)
            result = (self.formatter.url(1, 'mailto:' + email, css='mailto') +
                      self.formatter.text(text or email) +
                      self.formatter.url(0))
        else:
            # unknown user, maybe even a spambot, so
            # just return text as given in macro args

            if text:
                result = self.formatter.text(text + " ")
            else:
                result = ''

            result += (self.formatter.code(1) +
                       self.formatter.text("<%s>" % email) +
                       self.formatter.code(0))

        return result

    def macro_GetVal(self, page=None, key=None):
        page = wikiutil.get_unicode(self.request, page, 'page')

        key = wikiutil.get_unicode(self.request, key, 'key')
        if page is None or key is None:
            raise ValueError("You need to give: pagename, key")

        d = self.request.dicts.get(page, {})

        # Check acl only if dictionary is defined on a wiki page.
        if isinstance(d, WikiDict) and not self.request.user.may.read(page):
            raise ValueError("You don't have enough rights on this page")

        result = d.get(key, '')

        return self.formatter.text(result)


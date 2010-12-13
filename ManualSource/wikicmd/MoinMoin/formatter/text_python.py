# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - "text/python" Formatter

    Compiles pages to executable python code.

    @copyright: 2000-2004 by Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

import time
from MoinMoin import wikiutil


class Formatter:
    """
        Inserts '<<<>>>' into the page and adds python code to
        self.code_fragments for dynamic parts of the page
        (as macros, wikinames...).
        Static parts are formatted with an external formatter.
        Page must be assembled after the parsing to get working python code.
    """

    defaultDependencies = ["time"]

    def __init__(self, request, static=[], formatter=None, **kw):
        if formatter:
            self.formatter = formatter
        else:
            from MoinMoin.formatter.text_html import Formatter
            self.formatter = Formatter(request, store_pagelinks=1)
        self.static = static
        self.code_fragments = []
        self.__formatter = "formatter"
        self.__parser = "parser"
        self.request = request
        self.__lang = request.current_lang
        self.__in_p = 0
        self.__in_pre = 0
        self.text_cmd_begin = '\nrequest.write('
        self.text_cmd_end = ')\n'

    def assemble_code(self, text):
        """inserts the code into the generated text
        """
        # Automatic invalidation due to moin code changes:
        # we are called from Page.py, so moincode_timestamp is
        # mtime of MoinMoin directory. If we detect, that the
        # saved rendering code is older than the MoinMoin directory
        # we invalidate it by raising an exception. This avoids
        # calling functions that have changed by a code update.
        # Hint: we don't check the mtime of the directories within
        # MoinMoin, so better do a touch if you only modified stuff
        # in a subdirectory.
        waspcode_timestamp = int(time.time())
        source = ["""
moincode_timestamp = int(os.path.getmtime(os.path.dirname(__file__)))
cfg_mtime = getattr(request.cfg, "cfg_mtime", None)
if moincode_timestamp > %d or cfg_mtime is None or cfg_mtime > %d:
    raise Exception("CacheNeedsUpdate")
""" % (waspcode_timestamp, waspcode_timestamp)]


        text = text.split('<<<>>>', len(self.code_fragments))
        source.extend([self.text_cmd_begin, repr(text[0])])
        i = 0
        for t in text[1:]:
            source.extend([self.text_cmd_end,
                           self.code_fragments[i],
                           self.text_cmd_begin,
                           repr(text[i+1])])
            i += 1
        source.append(self.text_cmd_end)
        source.append(self.__adjust_formatter_state())

        self.code_fragments = [] # clear code fragments to make
                                 # this object reusable
        return "".join(source)

    def __cache_if_no_id(self, name, *args, **kw):
        if not 'id' in kw:
            return getattr(self.formatter, name)(*args, **kw)
        else:
            return self.__insert_code('request.write(%s.%s(*%r, **%r))' %
                (self.__formatter, name, args, kw))

    def __getattr__(self, name):
        """
        For every thing we have no method/attribute use the formatter
        unless there's an ID in the keywords.
        """
        attr = getattr(self.formatter, name)
        if callable(attr):
            return lambda *args, **kw: self.__cache_if_no_id(name, *args, **kw)
        else:
            return attr

    def __insert_code(self, call):
        """ returns the python code
        """
        self.code_fragments.append(call)
        return '<<<>>>'

    def __insert_fmt_call(self, function, *args, **kw):
        return self.__insert_code('request.write(%s.%s(*%r, **%r))' % (
            self.__formatter, function, args, kw))

    def __is_static(self, dependencies):
        for dep in dependencies:
            if dep not in self.static:
                return False
        return True

    def __adjust_language_state(self):
        """ Add current language state changing code to the cache """
        if self.__lang != self.request.current_lang:
            self.__lang = self.request.current_lang
            return 'request.current_lang = %r\n' % self.__lang
        return ''

    def __adjust_formatter_state(self):
        result = self.__adjust_language_state()
        if self.__in_p != self.formatter.in_p:
            result = "%s%s.in_p = %r\n" % (result, self.__formatter,
                                           self.formatter.in_p)
            self.__in_p = self.formatter.in_p
        if self.__in_pre != self.formatter.in_pre:
            result = "%s%s.in_pre = %r\n" % (result, self.__formatter,
                                           self.formatter.in_pre)
            self.__in_pre = self.formatter.in_pre
        return result

    # Public methods ---------------------------------------------------

    def pagelink(self, on, pagename='', page=None, **kw):
        if on:
            return self.__insert_code('page=Page(request, %r, formatter=%s);'
                                      'request.write(%s.pagelink(%r, page=page, **%r))' %
                                      (pagename, self.__formatter,
                                       self.__formatter, on, kw))
        else:
            return self.__insert_code('request.write(%s.pagelink(%r, page=page, **%r))' %
                                      (self.__formatter, on, kw))

    def attachment_link(self, on, url=None, **kw):
        return self.__insert_fmt_call('attachment_link', on, url, **kw)

    def attachment_image(self, url, **kw):
        return self.__insert_fmt_call('attachment_image', url, **kw)

    def attachment_drawing(self, url, text, **kw):
        return self.__insert_fmt_call('attachment_drawing', url, text, **kw)

    def attachment_inlined(self, url, text, **kw):
        return self.__insert_fmt_call('attachment_inlined', url, text, **kw)

    def heading(self, on, depth, **kw):
        if on:
            code = [
                self.__adjust_language_state(),
                'request.write(%s.heading(%r, %r, **%r))' % (self.__formatter,
                                                             on, depth, kw),
                ]
            return self.__insert_code(''.join(code))
        else:
            return self.formatter.heading(on, depth, **kw)

    def icon(self, type):
        if self.__is_static(['user']):
            return self.formatter.icon(type)
        else:
            return self.__insert_fmt_call('icon', type)

    smiley = icon

    def span(self, on, **kw):
        if on and 'comment' in kw.get('css_class', '').split():
            return self.__insert_fmt_call('span', on, **kw)
        else:
            return self.formatter.span(on, **kw)

    def div(self, on, **kw):
        if on and 'comment' in kw.get('css_class', '').split():
            return self.__insert_fmt_call('div', on, **kw)
        else:
            return self.formatter.div(on, **kw)

    def macro(self, macro_obj, name, args, markup=None):
        if self.__is_static(macro_obj.get_dependencies(name)):
            # XXX: why is this necessary??
            macro_obj.formatter = self
            return macro_obj.execute(name, args)
        else:
            return self.__insert_code(
                '%srequest.write(%s.macro(macro_obj, %r, %r, %r))' %
                (self.__adjust_formatter_state(),
                 self.__formatter, name, args, markup))

    def parser(self, parser_name, lines):
        """ parser_name MUST be valid!
            prints out the result instead of returning it!
        """
        try:
            Dependencies = wikiutil.searchAndImportPlugin(self.request.cfg, "parser", parser_name, "Dependencies")
        except (wikiutil.PluginMissingError, wikiutil.PluginAttributeError):
            Dependencies = self.defaultDependencies

        if self.__is_static(Dependencies):
            return self.formatter.parser(parser_name, lines)
        else:
            return self.__insert_code('%s%s.parser(%r, %r)' %
                                      (self.__adjust_formatter_state(),
                                       self.__formatter,
                                       parser_name, lines))

    def startContent(self, content_id='content', newline=True, **kw):
        # we need to tell the request about the start of the content
        return self.__insert_fmt_call('startContent', content_id, newline, **kw)

    def endContent(self, newline=True):
        # we need to tell the request about the end of the content
        return self.__insert_fmt_call('endContent', newline)

    def anchorlink(self, on, name='', **kw):
        # anchorlink depends on state now, namely the include ID in the request.
        if on:
            return self.__insert_fmt_call('anchorlink', on, name, **kw)
        else:
            return self.formatter.anchorlink(on, name=name, **kw)

    def line_anchorlink(self, on, lineno=0):
        # anchorlink depends on state now, namely the include ID in the request.
        if on:
            return self.__insert_fmt_call('line_anchorlink', on, lineno)
        else:
            return self.formatter.line_anchorlink(on, lineno)

    def code_area(self, on, code_id, code_type='code', show=0, start=-1, step=-1, msg=None):
        if on:
            # update state of the HTML formatter
            self.formatter._in_code_area = 1
            self.formatter._in_code_line = 0
            self.formatter._code_area_state = [None, show, start, step, start]
            return self.__insert_fmt_call('code_area', on, code_id, code_type, show,
                                          start, step)
        else:
            return self.formatter.code_area(False, code_id, code_type, show, start, step)

    def line_anchordef(self, lineno):
        return self.__insert_fmt_call('line_anchordef', lineno)

    def anchordef(self, id):
        return self.__insert_fmt_call('anchordef', id)

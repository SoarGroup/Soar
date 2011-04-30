# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - MoinMoin Wiki Markup Parser

    @copyright: 2000-2002 Juergen Hermann <jh@web.de>,
                2006-2008 MoinMoin:ThomasWaldmann,
                2007 by MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""

import re

from MoinMoin import log
logging = log.getLogger(__name__)

from MoinMoin import config, wikiutil, macro
from MoinMoin.Page import Page
from MoinMoin.support.python_compatibility import set

Dependencies = ['user'] # {{{#!wiki comment ... }}} has different output depending on the user's profile settings


_ = lambda x: x

class Parser:
    """
        Parse wiki format markup (and call the formatter to generate output).

        All formatting commands can be parsed one line at a time, though
        some state is carried over between lines.

        Methods named like _*_repl() are responsible to handle the named regex patterns.
    """

    extensions = ['.moin']
    # allow caching
    caching = 1
    Dependencies = Dependencies
    quickhelp = _(u"""\
 Emphasis:: <<Verbatim('')>>''italics''<<Verbatim('')>>; <<Verbatim(''')>>'''bold'''<<Verbatim(''')>>; <<Verbatim(''''')>>'''''bold italics'''''<<Verbatim(''''')>>; <<Verbatim('')>>''mixed ''<<Verbatim(''')>>'''''bold'''<<Verbatim(''')>> and italics''<<Verbatim('')>>; <<Verbatim(----)>> horizontal rule.
 Headings:: = Title 1 =; == Title 2 ==; === Title 3 ===; ==== Title 4 ====; ===== Title 5 =====.
 Lists:: space and one of: * bullets; 1., a., A., i., I. numbered items; 1.#n start numbering at n; space alone indents.
 Links:: <<Verbatim(JoinCapitalizedWords)>>; <<Verbatim([[target|linktext]])>>.
 Tables:: || cell text |||| cell text spanning 2 columns ||;    no trailing white space allowed after tables or titles.

(!) For more help, see HelpOnEditing or HelpOnMoinWikiSyntax.
""")

    # some common strings
    CHILD_PREFIX = wikiutil.CHILD_PREFIX
    CHILD_PREFIX_LEN = wikiutil.CHILD_PREFIX_LEN
    PARENT_PREFIX = wikiutil.PARENT_PREFIX
    PARENT_PREFIX_LEN = wikiutil.PARENT_PREFIX_LEN

    punct_pattern = re.escape(u'''"\'}]|:,.)?!''')
    url_scheme = u'|'.join(config.url_schemas)

    # some common rules
    url_rule = ur'''
        (?:^|(?<=\W))  # require either beginning of line or some non-alphanum char (whitespace, punctuation) to the left
        (?P<url_target>  # capture whole url there
         (?P<url_scheme>%(url_scheme)s)  # some scheme
         \:
         \S+?  # anything non-whitespace
        )
        (?:$|(?=\s|[%(punct)s]+(\s|$)))  # require either end of line or some whitespace or some punctuation+blank/eol afterwards
    ''' % {
        'url_scheme': url_scheme,
        'punct': punct_pattern,
    }

    # this is for a free (non-bracketed) interwiki link - to avoid false positives,
    # we are rather restrictive here (same as in moin 1.5: require that the
    # interwiki_wiki name starts with an uppercase letter A-Z. Later, the code
    # also checks whether the wiki name is in the interwiki map (if not, it renders
    # normal text, no link):
    interwiki_rule = ur'''
        (?:^|(?<=\W))  # require either beginning of line or some non-alphanum char (whitespace, punctuation) to the left
        (?P<interwiki_wiki>[A-Z][a-zA-Z]+)  # interwiki wiki name
        \:
        (?P<interwiki_page>  # interwiki page name
         (?=[^ ]*[%(u)s%(l)s0..9][^ ]*\ )  # make sure there is something non-blank with at least one alphanum letter following
         [^\s%(punct)s]+  # we take all until we hit some blank or punctuation char ...
        )
    ''' % {
        'u': config.chars_upper,
        'l': config.chars_lower,
        'punct': punct_pattern,
    }

    # BE CAREFUL: if you do changes to word_rule, consider doing them also to word_rule_js (see below)
    word_rule = ur'''
        (?:
         (?<![%(u)s%(l)s/])  # require anything not upper/lower/slash before
         |
         ^  # ... or beginning of line
        )
        (?P<word_bang>\!)?  # configurable: avoid getting CamelCase rendered as link
        (?P<word_name>
         (?:
          (%(parent)s)*  # there might be either ../ parent prefix(es)
          |
          ((?<!%(child)s)%(child)s)?  # or maybe a single / child prefix (but not if we already had it before)
         )
         (
          ((?<!%(child)s)%(child)s)?  # there might be / child prefix (but not if we already had it before)
          (?:[%(u)s][%(l)s]+){2,}  # at least 2 upper>lower transitions make CamelCase
         )+  # we can have MainPage/SubPage/SubSubPage ...
         (?:
          \#  # anchor separator          TODO check if this does not make trouble at places where word_rule is used
          (?P<word_anchor>\S+)  # some anchor name
         )?
        )
        (?:
         (?![%(u)s%(l)s/])  # require anything not upper/lower/slash following
         |
         $  # ... or end of line
        )
    ''' % {
        'u': config.chars_upper,
        'l': config.chars_lower,
        'child': re.escape(CHILD_PREFIX),
        'parent': re.escape(PARENT_PREFIX),
    }
    # simplified word_rule for FCKeditor's "unlink" plugin (puts a ! in front of a WikiName if WikiName matches word_rule_js),
    # because JavaScript can not use group names and verbose regular expressions!
    word_rule_js = (
        ur'''(?:(?<![%(u)s%(l)s/])|^)'''
        ur'''(?:'''
         ur'''(?:(%(parent)s)*|((?<!%(child)s)%(child)s)?)'''
         ur'''(((?<!%(child)s)%(child)s)?(?:[%(u)s][%(l)s]+){2,})+'''
         ur'''(?:\#(?:\S+))?'''
        ur''')'''
        ur'''(?:(?![%(u)s%(l)s/])|$)'''
    ) % {
        'u': config.chars_upper,
        'l': config.chars_lower,
        'child': re.escape(CHILD_PREFIX),
        'parent': re.escape(PARENT_PREFIX),
    }

    # link targets:
    extern_rule = r'(?P<extern_addr>(?P<extern_scheme>%s)\:.*)' % url_scheme
    attach_rule = r'(?P<attach_scheme>attachment|drawing)\:(?P<attach_addr>.*)'
    page_rule = r'(?P<page_name>.*)'

    link_target_rules = r'|'.join([
        extern_rule,
        attach_rule,
        page_rule,
    ])
    link_target_re = re.compile(link_target_rules, re.VERBOSE|re.UNICODE)

    link_rule = r"""
        (?P<link>
            \[
            \s*
            (?P<link_target>[^ ]+?)
            \s*
            (?P<link_desc>[^ ]+)?
            \]
        )
    """

    transclude_rule = r"""
        (?P<transclude>
            \{\{
            \s*(?P<transclude_target>[^|]+?)\s*  # usually image target (strip space)
            (\|\s*(?P<transclude_desc>[^|]+?)?\s*  # usually image alt text (optional, strip space)
                (\|\s*(?P<transclude_params>[^|]+?)?\s*  # transclusion parameters (usually key="value" format, optional, strip space)
                )?
            )?
            \}\}
        )
    """
    text_rule = r"""
        (?P<simple_text>
            [^|]+  # some text (not empty, does not contain separator)
        )
    """
    # link descriptions:
    link_desc_rules = r'|'.join([
            transclude_rule,
            text_rule,
    ])
    link_desc_re = re.compile(link_desc_rules, re.VERBOSE|re.UNICODE)

    # transclude descriptions:
    transclude_desc_rules = r'|'.join([
            text_rule,
    ])
    transclude_desc_re = re.compile(transclude_desc_rules, re.VERBOSE|re.UNICODE)

    # lists:
    ol_rule = ur"""
        ^\s+  # indentation
        (?:[0-9]+|[aAiI])\. # arabic, alpha, roman counting
        (?:\#\d+)?  # optional start number
        \s  # require one blank afterwards
    """
    ol_re = re.compile(ol_rule, re.VERBOSE|re.UNICODE)

    dl_rule = ur"""
        ^\s+  # indentation
        .*?::  # definition term::
        \s  # require on blank afterwards
    """
    dl_re = re.compile(dl_rule, re.VERBOSE|re.UNICODE)

    # others
    indent_re = re.compile(ur"^\s*", re.UNICODE)
    eol_re = re.compile(r'\r?\n', re.UNICODE)

    # this is used inside parser/pre sections (we just want to know when it's over):
    parser_unique = u''
    parser_scan_rule = ur"""
(?P<parser_end>
    %s\}\}\}  # in parser/pre, we only look for the end of the parser/pre
)
"""


    # the big, fat, less ugly one ;)
    # please be very careful: blanks and # must be escaped with \ !
    scan_rules = ur"""
(?P<emph>
    (
     (?<=[^a-zA-Z0-9])_
    |
     _(?=[^a-zA-Z0-9])
    |
     ^_
    |
     _$
    )
)|(?P<strong>
    (
     (?<=[^ 	])[*]
    |
     [*](?=[^ 	])
    |
     ^[*]
    |
     [*]$
    )
)|(?P<u>
    __ # underline
)|(?P<small>
    (
     (?P<small_on>\~-\ ?)  # small on (we eat a trailing blank if it is there)
    |
     (?P<small_off>-\~)  # small off
    )
)|(?P<big>
    (
     (?P<big_on>\~\+\ ?)  # big on (eat trailing blank)
    |
     (?P<big_off>\+\~)  # big off
    )
)|(?P<strike>
    (
     (?P<strike_on>--\()  # strike-through on
    |
     (?P<strike_off>\)--)  # strike-through off
    )
)|(?P<remark>
    (
     (^|(?<=\s))  # we require either beginning of line or some whitespace before a remark begin
     (?P<remark_on>/\*\s)  # inline remark on (require and eat whitespace after it)
    )
    |
    (
     (?P<remark_off>\s\*/)  # off (require and eat whitespace before it)
     (?=\s)  # we require some whitespace after a remark end
    )
)|(?P<sup>
    \^  # superscript on
    (?P<sup_text>.*?)  # capture the text
    \^  # off
)|(?P<sub>
    ,,  # subscript on
    (?P<sub_text>.*?)  # capture the text
    ,,  # off
)|(?P<tt>
    \{\{\{  # teletype on
    (?P<tt_text>.*?)  # capture the text
    \}\}\}  # off
)|(?P<tt_bt>
    `  # teletype (using a backtick) on
    (?P<tt_bt_text>.*?)  # capture the text
    `  # off
)|(?P<interwiki>
    %(interwiki_rule)s  # OtherWiki:PageName
)|
%(link_rule)s
|
%(transclude_rule)s
|(?P<url>
    %(url_rule)s
)|(?P<word>  # must come AFTER interwiki rule!
    %(word_rule)s  # CamelCase wiki words
)|(?P<email>
    [-\w._+]+  # name
    \@  # at
    [\w-]+(\.[\w-]+)+  # server/domain
)|(?P<smiley>
    (^|(?<=\s))  # we require either beginning of line or some space before a smiley
    (%(smiley)s)  # one of the smileys
    (?=\s)  # we require some space after the smiley
)|(?P<macro>
    <<
    (?P<macro_name>\w+)  # name of the macro
    (?:\((?P<macro_args>.*?)\))?  # optionally macro arguments
    >>
)|(?P<heading>
    ^(?P<hmarker>=+)\s+  # some === at beginning of line, eat trailing blanks
    (?P<heading_text>.*?)  # capture heading text
    \s+(?P=hmarker)\s$  # some === at end of line (matching amount as we have seen), eat blanks
)|(?P<parser>
    \{\{\{  # parser on
    (?P<parser_unique>(\{*|\w*))  # either some more {{{{ or some chars to solve the nesting problem
    (?P<parser_line>
     (
      \#!  # hash bang
      (?P<parser_name>\w*)  # we have a parser name (can be empty) directly following the {{{
      (
       \s+  # some space ...
       (?P<parser_args>.+?)  # followed by parser args
      )?  # parser args are optional
      \s*  # followed by whitespace (eat it) until EOL
     )
    |
     (?P<parser_nothing>\s*)  # no parser name, only whitespace up to EOL (eat it)
    )$
    # "parser off" detection is done with parser_scan_rule!
)|(?P<comment>
    ^\#\#.*$  # src code comment, rest of line
)|(?P<ol>
    %(ol_rule)s  # ordered list
)|(?P<dl>
    %(dl_rule)s  # definition list
)|(?P<li>
    ^\s+\*\s*  # unordered list
)|(?P<li_none>
    ^\s+\.\s*  # unordered list, no bullets
)|(?P<indent>
    ^\s+  # indented by some spaces
)|(?P<tableZ>
    \|\|\ $  # the right end of a table row
)|(?P<table>
    (?:\|\|)+(?:<(?!<)[^>]*?>)?(?!\|?\s$) # a table
)|(?P<rule>
    -{4,}  # hor. rule, min. 4 -
)|(?P<entity>
    &(
      ([a-zA-Z]+)  # symbolic entity, like &uuml;
      |
      (\#(\d{1,5}|x[0-9a-fA-F]+))  # numeric entities, like &#42; or &#x42;
     );
)|(?P<sgml_entity>  # must come AFTER entity rule!
    [<>&]  # needs special treatment for html/xml
)"""  % {
        'url_scheme': url_scheme,
        'url_rule': url_rule,
        'punct': punct_pattern,
        'ol_rule': ol_rule,
        'dl_rule': dl_rule,
        'interwiki_rule': interwiki_rule,
        'word_rule': word_rule,
        'link_rule': link_rule,
        'transclude_rule': transclude_rule,
        'u': config.chars_upper,
        'l': config.chars_lower,
        'smiley': u'|'.join([re.escape(s) for s in config.smileys])}
    scan_re = re.compile(scan_rules, re.UNICODE|re.VERBOSE)

    # Don't start p before these
    no_new_p_before = ("heading rule table tableZ tr td "
                       "ul ol dl dt dd li li_none indent "
                       "macro parser")
    no_new_p_before = no_new_p_before.split()
    no_new_p_before = dict(zip(no_new_p_before, [1] * len(no_new_p_before)))

    def __init__(self, raw, request, **kw):
        self.raw = raw
        self.request = request
        self.form = request.form # Macro object uses this
        self._ = request.getText
        self.cfg = request.cfg
        self.line_anchors = kw.get('line_anchors', True)
        self.start_line = kw.get('start_line', 0)
        self.macro = None

        # currently, there is only a single, optional argument to this parser and
        # (when given), it is used as class(es) for a div wrapping the formatter output
        # either use a single class like "comment" or multiple like "comment/red/dotted"
        self.wrapping_div_class = kw.get('format_args', '').strip().replace('/', ' ')

        self.is_em = 0 # must be int
        self.is_b = 0 # must be int
        self.is_u = False
        self.is_strike = False
        self.is_big = False
        self.is_small = False
        self.is_remark = False

        self.lineno = 0
        self.in_list = 0 # between <ul/ol/dl> and </ul/ol/dl>
        self.in_li = 0 # between <li> and </li>
        self.in_dd = 0 # between <dd> and </dd>

        # states of the parser concerning being inside/outside of some "pre" section:
        # None == we are not in any kind of pre section (was: 0)
        # 'search_parser' == we didn't get a parser yet, still searching for it (was: 1)
        # 'found_parser' == we found a valid parser (was: 2)
        self.in_pre = None

        self.in_table = 0
        self.inhibit_p = 0 # if set, do not auto-create a <p>aragraph

        # holds the nesting level (in chars) of open lists
        self.list_indents = []
        self.list_types = []

    def _close_item(self, result):
        #result.append("<!-- close item begin -->\n")
        if self.in_table:
            result.append(self.formatter.table(0))
            self.in_table = 0
        if self.in_li:
            self.in_li = 0
            if self.formatter.in_p:
                result.append(self.formatter.paragraph(0))
            result.append(self.formatter.listitem(0))
        if self.in_dd:
            self.in_dd = 0
            if self.formatter.in_p:
                result.append(self.formatter.paragraph(0))
            result.append(self.formatter.definition_desc(0))
        #result.append("<!-- close item end -->\n")

    def _u_repl(self, word, groups):
        """Handle underline."""
        self.is_u = not self.is_u
        return self.formatter.underline(self.is_u)

    def _remark_repl(self, word, groups):
        """Handle remarks."""
        on = groups.get('remark_on')
        if on and self.is_remark:
            return self.formatter.text(word)
        off = groups.get('remark_off')
        if off and not self.is_remark:
            return self.formatter.text(word)
        self.is_remark = not self.is_remark
        return self.formatter.span(self.is_remark, css_class='comment')
    _remark_on_repl = _remark_repl
    _remark_off_repl = _remark_repl

    def _strike_repl(self, word, groups):
        """Handle strikethrough."""
        on = groups.get('strike_on')
        if on and self.is_strike:
            return self.formatter.text(word)
        off = groups.get('strike_off')
        if off and not self.is_strike:
            return self.formatter.text(word)
        self.is_strike = not self.is_strike
        return self.formatter.strike(self.is_strike)
    _strike_on_repl = _strike_repl
    _strike_off_repl = _strike_repl

    def _small_repl(self, word, groups):
        """Handle small."""
        on = groups.get('small_on')
        if on and self.is_small:
            return self.formatter.text(word)
        off = groups.get('small_off')
        if off and not self.is_small:
            return self.formatter.text(word)
        self.is_small = not self.is_small
        return self.formatter.small(self.is_small)
    _small_on_repl = _small_repl
    _small_off_repl = _small_repl

    def _big_repl(self, word, groups):
        """Handle big."""
        on = groups.get('big_on')
        if on and self.is_big:
            return self.formatter.text(word)
        off = groups.get('big_off')
        if off and not self.is_big:
            return self.formatter.text(word)
        self.is_big = not self.is_big
        return self.formatter.big(self.is_big)
    _big_on_repl = _big_repl
    _big_off_repl = _big_repl

    def _emph_repl(self, word, groups):
        """Handle emphasis, i.e. * and _."""
        self.is_em = not self.is_em
        if self.is_em and self.is_b:
            self.is_em = 2
        return self.formatter.emphasis(self.is_em)

    def _strong_repl(self, word, groups):
        """Handle emphasis, i.e. * and _."""
        self.is_b = not self.is_b
        if self.is_em and self.is_b:
            self.is_b = 2
        return self.formatter.strong(self.is_b)

    def _sup_repl(self, word, groups):
        """Handle superscript."""
        text = groups.get('sup_text', '')
        return (self.formatter.sup(1) +
                self.formatter.text(text) +
                self.formatter.sup(0))
    _sup_text_repl = _sup_repl

    def _sub_repl(self, word, groups):
        """Handle subscript."""
        text = groups.get('sub_text', '')
        return (self.formatter.sub(1) +
               self.formatter.text(text) +
               self.formatter.sub(0))
    _sub_text_repl = _sub_repl

    def _tt_repl(self, word, groups):
        """Handle inline code."""
        tt_text = groups.get('tt_text', '')
        return (self.formatter.code(1) +
                self.formatter.text(tt_text) +
                self.formatter.code(0))
    _tt_text_repl = _tt_repl

    def _tt_bt_repl(self, word, groups):
        """Handle backticked inline code."""
        tt_bt_text = groups.get('tt_bt_text', '')
        return (self.formatter.code(1, css="backtick") +
                self.formatter.text(tt_bt_text) +
                self.formatter.code(0))
    _tt_bt_text_repl = _tt_bt_repl

    def _rule_repl(self, word, groups):
        """Handle sequences of dashes."""
        result = self._undent() + self._closeP()
        if len(word) <= 4:
            result += self.formatter.rule()
        else:
            # Create variable rule size 1 - 6. Actual size defined in css.
            size = min(len(word), 10) - 4
            result += self.formatter.rule(size)
        return result

    def _interwiki_repl(self, word, groups):
        """Handle InterWiki links."""
        wiki = groups.get('interwiki_wiki')
        page = groups.get('interwiki_page')

        wikitag_bad = wikiutil.resolve_interwiki(self.request, wiki, page)[3]
        if wikitag_bad:
            text = groups.get('interwiki')
            return self.formatter.text(text)
        else:
            page, anchor = wikiutil.split_anchor(page)
            return (self.formatter.interwikilink(1, wiki, page, anchor=anchor) +
                    self.formatter.text(page) +
                    self.formatter.interwikilink(0, wiki, page))
    _interwiki_wiki_repl = _interwiki_repl
    _interwiki_page_repl = _interwiki_repl

    def _word_repl(self, word, groups):
        """Handle WikiNames."""
        bang = ''
        bang_present = groups.get('word_bang')
        if bang_present:
            if self.cfg.bang_meta:
                # handle !NotWikiNames
                return self.formatter.nowikiword(word)
            else:
                bang = self.formatter.text('!')
        name = groups.get('word_name')
        current_page = self.formatter.page.page_name
        abs_name = wikiutil.AbsPageName(current_page, name)
        # if a simple, self-referencing link, emit it as plain text
        if abs_name == current_page:
            return self.formatter.text(word)
        else:
            abs_name, anchor = wikiutil.split_anchor(abs_name)
            return (bang +
                    self.formatter.pagelink(1, abs_name, anchor=anchor) +
                    self.formatter.pagelink(0, abs_name))
    _word_bang_repl = _word_repl
    _word_name_repl = _word_repl
    _word_anchor_repl = _word_repl

    def _url_repl(self, word, groups):
        """Handle literal URLs."""
        scheme = groups.get('url_scheme', 'http')
        target = groups.get('url_target', '')
        return (self.formatter.url(1, target, css=scheme) +
                self.formatter.text(target) +
                self.formatter.url(0))
    _url_target_repl = _url_repl
    _url_scheme_repl = _url_repl

    def _transclude_description(self, desc, default_text=''):
        """ parse a string <desc> valid as transclude description (text, ...)
            and return the description.

            We do NOT use wikiutil.escape here because it is html specific (the
            html formatter, if used, does this for all html attributes).

            We do NOT call formatter.text here because it sometimes is just used
            for some alt and/or title attribute, but not emitted as text.

            @param desc: the transclude description to parse
            @param default_text: use this text if parsing desc returns nothing.
        """
        m = self.transclude_desc_re.match(desc)
        if m:
            if m.group('simple_text'):
                desc = m.group('simple_text')
        else:
            desc = default_text
        return desc

    def _get_params(self, params, tag_attrs=None, acceptable_attrs=None, query_args=None):
        """ parse the parameters of link/transclusion markup,
            defaults can be a dict with some default key/values
            that will be in the result as given, unless overriden
            by the params.
        """
        if tag_attrs is None:
            tag_attrs = {}
        if query_args is None:
            query_args = {}
        if params:
            fixed, kw, trailing = wikiutil.parse_quoted_separated(params)
            # we ignore fixed and trailing args and only use kw args:
            if acceptable_attrs is None:
                acceptable_attrs = []
            for key, val in kw.items():
                # wikiutil.escape for key/val must be done by (html) formatter!
                if key in acceptable_attrs:
                    # tag attributes must be string type
                    tag_attrs[str(key)] = val
                elif key.startswith('&'):
                    key = key[1:]
                    query_args[key] = val
        return tag_attrs, query_args

    def _transclude_repl(self, word, groups):
        """Handles transcluding content, usually embedding images."""
        target = groups.get('transclude_target', '')
        target = wikiutil.url_unquote(target)
        desc = groups.get('transclude_desc', '') or ''
        params = groups.get('transclude_params', u'') or u''
        acceptable_attrs_img = ['class', 'title', 'longdesc', 'width', 'height', 'align', ] # no style because of JS
        acceptable_attrs_object = ['class', 'title', 'width', 'height', # no style because of JS
                                  'type', 'standby', ] # we maybe need a hack for <PARAM> here
        m = self.link_target_re.match(target)
        if m:
            if m.group('extern_addr'):
                # currently only supports ext. image inclusion
                target = m.group('extern_addr')
                desc = self._transclude_description(desc, target)
                tag_attrs, query_args = self._get_params(params,
                                                         tag_attrs={'class': 'external_image',
                                                                    'alt': desc,
                                                                    'title': desc, },
                                                         acceptable_attrs=acceptable_attrs_img)
                return self.formatter.image(src=target, **tag_attrs)
                # FF2 has a bug with target mimetype detection, it looks at the url path
                # and expects to find some "filename extension" there (like .png) and this
                # (not the response http headers) will set the default content-type of
                # the object. This will often work for staticly served files, but
                # fails for MoinMoin attachments (they don't have the filename.ext in the
                # path, but in the query string). FF3 seems to have this bug fixed, opera 9.2
                # also works.
                #return (self.formatter.transclusion(1, data=target) +
                #        desc +
                #        self.formatter.transclusion(0))

            elif m.group('attach_scheme'):
                scheme = m.group('attach_scheme')
                url = wikiutil.url_unquote(m.group('attach_addr'))
                if scheme == 'attachment':
                    mt = wikiutil.MimeType(filename=url)
                    if mt.major == 'text':
                        desc = self._transclude_description(desc, url)
                        return self.formatter.attachment_inlined(url, desc)
                    # destinguishs if browser need a plugin in place
                    elif mt.major == 'image' and mt.minor in config.browser_supported_images:
                        desc = self._transclude_description(desc, url)
                        tag_attrs, query_args = self._get_params(params,
                                                                 tag_attrs={'alt': desc,
                                                                            'title': desc, },
                                                                 acceptable_attrs=acceptable_attrs_img)
                        return self.formatter.attachment_image(url, **tag_attrs)
                    else:
                        from MoinMoin.action import AttachFile
                        current_pagename = self.formatter.page.page_name
                        pagename, filename = AttachFile.absoluteName(url, current_pagename)
                        if AttachFile.exists(self.request, pagename, filename):
                            href = AttachFile.getAttachUrl(pagename, filename, self.request)
                            tag_attrs, query_args = self._get_params(params,
                                                                     tag_attrs={'title': desc, },
                                                                     acceptable_attrs=acceptable_attrs_object)
                            return (self.formatter.transclusion(1, data=href, type=mt.spoil(), **tag_attrs) +
                                    self.formatter.text(self._transclude_description(desc, url)) +
                                    self.formatter.transclusion(0))
                        else:
                            return (self.formatter.attachment_link(1, url) +
                                    self.formatter.text(self._transclude_description(desc, url)) +
                                    self.formatter.attachment_link(0))

                        #NOT USED CURRENTLY:

                        # use EmbedObject for other mimetypes
                        if mt is not None:
                            from MoinMoin import macro
                            macro.request = self.request
                            macro.formatter = self.request.html_formatter
                            p = Parser("##\n", request)
                            m = macro.Macro(p)
                            pagename = self.formatter.page.page_name
                            return m.execute('EmbedObject', u'target=%s' % url)
                elif scheme == 'drawing':
                    url = wikiutil.drawing2fname(url)
                    desc = self._transclude_description(desc, url)
                    if desc:
                        tag_attrs= {'alt': desc, 'title': desc, }
                    else:
                        tag_attrs = {}
                    tag_attrs, query_args = self._get_params(params,
                                                             tag_attrs=tag_attrs,
                                                             acceptable_attrs=acceptable_attrs_img)
                    return self.formatter.attachment_drawing(url, desc, **tag_attrs)

            elif m.group('page_name'):
                # experimental client side transclusion
                page_name_all = m.group('page_name')
                if ':' in page_name_all:
                    wiki_name, page_name = page_name_all.split(':', 1)
                    wikitag, wikiurl, wikitail, err = wikiutil.resolve_interwiki(self.request, wiki_name, page_name)
                else:
                    err = True
                if err: # not a interwiki link / not in interwiki map
                    tag_attrs, query_args = self._get_params(params,
                                                             tag_attrs={'type': 'text/html',
                                                                        'width': '100%', },
                                                             acceptable_attrs=acceptable_attrs_object)
                    if 'action' not in query_args:
                        query_args['action'] = 'content'
                    url = Page(self.request, page_name_all).url(self.request, querystr=query_args)
                    return (self.formatter.transclusion(1, data=url, **tag_attrs) +
                            self.formatter.text(self._transclude_description(desc, page_name_all)) +
                            self.formatter.transclusion(0))
                    #return u"Error: <<Include(%s,%s)>> emulation missing..." % (page_name, args)
                else: # looks like a valid interwiki link
                    url = wikiutil.join_wiki(wikiurl, wikitail)
                    tag_attrs, query_args = self._get_params(params,
                                                             tag_attrs={'type': 'text/html',
                                                                        'width': '100%', },
                                                             acceptable_attrs=acceptable_attrs_object)
                    if 'action' not in query_args:
                        query_args['action'] = 'content' # XXX moin specific
                    url += '?%s' % wikiutil.makeQueryString(query_args)
                    return (self.formatter.transclusion(1, data=url, **tag_attrs) +
                            self.formatter.text(self._transclude_description(desc, page_name)) +
                            self.formatter.transclusion(0))
                    #return u"Error: <<RemoteInclude(%s:%s,%s)>> still missing." % (wiki_name, page_name, args)

            else:
                desc = self._transclude_description(desc, target)
                return self.formatter.text('{{%s|%s|%s}}' % (target, desc, params))
        return word +'???'
    _transclude_target_repl = _transclude_repl
    _transclude_desc_repl = _transclude_repl
    _transclude_params_repl = _transclude_repl

    def _link_description(self, desc, target='', default_text=''):
        """ parse a string <desc> valid as link description (text, transclusion, ...)
            and return formatted content.

            @param desc: the link description to parse
            @param default_text: use this text (formatted as text) if parsing
                                 desc returns nothing.
            @param target: target of the link (as readable markup) - used for
                           transcluded image's description
        """
        m = self.link_desc_re.match(desc)
        if m:
            if m.group('simple_text'):
                desc = m.group('simple_text')
                desc = self.formatter.text(desc)
            elif m.group('transclude'):
                groupdict = m.groupdict()
                if groupdict.get('transclude_desc') is None:
                    # if transcluded obj (image) has no description, use target for it
                    groupdict['transclude_desc'] = target
                desc = m.group('transclude')
                desc = self._transclude_repl(desc, groupdict)
        else:
            desc = default_text
            if desc:
                desc = self.formatter.text(desc)
        return desc

    def _link_repl(self, word, groups):
        """Handle [[target|text]] links."""
        target = groups.get('link_target', '')
        desc = groups.get('link_desc', '') or ''
        params = groups.get('link_params', u'') or u''
        acceptable_attrs = ['class', 'title', 'target', 'accesskey', 'rel', ] # no style because of JS
        mt = self.link_target_re.match(target)
        if mt:
            if mt.group('page_name'):
                page_name_and_anchor = mt.group('page_name')
                if ':' in page_name_and_anchor:
                    wiki_name, page_name = page_name_and_anchor.split(':', 1)
                    wikitag, wikiurl, wikitail, err = wikiutil.resolve_interwiki(self.request, wiki_name, page_name)
                else:
                    err = True
                if err: # not a interwiki link / not in interwiki map
                    page_name, anchor = wikiutil.split_anchor(page_name_and_anchor)
                    current_page = self.formatter.page.page_name
                    if not page_name:
                        page_name = current_page
                    # handle relative links
                    abs_page_name = wikiutil.AbsPageName(current_page, page_name)
                    tag_attrs, query_args = self._get_params(params,
                                                             tag_attrs={},
                                                             acceptable_attrs=acceptable_attrs)
                    return (self.formatter.pagelink(1, abs_page_name, anchor=anchor, querystr=query_args, **tag_attrs) +
                            self._link_description(desc, target, page_name_and_anchor) +
                            self.formatter.pagelink(0, abs_page_name))
                else: # interwiki link
                    page_name, anchor = wikiutil.split_anchor(page_name)
                    tag_attrs, query_args = self._get_params(params,
                                                             tag_attrs={},
                                                             acceptable_attrs=acceptable_attrs)
                    return (self.formatter.interwikilink(1, wiki_name, page_name, anchor=anchor, querystr=query_args, **tag_attrs) +
                            self._link_description(desc, target, page_name) +
                            self.formatter.interwikilink(0, wiki_name, page_name))

            elif mt.group('extern_addr'):
                scheme = mt.group('extern_scheme')
                target = mt.group('extern_addr')
                tag_attrs, query_args = self._get_params(params,
                                                         tag_attrs={'class': scheme, },
                                                         acceptable_attrs=acceptable_attrs)
                return (self.formatter.url(1, target, **tag_attrs) +
                        self._link_description(desc, target, target) +
                        self.formatter.url(0))

            elif mt.group('attach_scheme'):
                scheme = mt.group('attach_scheme')
                url = wikiutil.url_unquote(mt.group('attach_addr'))
                tag_attrs, query_args = self._get_params(params,
                                                         tag_attrs={'title': desc, },
                                                         acceptable_attrs=acceptable_attrs)
                if scheme == 'attachment': # ZZZ
                    return (self.formatter.attachment_link(1, url, querystr=query_args, **tag_attrs) +
                            self._link_description(desc, target, url) +
                            self.formatter.attachment_link(0))
                elif scheme == 'drawing':
                    url = wikiutil.drawing2fname(url)
                    return self.formatter.attachment_drawing(url, desc, alt=desc, **tag_attrs)
            else:
                if desc:
                    desc = '|' + desc
                return self.formatter.text('[[%s%s]]' % (target, desc))
    _link_target_repl = _link_repl
    _link_desc_repl = _link_repl
    _link_params_repl = _link_repl

    def _email_repl(self, word, groups):
        """Handle email addresses (without a leading mailto:)."""
        return (self.formatter.url(1, "mailto:%s" % word, css='mailto') +
                self.formatter.text(word) +
                self.formatter.url(0))

    def _sgml_entity_repl(self, word, groups):
        """Handle SGML entities."""
        return self.formatter.text(word)

    def _entity_repl(self, word, groups):
        """Handle numeric (decimal and hexadecimal) and symbolic SGML entities."""
        return self.formatter.rawHTML(word)

    def _indent_repl(self, match, groups):
        """Handle pure indentation (no - * 1. markup)."""
        result = []
        if not (self.in_li or self.in_dd):
            self._close_item(result)
            self.in_li = 1
            css_class = None
            if self.line_was_empty and not self.first_list_item:
                css_class = 'gap'
            result.append(self.formatter.listitem(1, css_class=css_class, style="list-style-type:none"))
        return ''.join(result)

    def _li_none_repl(self, match, groups):
        """Handle type=none (" .") lists."""
        result = []
        self._close_item(result)
        self.in_li = 1
        css_class = None
        if self.line_was_empty and not self.first_list_item:
            css_class = 'gap'
        result.append(self.formatter.listitem(1, css_class=css_class, style="list-style-type:none"))
        return ''.join(result)

    def _li_repl(self, match, groups):
        """Handle bullet (" *") lists."""
        result = []
        self._close_item(result)
        self.in_li = 1
        css_class = None
        if self.line_was_empty and not self.first_list_item:
            css_class = 'gap'
        result.append(self.formatter.listitem(1, css_class=css_class))
        return ''.join(result)

    def _ol_repl(self, match, groups):
        """Handle numbered lists."""
        return self._li_repl(match, groups)

    def _dl_repl(self, match, groups):
        """Handle definition lists."""
        result = []
        self._close_item(result)
        self.in_dd = 1
        result.extend([
            self.formatter.definition_term(1),
            self.formatter.text(match[1:-3].lstrip(' ')),
            self.formatter.definition_term(0),
            self.formatter.definition_desc(1),
        ])
        return ''.join(result)

    def _indent_level(self):
        """Return current char-wise indent level."""
        return len(self.list_indents) and self.list_indents[-1]

    def _indent_to(self, new_level, list_type, numtype, numstart):
        """Close and open lists."""
        openlist = []   # don't make one out of these two statements!
        closelist = []

        if self._indent_level() != new_level and self.in_table:
            closelist.append(self.formatter.table(0))
            self.in_table = 0

        while self._indent_level() > new_level:
            self._close_item(closelist)
            if self.list_types[-1] == 'ol':
                tag = self.formatter.number_list(0)
            elif self.list_types[-1] == 'dl':
                tag = self.formatter.definition_list(0)
            else:
                tag = self.formatter.bullet_list(0)
            closelist.append(tag)

            del self.list_indents[-1]
            del self.list_types[-1]

            if self.list_types: # we are still in a list
                if self.list_types[-1] == 'dl':
                    self.in_dd = 1
                else:
                    self.in_li = 1

        # Open new list, if necessary
        if self._indent_level() < new_level:
            self.list_indents.append(new_level)
            self.list_types.append(list_type)

            if self.formatter.in_p:
                closelist.append(self.formatter.paragraph(0))

            if list_type == 'ol':
                tag = self.formatter.number_list(1, numtype, numstart)
            elif list_type == 'dl':
                tag = self.formatter.definition_list(1)
            else:
                tag = self.formatter.bullet_list(1)
            openlist.append(tag)

            self.first_list_item = 1
            self.in_li = 0
            self.in_dd = 0

        # If list level changes, close an open table
        if self.in_table and (openlist or closelist):
            closelist[0:0] = [self.formatter.table(0)]
            self.in_table = 0

        self.in_list = self.list_types != []
        return ''.join(closelist) + ''.join(openlist)

    def _undent(self):
        """Close all open lists."""
        result = []
        #result.append("<!-- _undent start -->\n")
        self._close_item(result)
        for type in self.list_types[::-1]:
            if type == 'ol':
                result.append(self.formatter.number_list(0))
            elif type == 'dl':
                result.append(self.formatter.definition_list(0))
            else:
                result.append(self.formatter.bullet_list(0))
        #result.append("<!-- _undent end -->\n")
        self.list_indents = []
        self.list_types = []
        return ''.join(result)

    def _getTableAttrs(self, attrdef):
        attr_rule = r'^(\|\|)*<(?!<)(?P<attrs>[^>]*?)>'
        m = re.match(attr_rule, attrdef, re.U)
        if not m:
            return {}, ''
        attrdef = m.group('attrs')

        # extension for special table markup
        def table_extension(key, parser, attrs, wiki_parser=self):
            """ returns: tuple (found_flag, msg)
                found_flag: whether we found something and were able to process it here
                  true for special stuff like 100% or - or #AABBCC
                  false for style xxx="yyy" attributes
                msg: "" or an error msg
            """
            _ = wiki_parser._
            found = False
            msg = ''
            if key[0] in "0123456789":
                token = parser.get_token()
                if token != '%':
                    wanted = '%'
                    msg = _('Expected "%(wanted)s" after "%(key)s", got "%(token)s"') % {
                        'wanted': wanted, 'key': key, 'token': token}
                else:
                    try:
                        dummy = int(key)
                    except ValueError:
                        msg = _('Expected an integer "%(key)s" before "%(token)s"') % {
                            'key': key, 'token': token}
                    else:
                        found = True
                        attrs['width'] = '"%s%%"' % key
            elif key == '-':
                arg = parser.get_token()
                try:
                    dummy = int(arg)
                except ValueError:
                    msg = _('Expected an integer "%(arg)s" after "%(key)s"') % {
                        'arg': arg, 'key': key}
                else:
                    found = True
                    attrs['colspan'] = '"%s"' % arg
            elif key == '|':
                arg = parser.get_token()
                try:
                    dummy = int(arg)
                except ValueError:
                    msg = _('Expected an integer "%(arg)s" after "%(key)s"') % {
                        'arg': arg, 'key': key}
                else:
                    found = True
                    attrs['rowspan'] = '"%s"' % arg
            elif key == '(':
                found = True
                attrs['align'] = '"left"'
            elif key == ':':
                found = True
                attrs['align'] = '"center"'
            elif key == ')':
                found = True
                attrs['align'] = '"right"'
            elif key == '^':
                found = True
                attrs['valign'] = '"top"'
            elif key == 'v':
                found = True
                attrs['valign'] = '"bottom"'
            elif key == '#':
                arg = parser.get_token()
                try:
                    if len(arg) != 6:
                        raise ValueError
                    dummy = int(arg, 16)
                except ValueError:
                    msg = _('Expected a color value "%(arg)s" after "%(key)s"') % {
                        'arg': arg, 'key': key}
                else:
                    found = True
                    attrs['bgcolor'] = '"#%s"' % arg
            return found, self.formatter.rawHTML(msg)

        # scan attributes
        attr, msg = wikiutil.parseAttributes(self.request, attrdef, '>', table_extension)
        if msg:
            msg = '<strong class="highlight">%s</strong>' % msg
        #logging.debug("parseAttributes returned %r" % attr)
        return attr, msg

    def _tableZ_repl(self, word, groups):
        """Handle table row end."""
        if self.in_table:
            result = ''
            # REMOVED: check for self.in_li, p should always close
            if self.formatter.in_p:
                result = self.formatter.paragraph(0)
            result += self.formatter.table_cell(0) + self.formatter.table_row(0)
            return result
        else:
            return self.formatter.text(word)

    def _table_repl(self, word, groups):
        """Handle table cell separator."""
        if self.in_table:
            result = []
            # check for attributes
            attrs, attrerr = self._getTableAttrs(word)

            # start the table row?
            if self.table_rowstart:
                self.table_rowstart = 0
                result.append(self.formatter.table_row(1, attrs))
            else:
                # Close table cell, first closing open p
                # REMOVED check for self.in_li, paragraph should close always!
                if self.formatter.in_p:
                    result.append(self.formatter.paragraph(0))
                result.append(self.formatter.table_cell(0))

            # check for adjacent cell markers
            if word.count("|") > 2:
                if 'align' not in attrs and \
                   not ('style' in attrs and 'text-align' in attrs['style'].lower()):
                    # add center alignment if we don't have some alignment already
                    attrs['align'] = '"center"'
                if 'colspan' not in attrs:
                    attrs['colspan'] = '"%d"' % (word.count("|")/2)

            # return the complete cell markup
            result.append(self.formatter.table_cell(1, attrs) + attrerr)
            result.append(self._line_anchordef())
            return ''.join(result)
        else:
            return self.formatter.text(word)

    def _heading_repl(self, word, groups):
        """Handle section headings."""
        heading_text = groups.get('heading_text', '')
        depth = min(len(groups.get('hmarker')), 5)
        return ''.join([
            self._closeP(),
            self.formatter.heading(1, depth, id=heading_text),
            self.formatter.text(heading_text),
            self.formatter.heading(0, depth, id=heading_text),
        ])
    _heading_text_repl = _heading_repl

    def _parser_repl(self, word, groups):
        """Handle parsed code displays."""
        self.parser = None
        self.parser_name = None
        self.parser_lines = []
        parser_line = word = groups.get('parser_line', u'')
        parser_name = groups.get('parser_name', None)
        parser_args = groups.get('parser_args', None)
        parser_nothing = groups.get('parser_nothing', None)
        parser_unique = groups.get('parser_unique', u'') or u''
        #logging.debug("_parser_repl: parser_name %r parser_args %r parser_unique %r" % (parser_name, parser_args, parser_unique))
        if set(parser_unique) == set('{'): # just some more {{{{{{
            parser_unique = u'}' * len(parser_unique) # for symmetry cosmetic reasons
        self.parser_unique = parser_unique
        if parser_name is not None:
            # First try to find a parser for this
            if parser_name == u'':
                # empty bang paths lead to a normal code display
                # can be used to escape real, non-empty bang paths
                #logging.debug("_parser_repl: empty bangpath")
                parser_name = 'text'
                word = ''
        elif parser_nothing is None:
            # there was something non-whitespace following the {{{
            parser_name = 'text'

        self.setParser(parser_name)
        if not self.parser and parser_name:
            # loading the desired parser didn't work, retry a safe option:
            wanted_parser = parser_name
            parser_name = 'text'
            self.setParser(parser_name)
            word = '%s %s (-)' % (wanted_parser, parser_args)  # indication that it did not work

        if self.parser:
            self.parser_name = parser_name
            self.in_pre = 'found_parser'
            if word:
                self.parser_lines.append(word)
        else:
            self.in_pre = 'search_parser'

        #logging.debug("_parser_repl: in_pre %r line %d" % (self.in_pre, self.lineno))
        return ''
    _parser_unique_repl = _parser_repl
    _parser_line_repl = _parser_repl
    _parser_name_repl = _parser_repl
    _parser_args_repl = _parser_repl
    _parser_nothing_repl = _parser_repl

    def _parser_content(self, line):
        """ handle state and collecting lines for parser in pre/parser sections """
        #logging.debug("parser_content: %r" % line)
        if self.in_pre == 'search_parser' and line.strip():
            # try to find a parser specification
            parser_name = ''
            if line.strip().startswith("#!"):
                parser_name = line.strip()[2:]
            if parser_name:
                parser_name = parser_name.split()[0]
            else:
                parser_name = 'text'
            self.setParser(parser_name)

            if not self.parser:
                parser_name = 'text'
                self.setParser(parser_name)

            if self.parser:
                self.in_pre = 'found_parser'
                self.parser_lines.append(line)
                self.parser_name = parser_name

        elif self.in_pre == 'found_parser':
            # collect the content lines
            self.parser_lines.append(line)

        return ''  # we emit the content after reaching the end of the parser/pre section

    def _parser_end_repl(self, word, groups):
        """ when we reach the end of a parser/pre section,
            we call the parser with the lines we collected
        """
        #if self.in_pre:
        self.in_pre = None
        self.inhibit_p = 0
        #logging.debug("_parser_end_repl: in_pre %r line %d" % (self.in_pre, self.lineno))
        self.request.write(self._closeP())
        if self.parser_name is None:
            # we obviously did not find a parser specification
            self.parser_name = 'text'
        result = self.formatter.parser(self.parser_name, self.parser_lines)
        del self.parser_lines
        self.in_pre = None
        self.parser = None
        return result

    def _smiley_repl(self, word, groups):
        """Handle smileys."""
        return self.formatter.smiley(word)

    def _comment_repl(self, word, groups):
        # if we are in a paragraph, we must close it so that normal text following
        # in the line below the comment will reopen a new paragraph.
        if self.formatter.in_p:
            self.formatter.paragraph(0)
        self.line_is_empty = 1 # markup following comment lines treats them as if they were empty
        return self.formatter.comment(word)

    def _closeP(self):
        if self.formatter.in_p:
            return self.formatter.paragraph(0)
        return ''

    def _macro_repl(self, word, groups):
        """Handle macros."""
        macro_name = groups.get('macro_name')
        macro_args = groups.get('macro_args')
        self.inhibit_p = 0 # 1 fixed macros like UserPreferences (in the past, gone now), 0 fixes paragraph formatting for macros

        # create macro instance
        if self.macro is None:
            self.macro = macro.Macro(self)
        return self.formatter.macro(self.macro, macro_name, macro_args, markup=groups.get('macro'))
    _macro_name_repl = _macro_repl
    _macro_args_repl = _macro_repl

    def scan(self, line, inhibit_p=False):
        """ Scans one line
        Append text before match, invoke replace() with match, and add text after match.
        """
        result = []
        lastpos = 0 # absolute position within line
        line_length = len(line)

        ###result.append(u'<span class="info">[scan: <tt>"%s"</tt>]</span>' % line)
        while lastpos <= line_length: # it is <=, not <, because we need to process the empty line also
            parser_scan_re = re.compile(self.parser_scan_rule % re.escape(self.parser_unique), re.VERBOSE|re.UNICODE)
            scan_re = self.in_pre and parser_scan_re or self.scan_re
            match = scan_re.search(line, lastpos)
            if match:
                start = match.start()
                if lastpos < start:
                    if self.in_pre:
                        self._parser_content(line[lastpos:start])
                    else:
                        ###result.append(u'<span class="info">[add text before match: <tt>"%s"</tt>]</span>' % line[lastpos:match.start()])
                        if not (inhibit_p or self.inhibit_p or self.in_pre or self.formatter.in_p):
                            result.append(self.formatter.paragraph(1, css_class="line862"))
                        # add the simple text in between lastpos and beginning of current match
                        result.append(self.formatter.text(line[lastpos:start]))

                # Replace match with markup
                if not (inhibit_p or self.inhibit_p or self.in_pre or self.formatter.in_p or
                        self.in_table or self.in_list):
                    result.append(self.formatter.paragraph(1, css_class="line867"))
                result.append(self.replace(match, inhibit_p))
                end = match.end()
                lastpos = end
                if start == end:
                    # we matched an empty string
                    lastpos += 1 # proceed, we don't want to match this again
            else:
                if self.in_pre:
                    # ilastpos is more then 0 and result of line slice is empty make useless line
                    if not (lastpos > 0 and line[lastpos:] == ''):
                        self._parser_content(line[lastpos:])
                elif line[lastpos:]:
                    ###result.append('<span class="info">[no match, add rest: <tt>"%s"<tt>]</span>' % line[lastpos:])
                    if not (inhibit_p or self.inhibit_p or self.in_pre or self.formatter.in_p or
                            self.in_li or self.in_dd):
                        result.append(self.formatter.paragraph(1, css_class="line874"))
                    # add the simple text (no markup) after last match
                    result.append(self.formatter.text(line[lastpos:]))
                break # nothing left to do!
        return u''.join(result)

    def _replace(self, match):
        """ Same as replace() but with no magic """
        for name, text in match.groupdict().iteritems():
            if text is not None:
                # Get replace method and replace text
                replace_func = getattr(self, '_%s_repl' % name)
                result = replace_func(text, match.groupdict())
                return result

    def replace(self, match, inhibit_p=False):
        """ Replace match using type name """
        result = []
        for type, hit in match.groupdict().items():
            if hit is not None and not type in ["hmarker", ]:

                ##result.append(u'<span class="info">[replace: %s: "%s"]</span>' % (type, hit))
                # Open p for certain types
                if not (inhibit_p or self.inhibit_p or self.formatter.in_p
                        or self.in_pre or (type in self.no_new_p_before)):
                    result.append(self.formatter.paragraph(1, css_class="line891"))

                # Get replace method and replace hit
                replace_func = getattr(self, '_%s_repl' % type)
                result.append(replace_func(hit, match.groupdict()))
                return ''.join(result)
        else:
            # We should never get here
            import pprint
            raise Exception("Can't handle match %r\n%s\n%s" % (
                match,
                pprint.pformat(match.groupdict()),
                pprint.pformat(match.groups()),
            ))

        return ""

    def _line_anchordef(self):
        if self.line_anchors and not self.line_anchor_printed:
            self.line_anchor_printed = 1
            return self.formatter.line_anchordef(self.lineno)
        else:
            return ''

    def format(self, formatter, inhibit_p=False):
        """ For each line, scan through looking for magic
            strings, outputting verbatim any intervening text.
        """
        self.formatter = formatter
        self.hilite_re = self.formatter.page.hilite_re

        # get text and replace TABs
        rawtext = self.raw.expandtabs()

        # go through the lines

        for lineno in range(1, self.start_line + 1):
            self.request.write(self.formatter.line_anchordef(lineno))

        self.lineno = self.start_line
        self.lines = self.eol_re.split(rawtext)
        self.line_is_empty = 0

        self.in_processing_instructions = 1

        if self.wrapping_div_class:
            self.request.write(self.formatter.div(1, css_class=self.wrapping_div_class))

        # Main loop
        for line in self.lines:
            self.lineno += 1

            self.line_anchor_printed = 0
            if not self.in_table:
                self.request.write(self._line_anchordef())
            self.table_rowstart = 1
            self.line_was_empty = self.line_is_empty
            self.line_is_empty = 0
            self.first_list_item = 0
            self.inhibit_p = 0

            # ignore processing instructions
            if self.in_processing_instructions:
                found = False
                for pi in ("##", "#format", "#refresh", "#redirect", "#deprecated",
                           "#pragma", "#form", "#acl", "#language"):
                    if line.lower().startswith(pi):
                        self.request.write(self.formatter.comment(line))
                        found = True
                        break
                if not found:
                    self.in_processing_instructions = 0
                else:
                    continue # do not parse this line

            if not self.in_pre:
                # we don't have \n as whitespace any more
                # This is the space between lines we join to one paragraph
                line += ' '

                # Paragraph break on empty lines
                if not line.strip():
                    if self.in_table:
                        self.request.write(self.formatter.table(0))
                        self.request.write(self._line_anchordef())
                        self.in_table = 0
                    # CHANGE: removed check for not self.list_types
                    # p should close on every empty line
                    if self.formatter.in_p:
                        self.request.write(self.formatter.paragraph(0))
                    self.line_is_empty = 1
                    continue

                # Check indent level
                indent = self.indent_re.match(line)
                indlen = len(indent.group(0))
                indtype = "ul"
                numtype = None
                numstart = None
                if indlen:
                    match = self.ol_re.match(line)
                    if match:
                        numtype, numstart = match.group(0).strip().split('.')
                        numtype = numtype[0]

                        if numstart and numstart[0] == "#":
                            numstart = int(numstart[1:])
                        else:
                            numstart = None

                        indtype = "ol"
                    else:
                        match = self.dl_re.match(line)
                        if match:
                            indtype = "dl"

                # output proper indentation tags
                self.request.write(self._indent_to(indlen, indtype, numtype, numstart))

                # Table mode
                # TODO: move into function?
                if (not self.in_table and line[indlen:indlen + 2] == "||"
                    and line.endswith("|| ") and len(line) >= 5 + indlen):
                    # Start table
                    if self.list_types and not self.in_li:
                        self.request.write(self.formatter.listitem(1, style="list-style-type:none"))
                        ## CHANGE: no automatic p on li
                        ##self.request.write(self.formatter.paragraph(1))
                        self.in_li = 1

                    # CHANGE: removed check for self.in_li
                    # paragraph should end before table, always!
                    if self.formatter.in_p:
                        self.request.write(self.formatter.paragraph(0))
                    attrs, attrerr = self._getTableAttrs(line[indlen+2:])
                    self.request.write(self.formatter.table(1, attrs) + attrerr)
                    self.in_table = True # self.lineno
                elif (self.in_table and not
                      # intra-table comments should not break a table
                      (line.startswith("##") or
                       line[indlen:indlen + 2] == "||" and
                       line.endswith("|| ") and
                       len(line) >= 5 + indlen)):

                    # Close table
                    self.request.write(self.formatter.table(0))
                    self.request.write(self._line_anchordef())
                    self.in_table = 0

            # Scan line, format and write
            formatted_line = self.scan(line, inhibit_p=inhibit_p)
            self.request.write(formatted_line)


        # Close code displays, paragraphs, tables and open lists
        self.request.write(self._undent())
        if self.in_pre: self.request.write(self.formatter.preformatted(0))
        if self.formatter.in_p: self.request.write(self.formatter.paragraph(0))
        if self.in_table: self.request.write(self.formatter.table(0))

        if self.wrapping_div_class:
            self.request.write(self.formatter.div(0))


    # Private helpers ------------------------------------------------------------

    def setParser(self, name):
        """ Set parser to parser named 'name' """
        # XXX this is done by the formatter as well
        try:
            self.parser = wikiutil.searchAndImportPlugin(self.request.cfg, "parser", name)
        except wikiutil.PluginMissingError:
            self.parser = None

del _

# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - TableOfContents Macro

    The macro works as follows: First, it renders the page using
    the TOCFormatter (below) to get access to the outline of the
    page. During the page rendering, only macros whose
    'generates_headings' property is set and True are rendered,
    most macros don't generate any headings and thus need not be
    executed speeding up the process considerably.

    The generated outline is then written to the output.

    However, this is not all. Consider included pages that include
    a TOC themselves! First of all, TOCs don't generate headings
    so we avoid recursion during the collection process. Secondly,
    we always keep track of which content we are in and the
    formatter's heading method is responsible for making all
    IDs they generate unique. We use the same algorithm to make
    the IDs unique during the TOCFormatter rendering step so that
    in the end we can output the same IDs and the TOC is linked
    correctly, even in the case of multiple nested inclusions.

    @copyright: 2007 MoinMoin:JohannesBerg
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.formatter import FormatterBase
from MoinMoin.Page import Page
from MoinMoin import wikiutil


# cannot be cached because of TOCs in included pages
Dependencies = ['time']

class TOCFormatter(FormatterBase):
    def __init__(self, request, **kw):
        FormatterBase.__init__(self, request, **kw)
        self.in_heading = False
        self.collected_headings = request._tocfm_collected_headings

    def _text(self, text):
        if self.in_heading:
            self.collected_headings[-1][2] += text
        return text

    def startContent(self, *args, **kw):
        res = FormatterBase.startContent(self, *args, **kw)
        self.collected_headings.append([1, self.request.uid_generator.include_id, None])
        return res

    def endContent(self):
        res = FormatterBase.endContent(self)
        self.collected_headings.append([0, self.request.uid_generator.include_id, None])
        return res

    def heading(self, on, depth, **kw):
        id = kw.get('id', None)
        self.in_heading = on
        if not id is None:
            id = self.request._tocfm_orig_formatter.make_id_unique(id)
        if on:
            self.collected_headings.append([depth, id, u''])
        return ''

    def macro(self, macro_obj, name, args, markup=None):
        try:
            # plugins that are defined in the macro class itself
            # can't generate headings this way, but that's fine
            gen_headings = wikiutil.importPlugin(self.request.cfg, 'macro',
                                                 name, 'generates_headings')
            if gen_headings:
                return FormatterBase.macro(self, macro_obj, name, args, markup)
        except (wikiutil.PluginMissingError, wikiutil.PluginAttributeError):
            pass
        return ''

    def _anything_return_empty(self, *args, **kw):
        return ''

    lang = _anything_return_empty
    sysmsg = _anything_return_empty
    startDocument = _anything_return_empty
    endDocument = _anything_return_empty
    pagelink = _anything_return_empty
    interwikilink = _anything_return_empty
    url = _anything_return_empty
    attachment_link = _anything_return_empty
    attachment_image = _anything_return_empty
    attachment_drawing = _anything_return_empty
    attachment_inlined = _anything_return_empty
    anchordef = _anything_return_empty
    line_anchordef = _anything_return_empty
    anchorlink = _anything_return_empty
    line_anchorlink = _anything_return_empty
    image = _anything_return_empty
    smiley = _anything_return_empty
    nowikiword = _anything_return_empty
    strong = _anything_return_empty
    emphasis = _anything_return_empty
    underline = _anything_return_empty
    highlight = _anything_return_empty
    sup = _anything_return_empty
    sub = _anything_return_empty
    strike = _anything_return_empty
    code = _anything_return_empty
    preformatted = _anything_return_empty
    small = _anything_return_empty
    big = _anything_return_empty
    code_area = _anything_return_empty
    code_line = _anything_return_empty
    code_token = _anything_return_empty
    linebreak = _anything_return_empty
    paragraph = _anything_return_empty
    rule = _anything_return_empty
    icon = _anything_return_empty
    number_list = _anything_return_empty
    bullet_list = _anything_return_empty
    listitem = _anything_return_empty
    definition_list = _anything_return_empty
    definition_term = _anything_return_empty
    definition_desc = _anything_return_empty
    table = _anything_return_empty
    table_row = _anything_return_empty
    table_cell = _anything_return_empty
    _get_bang_args = _anything_return_empty
    parser = _anything_return_empty
    div = _anything_return_empty
    span = _anything_return_empty
    escapedText = _anything_return_empty
    comment = _anything_return_empty
    transclusion = _anything_return_empty

def macro_TableOfContents(macro, maxdepth=int):
    """
Prints a table of contents.

 maxdepth:: maximum depth the table of contents is generated for (defaults to unlimited)
    """
    try:
        mindepth = int(macro.request.getPragma('section-numbers', 1))
    except (ValueError, TypeError):
        mindepth = 1

    if maxdepth is None:
        maxdepth = 99

    pname = macro.formatter.page.page_name

    macro.request.uid_generator.push()

    macro.request._tocfm_collected_headings = []
    macro.request._tocfm_orig_formatter = macro.formatter

    tocfm = TOCFormatter(macro.request)
    p = Page(macro.request, pname, formatter=tocfm, rev=macro.request.rev)

    # this is so we get a correctly updated TOC if we just preview in the editor -
    # the new content is not stored on disk yet, but available as macro.parser.raw:
    p.set_raw_body(macro.parser.raw, modified=1)

    output = macro.request.redirectedOutput(p.send_page,
                                            content_only=True,
                                            count_hit=False,
                                            omit_footnotes=True)

    _ = macro.request.getText

    result = [
        macro.formatter.div(1, css_class="table-of-contents"),
        macro.formatter.paragraph(1, css_class="table-of-contents-heading"),
        macro.formatter.text(_('Contents')),
        macro.formatter.paragraph(0),
    ]


    # find smallest used level and use that as the outer-most indentation,
    # to fix pages like HelpOnMacros that only use h2 and lower levels.
    lastlvl = 100
    for lvl, id, txt in macro.request._tocfm_collected_headings:
        if txt is None:
            incl_id = id
            continue
        if lvl < mindepth or lvl > maxdepth or id is None:
            continue
        if lvl < lastlvl:
            lastlvl = lvl

    # headings are 1-based, lastlvl needs to be one less so that one is closed
    lastlvl -= 1

    for lvl, id, txt in macro.request._tocfm_collected_headings:
        if txt is None:
            incl_id = id
            continue
        if lvl < mindepth or lvl > maxdepth or id is None:
            continue

        # will be reset by pop_unique_ids below
        macro.request.uid_generator.include_id = incl_id

        need_li = lastlvl >= lvl
        while lastlvl > lvl:
            result.extend([
                macro.formatter.listitem(0),
                macro.formatter.number_list(0),
            ])
            lastlvl -= 1
        while lastlvl < lvl:
            result.extend([
                macro.formatter.number_list(1),
                macro.formatter.listitem(1),
            ])
            lastlvl += 1
        if need_li:
            result.extend([
                macro.formatter.listitem(0),
                macro.formatter.listitem(1),
            ])
        result.extend([
            '\n',
            macro.formatter.anchorlink(1, id),
            macro.formatter.text(txt),
            macro.formatter.anchorlink(0),
        ])

    while lastlvl > 0:
        result.append(macro.formatter.listitem(0))
        result.append(macro.formatter.number_list(0))
        lastlvl -= 1

    macro.request.uid_generator.pop()

    result.append(macro.formatter.div(0))
    return ''.join(result)

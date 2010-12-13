# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - FootNote Macro

    Collect and emit footnotes. Note that currently footnote
    text cannot contain wiki markup.

    @copyright: 2002 Juergen Hermann <jh@web.de>,
                2007 MoinMoin:ReimarBauer,
                2007 Johannes Berg
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import config, wikiutil
from MoinMoin.parser.text_moin_wiki import Parser as WikiParser
from MoinMoin.support.python_compatibility import hash_new

Dependencies = ["time"] # footnote macro cannot be cached

def execute(macro, args):
    request = macro.request
    formatter = macro.formatter

    # create storage for footnotes
    if not hasattr(request, 'footnotes'):
        request.footnotes = {}
        request.footnote_ctr = 0
        request.footnote_show_ctr = 0

    if not args:
        return emit_footnotes(request, formatter)
    else:
        # grab new footnote backref number
        idx = request.footnote_ctr
        request.footnote_ctr += 1

        shahex = hash_new('sha1', args.encode(config.charset)).hexdigest()
        backlink_id = "fndef-%s-%d" % (shahex, idx)
        fwdlink_id = "fnref-%s" % shahex

        if not args in request.footnotes:
            showidx = request.footnote_show_ctr
            request.footnote_show_ctr += 1
            request.footnotes[args] = ([], fwdlink_id, showidx)
        flist, dummy, showidx = request.footnotes[args]
        request.footnotes[args] = (flist + [(backlink_id, idx)], fwdlink_id, showidx)

        # do index -> text mapping in the same dict, that's fine because
        # args is always a string and idx alwas a number.
        request.footnotes[idx] = args

        return "%s%s%s%s%s" % (
            formatter.sup(1),
            formatter.anchorlink(1, fwdlink_id, id=backlink_id),
            formatter.text(str(showidx+1)),
            formatter.anchorlink(0),
            formatter.sup(0), )

    # nothing to do or emit
    return ''


def emit_footnotes(request, formatter):
    # emit collected footnotes
    if request.footnotes:
        result = []

        result.append(formatter.div(1, css_class='footnotes'))

        # Add footnotes list
        result.append(formatter.number_list(1))
        subidx = 0
        for ctr in range(request.footnote_ctr):
            fn_txt = request.footnotes[ctr]
            if not fn_txt in request.footnotes:
                continue
            this_txt_footnotes, fwdlink_id, showidx = request.footnotes[fn_txt]
            # this text was handled
            del request.footnotes[fn_txt]

            result.append(formatter.listitem(1))
            result.append(formatter.paragraph(1))
            result.append(formatter.anchorlink(1, None, id=fwdlink_id))
            result.append(formatter.anchorlink(0))
            result.append(wikiutil.renderText(request, WikiParser, fn_txt))

            items = []
            # ToDo check why that loop is needed?
            for backlink_id, idx in this_txt_footnotes:
                # Add item
                item = formatter.anchorlink(1, backlink_id)
                item += formatter.text(str(subidx+1))
                item += formatter.anchorlink(0)
                items.append(item)
                subidx += 1

            result.append(formatter.text(" ("))
            result.append(formatter.text(" ").join(items))
            result.append(formatter.text(")"))

            result.append(formatter.paragraph(0))
            result.append(formatter.listitem(0))

        result.append(formatter.number_list(0))

        # Finish div
        result.append(formatter.div(0))

        del request.footnotes
        del request.footnote_ctr
        del request.footnote_show_ctr
        return ''.join(result)

    return ''


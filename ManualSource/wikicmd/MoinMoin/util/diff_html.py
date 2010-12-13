# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Side by side diffs

    @copyright: 2002 Juergen Hermann <jh@web.de>,
                2002 Scott Moonen <smoonen@andstuff.org>
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.support import difflib
from MoinMoin.wikiutil import escape

def indent(line):
    eol = ''
    while line and line[0] == '\n':
        eol += '\n'
        line = line[1:]
    stripped = line.lstrip()
    if len(line) - len(stripped):
        line = "&nbsp;" * (len(line) - len(stripped)) + stripped
    #return "%d / %d / %s" % (len(line), len(stripped), line)
    return eol + line


# This code originally by Scott Moonen, used with permission.
def diff(request, old, new, old_top='', new_top='', old_bottom='', new_bottom='', old_top_class='', new_top_class='', old_bottom_class='', new_bottom_class=''):
    """ Find changes between old and new and return
        HTML markup visualising them.

        @param old: old text [unicode]
        @param new: new text [unicode]
        @param old_top: Custom html for adding ontop of old revision column (optional)
        @param old_bottom: Custom html for adding at bottom of old revision column (optional)
        @param new_top: Custom html for adding ontop of new revision column (optional)
        @param new_bottom: Custom html for adding at bottom of new revision column (optional)
        @param old_top_class: Custom class for <td> with old_top content (optional)
        @param new_top_class: Custom class for <td> with new_top content (optional)
        @param old_bottom_class: Custom class for <td> with old_bottom content (optional)
        @param new_bottom_class: Custom class for <td> with new_bottom content (optional)
    """
    _ = request.getText
    t_line = _("Line") + " %d"

    seq1 = old.splitlines()
    seq2 = new.splitlines()

    seqobj = difflib.SequenceMatcher(None, seq1, seq2)
    linematch = seqobj.get_matching_blocks()

    result = """
<table class="diff">
"""

    if old_top or new_top:
        result += '<tr><td class="%s">%s</td><td class="%s">%s</td></tr>' % (old_top_class, old_top, new_top_class, new_top)

    if len(seq1) == len(seq2) and linematch[0] == (0, 0, len(seq1)):
        # No differences.
        result += '<tr><td class="diff-same" colspan="2">' + _("No differences found!") + '</td></tr>'
    else:
        result += """
<tr>
<td class="diff-removed"><span>%s</span></td>
<td class="diff-added"><span>%s</span></td>
</tr>
""" % (_('Deletions are marked like this.'), _('Additions are marked like this.'), )

        lastmatch = (0, 0)

        # Print all differences
        for match in linematch:
            # Starts of pages identical?
            if lastmatch == match[0:2]:
                lastmatch = (match[0] + match[2], match[1] + match[2])
                continue
            llineno, rlineno = lastmatch[0]+1, lastmatch[1]+1
            result += """
<tr class="diff-title">
<td>%s:</td>
<td>%s:</td>
</tr>
""" % (request.formatter.line_anchorlink(1, llineno) + request.formatter.text(t_line % llineno) + request.formatter.line_anchorlink(0),
           request.formatter.line_anchorlink(1, rlineno) + request.formatter.text(t_line % rlineno) + request.formatter.line_anchorlink(0))

            leftpane = ''
            rightpane = ''
            linecount = max(match[0] - lastmatch[0], match[1] - lastmatch[1])
            for line in range(linecount):
                if line < match[0] - lastmatch[0]:
                    if line > 0:
                        leftpane += '\n'
                    leftpane += seq1[lastmatch[0] + line]
                if line < match[1] - lastmatch[1]:
                    if line > 0:
                        rightpane += '\n'
                    rightpane += seq2[lastmatch[1] + line]

            charobj = difflib.SequenceMatcher(None, leftpane, rightpane)
            charmatch = charobj.get_matching_blocks()

            if charobj.ratio() < 0.5:
                # Insufficient similarity.
                if leftpane:
                    leftresult = """<span>%s</span>""" % indent(escape(leftpane))
                else:
                    leftresult = ''

                if rightpane:
                    rightresult = """<span>%s</span>""" % indent(escape(rightpane))
                else:
                    rightresult = ''
            else:
                # Some similarities; markup changes.
                charlast = (0, 0)

                leftresult = ''
                rightresult = ''
                for thismatch in charmatch:
                    if thismatch[0] - charlast[0] != 0:
                        leftresult += """<span>%s</span>""" % indent(
                            escape(leftpane[charlast[0]:thismatch[0]]))
                    if thismatch[1] - charlast[1] != 0:
                        rightresult += """<span>%s</span>""" % indent(
                            escape(rightpane[charlast[1]:thismatch[1]]))
                    leftresult += escape(leftpane[thismatch[0]:thismatch[0] + thismatch[2]])
                    rightresult += escape(rightpane[thismatch[1]:thismatch[1] + thismatch[2]])
                    charlast = (thismatch[0] + thismatch[2], thismatch[1] + thismatch[2])

            leftpane = '<br>'.join([indent(x) for x in leftresult.splitlines()])
            rightpane = '<br>'.join([indent(x) for x in rightresult.splitlines()])

            # removed width="50%%"
            result += """
<tr>
<td class="diff-removed">%s</td>
<td class="diff-added">%s</td>
</tr>
""" % (leftpane, rightpane)

            lastmatch = (match[0] + match[2], match[1] + match[2])

    if old_bottom or new_bottom:
        result += '<tr><td class="%s">%s</td><td class="%s">%s</td></tr>' % (old_top_class, old_top, new_top_class, new_top)

    result += '</table>\n'
    return result


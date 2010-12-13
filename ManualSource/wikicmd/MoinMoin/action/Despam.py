# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Despam action

    Mass revert changes done by some specific author / bot.

    @copyright: 2005 by ???, Thomas Waldmann
    @license: GNU GPL, see COPYING for details.
"""

DAYS = 30 # we look for spam edits in the last x days

import time

from MoinMoin import log
logging = log.getLogger(__name__)

from MoinMoin.logfile import editlog
from MoinMoin.util.dataset import TupleDataset, Column
from MoinMoin.widget.browser import DataBrowserWidget
from MoinMoin import wikiutil, Page, PageEditor
from MoinMoin.macro import RecentChanges

def render(editor_tuple):
    etype, evalue = editor_tuple
    if etype in ('ip', 'email', ):
        ret = evalue
    elif etype == 'interwiki':
        ewiki, euser = evalue
        if ewiki == 'Self':
            ret = euser
        else:
            ret = '%s:%s' % evalue
    else:
        ret = repr(editor_tuple)
    return ret

def show_editors(request, pagename, timestamp):
    _ = request.getText

    timestamp = int(timestamp * 1000000)
    log = editlog.EditLog(request)
    editors = {}
    pages = {}
    for line in log.reverse():
        if line.ed_time_usecs < timestamp:
            break

        if not request.user.may.read(line.pagename):
            continue

        editor = line.getInterwikiEditorData(request)
        if not line.pagename in pages:
            pages[line.pagename] = 1
            editors[editor] = editors.get(editor, 0) + 1

    editors = [(nr, editor) for editor, nr in editors.iteritems()]
    editors.sort()
    editors.reverse()

    pg = Page.Page(request, pagename)

    dataset = TupleDataset()
    dataset.columns = [Column('editor', label=_("Editor"), align='left'),
                       Column('pages', label=_("Pages"), align='right'),
                       Column('link', label='', align='left')]
    for nr, editor in editors:
        dataset.addRow((render(editor), unicode(nr),
            pg.link_to(request, text=_("Select Author"),
                querystr={
                    'action': 'Despam',
                    'editor': repr(editor),
                })))

    table = DataBrowserWidget(request)
    table.setData(dataset)
    return table.render(method="GET")

class tmp:
    pass

def show_pages(request, pagename, editor, timestamp):
    _ = request.getText

    timestamp = int(timestamp * 1000000)
    log = editlog.EditLog(request)
    pages = {}
    #  mimic macro object for use of RecentChanges subfunctions
    macro = tmp()
    macro.request = request
    macro.formatter = request.html_formatter

    request.write("<table>")
    for line in log.reverse():
        if line.ed_time_usecs < timestamp:
            break

        if not request.user.may.read(line.pagename):
            continue

        if not line.pagename in pages:
            pages[line.pagename] = 1
            if repr(line.getInterwikiEditorData(request)) == editor:
                line.time_tuple = request.user.getTime(wikiutil.version2timestamp(line.ed_time_usecs))
                request.write(RecentChanges.format_page_edits(macro, [line], timestamp))

    request.write('''
</table>
<p>
<form method="post" action="%(url)s">
<input type="hidden" name="action" value="Despam">
<input type="hidden" name="ticket" value="%(ticket)s">
<input type="hidden" name="editor" value="%(editor)s">
<input type="submit" name="ok" value="%(label)s">
</form>
</p>
''' % dict(
        url=request.href(pagename),
        ticket=wikiutil.createTicket(request),
        editor=wikiutil.url_quote(editor),
        label=_("Revert all!"),
    ))


def revert_page(request, pagename, editor):
    if not request.user.may.revert(pagename):
        return

    log = editlog.EditLog(request, rootpagename=pagename)

    first = True
    rev = u"00000000"
    for line in log.reverse():
        if first:
            first = False
            if repr(line.getInterwikiEditorData(request)) != editor:
                return
        else:
            if repr(line.getInterwikiEditorData(request)) != editor:
                rev = line.rev
                break

    if rev == u"00000000": # page created by spammer
        comment = u"Page deleted by Despam action"
        pg = PageEditor.PageEditor(request, pagename, do_editor_backup=0)
        try:
            savemsg = pg.deletePage(comment)
        except pg.SaveError, msg:
            savemsg = unicode(msg)
    else: # page edited by spammer
        oldpg = Page.Page(request, pagename, rev=int(rev))
        pg = PageEditor.PageEditor(request, pagename, do_editor_backup=0)
        try:
            savemsg = pg.saveText(oldpg.get_raw_body(), 0, extra=rev, action="SAVE/REVERT")
        except pg.SaveError, msg:
            savemsg = unicode(msg)
    return savemsg

def revert_pages(request, editor, timestamp):
    _ = request.getText

    editor = wikiutil.url_unquote(editor)
    timestamp = int(timestamp * 1000000)
    log = editlog.EditLog(request)
    pages = {}
    revertpages = []
    for line in log.reverse():
        if line.ed_time_usecs < timestamp:
            break

        if not request.user.may.read(line.pagename):
            continue

        if not line.pagename in pages:
            pages[line.pagename] = 1
            if repr(line.getInterwikiEditorData(request)) == editor:
                revertpages.append(line.pagename)

    request.write("Pages to revert:<br>%s" % "<br>".join([wikiutil.escape(p) for p in revertpages]))
    for pagename in revertpages:
        request.write("Begin reverting %s ...<br>" % wikiutil.escape(pagename))
        msg = revert_page(request, pagename, editor)
        if msg:
            request.write("<p>%s: %s</p>" % (
                Page.Page(request, pagename).link_to(request), msg))
        request.write("Finished reverting %s.<br>" % wikiutil.escape(pagename))

def execute(pagename, request):
    _ = request.getText
    # check for superuser
    if not request.user.isSuperUser():
        request.theme.add_msg(_('You are not allowed to use this action.'), "error")
        return Page.Page(request, pagename).send_page()

    editor = request.values.get('editor')
    timestamp = time.time() - DAYS * 24 * 3600
    ok = request.form.get('ok', 0)
    logging.debug("editor: %r ok: %r" % (editor, ok))

    request.theme.send_title("Despam", pagename=pagename)
    # Start content (important for RTL support)
    request.write(request.formatter.startContent("content"))

    if (request.method == 'POST' and ok and
        wikiutil.checkTicket(request, request.form.get('ticket', ''))):
        revert_pages(request, editor, timestamp)
    elif editor:
        show_pages(request, pagename, editor, timestamp)
    else:
        request.write(show_editors(request, pagename, timestamp))

    # End content and send footer
    request.write(request.formatter.endContent())
    request.theme.send_footer(pagename)
    request.theme.send_closing_html()


# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - edit a page

    This either calls the text or the GUI page editor.

    @copyright: 2000-2004 Juergen Hermann <jh@web.de>,
                2006 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""
from MoinMoin import wikiutil
from MoinMoin.Page import Page
from MoinMoin.web.utils import check_surge_protect

def execute(pagename, request):
    """ edit a page """
    _ = request.getText

    if 'button_preview' in request.form and 'button_spellcheck' in request.form:
        # multiple buttons pressed at once? must be some spammer/bot
        check_surge_protect(request, kick=True) # get rid of him
        return

    if not request.user.may.write(pagename):
        page = wikiutil.getLocalizedPage(request, 'PermissionDeniedPage')
        page.body = _('You are not allowed to edit this page.')
        page.page_name = pagename
        page.send_page(send_special=True)
        return

    valideditors = ['text', 'gui', ]
    editor = ''
    if request.user.valid:
        editor = request.user.editor_default
    if editor not in valideditors:
        editor = request.cfg.editor_default

    editorparam = request.values.get('editor', editor)
    if editorparam == "guipossible":
        lasteditor = editor
    elif editorparam == "textonly":
        editor = lasteditor = 'text'
    else:
        editor = lasteditor = editorparam

    if request.cfg.editor_force:
        editor = request.cfg.editor_default

    # if it is still nothing valid, we just use the text editor
    if editor not in valideditors:
        editor = 'text'

    rev = request.rev or 0
    savetext = request.form.get('savetext')
    comment = request.form.get('comment', u'')
    category = request.form.get('category')
    rstrip = int(request.form.get('rstrip', '0'))
    trivial = int(request.form.get('trivial', '0'))

    if 'button_switch' in request.form:
        if editor == 'text':
            editor = 'gui'
        else: # 'gui'
            editor = 'text'

    # load right editor class
    if editor == 'gui':
        from MoinMoin.PageGraphicalEditor import PageGraphicalEditor
        pg = PageGraphicalEditor(request, pagename)
    else: # 'text'
        from MoinMoin.PageEditor import PageEditor
        pg = PageEditor(request, pagename)

    # is invoked without savetext start editing
    if savetext is None or 'button_load_draft' in request.form:
        pg.sendEditor()
        return

    # did user hit cancel button?
    cancelled = 'button_cancel' in request.form

    from MoinMoin.error import ConvertError
    try:
        if lasteditor == 'gui':
            # convert input from Graphical editor
            format = request.form.get('format', 'wiki')
            if format == 'wiki':
                converter_name = 'text_html_text_moin_wiki'
            else:
                converter_name = 'undefined' # XXX we don't have other converters yet
            convert = wikiutil.importPlugin(request.cfg, "converter", converter_name, 'convert')
            savetext = convert(request, pagename, savetext)

        # IMPORTANT: normalize text from the form. This should be done in
        # one place before we manipulate the text.
        savetext = pg.normalizeText(savetext, stripspaces=rstrip)
    except ConvertError:
        # we don't want to throw an exception if user cancelled anyway
        if not cancelled:
            raise

    if cancelled:
        pg.sendCancel(savetext or "", rev)
        pagedir = pg.getPagePath(check_create=0)
        import os
        if not os.listdir(pagedir):
            os.removedirs(pagedir)
        return

    comment = wikiutil.clean_input(comment)

    # Add category

    # TODO: this code does not work with extended links, and is doing
    # things behind your back, and in general not needed. Either we have
    # a full interface for categories (add, delete) or just add them by
    # markup.

    if category and category != _('<No addition>'): # opera 8.5 needs this
        # strip trailing whitespace
        savetext = savetext.rstrip()

        # Add category separator if last non-empty line contains
        # non-categories.
        lines = [line for line in savetext.splitlines() if line]
        if lines:

            #TODO: this code is broken, will not work for extended links
            #categories, e.g ["category hebrew"]
            categories = lines[-1].split()

            if categories:
                confirmed = wikiutil.filterCategoryPages(request, categories)
                if len(confirmed) < len(categories):
                    # This was not a categories line, add separator
                    savetext += u'\n----\n'

        # Add new category
        if savetext and savetext[-1] != u'\n':
            savetext += ' '
        savetext += category + u'\n' # Should end with newline!

    if (request.cfg.edit_ticketing and
        not wikiutil.checkTicket(request, request.form.get('ticket', ''))):
        request.theme.add_msg(_('Please use the interactive user interface to use action %(actionname)s!') % {'actionname': 'edit' }, "error")
        pg.sendEditor(preview=savetext, comment=comment, staytop=1)

    # Preview, spellcheck or spellcheck add new words
    elif ('button_preview' in request.form or
        'button_spellcheck' in request.form or
        'button_newwords' in request.form):
        pg.sendEditor(preview=savetext, comment=comment)

    # Preview with mode switch
    elif 'button_switch' in request.form:
        pg.sendEditor(preview=savetext, comment=comment, staytop=1)

    # Save new text
    else:
        try:
            from MoinMoin.security.textcha import TextCha
            if not TextCha(request).check_answer_from_form():
                raise pg.SaveError(_('TextCha: Wrong answer! Go back and try again...'))
            savemsg = pg.saveText(savetext, rev, trivial=trivial, comment=comment)
        except pg.EditConflict, e:
            msg = e.message

            # Handle conflict and send editor
            pg.set_raw_body(savetext, modified=1)

            pg.mergeEditConflict(rev)
            # We don't send preview when we do merge conflict
            pg.sendEditor(msg=msg, comment=comment)
            return

        except pg.SaveError, msg:
            # msg contains a unicode string
            savemsg = unicode(msg)

        # Send new page after save or after unsuccessful conflict merge.
        request.reset()
        pg = Page(request, pagename)

        # sets revision number to default for further actions
        request.rev = 0
        request.theme.add_msg(savemsg, "info")
        pg.send_page()

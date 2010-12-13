# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - language_setup

    The superuser gets a table of language packages listed dependent on the selected language
    for installation.

    @copyright: 2009 MoinMoin:ReimarBauer,
                     MoinMoin:ThomasWaldmnann

    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import i18n, packages, wikiutil
from MoinMoin.i18n import strings
i18n.strings = strings

from MoinMoin.action import AttachFile
from MoinMoin.util.dataset import TupleDataset, Column
from MoinMoin.widget.browser import DataBrowserWidget

def execute(pagename, request):
    _ = request.getText
    if not request.user or not request.user.isSuperUser():
        msg = _('Only superuser is allowed to use this action.')
        request.theme.add_msg(msg, "error")
        request.page.send_page()
        return ''
    fmt = request.html_formatter
    language_setup_page = 'LanguageSetup'
    not_translated_system_pages = 'not_translated_system_pages.zip'
    files = AttachFile._get_files(request, language_setup_page)
    if not files:
        msg = _('No page packages found.')
        request.theme.add_msg(msg, "error")
        request.page.send_page()
        return ''
    wiki_languages = list(set([lang_file.split('--')[0] for lang_file in files]) - set(['00_needs_fixing.zip']))
    wiki_languages.sort()

    lang = request.values.get('language') or 'English'
    target = request.values.get('target') or ''
    msg = ''
    # if target is given it tries to install the package.
    if target:
        dummy_pagename, dummy_target, targetpath = AttachFile._access_file(language_setup_page, request)
        package = packages.ZipPackage(request, targetpath)
        if package.isPackage():
            if package.installPackage():
                msg = _("Attachment '%(filename)s' installed.") % {'filename': target}
            else:
                msg = _("Installation of '%(filename)s' failed.") % {'filename': target}
        else:
            msg = _('The file %s is not a MoinMoin package file.') % target


    data = TupleDataset()
    data.columns = [
           Column('page package', label=_('page package')),
           Column('action', label=_('install')),
        ]

    label_install = _("install")
    for pageset_name in i18n.strings.pagesets:
        attachment = "%s--%s.zip" % (lang, pageset_name)
        # not_translated_system_pages are in english
        if attachment.endswith(not_translated_system_pages):
            attachment = 'English_not_translated_system_pages.zip'
        install_link = ''
        querystr = {'action': 'language_setup', 'target': attachment, 'language': lang}
        if AttachFile.exists(request, language_setup_page, attachment):
            install_link = request.page.link_to(request, label_install, querystr=querystr)
        data.addRow((pageset_name, install_link))

    table = DataBrowserWidget(request)
    table.setData(data)
    page_table = ''.join(table.format(method='GET'))

    fmt = request.formatter
    lang_links = [request.page.link_to_raw(request, _lang,
                                        querystr={'action': 'language_setup',
                                                  'language': _lang,
                                                  'pageset': pageset_name, })
                  for _lang in wiki_languages]

    lang_selector = u''.join([fmt.paragraph(1), _("Choose:"), ' ', ' '.join(lang_links), fmt.paragraph(0)])

    title = _("Install language packs for '%s'") % wikiutil.escape(lang)
    request.theme.add_msg(msg, "info")
    request.theme.send_title(title, page=request.page, pagename=pagename)
    request.write(request.formatter.startContent("content"))
    request.write(lang_selector)
    request.write(page_table)
    request.write(request.formatter.endContent())
    request.theme.send_footer(pagename)
    request.theme.send_closing_html()


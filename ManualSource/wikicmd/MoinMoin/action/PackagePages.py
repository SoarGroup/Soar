# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - PackagePages action

    This action allows you to package pages.

    TODO: use ActionBase class

    @copyright: 2005 MoinMoin:AlexanderSchremmer
                2007-2009 MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""
import cStringIO
import os
import zipfile
from datetime import datetime

from MoinMoin import wikiutil, config, user
from MoinMoin.Page import Page
from MoinMoin.action.AttachFile import _addLogEntry
from MoinMoin.packages import MOIN_PACKAGE_FILE, packLine, unpackLine
from MoinMoin.action import AttachFile
from MoinMoin.action.AttachFile import _get_files
from MoinMoin.search import searchPages

class ActionError(Exception):
    pass

class PackagePages:
    def __init__(self, pagename, request):
        self.request = request
        self.pagename = pagename
        self.page = Page(request, pagename)

    def allowed(self):
        """ Check if user is allowed to do this. """
        return not self.__class__.__name__ in self.request.cfg.actions_excluded

    def render(self):
        """ Render action

        This action returns a wiki page with optional message, or
        redirects to new page.
        """
        _ = self.request.getText

        if 'cancel' in self.request.values:
            # User canceled
            return self.page.send_page()

        try:
            if not self.allowed():
                self.request.theme.add_msg(_('You are not allowed to edit this page.'), "error")
                raise ActionError
            elif not self.page.exists():
                self.request.theme.add_msg(_('This page is already deleted or was never created!'))
                raise ActionError

            self.package()
        except ActionError, e:
            return self.page.send_page()

    def package(self):
        """ Calls collectpackage() with the arguments specified. """
        _ = self.request.getText

        # Get new name from form and normalize.
        pagelist = self.request.values.get('pagelist', u'')
        packagename = self.request.values.get('packagename', u'')
        include_attachments = self.request.values.get('include_attachments', False)

        if not self.request.values.get('submit'):
            self.request.theme.add_msg(self.makeform(), "dialog")
            raise ActionError

        target = wikiutil.taintfilename(packagename)

        if not target:
            self.request.theme.add_msg(self.makeform(_('Invalid filename "%s"!') % wikiutil.escape(packagename)), "error")
            raise ActionError

        request = self.request
        filelike = cStringIO.StringIO()
        package = self.collectpackage(unpackLine(pagelist, ","), filelike, target, include_attachments)
        request.headers['Content-Type'] = 'application/zip'
        request.headers['Content-Length'] = filelike.tell()
        request.headers['Content-Disposition'] = 'inline; filename="%s"' % target
        request.write(filelike.getvalue())
        filelike.close()

    def makeform(self, error=""):
        """ Display a package page form

        The form might contain an error that happened when package file was not given.
        """
        from MoinMoin.widget.dialog import Dialog
        _ = self.request.getText

        if error:
            error = u'<p class="error">%s</p>\n' % error

        d = {
            'url': self.request.href(self.pagename),
            'error': error,
            'action': self.__class__.__name__,
            'pagename': wikiutil.escape(self.pagename, True),
            'include_attachments_label': _('Include all attachments?'),
            'package': _('Package pages'),
            'cancel': _('Cancel'),
            'newname_label': _("Package name"),
            'list_label': _("List of page names - separated by a comma"),
        }
        form = '''
%(error)s
<form method="post" action="%(url)s">
<input type="hidden" name="action" value="%(action)s">
<table>
    <tr>
        <td class="label"><label>%(newname_label)s</label></td>
        <td class="content">
            <input type="text" name="packagename" value="package.zip" size="80">
        </td>
    </tr>
    <tr>
        <td class="label"><label>%(list_label)s</label></td>
        <td class="content">
            <input type="text" name="pagelist" size="80" maxlength="200" value="%(pagename)s">
        </td>
    </tr>
    <tr>
        <td class="label">
        %(include_attachments_label)s<input type="checkbox" name="include_attachments" value="0" %(include_attachments_label)s>
    </td>
    </tr>
    <tr>
        <td></td>
        <td class="buttons">
            <input type="submit" name="submit" value="%(package)s">
            <input type="submit" name="cancel" value="%(cancel)s">
        </td>
    </tr>
</table>
</form>''' % d

        return Dialog(self.request, content=form)

    def searchpackage(self, request, searchkey):
        """ Search MoinMoin for the string specified and return a list of
        matching pages, provided they are not system pages and not
        present in the underlay.

        @param request: current request
        @param searchkey: string to search for
        @rtype: list
        @return: list of pages matching searchkey
        """

        pagelist = searchPages(request, searchkey)

        titles = []
        for title in pagelist.hits:
            if not wikiutil.isSystemPage(request, title.page_name) or not title.page.getPageStatus()[0]:
                titles.append(title.page_name)
        return titles

    def collectpackage(self, pagelist, fileobject, pkgname="", include_attachments=False):
        """ Expects a list of pages as an argument, and fileobject to be an open
        file object, which a zipfile will get written to.

        @param pagelist: pages to package
        @param fileobject: open file object to write to
        @param pkgname: optional file name, to prevent self packaging
        @rtype: string or None
        @return: error message, if one happened
        @rtype: boolean
        @param include_attachments: True if you want attachments collected
        """
        _ = self.request.getText
        COMPRESSION_LEVEL = zipfile.ZIP_DEFLATED

        pages = []
        for pagename in pagelist:
            pagename = wikiutil.normalize_pagename(pagename, self.request.cfg)
            if pagename:
                page = Page(self.request, pagename)
                if page.exists() and self.request.user.may.read(pagename):
                    pages.append(page)
        if not pages:
            return (_('No pages like "%s"!') % wikiutil.escape(pagelist))

        # Set zipfile output
        zf = zipfile.ZipFile(fileobject, "w", COMPRESSION_LEVEL)

        cnt = 0
        userid = user.getUserIdentification(self.request)
        script = [packLine(['MoinMoinPackage', '1']), ]

        for page in pages:
            cnt += 1
            files = _get_files(self.request, page.page_name)
            script.append(packLine(["AddRevision", str(cnt), page.page_name, userid, "Created by the PackagePages action."]))

            timestamp = wikiutil.version2timestamp(page.mtime_usecs())
            zi = zipfile.ZipInfo(filename=str(cnt), date_time=datetime.fromtimestamp(timestamp).timetuple()[:6])
            zi.compress_type = COMPRESSION_LEVEL
            zf.writestr(zi, page.get_raw_body().encode("utf-8"))
            if include_attachments:
                for attname in files:
                    if attname != pkgname:
                        cnt += 1
                        zipname = "%d_attachment" % cnt
                        script.append(packLine(["AddAttachment", zipname, attname, page.page_name, userid, "Created by the PackagePages action."]))
                        filename = AttachFile.getFilename(self.request, page.page_name, attname)
                        zf.write(filename, zipname)
        script += [packLine(['Print', 'Thank you for using PackagePages!'])]

        zf.writestr(MOIN_PACKAGE_FILE, u"\n".join(script).encode("utf-8"))
        zf.close()

def execute(pagename, request):
    """ Glue code for actions """
    PackagePages(pagename, request).render()


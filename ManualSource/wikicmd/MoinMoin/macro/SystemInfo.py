# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - SystemInfo Macro

    This macro shows some info about your wiki, wiki software and your system.

    @copyright: 2006-2008 MoinMoin:ThomasWaldmann,
                2007 MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""

Dependencies = ['pages']

import sys, os
from StringIO import StringIO

from MoinMoin import wikiutil, version
from MoinMoin import action, macro, parser
from MoinMoin.logfile import editlog, eventlog
from MoinMoin.Page import Page

class SystemInfo:
    def __init__(self, macro):
        self.macro = macro
        self.request = macro.request
        self.formatter = macro.formatter

    def formatInReadableUnits(self, size):
        size = float(size)
        unit = u' Byte'
        if size > 9999:
            unit = u' KiB'
            size /= 1024
        if size > 9999:
            unit = u' MiB'
            size /= 1024
        if size > 9999:
            unit = u' GiB'
            size /= 1024
        return u"%.1f %s" % (size, unit)

    def getDirectorySize(self, path):
        try:
            dirsize = 0
            for root, dummy, files in os.walk(path):
                dirsize += sum([os.path.getsize(os.path.join(root, name)) for name in files])
        except EnvironmentError:
            dirsize = -1
        return dirsize

    def render(self):
        _ = self.request.getText
        return self.formatter.rawHTML(self.getInfo())

    def getInfo(self):
        _ = self.request.getText
        request = self.request

        buf = StringIO()

        row = lambda label, value, buf=buf: buf.write(u'<dt>%s</dt><dd>%s</dd>' % (label, value))

        buf.write(u'<dl>')
        row(_('Python Version'), sys.version)
        row(_('MoinMoin Version'), _('Release %s [Revision %s]') % (version.release, version.revision))

        if not request.user.valid:
            # for an anonymous user it ends here.
            buf.write(u'</dl>')
            return buf.getvalue()

        if request.user.isSuperUser():
            # superuser gets all page dependent stuff only
            try:
                import Ft
                ftversion = Ft.__version__
            except ImportError:
                ftversion = None
            except AttributeError:
                ftversion = 'N/A'

            if ftversion:
                row(_('4Suite Version'), ftversion)

            # TODO add python-xml check and display it

            # Get the full pagelist of the wiki
            pagelist = request.rootpage.getPageList(user='')
            systemPages = []
            totalsize = 0
            for page in pagelist:
                if wikiutil.isSystemPage(request, page):
                    systemPages.append(page)
                totalsize += Page(request, page).size()

            row(_('Number of pages'), str(len(pagelist)-len(systemPages)))
            row(_('Number of system pages'), str(len(systemPages)))

            row(_('Accumulated page sizes'), self.formatInReadableUnits(totalsize))
            data_dir = request.cfg.data_dir
            row(_('Disk usage of %(data_dir)s/pages/') % {'data_dir': data_dir},
                self.formatInReadableUnits(self.getDirectorySize(os.path.join(data_dir, 'pages'))))
            row(_('Disk usage of %(data_dir)s/') % {'data_dir': data_dir},
            self.formatInReadableUnits(self.getDirectorySize(data_dir)))

            edlog = editlog.EditLog(request)
            row(_('Entries in edit log'), "%s (%s)" % (edlog.lines(), self.formatInReadableUnits(edlog.size())))

            # This puts a heavy load on the server when the log is large
            eventlogger = eventlog.EventLog(request)
            row('Event log', self.formatInReadableUnits(eventlogger.size()))

        nonestr = _("NONE")
        # a valid user gets info about all installed extensions
        row(_('Global extension macros'), ', '.join(macro.modules) or nonestr)
        row(_('Local extension macros'),
            ', '.join(wikiutil.wikiPlugins('macro', self.macro.cfg)) or nonestr)

        glob_actions = [x for x in action.modules
                        if not x in request.cfg.actions_excluded]
        row(_('Global extension actions'), ', '.join(glob_actions) or nonestr)
        loc_actions = [x for x in wikiutil.wikiPlugins('action', self.macro.cfg)
                       if not x in request.cfg.actions_excluded]
        row(_('Local extension actions'), ', '.join(loc_actions) or nonestr)

        row(_('Global parsers'), ', '.join(parser.modules) or nonestr)
        row(_('Local extension parsers'),
            ', '.join(wikiutil.wikiPlugins('parser', self.macro.cfg)) or nonestr)

        try:
            import xapian
            xapVersion = 'Xapian %s' % xapian.version_string()
        except ImportError:
            xapian = None
            xapVersion = _('Xapian and/or Python Xapian bindings not installed')

        xapian_enabled = request.cfg.xapian_search
        xapState = (_('Disabled'), _('Enabled'))
        xapRow = '%s, %s' % (xapState[xapian_enabled], xapVersion)

        if xapian and xapian_enabled:
            from MoinMoin.search.Xapian.indexing import XapianIndex
            idx = XapianIndex(request)
            idxState = (_('index unavailable'), _('index available'))
            idx_exists = idx.exists()
            xapRow += ', %s' % idxState[idx_exists]
            if idx_exists:
                xapRow += ', %s' % (_('last modified: %s') %
                    request.user.getFormattedDateTime(idx.mtime()))

        row(_('Xapian search'), xapRow)

        if xapian and xapian_enabled:
            stems = xapian.Stem.get_available_languages()
            row(_('Stemming for Xapian'), xapState[request.cfg.xapian_stemming] +
                " (%s)" % (stems or nonestr))

        try:
            from threading import activeCount
            t_count = activeCount()
        except ImportError:
            t_count = None

        row(_('Active threads'), t_count or _('N/A'))
        buf.write(u'</dl>')

        return buf.getvalue()

def macro_SystemInfo(macro):
    if macro.request.isSpiderAgent: # reduce bot cpu usage
        return ''
    return SystemInfo(macro).render()


# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Package Installer

    @copyright: 2005 MoinMoin:AlexanderSchremmer,
                2007-2010 MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""

import os, re, sys
import zipfile

from MoinMoin import config, wikiutil, caching, user
from MoinMoin.Page import Page
from MoinMoin.PageEditor import PageEditor
from MoinMoin.logfile import editlog, eventlog
from MoinMoin.util import filesys

MOIN_PACKAGE_FILE = 'MOIN_PACKAGE'
MAX_VERSION = 1


# Exceptions
class PackageException(Exception):
    """ Raised if the package is broken. """
    pass

class ScriptException(Exception):
    """ Raised when there is a problem in the script. """

    def __unicode__(self):
        """ Return unicode error message """
        if isinstance(self.args[0], str):
            return unicode(self.args[0], config.charset)
        else:
            return unicode(self.args[0])

class RuntimeScriptException(ScriptException):
    """ Raised when the script problem occurs at runtime. """

class ScriptExit(Exception):
    """ Raised by the script commands when the script should quit. """

def event_logfile(self, pagename, pagefile):
    # add event log entry
    eventtype = 'SAVENEW'
    mtime_usecs = wikiutil.timestamp2version(os.path.getmtime(pagefile))
    elog = eventlog.EventLog(self.request)
    elog.add(self.request, eventtype, {'pagename': pagename}, 1, mtime_usecs)

def edit_logfile_append(self, pagename, pagefile, rev, action, logname='edit-log', comment=u'', author=u"Scripting Subsystem"):
    glog = editlog.EditLog(self.request, uid_override=author)
    pagelog = Page(self.request, pagename).getPagePath(logname, use_underlay=0, isfile=1)
    llog = editlog.EditLog(self.request, filename=pagelog,
                               uid_override=author)
    mtime_usecs = wikiutil.timestamp2version(os.path.getmtime(pagefile))
    host = '::1'
    extra = u''
    glog.add(self.request, mtime_usecs, rev, action, pagename, host, comment)
    llog.add(self.request, mtime_usecs, rev, action, pagename, host, extra, comment)
    event_logfile(self, pagename, pagefile)

# Parsing and (un)quoting for script files
def packLine(items, separator="|"):
    """ Packs a list of items into a string that is separated by `separator`. """
    return '|'.join([item.replace('\\', '\\\\').replace(separator, '\\' + separator) for item in items])

def unpackLine(string, separator="|"):
    """ Unpacks a string that was packed by packLine. """
    result = []
    token = None
    escaped = False
    for char in string:
        if token is None:
            token = ""
        if escaped and char in ('\\', separator):
            token += char
            escaped = False
            continue
        escaped = (char == '\\')
        if escaped:
            continue
        if char == separator:
            result.append(token)
            token = ""
        else:
            token += char
    if token is not None:
        result.append(token)
    return result

def str2boolean(string):
    """
    Converts the parameter to a boolean value by recognising different
    truth literals.
    """
    return (string.lower() in ('yes', 'true', '1'))

class ScriptEngine:
    """
    The script engine supplies the needed commands to execute the installation
    script.
    """

    def _extractToFile(self, source, target):
        """ Extracts source and writes the contents into target. """
        # TODO, add file dates
        target_file = open(target, "wb")
        target_file.write(self.extract_file(source))
        target_file.close()

    def __init__(self):
        self.themename = None
        self.ignoreExceptions = False
        self.goto = 0

        #Satisfy pylint
        self.msg = getattr(self, "msg", "")
        self.request = getattr(self, "request", None)

    def do_addattachment(self, zipname, filename, pagename, author=u"Scripting Subsystem", comment=u""):
        """
        Installs an attachment

        @param pagename: Page where the file is attached. Or in 2.0, the file itself.
        @param zipname: Filename of the attachment from the zip file
        @param filename: Filename of the attachment (just applicable for MoinMoin < 2.0)
        """
        if self.request.user.may.write(pagename):
            _ = self.request.getText

            attachments = Page(self.request, pagename).getPagePath("attachments", check_create=1)
            filename = wikiutil.taintfilename(filename)
            zipname = wikiutil.taintfilename(zipname)
            target = os.path.join(attachments, filename)
            page = PageEditor(self.request, pagename, do_editor_backup=0, uid_override=author)
            rev = page.current_rev()
            path = page.getPagePath(check_create=0)
            if not os.path.exists(target):
                self._extractToFile(zipname, target)
                if os.path.exists(target):
                    filesys.chmod(target, 0666 & config.umask)
                    action = 'ATTNEW'
                    edit_logfile_append(self, pagename, path, rev, action, logname='edit-log',
                                       comment=u'%(filename)s' % {"filename": filename}, author=author)
                self.msg += u"%(filename)s attached \n" % {"filename": filename}
            else:
                self.msg += u"%(filename)s not attached \n" % {"filename": filename}
        else:
            self.msg += u"action add attachment: not enough rights - nothing done \n"

    def do_delattachment(self, filename, pagename, author=u"Scripting Subsystem", comment=u""):
        """
        Removes an attachment

        @param pagename: Page where the file is attached. Or in 2.0, the file itself.
        @param filename: Filename of the attachment (just applicable for MoinMoin < 2.0)
        """
        if self.request.user.may.write(pagename):
            _ = self.request.getText

            attachments = Page(self.request, pagename).getPagePath("attachments", check_create=1)
            filename = wikiutil.taintfilename(filename)
            target = os.path.join(attachments, filename)
            page = PageEditor(self.request, pagename, do_editor_backup=0, uid_override=author)
            rev = page.current_rev()
            path = page.getPagePath(check_create=0)
            if os.path.exists(target):
                os.remove(target)
                action = 'ATTDEL'
                edit_logfile_append(self, pagename, path, rev, action, logname='edit-log',
                                    comment=u'%(filename)s' % {"filename": filename}, author=author)
                self.msg += u"%(filename)s removed \n" % {"filename": filename}
            else:
                self.msg += u"%(filename)s does not exist \n" % {"filename": filename}
        else:
            self.msg += u"action delete attachment: not enough rights - nothing done \n"

    def do_print(self, *param):
        """ Prints the parameters into output of the script. """
        self.msg += '; '.join(param) + "\n"

    def do_exit(self):
        """ Exits the script. """
        raise ScriptExit

    def do_ignoreexceptions(self, boolean):
        """ Sets the ignore exceptions setting. If exceptions are ignored, the
        script does not stop if one is encountered. """
        self.ignoreExceptions = str2boolean(boolean)

    def do_ensureversion(self, version, lines=0):
        """
        Ensures that the version of MoinMoin is greater or equal than
        version. If lines is unspecified, the script aborts. Otherwise,
        the next lines (amount specified by lines) are not executed.

        @param version: required version of MoinMoin (e.g. "1.3.4")
        @param lines: lines to ignore
        """
        _ = self.request.getText

        from MoinMoin.version import release
        version_int = [int(x) for x in version.split(".")]
        # use a regex here to get only the numbers of the release string (e.g. ignore betaX)
        release = re.compile('\d+').findall(release)[0:3]
        release = [int(x) for x in release]
        if version_int > release:
            if lines > 0:
                self.goto = lines
            else:
                raise RuntimeScriptException(_("The package needs a newer version"
                                               " of MoinMoin (at least %s).") %
                                             version)

    def do_setthemename(self, themename):
        """ Sets the name of the theme which will be altered next. """
        self.themename = wikiutil.taintfilename(str(themename))

    def do_copythemefile(self, filename, ftype, target):
        """ Copies a theme-related file (CSS, PNG, etc.) into a directory of the
        current theme.

        @param filename: name of the file in this package
        @param ftype:   the subdirectory of the theme directory, e.g. "css"
        @param target: filename, e.g. "screen.css"
        """
        _ = self.request.getText
        if self.themename is None:
            raise RuntimeScriptException(_("The theme name is not set."))

        from MoinMoin.web.static import STATIC_FILES_PATH as htdocs_dir
        if not os.access(htdocs_dir, os.W_OK):
            raise RuntimeScriptException(_("Theme files not installed! Write rights missing for %s.") % htdocs_dir)

        theme_file = os.path.join(htdocs_dir, self.themename,
                                  wikiutil.taintfilename(ftype),
                                  wikiutil.taintfilename(target))
        theme_dir = os.path.dirname(theme_file)
        if not os.path.exists(theme_dir):
            os.makedirs(theme_dir)
        self._extractToFile(filename, theme_file)

    def do_installplugin(self, filename, visibility, ptype, target):
        """
        Installs a python code file into the appropriate directory.

        @param filename: name of the file in this package
        @param visibility: 'local' will copy it into the plugin folder of the
            current wiki. 'global' will use the folder of the MoinMoin python
            package.
        @param ptype: the type of the plugin, e.g. "parser"
        @param target: the filename of the plugin, e.g. wiki.py
        """
        visibility = visibility.lower()
        ptype = wikiutil.taintfilename(ptype.lower())

        if visibility == 'global':
            basedir = os.path.dirname(__import__("MoinMoin").__file__)
        elif visibility == 'local':
            basedir = self.request.cfg.plugin_dir

        target = os.path.join(basedir, ptype, wikiutil.taintfilename(target))

        self._extractToFile(filename, target)
        wikiutil._wiki_plugins = {}

    def do_installpackage(self, pagename, filename):
        """
        Installs a package.

        @param pagename: Page where the file is attached. Or in 2.0, the file itself.
        @param filename: Filename of the attachment (just applicable for MoinMoin < 2.0)
        """
        _ = self.request.getText

        attachments = Page(self.request, pagename).getPagePath("attachments", check_create=0)
        package = ZipPackage(self.request, os.path.join(attachments, wikiutil.taintfilename(filename)))

        if package.isPackage():
            if not package.installPackage():
                raise RuntimeScriptException(_("Installation of '%(filename)s' failed.") % {
                    'filename': filename} + "\n" + package.msg)
        else:
            raise RuntimeScriptException(_('The file %s is not a MoinMoin package file.') % filename)

        self.msg += package.msg

    def do_addrevision(self, filename, pagename, author=u"Scripting Subsystem", comment=u"", trivial=u"No"):
        """ Adds a revision to a page.

        @param filename: name of the file in this package
        @param pagename: name of the target page
        @param author:   user name of the editor (optional)
        @param comment:  comment related to this revision (optional)
        @param trivial:  boolean, if it is a trivial edit
        """
        _ = self.request.getText
        trivial = str2boolean(trivial)
        if self.request.user.may.write(pagename):
            page = PageEditor(self.request, pagename, do_editor_backup=0)
            try:
                page.saveText(self.extract_file(filename).decode("utf-8"), 0, trivial=trivial, comment=comment)
            except PageEditor.Unchanged:
                pass
            else:
                self.msg += u"%(pagename)s added \n" % {"pagename": pagename}
        else:
            self.msg += u"action add revision: not enough rights - nothing done \n"

    def do_renamepage(self, pagename, newpagename, author=u"Scripting Subsystem", comment=u"Renamed by the scripting subsystem."):
        """ Renames a page.

        @param pagename: name of the target page
        @param newpagename: name of the new page
        @param author:   user name of the editor (optional)
        @param comment:  comment related to this revision (optional)
        """
        if self.request.user.may.write(pagename):
            _ = self.request.getText
            page = PageEditor(self.request, pagename, do_editor_backup=0, uid_override=author)
            if not page.exists():
                raise RuntimeScriptException(_("The page %s does not exist.") % pagename)
            newpage = PageEditor(self.request, newpagename)
            page.renamePage(newpage.page_name, comment=u"Renamed from '%s'" % (pagename))
            self.msg += u'%(pagename)s renamed to %(newpagename)s\n' % {
                            "pagename": pagename,
                            "newpagename": newpagename}
        else:
            self.msg += u"action rename page: not enough rights - nothing done \n"

    def do_deletepage(self, pagename, comment="Deleted by the scripting subsystem."):
        """ Marks a page as deleted (like the DeletePage action).

        @param pagename: page to delete
        @param comment:  the related comment (optional)
        """
        if self.request.user.may.write(pagename):
            _ = self.request.getText
            page = PageEditor(self.request, pagename, do_editor_backup=0)
            if not page.exists():
                raise RuntimeScriptException(_("The page %s does not exist.") % pagename)
            page.deletePage(comment)
        else:
            self.msg += u"action delete page: not enough rights - nothing done \n"

    def do_replaceunderlayattachment(self, zipname, filename, pagename, author=u"Scripting Subsystem", comment=u""):
        """
        overwrite underlay attachments

        @param pagename: Page where the file is attached. Or in 2.0, the file itself.
        @param zipname: Filename of the attachment from the zip file
        @param filename: Filename of the attachment (just applicable for MoinMoin < 2.0)
        """
        if self.request.user.may.write(pagename):
            _ = self.request.getText
            filename = wikiutil.taintfilename(filename)
            zipname = wikiutil.taintfilename(zipname)
            page = PageEditor(self.request, pagename, do_editor_backup=0, uid_override=author)
            pagedir = page.getPagePath(use_underlay=1, check_create=1)
            attachments = os.path.join(pagedir, 'attachments')
            if not os.path.exists(attachments):
                os.mkdir(attachments)
            target = os.path.join(attachments, filename)
            self._extractToFile(zipname, target)
            if os.path.exists(target):
                filesys.chmod(target, 0666 & config.umask)
        else:
            self.msg += u"action replace underlay attachment: not enough rights - nothing done \n"

    def do_replaceunderlay(self, filename, pagename):
        """
        Overwrites underlay pages. Implementational detail: This needs to be
        kept in sync with the page class.

        @param filename: name of the file in the package
        @param pagename: page to be overwritten
        """
        page = Page(self.request, pagename)

        pagedir = page.getPagePath(use_underlay=1, check_create=1)

        revdir = os.path.join(pagedir, 'revisions')
        cfn = os.path.join(pagedir, 'current')

        revstr = '%08d' % 1
        if not os.path.exists(revdir):
            os.mkdir(revdir)

        currentf = open(cfn, 'w')
        currentf.write(revstr + "\n")
        currentf.close()

        pagefile = os.path.join(revdir, revstr)
        self._extractToFile(filename, pagefile)
        # Clear caches
        # TODO Code from MoinMoin/script/maint/cleancache.py may be used

    def runScript(self, commands):
        """ Runs the commands.

        @param commands: list of strings which contain a command each
        @return True on success
        """
        _ = self.request.getText

        headerline = unpackLine(commands[0])

        if headerline[0].lower() != "MoinMoinPackage".lower():
            raise PackageException(_("Invalid package file header."))

        self.revision = int(headerline[1])
        if self.revision > MAX_VERSION:
            raise PackageException(_("Package file format unsupported."))

        lineno = 1
        success = True

        for line in commands[1:]:
            lineno += 1
            if self.goto > 0:
                self.goto -= 1
                continue

            if line.startswith("#") or len(line) == 0:
                continue
            elements = unpackLine(line)
            fnname = elements[0].strip().lower()
            if fnname == '':
                continue
            try:
                if fnname in self.request.cfg.packagepages_actions_excluded:
                    self.msg += u"action package %s: excluded \n" % elements[0].strip()
                    success = False
                    continue
                else:
                    fn = getattr(self, "do_" + fnname)
            except AttributeError:
                self.msg += u"Exception RuntimeScriptException: %s\n" % (
                        _("Unknown function %(func)s in line %(lineno)i.") %
                        {'func': elements[0], 'lineno': lineno}, )
                success = False
                break

            try:
                fn(*elements[1:])
            except ScriptExit:
                break
            except TypeError, e:
                self.msg += u"Exception %s (line %i): %s\n" % (e.__class__.__name__, lineno, unicode(e))
                success = False
                break
            except RuntimeScriptException, e:
                if not self.ignoreExceptions:
                    self.msg += u"Exception %s (line %i): %s\n" % (e.__class__.__name__, lineno, unicode(e))
                    success = False
                    break

        return success

class Package:
    """ A package consists of a bunch of files which can be installed. """
    def __init__(self, request):
        self.request = request
        self.msg = ""

    def installPackage(self):
        """ Opens the package and executes the script. """

        _ = self.request.getText

        if not self.isPackage():
            raise PackageException(_("The file %s was not found in the package.") % MOIN_PACKAGE_FILE)

        commands = self.getScript().splitlines()

        return self.runScript(commands)

    def getScript(self):
        """ Returns the script. """
        return self.extract_file(MOIN_PACKAGE_FILE).decode("utf-8").replace(u"\ufeff", "")

    def extract_file(self, filename):
        """ Returns the contents of a file in the package. """
        raise NotImplementedError

    def filelist(self):
        """ Returns a list of all files. """
        raise NotImplementedError

    def isPackage(self):
        """ Returns true if this package is recognised. """
        raise NotImplementedError

class ZipPackage(Package, ScriptEngine):
    """ A package that reads its files from a .zip file. """
    def __init__(self, request, filename):
        """ Initialise the package.

        @param request: RequestBase instance
        @param filename: filename of the .zip file
        """

        Package.__init__(self, request)
        ScriptEngine.__init__(self)
        self.filename = filename

        self._isZipfile = zipfile.is_zipfile(filename)
        if self._isZipfile:
            self.zipfile = zipfile.ZipFile(filename)
        # self.zipfile.getinfo(name)

    def extract_file(self, filename):
        """ Returns the contents of a file in the package. """
        _ = self.request.getText
        try:
            return self.zipfile.read(filename.encode("cp437"))
        except KeyError:
            raise RuntimeScriptException(_(
                "The file %s was not found in the package.") % filename)

    def filelist(self):
        """ Returns a list of all files. """
        return self.zipfile.namelist()

    def isPackage(self):
        """ Returns true if this package is recognised. """
        return self._isZipfile and MOIN_PACKAGE_FILE in self.zipfile.namelist()

def main():
    args = sys.argv
    if len(args)-1 not in (2, 3) or args[1] not in ('l', 'i'):
        print >> sys.stderr, """MoinMoin Package Installer v%(version)i

%(myname)s action packagefile [request URL]

action      - Either "l" for listing the script or "i" for installing.
packagefile - The path to the file containing the MoinMoin installer package
request URL - Just needed if you are running a wiki farm, used to differentiate
              the correct wiki.

Example:

%(myname)s i ../package.zip

""" % {"version": MAX_VERSION, "myname": os.path.basename(args[0])}
        raise SystemExit

    packagefile = args[2]
    if len(args) > 3:
        request_url = args[3]
    else:
        request_url = None

    # Setup MoinMoin environment
    from MoinMoin.web.contexts import ScriptContext
    request = ScriptContext(url=request_url)

    package = ZipPackage(request, packagefile)
    if not package.isPackage():
        print "The specified file %s is not a package." % packagefile
        raise SystemExit

    if args[1] == 'l':
        print package.getScript()
    elif args[1] == 'i':
        if package.installPackage():
            print "Installation was successful!"
        else:
            print "Installation failed."
        if package.msg:
            print package.msg

if __name__ == '__main__':
    main()

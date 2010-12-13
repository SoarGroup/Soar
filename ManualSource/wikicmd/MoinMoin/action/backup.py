# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - download a backup via http.

    Triggering backup action will check if you are authorized to do a backup
    and if yes, just send a <siteid>-<date>--<time>.tar.<format> to you.
    What exactly is contained in your backup depends on your wiki's
    configuration - please make sure you have everything you need BEFORE you
    really need it.

    Note: there is no restore support, you need somebody having access to your
          wiki installation via the server's file system, knowing about tar
          and restoring your data CAREFULLY (AKA "the server admin").

    @copyright: 2005-2008 by MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

import os, re, time

from MoinMoin import wikiutil
from MoinMoin.support import tarfile


def addFiles(path, tar, exclude_func):
    """ Add files in path to tar """
    for root, dirs, files in os.walk(path):
        files.sort() # sorted page revs may compress better
        for name in files:
            path = os.path.join(root, name)
            if exclude_func(path):
                continue
            tar.add(path)


def sendBackup(request):
    """ Send compressed tar file """
    dateStamp = time.strftime("%Y-%m-%d--%H-%M-%S-UTC", time.gmtime())
    filename = "%s-%s.tar.%s" % (request.cfg.siteid, dateStamp, request.cfg.backup_compression)
    request.headers['Content-Type'] = 'application/octet-stream'
    request.headers['Content-Disposition'] = 'inline; filename="%s"' % filename
    tar = tarfile.open(fileobj=request, mode="w|%s" % request.cfg.backup_compression)
    # allow GNU tar's longer file/pathnames
    tar.posix = False
    for path in request.cfg.backup_include:
        addFiles(path, tar, request.cfg.backup_exclude)
    tar.close()


def sendBackupForm(request, pagename):
    _ = request.getText
    request.setContentLanguage(request.lang)
    title = _('Wiki Backup')
    request.theme.send_title(title, pagename=pagename)
    request.write(request.formatter.startContent("content"))

    request.write(_("""= Downloading a backup =

Please note:
 * Store backups in a safe and secure place - they contain sensitive information.
 * Make sure your wiki configuration backup_* values are correct and complete.
 * Make sure the backup file you get contains everything you need in case of problems.
 * Make sure it is downloaded without problems.

To get a backup, just click here:""", wiki=True))

    request.write("""
<form action="%(baseurl)s/%(pagename)s" method="POST" enctype="multipart/form-data">
<input type="hidden" name="action" value="backup">
<input type="hidden" name="do" value="backup">
<input type="submit" value="%(backup_button)s">
</form>
""" % {
    'baseurl': request.script_root,
    'pagename': wikiutil.quoteWikinameURL(pagename),
    'backup_button': _('Backup'),
})

    request.write(request.formatter.endContent())
    request.theme.send_footer(pagename)
    request.theme.send_closing_html()

# NOTE: consider using ActionBase.render_msg instead of this function.
def sendMsg(request, pagename, msg, msgtype):
    """
    @param msg: Message to show. Should be escaped.
    """
    from MoinMoin import Page
    request.theme.add_msg(msg, msgtype)
    return Page.Page(request, pagename).send_page()


def backupAllowed(request):
    """ Return True if backup is allowed """
    action = __name__.split('.')[-1]
    user = request.user
    return user.valid and user.name in request.cfg.backup_users


def execute(pagename, request):
    _ = request.getText
    if not backupAllowed(request):
        return sendMsg(request, pagename,
                       msg=_('You are not allowed to do remote backup.'), msgtype="error")

    dowhat = request.form.get('do')
    if dowhat == 'backup':
        sendBackup(request)
    elif dowhat is None:
        sendBackupForm(request, pagename)
    else:
        return sendMsg(request, pagename,
                       msg=_('Unknown backup subaction: %s.') % wikiutil.escape(dowhat), msgtype="error")

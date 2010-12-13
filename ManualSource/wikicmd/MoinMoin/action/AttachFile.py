# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - AttachFile action

    This action lets a page have multiple attachment files.
    It creates a folder <data>/pages/<pagename>/attachments
    and keeps everything in there.

    Form values: action=Attachment
    1. with no 'do' key: returns file upload form
    2. do=attach: accept file upload and saves the file in
       ../attachment/pagename/
    3. /pagename/fname?action=Attachment&do=get[&mimetype=type]:
       return contents of the attachment file with the name fname.
    4. /pathname/fname, do=view[&mimetype=type]:create a page
       to view the content of the file

    To link to an attachment, use [[attachment:file.txt]],
    to embed an attachment, use {{attachment:file.png}}.

    @copyright: 2001 by Ken Sugino (sugino@mediaone.net),
                2001-2004 by Juergen Hermann <jh@web.de>,
                2005 MoinMoin:AlexanderSchremmer,
                2005 DiegoOngaro at ETSZONE (diego@etszone.com),
                2005-2007 MoinMoin:ReimarBauer,
                2007-2008 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

import os, time, zipfile, errno, datetime
from StringIO import StringIO

from werkzeug import http_date

from MoinMoin import log
logging = log.getLogger(__name__)

# keep both imports below as they are, order is important:
from MoinMoin import wikiutil
import mimetypes

from MoinMoin import config, packages
from MoinMoin.Page import Page
from MoinMoin.util import filesys, timefuncs
from MoinMoin.security.textcha import TextCha
from MoinMoin.events import FileAttachedEvent, FileRemovedEvent, send_event
from MoinMoin.support import tarfile

action_name = __name__.split('.')[-1]

#############################################################################
### External interface - these are called from the core code
#############################################################################

class AttachmentAlreadyExists(Exception):
    pass


def getBasePath(request):
    """ Get base path where page dirs for attachments are stored. """
    return request.rootpage.getPagePath('pages')


def getAttachDir(request, pagename, create=0):
    """ Get directory where attachments for page `pagename` are stored. """
    if request.page and pagename == request.page.page_name:
        page = request.page # reusing existing page obj is faster
    else:
        page = Page(request, pagename)
    return page.getPagePath("attachments", check_create=create)


def absoluteName(url, pagename):
    """ Get (pagename, filename) of an attachment: link
        @param url: PageName/filename.ext or filename.ext (unicode)
        @param pagename: name of the currently processed page (unicode)
        @rtype: tuple of unicode
        @return: PageName, filename.ext
    """
    url = wikiutil.AbsPageName(pagename, url)
    pieces = url.split(u'/')
    if len(pieces) == 1:
        return pagename, pieces[0]
    else:
        return u"/".join(pieces[:-1]), pieces[-1]


def get_action(request, filename, do):
    generic_do_mapping = {
        # do -> action
        'get': action_name,
        'view': action_name,
        'move': action_name,
        'del': action_name,
        'unzip': action_name,
        'install': action_name,
        'upload_form': action_name,
    }
    basename, ext = os.path.splitext(filename)
    do_mapping = request.cfg.extensions_mapping.get(ext, {})
    action = do_mapping.get(do, None)
    if action is None:
        # we have no special support for this,
        # look up whether we have generic support:
        action = generic_do_mapping.get(do, None)
    return action


def getAttachUrl(pagename, filename, request, addts=0, do='get'):
    """ Get URL that points to attachment `filename` of page `pagename`.
        For upload url, call with do='upload_form'.
        Returns the URL to do the specified "do" action or None,
        if this action is not supported.
    """
    action = get_action(request, filename, do)
    if action:
        args = dict(action=action, do=do, target=filename)
        if do not in ['get', 'view', # harmless
                      'modify', # just renders the applet html, which has own ticket
                      'move', # renders rename form, which has own ticket
            ]:
            # create a ticket for the not so harmless operations
            # we need action= here because the current action (e.g. "show" page
            # with a macro AttachList) may not be the linked-to action, e.g.
            # "AttachFile". Also, AttachList can list attachments of another page,
            # thus we need to give pagename= also.
            args['ticket'] = wikiutil.createTicket(request,
                                                   pagename=pagename, action=action_name)
        url = request.href(pagename, **args)
        return url


def getIndicator(request, pagename):
    """ Get an attachment indicator for a page (linked clip image) or
        an empty string if not attachments exist.
    """
    _ = request.getText
    attach_dir = getAttachDir(request, pagename)
    if not os.path.exists(attach_dir):
        return ''

    files = os.listdir(attach_dir)
    if not files:
        return ''

    fmt = request.formatter
    attach_count = _('[%d attachments]') % len(files)
    attach_icon = request.theme.make_icon('attach', vars={'attach_count': attach_count})
    attach_link = (fmt.url(1, request.href(pagename, action=action_name), rel='nofollow') +
                   attach_icon +
                   fmt.url(0))
    return attach_link


def getFilename(request, pagename, filename):
    """ make complete pathfilename of file "name" attached to some page "pagename"
        @param request: request object
        @param pagename: name of page where the file is attached to (unicode)
        @param filename: filename of attached file (unicode)
        @rtype: string (in config.charset encoding)
        @return: complete path/filename of attached file
    """
    if isinstance(filename, unicode):
        filename = filename.encode(config.charset)
    return os.path.join(getAttachDir(request, pagename, create=1), filename)


def exists(request, pagename, filename):
    """ check if page <pagename> has a file <filename> attached """
    fpath = getFilename(request, pagename, filename)
    return os.path.exists(fpath)


def size(request, pagename, filename):
    """ return file size of file attachment """
    fpath = getFilename(request, pagename, filename)
    return os.path.getsize(fpath)


def info(pagename, request):
    """ Generate snippet with info on the attachment for page `pagename`. """
    _ = request.getText

    attach_dir = getAttachDir(request, pagename)
    files = []
    if os.path.isdir(attach_dir):
        files = os.listdir(attach_dir)
    page = Page(request, pagename)
    link = page.url(request, {'action': action_name})
    attach_info = _('There are <a href="%(link)s">%(count)s attachment(s)</a> stored for this page.') % {
        'count': len(files),
        'link': wikiutil.escape(link)
        }
    return "\n<p>\n%s\n</p>\n" % attach_info


def _write_stream(content, stream, bufsize=8192):
    if hasattr(content, 'read'): # looks file-like
        import shutil
        shutil.copyfileobj(content, stream, bufsize)
    elif isinstance(content, str):
        stream.write(content)
    else:
        logging.error("unsupported content object: %r" % content)
        raise

def add_attachment(request, pagename, target, filecontent, overwrite=0):
    """ save <filecontent> to an attachment <target> of page <pagename>

        filecontent can be either a str (in memory file content),
        or an open file object (file content in e.g. a tempfile).
    """
    # replace illegal chars
    target = wikiutil.taintfilename(target)

    # get directory, and possibly create it
    attach_dir = getAttachDir(request, pagename, create=1)
    fpath = os.path.join(attach_dir, target).encode(config.charset)

    exists = os.path.exists(fpath)
    if exists:
        if overwrite:
            remove_attachment(request, pagename, target)
        else:
            raise AttachmentAlreadyExists

    # save file
    stream = open(fpath, 'wb')
    try:
        _write_stream(filecontent, stream)
    finally:
        stream.close()

    _addLogEntry(request, 'ATTNEW', pagename, target)

    filesize = os.path.getsize(fpath)
    event = FileAttachedEvent(request, pagename, target, filesize)
    send_event(event)

    return target, filesize


def remove_attachment(request, pagename, target):
    """ remove attachment <target> of page <pagename>
    """
    # replace illegal chars
    target = wikiutil.taintfilename(target)

    # get directory, do not create it
    attach_dir = getAttachDir(request, pagename, create=0)
    # remove file
    fpath = os.path.join(attach_dir, target).encode(config.charset)
    try:
        filesize = os.path.getsize(fpath)
        os.remove(fpath)
    except:
        # either it is gone already or we have no rights - not much we can do about it
        filesize = 0
    else:
        _addLogEntry(request, 'ATTDEL', pagename, target)

        event = FileRemovedEvent(request, pagename, target, filesize)
        send_event(event)

    return target, filesize


#############################################################################
### Internal helpers
#############################################################################

def _addLogEntry(request, action, pagename, filename):
    """ Add an entry to the edit log on uploads and deletes.

        `action` should be "ATTNEW" or "ATTDEL"
    """
    from MoinMoin.logfile import editlog
    t = wikiutil.timestamp2version(time.time())
    fname = wikiutil.url_quote(filename)

    # Write to global log
    log = editlog.EditLog(request)
    log.add(request, t, 99999999, action, pagename, request.remote_addr, fname)

    # Write to local log
    log = editlog.EditLog(request, rootpagename=pagename)
    log.add(request, t, 99999999, action, pagename, request.remote_addr, fname)


def _access_file(pagename, request):
    """ Check form parameter `target` and return a tuple of
        `(pagename, filename, filepath)` for an existing attachment.

        Return `(pagename, None, None)` if an error occurs.
    """
    _ = request.getText

    error = None
    if not request.values.get('target'):
        error = _("Filename of attachment not specified!")
    else:
        filename = wikiutil.taintfilename(request.values['target'])
        fpath = getFilename(request, pagename, filename)

        if os.path.isfile(fpath):
            return (pagename, filename, fpath)
        error = _("Attachment '%(filename)s' does not exist!") % {'filename': filename}

    error_msg(pagename, request, error)
    return (pagename, None, None)


def _build_filelist(request, pagename, showheader, readonly, mime_type='*'):
    _ = request.getText
    fmt = request.html_formatter

    # access directory
    attach_dir = getAttachDir(request, pagename)
    files = _get_files(request, pagename)

    if mime_type != '*':
        files = [fname for fname in files if mime_type == mimetypes.guess_type(fname)[0]]

    html = []
    if files:
        if showheader:
            html.append(fmt.rawHTML(_(
                "To refer to attachments on a page, use '''{{{attachment:filename}}}''', \n"
                "as shown below in the list of files. \n"
                "Do '''NOT''' use the URL of the {{{[get]}}} link, \n"
                "since this is subject to change and can break easily.",
                wiki=True
            )))

        label_del = _("del")
        label_move = _("move")
        label_get = _("get")
        label_edit = _("edit")
        label_view = _("view")
        label_unzip = _("unzip")
        label_install = _("install")

        may_read = request.user.may.read(pagename)
        may_write = request.user.may.write(pagename)
        may_delete = request.user.may.delete(pagename)

        html.append(fmt.bullet_list(1))
        for file in files:
            mt = wikiutil.MimeType(filename=file)
            fullpath = os.path.join(attach_dir, file).encode(config.charset)
            st = os.stat(fullpath)
            base, ext = os.path.splitext(file)
            parmdict = {'file': wikiutil.escape(file),
                        'fsize': "%.1f" % (float(st.st_size) / 1024),
                        'fmtime': request.user.getFormattedDateTime(st.st_mtime),
                       }

            links = []
            if may_delete and not readonly:
                links.append(fmt.url(1, getAttachUrl(pagename, file, request, do='del')) +
                             fmt.text(label_del) +
                             fmt.url(0))

            if may_delete and not readonly:
                links.append(fmt.url(1, getAttachUrl(pagename, file, request, do='move')) +
                             fmt.text(label_move) +
                             fmt.url(0))

            links.append(fmt.url(1, getAttachUrl(pagename, file, request)) +
                         fmt.text(label_get) +
                         fmt.url(0))

            links.append(fmt.url(1, getAttachUrl(pagename, file, request, do='view')) +
                         fmt.text(label_view) +
                         fmt.url(0))

            if may_write and not readonly:
                edit_url = getAttachUrl(pagename, file, request, do='modify')
                if edit_url:
                    links.append(fmt.url(1, edit_url) +
                                 fmt.text(label_edit) +
                                 fmt.url(0))

            try:
                is_zipfile = zipfile.is_zipfile(fullpath)
                if is_zipfile and not readonly:
                    is_package = packages.ZipPackage(request, fullpath).isPackage()
                    if is_package and request.user.isSuperUser():
                        links.append(fmt.url(1, getAttachUrl(pagename, file, request, do='install')) +
                                     fmt.text(label_install) +
                                     fmt.url(0))
                    elif (not is_package and mt.minor == 'zip' and
                          may_read and may_write and may_delete):
                        links.append(fmt.url(1, getAttachUrl(pagename, file, request, do='unzip')) +
                                     fmt.text(label_unzip) +
                                     fmt.url(0))
            except RuntimeError:
                # We don't want to crash with a traceback here (an exception
                # here could be caused by an uploaded defective zip file - and
                # if we crash here, the user does not get a UI to remove the
                # defective zip file again).
                # RuntimeError is raised by zipfile stdlib module in case of
                # problems (like inconsistent slash and backslash usage in the
                # archive).
                logging.exception("An exception within zip file attachment handling occurred:")

            html.append(fmt.listitem(1))
            html.append("[%s]" % "&nbsp;| ".join(links))
            html.append(" (%(fmtime)s, %(fsize)s KB) [[attachment:%(file)s]]" % parmdict)
            html.append(fmt.listitem(0))
        html.append(fmt.bullet_list(0))

    else:
        if showheader:
            html.append(fmt.paragraph(1))
            html.append(fmt.text(_("No attachments stored for %(pagename)s") % {
                                   'pagename': pagename}))
            html.append(fmt.paragraph(0))

    return ''.join(html)


def _get_files(request, pagename):
    attach_dir = getAttachDir(request, pagename)
    if os.path.isdir(attach_dir):
        files = [fn.decode(config.charset) for fn in os.listdir(attach_dir)]
        files.sort()
    else:
        files = []
    return files


def _get_filelist(request, pagename):
    return _build_filelist(request, pagename, 1, 0)


def error_msg(pagename, request, msg):
    msg = wikiutil.escape(msg)
    request.theme.add_msg(msg, "error")
    Page(request, pagename).send_page()


#############################################################################
### Create parts of the Web interface
#############################################################################

def send_link_rel(request, pagename):
    files = _get_files(request, pagename)
    for fname in files:
        url = getAttachUrl(pagename, fname, request, do='view')
        request.write(u'<link rel="Appendix" title="%s" href="%s">\n' % (
                      wikiutil.escape(fname, 1),
                      wikiutil.escape(url, 1)))

def send_uploadform(pagename, request):
    """ Send the HTML code for the list of already stored attachments and
        the file upload form.
    """
    _ = request.getText

    if not request.user.may.read(pagename):
        request.write('<p>%s</p>' % _('You are not allowed to view this page.'))
        return

    writeable = request.user.may.write(pagename)

    # First send out the upload new attachment form on top of everything else.
    # This avoids usability issues if you have to scroll down a lot to upload
    # a new file when the page already has lots of attachments:
    if writeable:
        request.write('<h2>' + _("New Attachment") + '</h2>')
        request.write("""
<form action="%(url)s" method="POST" enctype="multipart/form-data">
<dl>
<dt>%(upload_label_file)s</dt>
<dd><input type="file" name="file" size="50"></dd>
<dt>%(upload_label_target)s</dt>
<dd><input type="text" name="target" size="50" value="%(target)s"></dd>
<dt>%(upload_label_overwrite)s</dt>
<dd><input type="checkbox" name="overwrite" value="1" %(overwrite_checked)s></dd>
</dl>
%(textcha)s
<p>
<input type="hidden" name="action" value="%(action_name)s">
<input type="hidden" name="do" value="upload">
<input type="hidden" name="ticket" value="%(ticket)s">
<input type="submit" value="%(upload_button)s">
</p>
</form>
""" % {
    'url': request.href(pagename),
    'action_name': action_name,
    'upload_label_file': _('File to upload'),
    'upload_label_target': _('Rename to'),
    'target': wikiutil.escape(request.values.get('target', ''), 1),
    'upload_label_overwrite': _('Overwrite existing attachment of same name'),
    'overwrite_checked': ('', 'checked')[request.form.get('overwrite', '0') == '1'],
    'upload_button': _('Upload'),
    'textcha': TextCha(request).render(),
    'ticket': wikiutil.createTicket(request),
})

    request.write('<h2>' + _("Attached Files") + '</h2>')
    request.write(_get_filelist(request, pagename))

    if not writeable:
        request.write('<p>%s</p>' % _('You are not allowed to attach a file to this page.'))

#############################################################################
### Web interface for file upload, viewing and deletion
#############################################################################

def execute(pagename, request):
    """ Main dispatcher for the 'AttachFile' action. """
    _ = request.getText

    do = request.values.get('do', 'upload_form')
    handler = globals().get('_do_%s' % do)
    if handler:
        msg = handler(pagename, request)
    else:
        msg = _('Unsupported AttachFile sub-action: %s') % do
    if msg:
        error_msg(pagename, request, msg)


def _do_upload_form(pagename, request):
    upload_form(pagename, request)


def upload_form(pagename, request, msg=''):
    if msg:
        msg = wikiutil.escape(msg)
    _ = request.getText

    # Use user interface language for this generated page
    request.setContentLanguage(request.lang)
    request.theme.add_msg(msg, "dialog")
    request.theme.send_title(_('Attachments for "%(pagename)s"') % {'pagename': pagename}, pagename=pagename)
    request.write('<div id="content">\n') # start content div
    send_uploadform(pagename, request)
    request.write('</div>\n') # end content div
    request.theme.send_footer(pagename)
    request.theme.send_closing_html()


def _do_upload(pagename, request):
    _ = request.getText

    if not wikiutil.checkTicket(request, request.form.get('ticket', '')):
        return _('Please use the interactive user interface to use action %(actionname)s!') % {'actionname': 'AttachFile.upload' }

    # Currently we only check TextCha for upload (this is what spammers ususally do),
    # but it could be extended to more/all attachment write access
    if not TextCha(request).check_answer_from_form():
        return _('TextCha: Wrong answer! Go back and try again...')

    form = request.form

    file_upload = request.files.get('file')
    if not file_upload:
        # This might happen when trying to upload file names
        # with non-ascii characters on Safari.
        return _("No file content. Delete non ASCII characters from the file name and try again.")

    try:
        overwrite = int(form.get('overwrite', '0'))
    except:
        overwrite = 0

    if not request.user.may.write(pagename):
        return _('You are not allowed to attach a file to this page.')

    if overwrite and not request.user.may.delete(pagename):
        return _('You are not allowed to overwrite a file attachment of this page.')

    target = form.get('target', u'').strip()
    if not target:
        target = file_upload.filename or u''

    target = wikiutil.clean_input(target)

    if not target:
        return _("Filename of attachment not specified!")

    # add the attachment
    try:
        target, bytes = add_attachment(request, pagename, target, file_upload.stream, overwrite=overwrite)
        msg = _("Attachment '%(target)s' (remote name '%(filename)s')"
                " with %(bytes)d bytes saved.") % {
                'target': target, 'filename': file_upload.filename, 'bytes': bytes}
    except AttachmentAlreadyExists:
        msg = _("Attachment '%(target)s' (remote name '%(filename)s') already exists.") % {
            'target': target, 'filename': file_upload.filename}

    # return attachment list
    upload_form(pagename, request, msg)


class ContainerItem:
    """ A storage container (multiple objects in 1 tarfile) """

    def __init__(self, request, pagename, containername):
        self.request = request
        self.pagename = pagename
        self.containername = containername
        self.container_filename = getFilename(request, pagename, containername)

    def member_url(self, member):
        """ return URL for accessing container member
            (we use same URL for get (GET) and put (POST))
        """
        url = Page(self.request, self.pagename).url(self.request, {
            'action': 'AttachFile',
            'do': 'box',  # shorter to type than 'container'
            'target': self.containername,
            #'member': member,
        })
        return url + '&member=%s' % member
        # member needs to be last in qs because twikidraw looks for "file extension" at the end

    def get(self, member):
        """ return a file-like object with the member file data
        """
        tf = tarfile.TarFile(self.container_filename)
        return tf.extractfile(member)

    def put(self, member, content, content_length=None):
        """ save data into a container's member """
        tf = tarfile.TarFile(self.container_filename, mode='a')
        if isinstance(member, unicode):
            member = member.encode('utf-8')
        ti = tarfile.TarInfo(member)
        if isinstance(content, str):
            if content_length is None:
                content_length = len(content)
            content = StringIO(content) # we need a file obj
        elif not hasattr(content, 'read'):
            logging.error("unsupported content object: %r" % content)
            raise
        assert content_length >= 0  # we don't want -1 interpreted as 4G-1
        ti.size = content_length
        tf.addfile(ti, content)
        tf.close()

    def truncate(self):
        f = open(self.container_filename, 'w')
        f.close()

    def exists(self):
        return os.path.exists(self.container_filename)

def _do_del(pagename, request):
    _ = request.getText

    if not wikiutil.checkTicket(request, request.args.get('ticket', '')):
        return _('Please use the interactive user interface to use action %(actionname)s!') % {'actionname': 'AttachFile.del' }

    pagename, filename, fpath = _access_file(pagename, request)
    if not request.user.may.delete(pagename):
        return _('You are not allowed to delete attachments on this page.')
    if not filename:
        return # error msg already sent in _access_file

    remove_attachment(request, pagename, filename)

    upload_form(pagename, request, msg=_("Attachment '%(filename)s' deleted.") % {'filename': filename})


def move_file(request, pagename, new_pagename, attachment, new_attachment):
    _ = request.getText

    newpage = Page(request, new_pagename)
    if newpage.exists(includeDeleted=1) and request.user.may.write(new_pagename) and request.user.may.delete(pagename):
        new_attachment_path = os.path.join(getAttachDir(request, new_pagename,
                              create=1), new_attachment).encode(config.charset)
        attachment_path = os.path.join(getAttachDir(request, pagename),
                          attachment).encode(config.charset)

        if os.path.exists(new_attachment_path):
            upload_form(pagename, request,
                msg=_("Attachment '%(new_pagename)s/%(new_filename)s' already exists.") % {
                    'new_pagename': new_pagename,
                    'new_filename': new_attachment})
            return

        if new_attachment_path != attachment_path:
            filesize = os.path.getsize(attachment_path)
            filesys.rename(attachment_path, new_attachment_path)
            _addLogEntry(request, 'ATTDEL', pagename, attachment)
            event = FileRemovedEvent(request, pagename, attachment, filesize)
            send_event(event)
            _addLogEntry(request, 'ATTNEW', new_pagename, new_attachment)
            event = FileAttachedEvent(request, new_pagename, new_attachment, filesize)
            send_event(event)
            upload_form(pagename, request,
                        msg=_("Attachment '%(pagename)s/%(filename)s' moved to '%(new_pagename)s/%(new_filename)s'.") % {
                            'pagename': pagename,
                            'filename': attachment,
                            'new_pagename': new_pagename,
                            'new_filename': new_attachment})
        else:
            upload_form(pagename, request, msg=_("Nothing changed"))
    else:
        upload_form(pagename, request, msg=_("Page '%(new_pagename)s' does not exist or you don't have enough rights.") % {
            'new_pagename': new_pagename})


def _do_attachment_move(pagename, request):
    _ = request.getText

    if 'cancel' in request.form:
        return _('Move aborted!')
    if not wikiutil.checkTicket(request, request.form.get('ticket', '')):
        return _('Please use the interactive user interface to use action %(actionname)s!') % {'actionname': 'AttachFile.move' }
    if not request.user.may.delete(pagename):
        return _('You are not allowed to move attachments from this page.')

    if 'newpagename' in request.form:
        new_pagename = request.form.get('newpagename')
    else:
        upload_form(pagename, request, msg=_("Move aborted because new page name is empty."))
    if 'newattachmentname' in request.form:
        new_attachment = request.form.get('newattachmentname')
        if new_attachment != wikiutil.taintfilename(new_attachment):
            upload_form(pagename, request, msg=_("Please use a valid filename for attachment '%(filename)s'.") % {
                                  'filename': new_attachment})
            return
    else:
        upload_form(pagename, request, msg=_("Move aborted because new attachment name is empty."))

    attachment = request.form.get('oldattachmentname')
    move_file(request, pagename, new_pagename, attachment, new_attachment)


def _do_move(pagename, request):
    _ = request.getText

    pagename, filename, fpath = _access_file(pagename, request)
    if not request.user.may.delete(pagename):
        return _('You are not allowed to move attachments from this page.')
    if not filename:
        return # error msg already sent in _access_file

    # move file
    d = {'action': action_name,
         'url': request.href(pagename),
         'do': 'attachment_move',
         'ticket': wikiutil.createTicket(request),
         'pagename': wikiutil.escape(pagename, 1),
         'attachment_name': wikiutil.escape(filename, 1),
         'move': _('Move'),
         'cancel': _('Cancel'),
         'newname_label': _("New page name"),
         'attachment_label': _("New attachment name"),
        }
    formhtml = '''
<form action="%(url)s" method="POST">
<input type="hidden" name="action" value="%(action)s">
<input type="hidden" name="do" value="%(do)s">
<input type="hidden" name="ticket" value="%(ticket)s">
<table>
    <tr>
        <td class="label"><label>%(newname_label)s</label></td>
        <td class="content">
            <input type="text" name="newpagename" value="%(pagename)s" size="80">
        </td>
    </tr>
    <tr>
        <td class="label"><label>%(attachment_label)s</label></td>
        <td class="content">
            <input type="text" name="newattachmentname" value="%(attachment_name)s" size="80">
        </td>
    </tr>
    <tr>
        <td></td>
        <td class="buttons">
            <input type="hidden" name="oldattachmentname" value="%(attachment_name)s">
            <input type="submit" name="move" value="%(move)s">
            <input type="submit" name="cancel" value="%(cancel)s">
        </td>
    </tr>
</table>
</form>''' % d
    thispage = Page(request, pagename)
    request.theme.add_msg(formhtml, "dialog")
    return thispage.send_page()


def _do_box(pagename, request):
    _ = request.getText

    pagename, filename, fpath = _access_file(pagename, request)
    if not request.user.may.read(pagename):
        return _('You are not allowed to get attachments from this page.')
    if not filename:
        return # error msg already sent in _access_file

    timestamp = datetime.datetime.fromtimestamp(os.path.getmtime(fpath))
    if_modified = request.if_modified_since
    if if_modified and if_modified >= timestamp:
        request.status_code = 304
    else:
        ci = ContainerItem(request, pagename, filename)
        filename = wikiutil.taintfilename(request.values['member'])
        mt = wikiutil.MimeType(filename=filename)
        content_type = mt.content_type()
        mime_type = mt.mime_type()

        # TODO: fix the encoding here, plain 8 bit is not allowed according to the RFCs
        # There is no solution that is compatible to IE except stripping non-ascii chars
        filename_enc = filename.encode(config.charset)

        # for dangerous files (like .html), when we are in danger of cross-site-scripting attacks,
        # we just let the user store them to disk ('attachment').
        # For safe files, we directly show them inline (this also works better for IE).
        dangerous = mime_type in request.cfg.mimetypes_xss_protect
        content_dispo = dangerous and 'attachment' or 'inline'

        now = time.time()
        request.headers['Date'] = http_date(now)
        request.headers['Content-Type'] = content_type
        request.headers['Last-Modified'] = http_date(timestamp)
        request.headers['Expires'] = http_date(now - 365 * 24 * 3600)
        #request.headers['Content-Length'] = os.path.getsize(fpath)
        content_dispo_string = '%s; filename="%s"' % (content_dispo, filename_enc)
        request.headers['Content-Disposition'] = content_dispo_string

        # send data
        request.send_file(ci.get(filename))


def _do_get(pagename, request):
    _ = request.getText

    pagename, filename, fpath = _access_file(pagename, request)
    if not request.user.may.read(pagename):
        return _('You are not allowed to get attachments from this page.')
    if not filename:
        return # error msg already sent in _access_file

    timestamp = datetime.datetime.fromtimestamp(os.path.getmtime(fpath))
    if_modified = request.if_modified_since
    if if_modified and if_modified >= timestamp:
        request.status_code = 304
    else:
        mt = wikiutil.MimeType(filename=filename)
        content_type = mt.content_type()
        mime_type = mt.mime_type()

        # TODO: fix the encoding here, plain 8 bit is not allowed according to the RFCs
        # There is no solution that is compatible to IE except stripping non-ascii chars
        filename_enc = filename.encode(config.charset)

        # for dangerous files (like .html), when we are in danger of cross-site-scripting attacks,
        # we just let the user store them to disk ('attachment').
        # For safe files, we directly show them inline (this also works better for IE).
        dangerous = mime_type in request.cfg.mimetypes_xss_protect
        content_dispo = dangerous and 'attachment' or 'inline'

        now = time.time()
        request.headers['Date'] = http_date(now)
        request.headers['Content-Type'] = content_type
        request.headers['Last-Modified'] = http_date(timestamp)
        request.headers['Expires'] = http_date(now - 365 * 24 * 3600)
        request.headers['Content-Length'] = os.path.getsize(fpath)
        content_dispo_string = '%s; filename="%s"' % (content_dispo, filename_enc)
        request.headers['Content-Disposition'] = content_dispo_string

        # send data
        request.send_file(open(fpath, 'rb'))


def _do_install(pagename, request):
    _ = request.getText

    if not wikiutil.checkTicket(request, request.args.get('ticket', '')):
        return _('Please use the interactive user interface to use action %(actionname)s!') % {'actionname': 'AttachFile.install' }

    pagename, target, targetpath = _access_file(pagename, request)
    if not request.user.isSuperUser():
        return _('You are not allowed to install files.')
    if not target:
        return

    package = packages.ZipPackage(request, targetpath)

    if package.isPackage():
        if package.installPackage():
            msg = _("Attachment '%(filename)s' installed.") % {'filename': target}
        else:
            msg = _("Installation of '%(filename)s' failed.") % {'filename': target}
        if package.msg:
            msg += " " + package.msg
    else:
        msg = _('The file %s is not a MoinMoin package file.') % target

    upload_form(pagename, request, msg=msg)


def _do_unzip(pagename, request, overwrite=False):
    _ = request.getText

    if not wikiutil.checkTicket(request, request.args.get('ticket', '')):
        return _('Please use the interactive user interface to use action %(actionname)s!') % {'actionname': 'AttachFile.unzip' }

    pagename, filename, fpath = _access_file(pagename, request)
    if not (request.user.may.delete(pagename) and request.user.may.read(pagename) and request.user.may.write(pagename)):
        return _('You are not allowed to unzip attachments of this page.')

    if not filename:
        return # error msg already sent in _access_file

    try:
        if not zipfile.is_zipfile(fpath):
            return _('The file %(filename)s is not a .zip file.') % {'filename': filename}

        # determine how which attachment names we have and how much space each is occupying
        curr_fsizes = dict([(f, size(request, pagename, f)) for f in _get_files(request, pagename)])

        # Checks for the existance of one common prefix path shared among
        # all files in the zip file. If this is the case, remove the common prefix.
        # We also prepare a dict of the new filenames->filesizes.
        zip_path_sep = '/'  # we assume '/' is as zip standard suggests
        fname_index = None
        mapping = []
        new_fsizes = {}
        zf = zipfile.ZipFile(fpath)
        for zi in zf.infolist():
            name = zi.filename
            if not name.endswith(zip_path_sep):  # a file (not a directory)
                if fname_index is None:
                    fname_index = name.rfind(zip_path_sep) + 1
                    path = name[:fname_index]
                if (name.rfind(zip_path_sep) + 1 != fname_index  # different prefix len
                    or
                    name[:fname_index] != path): # same len, but still different
                    mapping = []  # zip is not acceptable
                    break
                if zi.file_size >= request.cfg.unzip_single_file_size:  # file too big
                    mapping = []  # zip is not acceptable
                    break
                finalname = name[fname_index:]  # remove common path prefix
                finalname = finalname.decode(config.charset, 'replace')  # replaces trash with \uFFFD char
                mapping.append((name, finalname))
                new_fsizes[finalname] = zi.file_size

        # now we either have an empty mapping (if the zip is not acceptable),
        # an identity mapping (no subdirs in zip, just all flat), or
        # a mapping (origname, finalname) where origname is the zip member filename
        # (including some prefix path) and finalname is a simple filename.

        # calculate resulting total file size / count after unzipping:
        if overwrite:
            curr_fsizes.update(new_fsizes)
            total = curr_fsizes
        else:
            new_fsizes.update(curr_fsizes)
            total = new_fsizes
        total_count = len(total)
        total_size = sum(total.values())

        if not mapping:
            msg = _("Attachment '%(filename)s' not unzipped because some files in the zip "
                    "are either not in the same directory or exceeded the single file size limit (%(maxsize_file)d kB)."
                   ) % {'filename': filename,
                        'maxsize_file': request.cfg.unzip_single_file_size / 1000, }
        elif total_size > request.cfg.unzip_attachments_space:
            msg = _("Attachment '%(filename)s' not unzipped because it would have exceeded "
                    "the per page attachment storage size limit (%(size)d kB).") % {
                        'filename': filename,
                        'size': request.cfg.unzip_attachments_space / 1000, }
        elif total_count > request.cfg.unzip_attachments_count:
            msg = _("Attachment '%(filename)s' not unzipped because it would have exceeded "
                    "the per page attachment count limit (%(count)d).") % {
                        'filename': filename,
                        'count': request.cfg.unzip_attachments_count, }
        else:
            not_overwritten = []
            for origname, finalname in mapping:
                try:
                    # Note: reads complete zip member file into memory. ZipFile does not offer block-wise reading:
                    add_attachment(request, pagename, finalname, zf.read(origname), overwrite)
                except AttachmentAlreadyExists:
                    not_overwritten.append(finalname)
            if not_overwritten:
                msg = _("Attachment '%(filename)s' partially unzipped (did not overwrite: %(filelist)s).") % {
                        'filename': filename,
                        'filelist': ', '.join(not_overwritten), }
            else:
                msg = _("Attachment '%(filename)s' unzipped.") % {'filename': filename}
    except RuntimeError, err:
        # We don't want to crash with a traceback here (an exception
        # here could be caused by an uploaded defective zip file - and
        # if we crash here, the user does not get a UI to remove the
        # defective zip file again).
        # RuntimeError is raised by zipfile stdlib module in case of
        # problems (like inconsistent slash and backslash usage in the
        # archive).
        logging.exception("An exception within zip file attachment handling occurred:")
        msg = _("A severe error occurred:") + ' ' + str(err)

    upload_form(pagename, request, msg=msg)


def send_viewfile(pagename, request):
    _ = request.getText
    fmt = request.html_formatter

    pagename, filename, fpath = _access_file(pagename, request)
    if not filename:
        return

    request.write('<h2>' + _("Attachment '%(filename)s'") % {'filename': filename} + '</h2>')
    # show a download link above the content
    label = _('Download')
    link = (fmt.url(1, getAttachUrl(pagename, filename, request, do='get'), css_class="download") +
            fmt.text(label) +
            fmt.url(0))
    request.write('%s<br><br>' % link)

    if filename.endswith('.tdraw') or filename.endswith('.adraw'):
        request.write(fmt.attachment_drawing(filename, ''))
        return

    mt = wikiutil.MimeType(filename=filename)

    # destinguishs if browser need a plugin in place
    if mt.major == 'image' and mt.minor in config.browser_supported_images:
        url = getAttachUrl(pagename, filename, request)
        request.write('<img src="%s" alt="%s">' % (
            wikiutil.escape(url, 1),
            wikiutil.escape(filename, 1)))
        return
    elif mt.major == 'text':
        ext = os.path.splitext(filename)[1]
        Parser = wikiutil.getParserForExtension(request.cfg, ext)
        if Parser is not None:
            try:
                content = file(fpath, 'r').read()
                content = wikiutil.decodeUnknownInput(content)
                colorizer = Parser(content, request, filename=filename)
                colorizer.format(request.formatter)
                return
            except IOError:
                pass

        request.write(request.formatter.preformatted(1))
        # If we have text but no colorizing parser we try to decode file contents.
        content = open(fpath, 'r').read()
        content = wikiutil.decodeUnknownInput(content)
        content = wikiutil.escape(content)
        request.write(request.formatter.text(content))
        request.write(request.formatter.preformatted(0))
        return

    try:
        package = packages.ZipPackage(request, fpath)
        if package.isPackage():
            request.write("<pre><b>%s</b>\n%s</pre>" % (_("Package script:"), wikiutil.escape(package.getScript())))
            return

        if zipfile.is_zipfile(fpath) and mt.minor == 'zip':
            zf = zipfile.ZipFile(fpath, mode='r')
            request.write("<pre>%-46s %19s %12s\n" % (_("File Name"), _("Modified")+" "*5, _("Size")))
            for zinfo in zf.filelist:
                date = "%d-%02d-%02d %02d:%02d:%02d" % zinfo.date_time
                request.write(wikiutil.escape("%-46s %s %12d\n" % (zinfo.filename, date, zinfo.file_size)))
            request.write("</pre>")
            return
    except RuntimeError:
        # We don't want to crash with a traceback here (an exception
        # here could be caused by an uploaded defective zip file - and
        # if we crash here, the user does not get a UI to remove the
        # defective zip file again).
        # RuntimeError is raised by zipfile stdlib module in case of
        # problems (like inconsistent slash and backslash usage in the
        # archive).
        logging.exception("An exception within zip file attachment handling occurred:")
        return

    from MoinMoin import macro
    from MoinMoin.parser.text import Parser

    macro.request = request
    macro.formatter = request.html_formatter
    p = Parser("##\n", request)
    m = macro.Macro(p)

    # use EmbedObject to view valid mime types
    if mt is None:
        request.write('<p>' + _("Unknown file type, cannot display this attachment inline.") + '</p>')
        link = (fmt.url(1, getAttachUrl(pagename, filename, request)) +
                fmt.text(filename) +
                fmt.url(0))
        request.write('For using an external program follow this link %s' % link)
        return
    request.write(m.execute('EmbedObject', u'target="%s", pagename="%s"' % (filename, pagename)))
    return


def _do_view(pagename, request):
    _ = request.getText

    orig_pagename = pagename
    pagename, filename, fpath = _access_file(pagename, request)
    if not request.user.may.read(pagename):
        return _('You are not allowed to view attachments of this page.')
    if not filename:
        return

    request.formatter.page = Page(request, pagename)

    # send header & title
    # Use user interface language for this generated page
    request.setContentLanguage(request.lang)
    title = _('attachment:%(filename)s of %(pagename)s') % {
        'filename': filename, 'pagename': pagename}
    request.theme.send_title(title, pagename=pagename)

    # send body
    request.write(request.formatter.startContent())
    send_viewfile(orig_pagename, request)
    send_uploadform(pagename, request)
    request.write(request.formatter.endContent())

    request.theme.send_footer(pagename)
    request.theme.send_closing_html()


#############################################################################
### File attachment administration
#############################################################################

def do_admin_browser(request):
    """ Browser for SystemAdmin macro. """
    from MoinMoin.util.dataset import TupleDataset, Column
    _ = request.getText

    data = TupleDataset()
    data.columns = [
        Column('page', label=('Page')),
        Column('file', label=('Filename')),
        Column('size', label=_('Size'), align='right'),
    ]

    # iterate over pages that might have attachments
    pages = request.rootpage.getPageList()
    for pagename in pages:
        # check for attachments directory
        page_dir = getAttachDir(request, pagename)
        if os.path.isdir(page_dir):
            # iterate over files of the page
            files = os.listdir(page_dir)
            for filename in files:
                filepath = os.path.join(page_dir, filename)
                data.addRow((
                    (Page(request, pagename).link_to(request,
                                querystr="action=AttachFile"), wikiutil.escape(pagename, 1)),
                    wikiutil.escape(filename.decode(config.charset)),
                    os.path.getsize(filepath),
                ))

    if data:
        from MoinMoin.widget.browser import DataBrowserWidget

        browser = DataBrowserWidget(request)
        browser.setData(data, sort_columns=[0, 1])
        return browser.render(method="GET")

    return ''


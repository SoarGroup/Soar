# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - anywikidraw

    This action is used to call anywikidraw (http://sourceforge.net/projects/anywikidraw/)

    @copyright: 2001 by Ken Sugino (sugino@mediaone.net),
                2001-2004 by Juergen Hermann <jh@web.de>,
                2005 MoinMoin:AlexanderSchremmer,
                2005 DiegoOngaro at ETSZONE (diego@etszone.com),
                2007-2008 MoinMoin:ThomasWaldmann,
                2005-2009 MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""
import os, re

from MoinMoin import log
logging = log.getLogger(__name__)

from MoinMoin import config, wikiutil
from MoinMoin.action import AttachFile, do_show
from MoinMoin.action.AttachFile import _write_stream

action_name = __name__.split('.')[-1]


from MoinMoin.action.twikidraw import gedit_drawing


def attachment_drawing(self, url, text, **kw):
    # This is called for displaying a clickable drawing image by text_html formatter.
    # XXX text arg is unused!
    _ = self.request.getText
    pagename, drawing = AttachFile.absoluteName(url, self.page.page_name)
    containername = wikiutil.taintfilename(drawing)

    drawing_url = AttachFile.getAttachUrl(pagename, containername, self.request, do='modify')
    ci = AttachFile.ContainerItem(self.request, pagename, containername)
    if not ci.exists():
        title = _('Create new drawing "%(filename)s (opens in new window)"') % {'filename': self.text(containername)}
        img = self.icon('attachimg')  # TODO: we need a new "drawimg" in similar grey style and size
        css = 'nonexistent'
        return self.url(1, drawing_url, css=css, title=title) + img + self.url(0)

    title = _('Edit drawing %(filename)s (opens in new window)') % {'filename': self.text(containername)}
    kw['src'] = src = ci.member_url('drawing.png')
    kw['css'] = 'drawing'

    try:
        mapfile = ci.get('drawing.map')
        map = mapfile.read()
        mapfile.close()
        map = map.decode(config.charset)
    except (KeyError, IOError, OSError):
        map = u''
    if map:
        # ToDo mapid must become uniq
        # we have a image map. inline it and add a map ref to the img tag
        # we have also to set a unique ID
        mapid = u'ImageMapOf%s%s' % (self.request.uid_generator(pagename), drawing)

        map = map.replace(u'id="%s.svg"' % drawing, '')
        map = map.replace(u'name="%s.svg"' % drawing, u'name="%s"' % mapid)
        # unxml, because 4.01 concrete will not validate />
        map = map.replace(u'/>', u'>')
        title = _('Clickable drawing: %(filename)s') % {'filename': self.text(containername)}
        if 'title' not in kw:
            kw['title'] = title
        if 'alt' not in kw:
            kw['alt'] = kw['title']
        kw['usemap'] = '#'+mapid
        return self.url(1, drawing_url) + map + self.image(**kw) + self.url(0)
    else:
        if 'title' not in kw:
            kw['title'] = title
        if 'alt' not in kw:
            kw['alt'] = kw['title']
        return self.url(1, drawing_url) + self.image(**kw) + self.url(0)


class AnyWikiDraw(object):
    """ anywikidraw action """
    def __init__(self, request, pagename, target):
        self.request = request
        self.pagename = pagename
        self.target = target

    def save(self):
        request = self.request
        _ = request.getText

        if not wikiutil.checkTicket(request, request.args.get('ticket', '')):
            return _('Please use the interactive user interface to use action %(actionname)s!') % {'actionname': 'anywikidraw.save' }

        pagename = self.pagename
        target = self.target
        if not request.user.may.write(pagename):
            return _('You are not allowed to save a drawing on this page.')
        if not target:
            return _("Empty target name given.")

        file_upload = request.files.get('filepath')
        if not file_upload:
            # This might happen when trying to upload file names
            # with non-ascii characters on Safari.
            return _("No file content. Delete non ASCII characters from the file name and try again.")

        filename = request.form['filename']
        basepath, basename = os.path.split(filename)
        basename, ext = os.path.splitext(basename)

        ci = AttachFile.ContainerItem(request, pagename, target)
        filecontent = file_upload.stream
        content_length = None
        if ext == '.svg': # AnyWikiDraw POSTs this first
            AttachFile._addLogEntry(request, 'ATTDRW', pagename, target)
            ci.truncate()
            filecontent = filecontent.read() # read file completely into memory
            filecontent = filecontent.replace("\r", "")
        elif ext == '.map':
            # touch attachment directory to invalidate cache if new map is saved
            attach_dir = AttachFile.getAttachDir(request, pagename)
            os.utime(attach_dir, None)
            filecontent = filecontent.read() # read file completely into memory
            filecontent = filecontent.strip()
        else:
            #content_length = file_upload.content_length
            # XXX gives -1 for wsgiref :( If this is fixed, we could use the file obj,
            # without reading it into memory completely:
            filecontent = filecontent.read()

        if filecontent:
            ci.put('drawing' + ext, filecontent, content_length)

    def render(self):
        request = self.request
        _ = request.getText
        pagename = self.pagename
        target = self.target
        if not request.user.may.read(pagename):
            return _('You are not allowed to view attachments of this page.')
        if not target:
            return _("Empty target name given.")

        ci = AttachFile.ContainerItem(request, pagename, target)
        if ci.exists():
            drawurl = ci.member_url('drawing.svg')
        else:
            drawurl = ''
        pageurl = request.href(pagename)
        saveurl = request.href(pagename, action=action_name, do='save', target=target,
                               ticket=wikiutil.createTicket(request))
        helpurl = request.href("HelpOnActions/AttachFile")

        html = """
<p>
<applet code="org.anywikidraw.moinmoin.MoinMoinDrawingApplet.class" codebase="."
        archive="%(htdocs)s/applets/anywikidraw/lib/AnyWikiDrawForMoinMoin.jar" width="800" height="620">

    <!-- The following parameters are used to tell AnyWikiDraw how to communicate with MoinMoin. -->
    <param name="DrawingName" value="%(basename)s.svg">
    <param name="DrawingURL" value="%(drawurl)s">
    <param name="PageURL" value="%(pageurl)s">
    <param name="UploadURL" value="%(saveurl)s">

    <!-- The following parameters are used to configure the drawing applet -->
    <param name="Locale" value="en">

    <!-- The following parameters are used to configure Sun's Java Plug-In -->
    <param name="codebase_lookup" value="false">
    <param name="classloader_cache" value="false">
    <!-- The following makes trouble with FF3 on Ubuntu 9.04 as client and
         Apache2 / mod_wsgi on Debian Lenny as server, it seems to confuse
         .gz files with gzip content-encoding and fails miserably somehow:
         param name="java_arguments" value="-Djnlp.packEnabled=true" -->
    <param name="boxborder" value="false">
    <param name="centerimage" value="true">
    <strong>NOTE:</strong> You need a Java enabled browser to edit the drawing.
</applet>
</p>
""" % dict(
    htdocs=request.cfg.url_prefix_static,
    basename=wikiutil.escape(target, 1),
    drawurl=wikiutil.escape(drawurl, 1),
    pageurl=wikiutil.escape(pageurl, 1),
    saveurl=wikiutil.escape(saveurl, 1),
)

        title = '%s %s:%s' % (_('Edit drawing'), pagename, target)
        request.theme.send_title(title, page=request.page, pagename=pagename)
        request.write(request.formatter.startContent("content"))
        request.write(request.formatter.rawHTML(html))
        request.write(request.formatter.endContent())
        request.theme.send_footer(pagename)
        request.theme.send_closing_html()


def execute(pagename, request):
    target = request.values.get('target')
    awd = AnyWikiDraw(request, pagename, target)

    do = request.values.get('do')
    if do == 'save':
        msg = awd.save()
    else:
        msg = awd.render()
    if msg:
        request.theme.add_msg(wikiutil.escape(msg), 'error')
        do_show(pagename, request)


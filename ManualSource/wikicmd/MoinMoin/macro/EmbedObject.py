# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - EmbedObject Macro

    This macro is used to embed an object into a wiki page. Optionally, the
    size of the object can get adjusted. Further keywords are dependent on
    the kind of application, see HelpOnMacros/EmbedObject

    <<EmbedObject(attachment[,width=width][,height=height][,alt=alternate Text])>>

    @copyright: 2006-2009 MoinMoin:ReimarBauer,
                2006 TomSi,
                2007 OliverSiemoneit

    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import wikiutil
from MoinMoin.action import AttachFile

extension_type, extension_name = __name__.split('.')[-2:]

def _check_object_value(param, value):
    """ helps to omit useless lines of object values

    @param param: definition of object param
    @param value: value of param
    """
    if value:
        return '%(param)s="%(value)s"' % {"param": param, "value": wikiutil.escape(value, True)}
    else:
        return ""

def _check_param_value(param, value, valuetype):
    """ helps to ommit useless lines of param values

    @param param: param name defintion
    @param value: the value
    @param valuetype: the type of the value
    """
    # Because plugins do have different defaults we have to write "False" too.
    if isinstance(value, bool):
        value = str(value)

    if value:
        return '''
<param name="%(param)s" value="%(value)s" valuetype="%(valuetype)s">''' % {"param": param,
                                                                           "value": wikiutil.escape(value, True),
                                                                           "valuetype": valuetype}
    else:
        return ""

def macro_EmbedObject(macro, target=wikiutil.required_arg(unicode), pagename=None,
                      width=wikiutil.UnitArgument(None, float, ['px', 'em', 'pt', 'in', 'mm', '%'], defaultunit='px'),
                      height=wikiutil.UnitArgument(None, float, ['px', 'em', 'pt', 'in', 'mm', '%'], defaultunit='px'),
                      alt=u'',
                      play=False, stop=True, loop=False, quality=(u'high', u'low', u'medium'),
                      op=True, repeat=False, autostart=False, align=(u'middle', u'top', u'bottom'), hidden=False,
                      menu=True, wmode=u'transparent', url_mimetype=None):
    """ This macro is used to embed an object into a wiki page """
    # Join unit arguments with their units
    if width:
        if width[1] == 'px':
            width = '%dpx' % int(width[0])
        else:
            width = '%g%s' % width

    if height:
        if height[1] == 'px':
            height = '%dpx' % int(height[0])
        else:
            height = '%g%s' % height

    request = macro.request
    _ = macro.request.getText
    fmt = macro.formatter

    # AttachFile calls always with pagename. Users can call the macro from a different page as the attachment is saved.
    if not pagename:
        pagename = fmt.page.page_name

    if not wikiutil.is_URL(target):
        pagename, fname = AttachFile.absoluteName(target, pagename)

        if not AttachFile.exists(request, pagename, fname):
            linktext = _('Upload new attachment "%(filename)s"') % {'filename': fname}
            target = AttachFile.getAttachUrl(pagename, fname, request, do='upload_form')
            return (fmt.url(1, target) +
                    fmt.text(linktext) +
                    fmt.url(0))

        url = AttachFile.getAttachUrl(pagename, fname, request)
        mt = wikiutil.MimeType(filename=fname)
    else:
        if not url_mimetype:
            return fmt.text(_('%(extension_name)s %(extension_type)s: Required argument %(argument_name)s missing.') % {
                "extension_name": extension_name,
                "extension_type": extension_type,
                "argument_name": "url_mimetype",
            })
        else:
            url = target
            mt = wikiutil.MimeType() # initialize dict
            try:
                mt.major, mt.minor = url_mimetype.split('/')
            except ValueError:
                return fmt.text(_('%(extension_name)s %(extension_type)s: Invalid %(argument_name)s=%(argument_value)s!') % {
                   "extension_name": extension_name,
                   "extension_type": extension_type,
                   "argument_name": "url_mimetype",
                   "argument_value": str(url_mimetype),
                })

    mime_type = "%s/%s" % (mt.major, mt.minor, )
    dangerous = mime_type in request.cfg.mimetypes_xss_protect

    if not mime_type in request.cfg.mimetypes_embed or dangerous:
        return "%s: %s%s%s" % (fmt.text(
                _("Current configuration does not allow embedding of the file %(file)s because of its mimetype %(mimetype)s.") % {
                    "mimetype": mime_type,
                    "file": target}),
                fmt.url(1, url),
                fmt.text(target),
                fmt.url(0))

    if not alt:
        alt = "%(text)s %(mime_type)s" % {'text': _("Embedded"), 'mime_type': mime_type}

    embed_src = ''
    if mt.major == 'video':
        if not width and not height:
            width = '400px'
            height = '400px'

        embed_src = '''
<object %(ob_data)s %(ob_type)s %(ob_width)s %(ob_height)s %(ob_align)s %(ob_standby)s %(ob_stop)s>
%(wmode)s%(movie)s%(play)s%(stop)s%(repeat)s%(autostart)s%(op)s%(menu)s
<p>%(alt)s</p>
</object>''' % {
    "ob_data": _check_object_value("data", url),
    "ob_type": _check_object_value("type", mime_type),
    "ob_width": _check_object_value("width", width),
    "ob_height": _check_object_value("height", height),
    "ob_align": _check_object_value("align", align),
    "ob_standby": _check_object_value("standby", alt),
    "ob_stop": _check_object_value("stop", stop),
    "wmode": _check_param_value("wmode", wmode, "data"),
    "movie": _check_param_value("movie", url, "data"),
    "play": _check_param_value("play", play, "data"),
    "stop": _check_param_value("stop", stop, "data"),
    "repeat": _check_param_value("repeat", repeat, "data"),
    "autostart": _check_param_value("autostart", autostart, "data"),
    "op": _check_param_value("op", op, "data"),
    "menu": _check_param_value("menu", menu, "data"),
    "alt": wikiutil.escape(alt),
}

    elif mt.major in ['image', 'chemical', 'x-world']:
        embed_src = '''
<object %(ob_data)s %(ob_type)s  %(ob_width)s %(ob_height)s %(ob_align)s>
%(name)s
<p>%(alt)s</p>
</object>''' % {
    "mime_type": mime_type,
    "ob_data": _check_object_value("data", url),
    "ob_width": _check_object_value("width", width),
    "ob_height": _check_object_value("height", height),
    "ob_type": _check_object_value("type", mime_type),
    "ob_align": _check_object_value("align", align),
    "name": _check_param_value("name", url, "data"),
    "alt": wikiutil.escape(alt),
}

    elif mt.major == 'audio':
        if not width and not height:
            width = '400px'
            height = '100px'
        embed_src = '''
<object %(ob_data)s %(ob_type)s  %(ob_width)s %(ob_height)s %(ob_align)s>
%(audio)s%(repeat)s%(autostart)s%(op)s%(play)s%(stop)s%(hidden)s<p>%(alt)s</p>
</object>''' % {
    "ob_data": _check_object_value("data", url),
    "ob_width": _check_object_value("width", width or "60"),
    "ob_height": _check_object_value("height", height or "20"),
    "ob_type": _check_object_value("type", mime_type),
    "ob_align": _check_object_value("align", align),
    "audio": _check_param_value("audio", url, "data"),
    "repeat": _check_param_value("repeat", repeat, "data"),
    "autostart": _check_param_value("autostart", autostart, "data"),
    "op": _check_param_value("op", op, "data"),
    "play": _check_param_value("play", play, "data"),
    "stop": _check_param_value("stop", stop, "data"),
    "hidden": _check_param_value("hidden", hidden, "data"),
    "alt": wikiutil.escape(alt),
}

    elif mt.major == 'application':
        # workaround for the acroread browser plugin not knowing the size to embed
        # we use a width of 100% for the case that there is no width given.
        # A height of 100% gives a fullscreen pdf file view without embedding it into the wikicontent.
        if mt.minor == 'pdf':
            width = width or '100%'
            height = height or '800px'
            embed_src = '''
<object %(ob_data)s %(ob_type)s %(ob_width)s %(ob_height)s %(ob_align)s>
<p>%(alt)s</p>
</object>''' % {
    "ob_data": _check_object_value("data", url),
    "ob_width": _check_object_value("width", width),
    "ob_height": _check_object_value("height", height),
    "ob_type": _check_object_value("type", mime_type),
    "ob_align": _check_object_value("align", align),
    "alt": wikiutil.escape(alt),
}
        else:
            embed_src = '''
<object %(ob_data)s %(ob_type)s %(ob_width)s %(ob_height)s %(ob_align)s>
%(movie)s%(quality)s%(wmode)s%(autostart)s%(play)s%(loop)s%(menu)s<p>%(alt)s</p>
</object>''' % {
    "ob_data": _check_object_value("data", url),
    "ob_width": _check_object_value("width", width),
    "ob_height": _check_object_value("height", height),
    "ob_type": _check_object_value("type", mime_type),
    "ob_align": _check_object_value("align", align),
    "movie": _check_param_value("movie", url, "data"),
    "quality": _check_param_value("quality", quality, "data"),
    "wmode": _check_param_value("wmode", wmode, "data"),
    "autostart": _check_param_value("autostart", autostart, "data"),
    "play": _check_param_value("play", play, "data"),
    "loop": _check_param_value("loop", loop, "data"),
    "menu": _check_param_value("menu", menu, "data"),
    "alt": wikiutil.escape(alt),
}

    return fmt.rawHTML(embed_src)

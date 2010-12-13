# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Wiki Configuration
"""
from MoinMoin.config import multiconfig

Dependencies = ['user'] # table headings are translated to user language
generates_headings = True

def macro_WikiConfig(macro):
    request = macro.request
    _ = request.getText
    f = macro.request.formatter
    ret = []

    if not request.user or not request.user.isSuperUser():
        return ''

    settings = {}
    for groupname in multiconfig.options:
        heading, desc, opts = multiconfig.options[groupname]
        for name, default, description in opts:
            name = groupname + '_' + name
            if isinstance(default, multiconfig.DefaultExpression):
                default = default.value
            settings[name] = default
    for groupname in multiconfig.options_no_group_name:
        heading, desc, opts = multiconfig.options_no_group_name[groupname]
        for name, default, description in opts:
            if isinstance(default, multiconfig.DefaultExpression):
                default = default.value
            settings[name] = default

    ret.extend([
        f.heading(1, 1, id='current_config'),
        f.text(_("Wiki configuration")),
        f.heading(0, 1),
        f.paragraph(1),
        _("This table shows all settings in this wiki that do not have default values. "
          "Settings that the configuration system doesn't know about are shown in ''italic'', "
          "those may be due to third-party extensions needing configuration or settings that "
          "were removed from Moin.",
          wiki=True),
        f.paragraph(0),
    ])
    ret.extend([
        f.table(1),
        f.table_row(1),
        f.table_cell(1), f.strong(1), f.text(_('Variable name')), f.strong(0), f.table_cell(0),
        f.table_cell(1), f.strong(1), f.text(_('Setting')), f.strong(0), f.table_cell(0),
        f.table_row(0),
    ])

    def iter_vnames(cfg):
        dedup = {}
        for name in cfg.__dict__:
            dedup[name] = True
            yield name, cfg.__dict__[name]
        for cls in cfg.__class__.mro():
            if cls == multiconfig.ConfigFunctionality:
                break
            for name in cls.__dict__:
                if not name in dedup:
                    dedup[name] = True
                    yield name, cls.__dict__[name]

    found = []
    for vname, value in iter_vnames(request.cfg):
        if hasattr(multiconfig.ConfigFunctionality, vname):
            continue
        if vname in settings and settings[vname] == value:
            continue
        found.append((vname, value))
    found.sort()
    for vname, value in found:
        vname = f.text(vname)
        if not vname in settings:
            vname = f.emphasis(1) + vname + f.emphasis(0)
        vtxt = '%r' % (value, )
        ret.extend([
            f.table_row(1),
            f.table_cell(1), vname, f.table_cell(0),
            f.table_cell(1), f.code(1, css="backtick"), f.text(vtxt), f.code(0), f.table_cell(0),
            f.table_row(0),
        ])
    ret.append(f.table(0))

    return ''.join(ret)

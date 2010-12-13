# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Wiki Configuration Help
"""
from MoinMoin.config import multiconfig

Dependencies = ['user'] # table headings are translated to user language
generates_headings = True

def macro_WikiConfigHelp(macro, section=None, show_heading=True, show_descriptions=True, heading_level=2):
    request = macro.request
    _ = request.getText
    f = macro.request.formatter
    ret = []

    groups = []
    if section and section in multiconfig.options:
        groups.append((section, True, multiconfig.options))
    else:
        for groupname in multiconfig.options:
            groups.append((groupname, True, multiconfig.options))
        for groupname in multiconfig.options_no_group_name:
            groups.append((groupname, False, multiconfig.options_no_group_name))
    groups.sort()

    for groupname, addgroup, optsdict in groups:
        heading, desc, opts = optsdict[groupname]
        if show_heading:
            ret.extend([
                f.heading(1, heading_level, id=groupname),
                ## XXX: translate description?
                f.text(heading),
                f.heading(0, heading_level),
            ])

        if desc and show_descriptions:
            ret.extend([
                f.paragraph(1),
                f.text(desc),
                f.paragraph(0)
            ])
        ret.extend([
            f.table(1),
            f.table_row(1, style="background-color: #ffffcc"),
            f.table_cell(1), f.strong(1), f.text(_('Variable name')), f.strong(0), f.table_cell(0),
            f.table_cell(1), f.strong(1), f.text(_('Default')), f.strong(0), f.table_cell(0),
            f.table_cell(1), f.strong(1), f.text(_('Description')), f.strong(0), f.table_cell(0),
            f.table_row(0),
        ])
        opts = list(opts)
        opts.sort()
        for name, default, description in opts:
            if addgroup:
                name = groupname + '_' + name
            if isinstance(default, multiconfig.DefaultExpression):
                default_txt = default.text
            else:
                default_txt = '%r' % (default, )
                if len(default_txt) <= 30:
                    default_txt = f.text(default_txt)
                else:
                    default_txt = f.span(1, title=default_txt) + f.text('...') + f.span(0)
                description = _(description or '', wiki=True)
            ret.extend([
                f.table_row(1),
                f.table_cell(1), f.text(name), f.table_cell(0),
                f.table_cell(1), f.code(1, css="backtick"), default_txt, f.code(0), f.table_cell(0),
                f.table_cell(1), description, f.table_cell(0),
                f.table_row(0),
            ])
        ret.append(f.table(0))

    return ''.join(ret)

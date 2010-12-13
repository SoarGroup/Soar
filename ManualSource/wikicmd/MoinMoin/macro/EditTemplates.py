# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Create a list of currentpage?action=edit&template=X links
    for all available templates X. Used by MissingPage.

    @copyright: 2004 Johannes Berg <johannes@sipsolutions.de>
    @license: GNU GPL, see COPYING for details.
"""

Dependencies = ["language"]

def macro_EditTemplates(macro):
    result = ''
    # we don't want to spend much CPU for spiders requesting nonexisting pages
    if not macro.request.isSpiderAgent:
        # Get list of template pages readable by current user
        filterfn = macro.request.cfg.cache.page_template_regexact.search
        templates = macro.request.rootpage.getPageList(filter=filterfn)
        result = []
        if templates:
            templates.sort()
            page = macro.formatter.page
            # send list of template pages
            result.append(macro.formatter.bullet_list(1))
            for template in templates:
                result.append(macro.formatter.listitem(1))
                result.append(page.link_to(macro.request, template, querystr={'action': 'edit', 'template': template}))
                result.append(macro.formatter.listitem(0))
            result.append(macro.formatter.bullet_list(0))
        result = ''.join(result)
    return result


# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Create an action link

    @copyright: 2004, 2007 Johannes Berg <johannes@sipsolutions.net>
                2007 by MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import wikiutil

Dependencies = ["language"]


def _get_valid_actions(macro):
    """ lists all valid actions """
    from MoinMoin import action
    # builtin
    actions_builtin = action.names
    # global
    actions_global = ([x for x in action.modules
                       if not x in macro.request.cfg.actions_excluded])
    # local
    actions_local = ([x for x in wikiutil.wikiPlugins('action', macro.cfg)
                      if not x in macro.request.cfg.actions_excluded])

    return actions_builtin + actions_global + actions_local

def macro_Action(macro, action=u'show', text=None, _kwargs=None):
    _ = macro.request.getText
    if text is None:
        text = action
    if not _kwargs:
        _kwargs = {}

    text = _(text)
    if action in _get_valid_actions(macro):
        page = macro.formatter.page
        _kwargs['action'] = action
        url = page.url(macro.request, querystr=_kwargs)
        return ''.join([
            macro.formatter.url(1, url, css='action'),
            macro.formatter.text(text),
            macro.formatter.url(0),
        ])
    else:
        return macro.formatter.text(text)

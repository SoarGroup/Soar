# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - (re)building of Xapian indices

    @copyright: 2007 MoinMoin:KarolNowak
    @license: GNU GPL, see COPYING for details.
"""

import MoinMoin.events as ev


def _get_index(request):
    try:
        from MoinMoin.search.Xapian import XapianIndex
        return XapianIndex(request)
    except ImportError:
        pass


def handle_renamed(event):
    """Updates Xapian index when a page changes its name"""

    request = event.request

    if request.cfg.xapian_search:
        index = _get_index(request)
        if index and index.exists():
            index.update_item(event.old_page.page_name, now=False)
            index.update_item(event.page.page_name)


def handle_copied(event):
    """Updates Xapian index when a page is copied"""

    request = event.request

    if request.cfg.xapian_search:
        index = _get_index(request)
        if index and index.exists():
            index.update_item(event.page.page_name)


def handle_changed(event):
    """Updates Xapian index when a page is changed"""

    request = event.request

    if request.cfg.xapian_search:
        index = _get_index(request)
        if index and index.exists():
            index.update_item(event.page.page_name)


def handle_deleted(event):
    """Updates Xapian index when a page is deleted"""
    event = ev.PageChangedEvent(event.request, event.page, event.comment)
    handle_changed(event)


def handle_attachment_change(event):
    """Updates Xapian index when attachment is added or removed"""

    request = event.request

    if request.cfg.xapian_search:
        index = _get_index(request)
        if index and index.exists():
            index.update_item(event.pagename, event.filename)


def handle(event):
    if isinstance(event, ev.PageRenamedEvent):
        handle_renamed(event)
    elif isinstance(event, ev.PageCopiedEvent):
        handle_copied(event)
    elif isinstance(event, (ev.PageChangedEvent, ev.TrivialPageChangedEvent)):
        handle_changed(event)
    elif isinstance(event, ev.PageDeletedEvent):
        handle_deleted(event)
    elif isinstance(event, (ev.FileAttachedEvent, ev.FileRemovedEvent)):
        handle_attachment_change(event)


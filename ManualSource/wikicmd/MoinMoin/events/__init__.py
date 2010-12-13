# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - event (notification) framework

    This code abstracts event handling in MoinMoin,
    currently for notifications. It implements the observer pattern.

    @copyright: 2007 by Karol Nowak <grywacz@gmail.com>
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import log
logging = log.getLogger(__name__)

from MoinMoin import wikiutil
from MoinMoin.util import pysupport
from MoinMoin.wikiutil import PluginAttributeError

# Create a list of extension actions from the package directory
modules = pysupport.getPackageModules(__file__)

# Dummy pseudo-getText function used in event descriptions,
# so that they get into .po files
_ = lambda x: x


class Event(object):
    """A class handling information common to all events."""

    # NOTE: each Event subclass must have a unique name attribute
    name = u"Event"

    def __init__(self, request):
        self.request = request


class PageEvent(Event):
    """An event related to a page change"""

    name = u"PageEvent"

    def __init__(self, request):
        Event.__init__(self, request)


class PageChangedEvent(PageEvent):

    name = u"PageChangedEvent"
    description = _(u"""Page has been modified""")
    req_superuser = False

    def __init__(self, request, page, comment):
        PageEvent.__init__(self, request)
        self.page = page
        self.comment = comment


class TrivialPageChangedEvent(PageEvent):

    name = u"TrivialPageChangedEvent"
    description = _(u"Page has been modified in a trivial fashion")
    req_superuser = False

    def __init__(self, request, page, comment):
        PageEvent.__init__(self, request)
        self.page = page
        self.comment = comment


class PageRenamedEvent(PageEvent):

    name = u"PageRenamedEvent"
    description = _(u"""Page has been renamed""")
    req_superuser = False

    def __init__(self, request, page, old_page, comment=""):
        PageEvent.__init__(self, request)
        self.page = page
        self.old_page = old_page
        self.comment = comment


class PageDeletedEvent(PageEvent):

    name = u"PageDeletedEvent"
    description = _(u"""Page has been deleted""")
    req_superuser = False

    def __init__(self, request, page, comment):
        PageEvent.__init__(self, request)
        self.page = page
        self.comment = comment


class PageCopiedEvent(PageEvent):

    name = u"PageCopiedEvent"
    description = _(u"""Page has been copied""")
    req_superuser = False

    def __init__(self, request, page, old_page, comment):
        PageEvent.__init__(self, request)
        self.page = page
        self.old_page = old_page
        self.comment = comment


class FileAttachedEvent(PageEvent):

    name = u"FileAttachedEvent"
    description = _(u"""A new attachment has been added""")
    req_superuser = False

    def __init__(self, request, pagename, filename, size):
        PageEvent.__init__(self, request)
        self.request = request
        self.pagename = pagename
        self.filename = filename
        self.size = size


class FileRemovedEvent(PageEvent):

    name = u"FileRemovedEvent"
    description = _(u"""An attachment has been removed""")
    req_superuser = False

    def __init__(self, request, pagename, filename, size):
        PageEvent.__init__(self, request)
        self.request = request
        self.pagename = pagename
        self.filename = filename
        self.size = size


class PageRevertedEvent(PageEvent):

    name = u"PageRevertedEvent"
    description = _(u"""A page has been reverted to a previous state""")
    req_superuser = False

    def __init__(self, request, pagename, previous, current):
        PageEvent.__init__(self, request)
        self.pagename = pagename
        self.previous = previous
        self.current = current


class SubscribedToPageEvent(PageEvent):

    name = u"SubscribedToPageEvent"
    description = _(u"""A user has subscribed to a page""")
    req_superuser = True

    def __init__(self, request, pagename, username):
        PageEvent.__init__(self, request)
        self.pagename = pagename
        self.username = username


class JabberIDSetEvent(Event):
    """ Sent when user changes her Jabber ID """

    def __init__(self, request, jid):
        Event.__init__(self, request)
        self.jid = jid


class JabberIDUnsetEvent(Event):
    """ Sent when Jabber ID is no longer used

    Obviously this will be usually sent along with JabberIDSetEvent,
    because we require user's jabber id to be unique by default.

    """
    def __init__(self, request, jid):
        Event.__init__(self, request)
        self.jid = jid


class UserCreatedEvent(Event):
    """ Sent when a new user has been created """

    name = u"UserCreatedEvent"
    description = _(u"""A new account has been created""")
    req_superuser = True

    def __init__(self, request, user):
        Event.__init__(self, request)
        self.user = user


class PagePreSaveEvent(Event):
    """ Event sent when a page is about to be saved

    This can be used to abort a save, for instance, if handler returns Abort.
    """

    name = u"PagePreSaveEvent"

    def __init__(self, request, page_editor, new_text):
        Event.__init__(self, request)
        self.page_editor = page_editor
        self.new_text = new_text


class EventResult:
    """ This is a base class for messages passed from event handlers """
    pass


class Abort(EventResult):
    """ Result returned if handler wants to abort operation that sent the event """
    def __init__(self, reason):
        """
        @param reason: human-readable reason of failure
        """
        self.reason = reason


def get_handlers(cfg):
    """Create a list of available event handlers.

    Each handler is a handle() function defined in a plugin,
    pretty much like in case of actions.

    TODO: maybe make it less dumb? ;-)
    """
    event_handlers = []
    names = wikiutil.getPlugins("events", cfg)

    for name in names:
        try:
            handler = wikiutil.importPlugin(cfg, "events", name, "handle")
        except PluginAttributeError:
            handler = None

        if handler is not None:
            event_handlers.append(handler)

    return event_handlers


def send_event(event):
    """Function called from outside to process an event

    @return: a list of messages returned by handlers
    @rtype: list
    """

    # A list of messages generated by event handlers, passed back to caller
    msg = []
    cfg = event.request.cfg

    # Try to handle the event with each available handler (for now)
    for handle in cfg.event_handlers:
        retval = handle(event)

        assert retval is None or isinstance(retval, EventResult)

        if retval:
            msg.append(retval)

    return msg


def get_subscribable_events():
    """Create and return a list of user-visible events

    @return: A list of user-visible events described by dictionaries
    @rtype: dict
    """
    defs = globals()
    subscribable_events = {}

    for ev in defs.values():
        if type(ev) is type and issubclass(ev, Event) and ev.__dict__.has_key("description") and ev.__dict__.has_key("name"):
            subscribable_events[ev.name] = {'desc': ev.description, 'superuser': ev.req_superuser}

    return subscribable_events

# Get rid of the dummy getText so that it doesn't get imported with *
del _


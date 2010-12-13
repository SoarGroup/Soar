# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - common functions for notification framework

    Code for building messages informing about events (changes)
    happening in the wiki.

    @copyright: 2007 by Karol Nowak <grywacz@gmail.com>
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import user, wikiutil
from MoinMoin.events import EventResult


class Result(EventResult):
    """ A base class for results of notification handlers"""
    pass


class Failure(Result):
    """ Used to report a failure in sending notifications """
    def __init__(self, reason, recipients = None):
        """
        @param recipients: a set of recipients
        @type recipients: set
        """
        self.reason = reason
        self.recipient = None

    def __str__(self):
        return self.reason or u""


class Success(Result):
    """ Used to indicate successfull notifications """

    def __init__(self, recipients):
        """
        @param recipients: a set of recipients
        @type recipients: set
        """
        self.recipients = recipients


class UnknownChangeType(Exception):
    """ Used to signal an invalid page change event """
    pass


def page_link(request, page, querystr):
    """Create an absolute url to a given page with optional action

    @param page: a page to link to
    @type page: MoinMoin.Page.Page
    @param querystr: a dict passed to wikiutil.makeQueryString

    """
    return request.getQualifiedURL(page.url(request, querystr))


def page_change_message(msgtype, request, page, lang, **kwargs):
    """Prepare a notification text for a page change of given type

    @param msgtype: a type of message to send (page_changed, page_renamed, ...)
    @type msgtype: str or unicode
    @param **kwargs: a dictionary of additional parameters, which depend on msgtype

    @return: dictionary containing data about the changed page
    @rtype: dict

    """
    _ = lambda text: request.getText(text, lang=lang)
    changes = {'page_name': page.page_name, 'revision': str(page.getRevList()[0])}

    if msgtype == "page_changed":
        revisions = kwargs['revisions']

    if msgtype == "page_changed":
        changes['text'] = _("Dear Wiki user,\n\n"
        'You have subscribed to a wiki page or wiki category on "%(sitename)s" for change notification.\n\n'
        'The "%(pagename)s" page has been changed by %(editor)s:\n') % {
            'pagename': page.page_name,
            'editor': page.uid_override or user.getUserIdentification(request),
            'sitename': page.cfg.sitename or request.url_root,
        }

        # append a diff (or append full page text if there is no diff)
        if len(revisions) < 2:
            changes['diff'] = _("New page:\n") + page.get_raw_body()
        else:
            lines = wikiutil.pagediff(request, page.page_name, revisions[1],
                                      page.page_name, revisions[0])
            if lines:
                changes['diff'] = '\n'.join(lines)
            else:
                changes['diff'] = _("No differences found!\n")

    elif msgtype == "page_deleted":
        changes['text'] = _("Dear wiki user,\n\n"
            'You have subscribed to a wiki page "%(sitename)s" for change notification.\n\n'
            'The page "%(pagename)s" has been deleted by %(editor)s:\n\n') % {
                'pagename': page.page_name,
                'editor': page.uid_override or user.getUserIdentification(request),
                'sitename': page.cfg.sitename or request.url_root,
        }

    elif msgtype == "page_renamed":
        changes['text'] = _("Dear wiki user,\n\n"
            'You have subscribed to a wiki page "%(sitename)s" for change notification.\n\n'
            'The page "%(pagename)s" has been renamed from "%(oldname)s" by %(editor)s:\n') % {
                'editor': page.uid_override or user.getUserIdentification(request),
                'pagename': page.page_name,
                'sitename': page.cfg.sitename or request.url_root,
                'oldname': kwargs['old_name']
        }

        changes['old_name'] = kwargs['old_name']

    else:
        raise UnknownChangeType()

    changes['editor'] = page.uid_override or user.getUserIdentification(request)
    if 'comment' in kwargs and kwargs['comment']:
        changes['comment'] = kwargs['comment']

    return changes


def user_created_message(request, _, sitename, username, email):
    """Formats a message used to notify about accounts being created

    @return: a dict containing message body and subject
    """
    subject = _("[%(sitename)s] New user account created") % {'sitename': sitename or "Wiki"}
    text = _("""Dear Superuser, a new user has just been created on "%(sitename)s". Details follow:

    User name: %(username)s
    Email address: %(useremail)s""") % {
         'username': username,
         'useremail': email,
         'sitename': sitename or "Wiki",
         }

    return {'subject': subject, 'text': text}


def attachment_added(request, _, page_name, attach_name, attach_size):
    """Formats a message used to notify about new attachments

    @param _: a gettext function
    @return: a dict with notification data

    """
    data = {}

    data['subject'] = _("[%(sitename)s] New attachment added to page %(pagename)s") % {
                'pagename': page_name,
                'sitename': request.cfg.sitename or request.url_root,
                }

    data['text'] = _("Dear Wiki user,\n\n"
    'You have subscribed to a wiki page "%(page_name)s" for change notification. '
    "An attachment has been added to that page by %(editor)s. "
    "Following detailed information is available:\n\n"
    "Attachment name: %(attach_name)s\n"
    "Attachment size: %(attach_size)s\n") % {
        'editor': user.getUserIdentification(request),
        'page_name': page_name,
        'attach_name': attach_name,
        'attach_size': attach_size,
    }

    data['editor'] = user.getUserIdentification(request)
    data['page_name'] = page_name
    data['attach_size'] = attach_size
    data['attach_name'] = attach_name

    return data


def attachment_removed(request, _, page_name, attach_name, attach_size):
    """Formats a message used to notify about removed attachments

    @param _: a gettext function
    @return: a dict with notification data

    """
    data = {}

    data['subject'] = _("[%(sitename)s] Removed attachment from page %(pagename)s") % {
                'pagename': page_name,
                'sitename': request.cfg.sitename or request.url_root,
                }

    data['text'] = _("Dear Wiki user,\n\n"
    'You have subscribed to a wiki page "%(page_name)s" for change notification. '
    "An attachment has been removed from that page by %(editor)s. "
    "Following detailed information is available:\n\n"
    "Attachment name: %(attach_name)s\n"
    "Attachment size: %(attach_size)s\n") % {
        'editor': user.getUserIdentification(request),
        'page_name': page_name,
        'attach_name': attach_name,
        'attach_size': attach_size,
    }

    data['editor'] = user.getUserIdentification(request)
    data['page_name'] = page_name
    data['attach_size'] = attach_size
    data['attach_name'] = attach_name

    return data


# XXX: clean up this method to take a notification type instead of bool for_jabber
def filter_subscriber_list(event, subscribers, for_jabber):
    """Filter a list of page subscribers to honor event subscriptions

    @param subscribers: list of subscribers (dict of lists, language is the key)
    @param for_jabber: require jid
    @type subscribers: dict

    """
    event_name = event.name

    # Filter the list by removing users who don't want to receive
    # notifications about this type of event
    for lang in subscribers.keys():
        userlist = []

        if for_jabber:
            for usr in subscribers[lang]:
                if usr.jid and event_name in usr.jabber_subscribed_events:
                    userlist.append(usr)
        else:
            for usr in subscribers[lang]:
                if usr.email and event_name in usr.email_subscribed_events:
                    userlist.append(usr)

        subscribers[lang] = userlist


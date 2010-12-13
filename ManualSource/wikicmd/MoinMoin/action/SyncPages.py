# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - SyncPages action

    This action allows you to synchronise pages of two wikis.

    @copyright: 2006 MoinMoin:AlexanderSchremmer
    @license: GNU GPL, see COPYING for details.
"""

import re
import traceback
import StringIO # not relevant for speed, so we do not need cStringIO


from MoinMoin import wikiutil
from MoinMoin.packages import unpackLine, packLine
from MoinMoin.PageEditor import PageEditor, conflict_markers
from MoinMoin.Page import Page
from MoinMoin.wikisync import TagStore, UnsupportedWikiException, SyncPage, NotAllowedException
from MoinMoin.wikisync import MoinLocalWiki, MoinRemoteWiki, UP, DOWN, BOTH, MIMETYPE_MOIN
from MoinMoin.support.python_compatibility import set
from MoinMoin.util.bdiff import decompress, patch, compress, textdiff
from MoinMoin.util import diff3, rpc_aggregator


debug = False


# map sync directions
directions_map = {"up": UP, "down": DOWN, "both": BOTH}


class ActionStatus(Exception):
    pass


class ActionClass(object):
    INFO, WARN, ERROR = zip(range(3), ("", "<!>", "/!\\")) # used for logging

    def __init__(self, pagename, request):
        self.request = request
        self.pagename = pagename
        self.page = PageEditor(request, pagename)
        self.status = []
        self.rollback = set()

    def log_status(self, level, message=u"", substitutions=(), raw_suffix=u""):
        """ Appends the message with a given importance level to the internal log. """
        if isinstance(message, str):
            message = message.decode("utf-8")
        if isinstance(raw_suffix, str):
            raw_suffix = raw_suffix.decode("utf-8")
        self.status.append((level, message, substitutions, raw_suffix))

    def register_rollback(self, func):
        self.rollback.add(func)

    def remove_rollback(self, func):
        self.rollback.remove(func)

    def call_rollback_funcs(self):
        _ = lambda x: x

        for func in self.rollback:
            try:
                page_name = func()
                self.log_status(self.INFO, _("Rolled back changes to the page %s."), (page_name, ))
            except Exception:
                temp_file = StringIO.StringIO()
                traceback.print_exc(file=temp_file)
                self.log_status(self.ERROR, _("Exception while calling rollback function:"), raw_suffix=temp_file.getvalue())

    def generate_log_table(self):
        """ Transforms self.status into a user readable table. """
        table_line = u"|| %(smiley)s || %(message)s%(raw_suffix)s ||"
        table = []

        for line in self.status:
            level, message, substitutions, raw_suffix = line
            if message:
                if substitutions:
                    macro_args = [message] + list(substitutions)
                    message = u"<<GetText2(|%s)>>" % (packLine(macro_args), )
                else:
                    message = u"<<GetText(%s)>>" % (message, )
            else:
                message = u""
            table.append(table_line % {"smiley": level[1],
                                       "message": message,
                                       "raw_suffix": raw_suffix.replace("\n", "<<BR>>")})

        return "\n".join(table)

    def parse_page(self):
        """ Parses the parameter page and returns the read arguments. """
        options = {
            "remotePrefix": "",
            "localPrefix": "",
            "remoteWiki": "",
            "pageMatch": None,
            "pageList": None,
            "groupList": None,
            "direction": "foo", # is defaulted below
            "user": "", # XXX should be refactored into a password agent or OpenID like solution
            "password": "",
        }

        options.update(self.request.dicts[self.pagename])

        # Convert page and group list strings to lists
        if options["pageList"] is not None:
            options["pageList"] = unpackLine(options["pageList"], ",")
        if options["groupList"] is not None:
            options["groupList"] = unpackLine(options["groupList"], ",")

        options["direction"] = directions_map.get(options["direction"].lower(), BOTH)

        return options

    def fix_params(self, params):
        """ Does some fixup on the parameters. """
        # merge the pageList case into the pageMatch case
        if params["pageList"] is not None:
            params["pageMatch"] = u'|'.join([r'^%s$' % re.escape(name)
                                             for name in params["pageList"]])

        if params["pageMatch"] is not None:
            params["pageMatch"] = re.compile(params["pageMatch"], re.U)

        # we do not support matching or listing pages if there is a group of pages
        if params["groupList"]:
            params["pageMatch"] = None
            params["pageList"] = None

        return params

    def show_password_form(self, name, password):
        _ = self.request.getText
        d = {"message": _(r"Please enter your password of your account at the remote wiki below. <<BR>> /!\ You should trust both wikis because the password could be read by the particular administrators.", wiki=True),
             "namelabel": _("Name"),
             "name": name,
             "passwordlabel": _("Password"),
             "password": password,
             "ticket": wikiutil.createTicket(self.request),
             "submit": _("Login"),
             "cancel": _("Cancel"),
        }
        html_form = """
%(message)s
<form method="post">
<div>
<input type="hidden" name="action" value="SyncPages">
<input type="hidden" name="ticket" value="%(ticket)s">
<label for="iName" style="font-weight: bold;">%(namelabel)s:</label>
<input type="text" name="name" id="iName" size="32" value="%(name)s">
</div>
<div>
<label for="iPassword" style="font-weight: bold;">%(passwordlabel)s:</label>
<input type="password" name="password" id="iPassword" size="32" value="%(password)s">
</div>
<div style="margin-top:1em; margin-bottom:1em;">
<div style="float:left">
<input type="submit" value="%(submit)s">
</div>
<div style="margin-left: 10em; margin-right: 10em;">
<input type="submit" value="%(cancel)s" name="cancel">
</div>
</div>
</form>
""" % d
        self.request.theme.add_msg(html_form, "dialog")
        self.page.send_page()

    def render(self):
        """ Render action

        This action returns a status message.
        """
        _ = self.request.getText

        params = self.fix_params(self.parse_page())

        if self.request.method != 'POST':
            # display the username / password dialog if we were just called by a GET request
            return self.show_password_form(params["user"], params["password"])

        try:
            if "cancel" in self.request.values:
                raise ActionStatus(_("Operation was canceled."), "error")

            if not wikiutil.checkTicket(self.request, self.request.form.get('ticket', '')):
                raise ActionStatus(_('Please use the interactive user interface to use action %(actionname)s!') % {'actionname': 'SyncPages' })

            name = self.request.form.get('name', '')
            password = self.request.form.get('password', '')

            if params["direction"] == UP:
                raise ActionStatus(_("The only supported directions are BOTH and DOWN."), "error")

            if not self.request.cfg.interwikiname:
                raise ActionStatus(_("Please set an interwikiname in your wikiconfig (see HelpOnConfiguration) to be able to use this action.", wiki=True), "error")

            if not params["remoteWiki"]:
                raise ActionStatus(_("Incorrect parameters. Please supply at least the ''remoteWiki'' parameter. Refer to HelpOnSynchronisation for help.", wiki=True), "error")

            local = MoinLocalWiki(self.request, params["localPrefix"], params["pageList"])
            try:
                remote = MoinRemoteWiki(self.request, params["remoteWiki"], params["remotePrefix"], params["pageList"], name, password, verbose=debug)
            except (UnsupportedWikiException, NotAllowedException), (msg, ):
                raise ActionStatus(msg, "error")

            if not remote.valid:
                raise ActionStatus(_("The ''remoteWiki'' is unknown.", wiki=True), "error")
        except ActionStatus, e:
            self.request.theme.add_msg(*e.args)
        else:
            try:
                try:
                    self.sync(params, local, remote)
                except Exception, e:
                    temp_file = StringIO.StringIO()
                    traceback.print_exc(file=temp_file)
                    self.log_status(self.ERROR, _("A severe error occurred:"), raw_suffix=temp_file.getvalue())
                    raise
                else:
                    self.request.theme.add_msg(u"%s" % (_("Synchronisation finished. Look below for the status messages."), ), "info")
            finally:
                self.call_rollback_funcs()
                # XXX aquire readlock on self.page
                self.page.saveText(self.page.get_raw_body() + "\n\n" + self.generate_log_table(), 0)
                # XXX release readlock on self.page

                remote.delete_auth_token()

        return self.page.send_page()

    def sync(self, params, local, remote):
        """ This method does the synchronisation work.
            Currently, it handles nearly all cases.
            The major missing part is rename handling.
            There are a few other cases left that have to be implemented:
                Wiki A    | Wiki B   | Remark
                ----------+----------+------------------------------
                exists    | non-     | Now the wiki knows that the page was renamed.
                with tags | existing | There should be an RPC method that asks
                          |          | for the new name (which could be recorded
                          |          | on page rename). Then the page is
                          |          | renamed in Wiki A as well and the sync
                          |          | is done normally.
                          |          | Every wiki retains a dict that maps
                          |          | (IWID, oldname) => newname and that is
                          |          | updated on every rename. oldname refers
                          |          | to the pagename known by the old wiki (can be
                          |          | gathered from tags).
                ----------+----------+-------------------------------
                exists    | any case | Try a rename search first, then
                          |          | do a sync without considering tags
                with tags | with non | to ensure data integrity.
                          | matching | Hmm, how do we detect this
                          | tags     | case if the unmatching tags are only
                          |          | on the remote side?
                ----------+----------+-------------------------------
        """
        _ = lambda x: x # we will translate it later

        direction = params["direction"]
        if direction == BOTH:
            match_direction = direction
        else:
            match_direction = None

        local_full_iwid = packLine([local.get_iwid(), local.get_interwiki_name()])
        remote_full_iwid = remote.iwid_full

        self.log_status(self.INFO, _("Synchronisation started -"), raw_suffix=" <<DateTime(%s)>>" % self.page._get_local_timestamp())

        l_pages = local.get_pages()
        r_pages = remote.get_pages(exclude_non_writable=direction != DOWN)

        if params["groupList"]:
            pages_from_groupList = set(local.getGroupItems(params["groupList"]))
            r_pages = SyncPage.filter(r_pages, pages_from_groupList.__contains__)
            l_pages = SyncPage.filter(l_pages, pages_from_groupList.__contains__)

        m_pages = [elem.add_missing_pagename(local, remote) for elem in SyncPage.merge(l_pages, r_pages)]

        self.log_status(self.INFO, _("Got a list of %s local and %s remote pages. This results in %s pages to process."),
                        (str(len(l_pages)), str(len(r_pages)), str(len(m_pages))))

        if params["pageMatch"]:
            m_pages = SyncPage.filter(m_pages, params["pageMatch"].match)
            self.log_status(self.INFO, _("After filtering: %s pages"), (str(len(m_pages)), ))

        class handle_page(rpc_aggregator.RPCYielder):
            def run(yielder, sp):
                # XXX add locking, acquire read-lock on sp
                if debug:
                    self.log_status(ActionClass.INFO, raw_suffix="Processing %r" % sp)

                local_pagename = sp.local_name
                if not self.request.user.may.write(local_pagename):
                    self.log_status(ActionClass.WARN, _("Skipped page %s because of no write access to local page."), (local_pagename, ))
                    return

                current_page = PageEditor(self.request, local_pagename) # YYY direct access
                comment = u"Local Merge - %r" % (remote.get_interwiki_name() or remote.get_iwid())

                tags = TagStore(current_page)

                matching_tags = tags.fetch(iwid_full=remote.iwid_full, direction=match_direction)
                matching_tags.sort()
                if debug:
                    self.log_status(ActionClass.INFO, raw_suffix="Tags: %r <<BR>> All: %r" % (matching_tags, tags.tags))

                # some default values for non matching tags
                normalised_name = None
                remote_rev = None
                local_rev = sp.local_rev # merge against the newest version
                old_contents = ""

                if matching_tags:
                    newest_tag = matching_tags[-1]

                    local_change = newest_tag.current_rev != sp.local_rev
                    remote_change = newest_tag.remote_rev != sp.remote_rev

                    # handle some cases where we cannot continue for this page
                    if not remote_change and (direction == DOWN or not local_change):
                        return # no changes done, next page
                    if sp.local_deleted and sp.remote_deleted:
                        return
                    if sp.remote_deleted and not local_change:
                        msg = local.delete_page(sp.local_name, comment)
                        if not msg:
                            self.log_status(ActionClass.INFO, _("Deleted page %s locally."), (sp.name, ))
                        else:
                            self.log_status(ActionClass.ERROR, _("Error while deleting page %s locally:"), (sp.name, ), msg)
                        return
                    if sp.local_deleted and not remote_change:
                        if direction == DOWN:
                            return
                        yield remote.delete_page_pre(sp.remote_name, sp.remote_rev, local_full_iwid)
                        msg = remote.delete_page_post(yielder.fetch_result())
                        if not msg:
                            self.log_status(ActionClass.INFO, _("Deleted page %s remotely."), (sp.name, ))
                        else:
                            self.log_status(ActionClass.ERROR, _("Error while deleting page %s remotely:"), (sp.name, ), msg)
                        return
                    if sp.local_mime_type != MIMETYPE_MOIN and not (local_change ^ remote_change):
                        self.log_status(ActionClass.WARN, _("The item %s cannot be merged automatically but was changed in both wikis. Please delete it in one of both wikis and try again."), (sp.name, ))
                        return
                    if sp.local_mime_type != sp.remote_mime_type:
                        self.log_status(ActionClass.WARN, _("The item %s has different mime types in both wikis and cannot be merged. Please delete it in one of both wikis or unify the mime type, and try again."), (sp.name, ))
                        return
                    if newest_tag.normalised_name != sp.name:
                        self.log_status(ActionClass.WARN, _("The item %s was renamed locally. This is not implemented yet. Therefore the full synchronisation history is lost for this page."), (sp.name, )) # XXX implement renames
                    else:
                        normalised_name = newest_tag.normalised_name
                        local_rev = newest_tag.current_rev
                        remote_rev = newest_tag.remote_rev
                        old_contents = Page(self.request, local_pagename, rev=newest_tag.current_rev).get_raw_body_str() # YYY direct access
                else:
                    if (sp.local_deleted and not sp.remote_rev) or (
                        sp.remote_deleted and not sp.local_rev):
                        return

                self.log_status(ActionClass.INFO, _("Synchronising page %s with remote page %s ..."), (local_pagename, sp.remote_name))

                if direction == DOWN:
                    remote_rev = None # always fetch the full page, ignore remote conflict check
                    patch_base_contents = ""
                else:
                    patch_base_contents = old_contents

                # retrieve remote contents diff
                if remote_rev != sp.remote_rev:
                    if sp.remote_deleted: # ignore remote changes
                        current_remote_rev = sp.remote_rev
                        is_remote_conflict = False
                        diff = None
                        self.log_status(ActionClass.WARN, _("The page %s was deleted remotely but changed locally."), (sp.name, ))
                    else:
                        yield remote.get_diff_pre(sp.remote_name, remote_rev, None, normalised_name)
                        diff_result = remote.get_diff_post(yielder.fetch_result())
                        if diff_result is None:
                            self.log_status(ActionClass.ERROR, _("The page %s could not be synced. The remote page was renamed. This is not supported yet. You may want to delete one of the pages to get it synced."), (sp.remote_name, ))
                            return
                        is_remote_conflict = diff_result["conflict"]
                        assert diff_result["diffversion"] == 1
                        diff = diff_result["diff"]
                        current_remote_rev = diff_result["current"]
                else:
                    current_remote_rev = remote_rev
                    if sp.local_mime_type == MIMETYPE_MOIN:
                        is_remote_conflict = wikiutil.containsConflictMarker(old_contents.decode("utf-8"))
                    else:
                        is_remote_conflict = NotImplemented
                    diff = None

                # do not sync if the conflict is remote and local, or if it is local
                # and the page has never been synchronised
                if (sp.local_mime_type == MIMETYPE_MOIN and wikiutil.containsConflictMarker(current_page.get_raw_body()) # YYY direct access
                    and (remote_rev is None or is_remote_conflict)):
                    self.log_status(ActionClass.WARN, _("Skipped page %s because of a locally or remotely unresolved conflict."), (local_pagename, ))
                    return

                if remote_rev is None and direction == BOTH:
                    self.log_status(ActionClass.INFO, _("This is the first synchronisation between the local and the remote wiki for the page %s."), (sp.name, ))

                # calculate remote page contents from diff
                if sp.remote_deleted:
                    remote_contents = ""
                elif diff is None:
                    remote_contents = old_contents
                else:
                    remote_contents = patch(patch_base_contents, decompress(diff))

                if diff is None: # only a local change
                    if debug:
                        self.log_status(ActionClass.INFO, raw_suffix="Only local changes for %r" % sp.name)
                    merged_text_raw = current_page.get_raw_body_str()
                    if sp.local_mime_type == MIMETYPE_MOIN:
                        merged_text = merged_text_raw.decode("utf-8")
                elif local_rev == sp.local_rev:
                    if debug:
                        self.log_status(ActionClass.INFO, raw_suffix="Only remote changes for %r" % sp.name)
                    merged_text_raw = remote_contents
                    if sp.local_mime_type == MIMETYPE_MOIN:
                        merged_text = merged_text_raw.decode("utf-8")
                else:
                    # this is guaranteed by a check above
                    assert sp.local_mime_type == MIMETYPE_MOIN
                    remote_contents_unicode = remote_contents.decode("utf-8")
                    # here, the actual 3-way merge happens
                    merged_text = diff3.text_merge(old_contents.decode("utf-8"), remote_contents_unicode, current_page.get_raw_body(), 1, *conflict_markers) # YYY direct access
                    if debug:
                        self.log_status(ActionClass.INFO, raw_suffix="Merging %r, %r and %r into %r" % (old_contents.decode("utf-8"), remote_contents_unicode, current_page.get_raw_body(), merged_text))
                    merged_text_raw = merged_text.encode("utf-8")

                # generate binary diff
                diff = textdiff(remote_contents, merged_text_raw)
                if debug:
                    self.log_status(ActionClass.INFO, raw_suffix="Diff against %r" % remote_contents)

                # XXX upgrade to write lock
                try:
                    local_change_done = True
                    current_page.saveText(merged_text, sp.local_rev or 0, comment=comment) # YYY direct access
                except PageEditor.Unchanged:
                    local_change_done = False
                except PageEditor.EditConflict:
                    local_change_done = False
                    assert False, "You stumbled on a problem with the current storage system - I cannot lock pages"

                new_local_rev = current_page.get_real_rev() # YYY direct access

                def rollback_local_change(): # YYY direct local access
                    comment = u"Wikisync rollback"
                    rev = new_local_rev - 1
                    revstr = '%08d' % rev
                    oldpg = Page(self.request, sp.local_name, rev=rev)
                    pg = PageEditor(self.request, sp.local_name)
                    if not oldpg.exists():
                        pg.deletePage(comment)
                    else:
                        try:
                            savemsg = pg.saveText(oldpg.get_raw_body(), 0, comment=comment, extra=revstr, action="SAVE/REVERT")
                        except PageEditor.Unchanged:
                            pass
                    return sp.local_name

                if local_change_done:
                    self.register_rollback(rollback_local_change)

                if direction == BOTH:
                    yield remote.merge_diff_pre(sp.remote_name, compress(diff), new_local_rev, current_remote_rev, current_remote_rev, local_full_iwid, sp.name)
                    try:
                        very_current_remote_rev = remote.merge_diff_post(yielder.fetch_result())
                    except NotAllowedException:
                        self.log_status(ActionClass.ERROR, _("The page %s could not be merged because you are not allowed to modify the page in the remote wiki."), (sp.name, ))
                        return
                else:
                    very_current_remote_rev = current_remote_rev


                if local_change_done:
                    self.remove_rollback(rollback_local_change)

                # this is needed at least for direction both and cgi sync to standalone for immutable pages on both
                # servers. It is not needed for the opposite direction
                try:
                    tags.add(remote_wiki=remote_full_iwid, remote_rev=very_current_remote_rev, current_rev=new_local_rev, direction=direction, normalised_name=sp.name)
                except:
                    self.log_status(ActionClass.ERROR, _("The page %s could not be merged because you are not allowed to modify the page in the remote wiki."), (sp.name, ))
                    return

                if sp.local_mime_type != MIMETYPE_MOIN or not wikiutil.containsConflictMarker(merged_text):
                    self.log_status(ActionClass.INFO, _("Page %s successfully merged."), (sp.name, ))
                elif is_remote_conflict:
                    self.log_status(ActionClass.WARN, _("Page %s contains conflicts that were introduced on the remote side."), (sp.name, ))
                else:
                    self.log_status(ActionClass.WARN, _("Page %s merged with conflicts."), (sp.name, ))

                # XXX release lock

        rpc_aggregator.scheduler(remote.create_multicall_object, handle_page, m_pages, 8, remote.prepare_multicall)


def execute(pagename, request):
    ActionClass(pagename, request).render()


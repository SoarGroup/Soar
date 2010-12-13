# -*- coding: iso-8859-1 -*-
"""
MoinMoin - wiki group backend

The wiki_groups backend allows to define groups on wiki pages. See
SystemPagesGroup as example of a group page.

Normally, the name of the group page has to end with Group like
FriendsGroup. This lets MoinMoin recognize it as a group. This default
pattern could be changed (e.g. for non-english languages etc.), see
HelpOnConfiguration.

MoinMoin.formatter.groups is used to extract group members from a
page.


@copyright: 2008 MoinMoin:ThomasWaldmann,
            2009 MoinMoin:DmitrijsMilajevs
@license: GPL, see COPYING for details
"""

from MoinMoin import caching, wikiutil
from MoinMoin.Page import Page
from MoinMoin.datastruct.backends import GreedyGroup, BaseGroupsBackend, GroupDoesNotExistError
from MoinMoin.formatter.groups import Formatter


class WikiGroup(GreedyGroup):

    def _load_group(self):
        request = self.request
        group_name = self.name

        page = Page(request, group_name)
        if page.exists():
            arena = 'pagegroups'
            key = wikiutil.quoteWikinameFS(group_name)
            cache = caching.CacheEntry(request, arena, key, scope='wiki', use_pickle=True)
            try:
                cache_mtime = cache.mtime()
                page_mtime = wikiutil.version2timestamp(page.mtime_usecs())
                # TODO: fix up-to-date check mtime granularity problems.
                #
                # cache_mtime is float while page_mtime is integer
                # The comparision needs to be done on the lowest type of both
                if int(cache_mtime) > int(page_mtime):
                    # cache is uptodate
                    return cache.content()
                else:
                    raise caching.CacheError
            except caching.CacheError:
                # either cache does not exist, is erroneous or not uptodate: recreate it
                members, member_groups = super(WikiGroup, self)._load_group()
                cache.update((members, member_groups))
                return members, member_groups
        else:
            raise GroupDoesNotExistError(group_name)


class WikiGroups(BaseGroupsBackend):

    def __contains__(self, group_name):
        return self.is_group_name(group_name) and Page(self.request, group_name).exists()

    def __iter__(self):
        """
        To find group pages, request.cfg.cache.page_group_regexact pattern is used.
        """
        return iter(self.request.rootpage.getPageList(user='', filter=self.page_group_regex.search))

    def __getitem__(self, group_name):
        return WikiGroup(request=self.request, name=group_name, backend=self)

    def _retrieve_members(self, group_name):
        """
        MoinMoin.formatter.groups is used to extract group members from a page.
        """
        formatter = Formatter(self.request)
        page = Page(self.request, group_name, formatter=formatter)

        request_page = getattr(self.request, "page", None)
        self.request.page = page
        # send_special is set to True because acl of the page should
        # not be processed to avoid infinite recursion in the
        # following case.
        #
        # Consider page UserGroup content:
        #
        # #acl UserGroup:read,write,admin All:read
        #
        #  * ExampleUser
        #  * TestGroup
        #
        page.send_page(content_only=True, send_special=True)

        if request_page:
            self.request.page = request_page
        else:
            del self.request.page

        return formatter.members


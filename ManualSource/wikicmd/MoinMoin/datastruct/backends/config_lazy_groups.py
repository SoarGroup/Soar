# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - config group lazy backend.

    The config group backend allows one to define groups in a
    configuration file.

    NOTE that this is proof-of-concept implementation. LDAP backend
    should be based on this concept.

    @copyright: 2009 MoinMoin:DmitrijsMilajevs
    @license: GPL, see COPYING for details
"""

from MoinMoin.datastruct.backends import LazyGroup, LazyGroupsBackend


class ConfigLazyGroup(LazyGroup):
    pass


class ConfigLazyGroups(LazyGroupsBackend):

    def __init__(self, request, groups):
        super(ConfigLazyGroups, self).__init__(request)

        self._groups = groups

    def __contains__(self, group_name):
        return group_name in self._groups

    def __iter__(self):
        return self._groups.iterkeys()

    def __getitem__(self, group_name):
        return ConfigLazyGroup(self.request, group_name, self)

    def _iter_group_members(self, group_name):
        if group_name in self:
            return self._groups[group_name].__iter__()

    def _group_has_member(self, group_name, member):
        return group_name in self and member in self._groups[group_name]

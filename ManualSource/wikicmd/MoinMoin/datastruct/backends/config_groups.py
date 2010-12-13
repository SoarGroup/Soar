# -*- coding: iso-8859-1 -*-
"""
MoinMoin - config groups backend

The config_groups backend enables one to define groups and their
members in a configuration file.

@copyright: 2009 MoinMoin:DmitrijsMilajevs
@license: GPL, see COPYING for details
"""

from MoinMoin.datastruct.backends import GreedyGroup, BaseGroupsBackend, GroupDoesNotExistError


class ConfigGroup(GreedyGroup):
    pass


class ConfigGroups(BaseGroupsBackend):

    def __init__(self, request, groups):
        """
        @param groups: Dictionary of groups where key is group name,
        and value is list of members of that group.
        """
        super(ConfigGroups, self).__init__(request)

        self._groups = groups

    def __contains__(self, group_name):
        return group_name in self._groups

    def __iter__(self):
        return self._groups.iterkeys()

    def __getitem__(self, group_name):
        return ConfigGroup(request=self.request, name=group_name, backend=self)

    def _retrieve_members(self, group_name):
        try:
            return self._groups[group_name]
        except KeyError:
            raise GroupDoesNotExistError(group_name)


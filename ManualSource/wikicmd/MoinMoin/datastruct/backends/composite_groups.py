# -*- coding: iso-8859-1 -*-
"""
MoinMoin - group access via various backends.

The composite_groups is a backend that does not have direct storage,
but composes other backends to a new one, so group definitions are
retrieved from several backends. This allows to mix different
backends.

@copyright: 2009 DmitrijsMilajevs
@license: GPL, see COPYING for details
"""

from MoinMoin.datastruct.backends import BaseGroupsBackend, GroupDoesNotExistError


class CompositeGroups(BaseGroupsBackend):
    """
    Manage several group backends.
    """

    def __init__(self, request, *backends):
        """
        @param backends: list of group backends which are used to get
                         access to the group definitions.
        """
        super(CompositeGroups, self).__init__(request)
        self._backends = backends

    def __getitem__(self, group_name):
        """
        Get a group by its name. First match counts.
        """
        for backend in self._backends:
            try:
                return backend[group_name]
            except GroupDoesNotExistError:
                pass
        raise GroupDoesNotExistError(group_name)

    def __iter__(self):
        """
        Iterate over group names in all backends (filtering duplicates).

        If a group with same name is defined in several backends, the
        composite_groups backend yields only backend which is listed
        earlier in self._backends.
        """
        yielded_groups = set()

        for backend in self._backends:
            for group_name in backend:
                if group_name not in yielded_groups:
                    yield group_name
                    yielded_groups.add(group_name)

    def __contains__(self, group_name):
        """
        Check if a group called group_name is available in any of the backends.

        @param group_name: name of the group [unicode]
        """
        for backend in self._backends:
            if group_name in backend:
                return True
        return False

    def __repr__(self):
        return "<%s backends=%s>" % (self.__class__, self._backends)


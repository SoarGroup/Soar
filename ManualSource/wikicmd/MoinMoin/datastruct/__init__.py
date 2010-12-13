# -*- coding: iso-8859-1 -*-
"""
MoinMoin - datastruct (groups and dicts) support.

@copyright: 2009 MoinMoin:DmitrijsMilajevs
@license: GPL, see COPYING for details
"""

from MoinMoin.datastruct.backends.wiki_dicts import WikiDicts
from MoinMoin.datastruct.backends.config_dicts import ConfigDicts
from MoinMoin.datastruct.backends.composite_dicts import CompositeDicts

from MoinMoin.datastruct.backends.wiki_groups import WikiGroups
from MoinMoin.datastruct.backends.config_groups import ConfigGroups
from MoinMoin.datastruct.backends.composite_groups import CompositeGroups

from MoinMoin.datastruct.backends import GroupDoesNotExistError
from MoinMoin.datastruct.backends import DictDoesNotExistError


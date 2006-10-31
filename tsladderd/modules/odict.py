# odict.py
# An Ordered Dictionary object
# Copyright (C) 2005 Nicola Larosa, Michael Foord
# E-mail: nico-NoSp@m-tekNico.net, fuzzyman AT voidspace DOT org DOT uk

# This software is licensed under the terms of the BSD license.
# http://www.voidspace.org.uk/documents/BSD-LICENSE.txt
# Basically you're free to copy, modify, distribute and relicense it,
# So long as you keep a copy of the license with it.

# Scripts maintained at http://www.voidspace.org.uk/python/index.shtml
# For information about bugfixes, updates and support, please join the
# Rest2Web mailing list:
# http://lists.sourceforge.net/lists/listinfo/rest2web-develop
# Comments, suggestions and bug reports welcome.

"""A dict that keeps keys in insertion order"""

__author__ = ('Nicola Larosa <nico-NoSp@m-tekNico.net>,'
    'Michael Foord <fuzzyman AT voidspace DOT org DOT uk>')

__docformat__ = "restructuredtext en"

__revision__ = '$Id: odict.py 129 2005-09-12 18:15:28Z teknico $'

__version__ = '0.1.2'

__all__ = ['OrderedDict']

from __future__ import generators

import sys
INTP_VER = sys.version_info[:2]
if INTP_VER < (2, 2):
    raise RuntimeError("Python v.2.2 or later needed")

class OrderedDict(dict):
    """
    A class of dictionary that keeps the insertion order of keys.
    
    All appropriate methods return keys, items, or values in an ordered way,
    following the order of the ``sequence`` attribute.
    
    All normal dictionary methods are available. Update and comparison is
    restricted to other OrderedDict objects.
    
    __contains__ tests:
    
    >>> d = OrderedDict(((1, 3),))
    >>> 1 in d
    1
    >>> 4 in d
    0
    
    __getitem__ tests:
    
    >>> OrderedDict(((1, 3), (3, 2), (2, 1)))[2]
    1
    >>> OrderedDict(((1, 3), (3, 2), (2, 1)))[4]
    Traceback (most recent call last):
    KeyError: 4
    
    __len__ tests:
    
    >>> len(OrderedDict())
    0
    >>> len(OrderedDict(((1, 3), (3, 2), (2, 1))))
    3
    
    get tests:
    
    >>> d = OrderedDict(((1, 3), (3, 2), (2, 1)))
    >>> d.get(1)
    3
    >>> d.get(4) is None
    1
    
    has_key tests:
    
    >>> d = OrderedDict(((1, 3), (3, 2), (2, 1)))
    >>> d.has_key(1)
    1
    >>> d.has_key(4)
    0
    """

    def __init__(self, init_val = ()):
        """
        Create a new ordered dictionary. Cannot init from a normal dict,
        nor from kwargs, since items order is undefined in those cases.
        
        >>> OrderedDict()
        {}
        >>> OrderedDict({1: 1})
        Traceback (most recent call last):
        TypeError: undefined order, cannot get items from dict
        >>> d = OrderedDict(((1, 3), (3, 2), (2, 1)))
        >>> d
        {1: 3, 3: 2, 2: 1}
        >>> OrderedDict(d)
        {1: 3, 3: 2, 2: 1}
        """
        dict.__init__(self)
        self.sequence = []
        self.update(init_val)

### Special methods ###

    def __cmp__(self, other):
        """
        Inequality undefined for OrderedDicts; equality managed by __eq__
        
        >>> OrderedDict() < OrderedDict()
        Traceback (most recent call last):
        TypeError: Inequality undefined for OrderedDicts
        >>> OrderedDict() < {}
        Traceback (most recent call last):
        TypeError: Inequality undefined for OrderedDicts
        >>> {} < OrderedDict()
        Traceback (most recent call last):
        TypeError: Inequality undefined for OrderedDicts
        """
        raise TypeError('Inequality undefined for OrderedDicts')

    def __delitem__(self, key):
        """
        >>> d = OrderedDict(((1, 3), (3, 2), (2, 1)))
        >>> del d[3]
        >>> d
        {1: 3, 2: 1}
        >>> del d[3]
        Traceback (most recent call last):
        KeyError: 3
        """
        # do the dict.__delitem__ *first* as it raises
        # the more appropriate error
        dict.__delitem__(self, key)
        self.sequence.remove(key)

    def __eq__(self, other):
        """
        >>> d = OrderedDict(((1, 3), (3, 2), (2, 1)))
        >>> d == OrderedDict(d)
        1
        >>> d == OrderedDict(((1, 3), (2, 1), (3, 2)))
        0
        >>> d == OrderedDict(((1, 0), (3, 2), (2, 1)))
        0
        >>> d == OrderedDict(((0, 3), (3, 2), (2, 1)))
        0
        >>> d == dict(d)
        Traceback (most recent call last):
        TypeError: Can only compare with other OrderedDicts
        """
        if not isinstance(other, OrderedDict):
            raise TypeError('Can only compare with other OrderedDicts')
        return (self.items() == other.items())

    def __repr__(self):
        """
        Used for __repr__ and __str__
        
        >>> r1 = repr(OrderedDict((('a', 'b'), ('c', 'd'), ('e', 'f'))))
        >>> r1
        "{'a': 'b', 'c': 'd', 'e': 'f'}"
        >>> r2 = repr(OrderedDict((('a', 'b'), ('e', 'f'), ('c', 'd'))))
        >>> r2
        "{'a': 'b', 'e': 'f', 'c': 'd'}"
        >>> r1 == str(OrderedDict((('a', 'b'), ('c', 'd'), ('e', 'f'))))
        1
        >>> r2 == str(OrderedDict((('a', 'b'), ('e', 'f'), ('c', 'd'))))
        1
        """
        return '{%s}' % ', '.join(
            ['%r: %r' % (key, self[key]) for key in self.sequence])

    def __setitem__(self, key, val):
        """
        >>> d = OrderedDict()
        >>> d['a'] = 'b'
        >>> d['b'] = 'a'
        >>> d[3] = 12
        >>> d
        {'a': 'b', 'b': 'a', 3: 12}
        """
        if not self.has_key(key):
            self.sequence.append(key)
        dict.__setitem__(self, key, val)

    __str__ = __repr__

### Read-only methods ###

    def copy(self):
        """
        >>> OrderedDict(((1, 3), (3, 2), (2, 1))).copy()
        {1: 3, 3: 2, 2: 1}
        """
        return OrderedDict(self)

    def items(self):
        """
        >>> OrderedDict(((1, 3), (3, 2), (2, 1))).items()
        [(1, 3), (3, 2), (2, 1)]
        """
        return zip(self.sequence, self.values())

    def keys(self):
        """
        >>> OrderedDict(((1, 3), (3, 2), (2, 1))).keys()
        [1, 3, 2]
        """
        return self.sequence[:]

    def values(self):
        """
        >>> OrderedDict(((1, 3), (3, 2), (2, 1))).values()
        [3, 2, 1]
        """
        return [self[key] for key in self.sequence]

    def iteritems(self):
        """
        >>> ii = OrderedDict(((1, 3), (3, 2), (2, 1))).iteritems()
        >>> ii.next()
        (1, 3)
        >>> ii.next()
        (3, 2)
        >>> ii.next()
        (2, 1)
        >>> ii.next()
        Traceback (most recent call last):
        StopIteration
        """
        def make_iter(self=self):
            keys = self.iterkeys()
            while True:
                key = keys.next()
                yield (key, self[key])
        return make_iter()

    def iterkeys(self):
        """
        >>> ii = OrderedDict(((1, 3), (3, 2), (2, 1))).iterkeys()
        >>> ii.next()
        1
        >>> ii.next()
        3
        >>> ii.next()
        2
        >>> ii.next()
        Traceback (most recent call last):
        StopIteration
        """
        return iter(self.sequence)

    __iter__ = iterkeys

    def itervalues(self):
        """
        >>> iv = OrderedDict(((1, 3), (3, 2), (2, 1))).itervalues()
        >>> iv.next()
        3
        >>> iv.next()
        2
        >>> iv.next()
        1
        >>> iv.next()
        Traceback (most recent call last):
        StopIteration
        """
        def make_iter(self=self):
            keys = self.iterkeys()
            while True:
                yield self[keys.next()]
        return make_iter()

### Read-write methods ###

    def clear(self):
        """
        >>> d = OrderedDict(((1, 3), (3, 2), (2, 1)))
        >>> d.clear()
        >>> d
        {}
        """
        dict.clear(self)
        self.sequence = []

    def pop(self, key, *args):
        """
        No dict.pop in Python 2.2, gotta reimplement it
        
        >>> d = OrderedDict(((1, 3), (3, 2), (2, 1)))
        >>> d.pop(3)
        2
        >>> d
        {1: 3, 2: 1}
        >>> d.pop(4)
        Traceback (most recent call last):
        KeyError: 4
        >>> d.pop(4, 0)
        0
        >>> d.pop(4, 0, 1)
        Traceback (most recent call last):
        TypeError: pop expected at most 2 arguments, got 3
        """
        if len(args) > 1:
            raise TypeError, ('pop expected at most 2 arguments, got %s' %
                (len(args) + 1))
        if key in self:
            val = self[key]
            del self[key]
        else:
            try:
                val = args[0]
            except IndexError:
                raise KeyError(key)
        return val

    def popitem(self):
        """
        Always delete and return the last item, not a random one as in dict
        
        >>> d = OrderedDict(((1, 3), (3, 2), (2, 1)))
        >>> d.popitem()
        (2, 1)
        >>> d
        {1: 3, 3: 2}
        >>> OrderedDict().popitem()
        Traceback (most recent call last):
        KeyError
        """
        try:
            key = self.sequence[-1]
        except IndexError:
            raise KeyError
        val = self[key]
        del self[key]
        return (key, val)

    def setdefault(self, key, defval = None):
        """
        >>> d = OrderedDict(((1, 3), (3, 2), (2, 1)))
        >>> d.setdefault(1)
        3
        >>> d.setdefault(4) is None
        1
        >>> d
        {1: 3, 3: 2, 2: 1, 4: None}
        >>> d.setdefault(5, 0)
        0
        >>> d
        {1: 3, 3: 2, 2: 1, 4: None, 5: 0}
        """
        if key in self:
            return self[key]
        else:
            self[key] = defval
            return defval

    def update(self, from_od):
        """
        Update from another OrderedDict or sequence of (key, value) pairs
        
        >>> d = OrderedDict()
        >>> d.update(OrderedDict(((1, 3), (3, 2), (2, 1))))
        >>> d
        {1: 3, 3: 2, 2: 1}
        >>> d.update({4: 4})
        Traceback (most recent call last):
        TypeError: undefined order, cannot get items from dict
        >>> d.update((4, 4))
        Traceback (most recent call last):
        TypeError: cannot convert dictionary update sequence element #0 to a sequence
        """
        if isinstance(from_od, OrderedDict):
            for key, val in from_od.items():
                self[key] = val
        elif isinstance(from_od, dict):
            # we lose compatibility with other ordered dict types this way
            raise TypeError('undefined order, cannot get items from dict')
        else:
            idx = 0
            # sequence of 2-item sequences, or error
            for item in from_od:
                try:
                    key, val = item
                except TypeError:
                    raise TypeError('cannot convert dictionary update'
                        ' sequence element #%d to a sequence' % idx)
                self[key] = val
                idx += 1

if __name__ == '__main__':
    # run the code tests in doctest format
    import doctest
    m = sys.modules.get('__main__')
    globs = m.__dict__.copy()
    globs.update({
        'INTP_VER': INTP_VER,
    })
    doctest.testmod(m, globs=globs)
    
"""
    CHANGELOG
    =========
    
    2005/09/10
    ----------
    
    By Nicola Larosa, based on code from Tim Wegener
      <twegener AT radlogic DOT com DOT au>
    
    Create itervalues and iteritems without creating the list up-front
    
    Added doctests for iter methods, and others.
    
    Optimized __setitem__ to be O(1) rather than O(N)
    
    Removed redefined methods that did not alter dict method behaviour,
      related doctests moved to the class docstring
    
    Added support for sequences of (key, value) pairs to update
    
    Removed redundant condition from __eq__
    
    Removed incorrect implementation of __str__
    
    2005/08/28
    ----------
    
    By Michael Foord
    
    Added __all__
    
    More than two arguments to ``pop`` now raises an error
    
    Version 0.1.0 finalised
    
    2005/08/13
    ----------
    
    By Nicola Larosa
    
    Added doctests everywhere, fixed much part of implementation
    
    Added comments at top, other doc vars
    
    2005/08/01
    ----------
    
    By Michael Foord
    
    Type tests changed to isinstance
    
    _keys changed to sequence attribute
    
    Allowed creating a dictionary by passing keyword arguments
    
    Shortened __repr__
    
    Fixed bug in popitem
    
    Other minor changes
"""


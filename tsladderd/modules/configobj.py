# configobj.py
# A config file reader/writer that supports nested sections in config files.
# Copyright (C) 2005 Michael Foord, Nicola Larosa
# E-mail: fuzzyman AT voidspace DOT org DOT uk
#         nico AT tekNico DOT net

# ConfigObj 4

# Released subject to the BSD License
# Please see http://www.voidspace.org.uk/documents/BSD-LICENSE.txt

# Scripts maintained at http://www.voidspace.org.uk/python/index.shtml
# For information about bugfixes, updates and support, please join the
# ConfigObj mailing list:
# http://lists.sourceforge.net/lists/listinfo/configobj-develop
# Comments, suggestions and bug reports welcome.

"""
    >>> z = ConfigObj()
    >>> z['a'] = 'a'
    >>> z['sect'] = {
    ...    'subsect': {
    ...         'a': 'fish',
    ...         'b': 'wobble',
    ...     },
    ...     'member': 'value',
    ... }
    >>> x = ConfigObj(z.write())
    >>> z == x
    1
"""

import sys
INTP_VER = sys.version_info[:2]
if INTP_VER < (2, 2):
    raise RuntimeError("Python v.2.2 or later needed")

import os, re
from types import StringTypes

# the UTF8 BOM - from codecs module
BOM_UTF8 = '\xef\xbb\xbf'

__version__ = '4.0.0'

__revision__ = '$Id: configobj.py 140 2005-10-20 10:04:58Z fuzzyman $'

__docformat__ = "restructuredtext en"

__all__ = (
    '__version__',
    'BOM_UTF8',
    'DEFAULT_INDENT_TYPE',
    'NUM_INDENT_SPACES',
    'MAX_INTERPOL_DEPTH',
    'ConfigObjError',
    'NestingError',
    'ParseError',
    'DuplicateError',
    'ConfigspecError',
    'ConfigObj',
    'SimpleVal',
    'InterpolationError',
    'InterpolationDepthError',
    'MissingInterpolationOption',
    'RepeatSectionError',
    '__docformat__',
)

DEFAULT_INDENT_TYPE = ' '
NUM_INDENT_SPACES = 4
MAX_INTERPOL_DEPTH = 10

OPTION_DEFAULTS = {
    'interpolation': True,
    'raise_errors': False,
    'list_values': True,
    'create_empty': False,
    'file_error': False,
    'configspec': None,
    'stringify': True,
    # option may be set to one of ('', ' ', '\t')
    'indent_type': None,
}

class ConfigObjError(SyntaxError):
    """
    This is the base class for all errors that ConfigObj raises.
    It is a subclass of SyntaxError.
    
    >>> raise ConfigObjError
    Traceback (most recent call last):
    ConfigObjError
    """
    def __init__(self, message='', line_number=None, line=''):
        self.line = line
        self.line_number = line_number
        self.message = message
        SyntaxError.__init__(self, message)

class NestingError(ConfigObjError):
    """
    This error indicates a level of nesting that doesn't match.
    
    >>> raise NestingError
    Traceback (most recent call last):
    NestingError
    """

class ParseError(ConfigObjError):
    """
    This error indicates that a line is badly written.
    It is neither a valid ``key = value`` line,
    nor a valid section marker line.
    
    >>> raise ParseError
    Traceback (most recent call last):
    ParseError
    """

class DuplicateError(ConfigObjError):
    """
    The keyword or section specified already exists.
    
    >>> raise DuplicateError
    Traceback (most recent call last):
    DuplicateError
    """

class ConfigspecError(ConfigObjError):
    """
    An error occured whilst parsing a configspec.
    
    >>> raise ConfigspecError
    Traceback (most recent call last):
    ConfigspecError
    """

class InterpolationError(ConfigObjError):
    """Base class for the two interpolation errors."""

class InterpolationDepthError(InterpolationError):
    """Maximum interpolation depth exceeded in string interpolation."""

    def __init__(self, option):
        """
        >>> raise InterpolationDepthError('yoda')
        Traceback (most recent call last):
        InterpolationDepthError: max interpolation depth exceeded in value "yoda".
        """
        InterpolationError.__init__(
            self,
            'max interpolation depth exceeded in value "%s".' % option)

class RepeatSectionError(ConfigObjError):
    """
    This error indicates additional sections in a section with a
    ``__many__`` (repeated) section.
    
    >>> raise RepeatSectionError
    Traceback (most recent call last):
    RepeatSectionError
    """

class MissingInterpolationOption(InterpolationError):
    """A value specified for interpolation was missing."""

    def __init__(self, option):
        """
        >>> raise MissingInterpolationOption('yoda')
        Traceback (most recent call last):
        MissingInterpolationOption: missing option "yoda" in interpolation.
        """
        InterpolationError.__init__(
            self,
            'missing option "%s" in interpolation.' % option)

class Section(dict):
    """
    A dictionary-like object that represents a section in a config file.
    
    It does string interpolation if the 'interpolate' attribute
    of the 'main' object is set to True.
    
    Interpolation is tried first from the 'DEFAULT' section of this object,
    next from the 'DEFAULT' section of the parent, lastly the main object.
    
    A Section will behave like an ordered dictionary - following the
    order of the ``scalars`` and ``sections`` attributes.
    You can use this to change the order of members.
    
    Iteration follows the order: scalars, then sections.
    """

    _KEYCRE = re.compile(r"%\(([^)]*)\)s|.")

    def __init__(self, parent, depth, main, indict=None, name=None):
        """
        parent is the section above
        depth is the depth level of this section
        main is the main ConfigObj
        indict is a dictionary to initialise the section with
        """
        if indict is None:
            indict = {}
        dict.__init__(self)
        # used for nesting level *and* interpolation
        self.parent = parent
        # used for the interpolation attribute
        self.main = main
        # level of nesting depth of this Section
        self.depth = depth
        # the sequence of scalar values in this Section
        self.scalars = []
        # the sequence of sections in this Section
        self.sections = []
        # purely for information
        self.name = name
        # for comments :-)
        self.comments = {}
        self.inline_comments = {}
        # for the configspec
        self.configspec = {}
        # for defaults
        self.defaults = []
        #
        # we do this explicitly so that __setitem__ is used properly
        # (rather than just passing to ``dict.__init__``)
        for entry in indict:
            self[entry] = indict[entry]

    def _interpolate(self, value):
        """Nicked from ConfigParser."""
        depth = MAX_INTERPOL_DEPTH
        # loop through this until it's done
        while depth:
            depth -= 1
            if value.find("%(") != -1:
                value = self._KEYCRE.sub(self._interpolation_replace, value)
            else:
                break
        else:
            raise InterpolationDepthError(value)
        return value

    def _interpolation_replace(self, match):
        """ """
        s = match.group(1)
        if s is None:
            return match.group()
        else:
            # switch off interpolation before we try and fetch anything !
            self.main.interpolation = False
            # try the 'DEFAULT' member of *this section* first
            val = self.get('DEFAULT', {}).get(s)
            # try the 'DEFAULT' member of the *parent section* next
            if val is None:
                val = self.parent.get('DEFAULT', {}).get(s)
            # last, try the 'DEFAULT' member of the *main section*
            if val is None:
                val = self.main.get('DEFAULT', {}).get(s)
            self.main.interpolation = True
            if val is None:
                raise MissingInterpolationOption(s)
            return val

    def __getitem__(self, key):
        """Fetch the item and do string interpolation."""
        val = dict.__getitem__(self, key)
        if self.main.interpolation and isinstance(val, StringTypes):
            return self._interpolate(val)
        return val

    def __setitem__(self, key, value):
        """
        Correctly set a value.
        
        Making dictionary values Section instances.
        (We have to special case 'Section' instances - which are also dicts)
        
        Keys must be strings.
        Values need only be strings (or lists of strings) if
        ``main.stringify`` is set.
        """
        if not isinstance(key, StringTypes):
            raise ValueError, 'The key "%s" is not a string.' % key
##        if self.depth is None:
##            self.depth = 0
        # add the comment
        if not self.comments.has_key(key):
            self.comments[key] = []
            self.inline_comments[key] = ''
        # remove the entry from defaults
        if key in self.defaults:
            self.defaults.remove(key)
        #
        if isinstance(value, Section):
            if not self.has_key(key):
                self.sections.append(key)
            dict.__setitem__(self, key, value)
        elif isinstance(value, dict):
            # First create the new depth level,
            # then create the section
            if not self.has_key(key):
                self.sections.append(key)
            new_depth = self.depth + 1
            dict.__setitem__(
                self,
                key,
                Section(
                    self,
                    new_depth,
                    self.main,
                    indict=value,
                    name=key))
        else:
            if not self.has_key(key):
                self.scalars.append(key)
            if not self.main.stringify:
                if isinstance(value, StringTypes):
                    pass
                elif isinstance(value, (list, tuple)):
                    for entry in value:
                        if not isinstance(entry, StringTypes):
                            raise TypeError, (
                                'Value is not a string "%s".' % entry)
                else:
                    raise TypeError, 'Value is not a string "%s".' % value
            dict.__setitem__(self, key, value)

    def __delitem__(self, key):
        """Remove items from the sequence when deleting."""
        dict. __delitem__(self, key)
        if key in self.scalars:
            self.scalars.remove(key)
        else:
            self.sections.remove(key)
        del self.comments[key]
        del self.inline_comments[key]

    def get(self, key, default=None):
        """A version of ``get`` that doesn't bypass string interpolation."""
        try:
            return self[key]
        except KeyError:
            return default

    def update(self, indict):
        """A version of update that uses our ``__setitem__``."""
        for entry in indict:
            self[entry] = indict[entry]

    def pop(self, key, *args):
        """ """
        val = dict.pop(self, key, *args)
        if key in self.scalars:
            del self.comments[key]
            del self.inline_comments[key]
            self.scalars.remove(key)
        elif key in self.sections:
            del self.comments[key]
            del self.inline_comments[key]
            self.sections.remove(key)
        if self.main.interpolation and isinstance(val, StringTypes):
            return self._interpolate(val)
        return val

    def popitem(self):
        """Pops the first (key,val)"""
        sequence = (self.scalars + self.sections)
        if not sequence:
            raise KeyError, ": 'popitem(): dictionary is empty'"
        key = sequence[0]
        val =  self[key]
        del self[key]
        return key, val

    def clear(self):
        """
        A version of clear that also affects scalars/sections
        Also clears comments and configspec.
        
        Leaves other attributes alone :
            depth/main/parent are not affected
        """
        dict.clear(self)
        self.scalars = []
        self.sections = []
        self.comments = {}
        self.inline_comments = {}
        self.configspec = {}

    def setdefault(self, key, default=None):
        """A version of setdefault that sets sequence if appropriate."""
        try:
            return self[key]
        except KeyError:
            self[key] = default
            return self[key]

    def items(self):
        """ """
        return zip((self.scalars + self.sections), self.values())

    def keys(self):
        """ """
        return (self.scalars + self.sections)

    def values(self):
        """ """
        return [self[key] for key in (self.scalars + self.sections)]

    def iteritems(self):
        """ """
        return iter(self.items())

    def iterkeys(self):
        """ """
        return iter((self.scalars + self.sections))

    __iter__ = iterkeys

    def itervalues(self):
        """ """
        return iter(self.values())

    def __repr__(self):
        return '{%s}' % ', '.join([('%s: %s' % (repr(key), repr(self[key])))
            for key in (self.scalars + self.sections)])

    __str__ = __repr__

    # Extra methods - not in a normal dictionary

    def dict(self):
        """
        Return a deepcopy of self as a dictionary.
        
        All members that are ``Section`` instances are recursively turned to
        ordinary dictionaries - by calling their ``dict`` method.
        
        >>> n = a.dict()
        >>> n == a
        1
        >>> n is a
        0
        """
        newdict = {}
        for entry in self:
            this_entry = self[entry]
            if isinstance(this_entry, Section):
                this_entry = this_entry.dict()
            elif isinstance(this_entry, (list, tuple)):
                # create a copy rather than a reference
                this_entry = list(this_entry)
            newdict[entry] = this_entry
        return newdict

    def rename(self, oldkey, newkey):
        """
        Change a keyname to another, without changing position in sequence.
        
        Implemented so that transformations can be made on keys,
        as well as on values. (used by encode and decode)
        
        Also renames comments.
        """
        if oldkey in self.scalars:
            the_list = self.scalars
        elif oldkey in self.sections:
            the_list = self.sections
        else:
            raise KeyError, 'Key "%s" not found.' % oldkey
        pos = the_list.index(oldkey)
        #
        val = self[oldkey]
        dict.__delitem__(self, oldkey)
        dict.__setitem__(self, newkey, val)
        the_list.remove(oldkey)
        the_list.insert(pos, newkey)
        comm = self.comments[oldkey]
        inline_comment = self.inline_comments[oldkey]
        del self.comments[oldkey]
        del self.inline_comments[oldkey]
        self.comments[newkey] = comm
        self.inline_comments[newkey] = inline_comment

    def walk(self, function, raise_errors=True,
            call_on_sections=False, **keywargs):
        """
        Walk every member and call a function on the keyword and value.
        
        Return a dictionary of the return values
        
        If the function raises an exception, raise the errror
        unless ``raise_errors=False``, in which case set the return value to
        ``False``.
        
        Any unrecognised keyword arguments you pass to walk, will be pased on
        to the function you pass in.
        
        Note: if ``call_on_sections`` is ``True`` then - on encountering a
        subsection, *first* the function is called for the *whole* subsection,
        and then recurses into it's members. This means your function must be
        able to handle strings, dictionaries and lists. This allows you
        to change the key of subsections as well as for ordinary members. The
        return value when called on the whole subsection has to be discarded.
        
        See  the encode and decode methods for examples, including functions.
        """
        out = {}
        # scalars first
        for entry in self.scalars[:]:
            try:
                out[entry] = function(self, entry, **keywargs)
            except Exception:
                if raise_errors:
                    raise
                else:
                    out[entry] = False
        # then sections
        for entry in self.sections[:]:
            if call_on_sections:
                try:
                    function(self, entry, **keywargs)
                except Exception:
                    if raise_errors:
                        raise
                    else:
                        out[entry] = False
            # previous result is discarded
            out[entry] = self[entry].walk(
                function,
                raise_errors=raise_errors,
                call_on_sections=call_on_sections,
                **keywargs)
        return out

    def decode(self, encoding):
        """
        Decode all strings and values to unicode, using the specified encoding.
        
        Works with subsections and list values.
        
        Uses the ``walk`` method.
        
        Testing ``encode`` and ``decode``.
        >>> m = ConfigObj(a)
        >>> m.decode('ascii')
        >>> def testuni(val):
        ...     for entry in val:
        ...         if not isinstance(entry, unicode):
        ...             print >> sys.stderr, type(entry)
        ...             raise AssertionError, 'decode failed.'
        ...         if isinstance(val[entry], dict):
        ...             testuni(val[entry])
        ...         elif not isinstance(val[entry], unicode):
        ...             raise AssertionError, 'decode failed.'
        >>> testuni(m)
        >>> m.encode('ascii')
        >>> a == m
        1
        """
        def decode(section, key, encoding=encoding):
            """ """
            val = section[key]
            if isinstance(val, (list, tuple)):
                newval = []
                for entry in val:
                    newval.append(entry.decode(encoding))
            elif isinstance(val, dict):
                newval = val
            else:
                newval = val.decode(encoding)
            newkey = key.decode(encoding)
            section.rename(key, newkey)
            section[newkey] = newval
        # using ``call_on_sections`` allows us to modify section names
        self.walk(decode, call_on_sections=True)

    def encode(self, encoding):
        """
        Encode all strings and values from unicode,
        using the specified encoding.
        
        Works with subsections and list values.
        Uses the ``walk`` method.
        """
        def encode(section, key, encoding=encoding):
            """ """
            val = section[key]
            if isinstance(val, (list, tuple)):
                newval = []
                for entry in val:
                    newval.append(entry.encode(encoding))
            elif isinstance(val, dict):
                newval = val
            else:
                newval = val.encode(encoding)
            newkey = key.encode(encoding)
            section.rename(key, newkey)
            section[newkey] = newval
        self.walk(encode, call_on_sections=True)

class ConfigObj(Section):
    """
    An object to read, create, and write config files.
    
    Testing with duplicate keys and sections.
    
    >>> c = '''
    ... [hello]
    ... member = value
    ... [hello again]
    ... member = value
    ... [ "hello" ]
    ... member = value
    ... '''
    >>> ConfigObj(c.split('\\n'), raise_errors = True)
    Traceback (most recent call last):
    DuplicateError: Duplicate section name at line 5.
    
    >>> d = '''
    ... [hello]
    ... member = value
    ... [hello again]
    ... member1 = value
    ... member2 = value
    ... 'member1' = value
    ... [ "and again" ]
    ... member = value
    ... '''
    >>> ConfigObj(d.split('\\n'), raise_errors = True)
    Traceback (most recent call last):
    DuplicateError: Duplicate keyword name at line 6.
    """

    _keyword = re.compile(r'''^ # line start
        (\s*)                   # indentation
        (                       # keyword
            (?:".*?")|          # double quotes
            (?:'.*?')|          # single quotes
            (?:[^'"=].*?)       # no quotes
        )
        \s*=\s*                 # divider
        (.*)                    # value (including list values and comments)
        $   # line end
        ''',
        re.VERBOSE)

    _sectionmarker = re.compile(r'''^
        (\s*)                     # 1: indentation
        ((?:\[\s*)+)              # 2: section marker open
        (                         # 3: section name open
            (?:"\s*\S.*?\s*")|    # at least one non-space with double quotes
            (?:'\s*\S.*?\s*')|    # at least one non-space with single quotes
            (?:[^'"\s].*?)        # at least one non-space unquoted
        )                         # section name close
        ((?:\s*\])+)              # 4: section marker close
        \s*(\#.*)?                # 5: optional comment
        $''',
        re.VERBOSE)

    # this regexp pulls list values out as a single string
    # or single values and comments
    _valueexp = re.compile(r'''^
        (?:
            (?:
                (
                    (?:
                        (?:
                            (?:".*?")|              # double quotes
                            (?:'.*?')|              # single quotes
                            (?:[^'",\#][^,\#]*?)       # unquoted
                        )
                        \s*,\s*                     # comma
                    )*      # match all list items ending in a comma (if any)
                )
                (
                    (?:".*?")|                      # double quotes
                    (?:'.*?')|                      # single quotes
                    (?:[^'",\#\s][^,]*?)             # unquoted
                )?          # last item in a list - or string value
            )|
            (,)             # alternatively a single comma - empty list
        )
        \s*(\#.*)?          # optional comment
        $''',
        re.VERBOSE)

    # use findall to get the members of a list value
    _listvalueexp = re.compile(r'''
        (
            (?:".*?")|          # double quotes
            (?:'.*?')|          # single quotes
            (?:[^'",\#].*?)       # unquoted
        )
        \s*,\s*                 # comma
        ''',
        re.VERBOSE)

    # this regexp is used for the value
    # when lists are switched off
    _nolistvalue = re.compile(r'''^
        (
            (?:".*?")|          # double quotes
            (?:'.*?')|          # single quotes
            (?:[^'"\#].*?)      # unquoted
        )
        \s*(\#.*)?              # optional comment
        $''',
        re.VERBOSE)

    # regexes for finding triple quoted values on one line
    _single_line_single = re.compile(r"^'''(.*?)'''\s*(#.*)?$")
    _single_line_double = re.compile(r'^"""(.*?)"""\s*(#.*)?$')
    _multi_line_single = re.compile(r"^(.*?)'''\s*(#.*)?$")
    _multi_line_double = re.compile(r'^(.*?)"""\s*(#.*)?$')

    _triple_quote = {
        "'''": (_single_line_single, _multi_line_single),
        '"""': (_single_line_double, _multi_line_double),
    }

    def __init__(self, infile=None, options=None, **kwargs):
        """
        Parse or create a config file object.
        
        ``ConfigObj(infile=None, options=None, **kwargs)``
        """
        if infile is None:
            infile = []
        if options is None:
            options = {}
        # keyword arguments take precedence over an options dictionary
        options.update(kwargs)
        # init the superclass
        Section.__init__(self, self, 0, self)
        #
        defaults = OPTION_DEFAULTS.copy()
        for entry in options.keys():
            if entry not in defaults.keys():
                raise TypeError, 'Unrecognised option "%s".' % entry
        # TODO: check the values too
        # add the explicit options to the defaults
        defaults.update(options)
        #
        # initialise a few variables
        self._errors = []
        self.raise_errors = defaults['raise_errors']
        self.interpolation = defaults['interpolation']
        self.list_values = defaults['list_values']
        self.create_empty = defaults['create_empty']
        self.file_error = defaults['file_error']
        self.stringify = defaults['stringify']
        self.indent_type = defaults['indent_type']
        # used by the write method
        self.BOM = None
        #
        self.initial_comment = []
        self.final_comment = []
        #
        if isinstance(infile, StringTypes):
            self.filename = os.path.abspath(infile)
            if os.path.isfile(self.filename):
                infile = open(self.filename).readlines()
            elif self.file_error:
                # raise an error if the file doesn't exist
                raise IOError, 'Config file not found: "%s".' % self.filename
            else:
                # file doesn't already exist
                if self.create_empty:
                    # this is a good test that the filename specified
                    # isn't impossible - like on a non existent device
                    h = open(self.filename)
                    h.write('')
                    h.close()
                infile = []
        elif isinstance(infile, (list, tuple)):
            self.filename = None
        elif isinstance(infile, dict):
            # initialise self
            # the Section class handles creating subsections
            if isinstance(infile, ConfigObj):
                # get a copy of our ConfigObj
                infile = infile.dict()
            for entry in infile:
                self[entry] = infile[entry]
            self.filename = None
            del self._errors
            if defaults['configspec'] is not None:
                self._handle_configspec(defaults['configspec'])
            else:
                self.configspec = None
            return
        elif hasattr(infile, 'seek'):
            # this supports StringIO instances and even file objects
            self.filename = infile
            infile.seek(0)
            infile = infile.readlines()
            self.filename.seek(0)
        else:
            raise TypeError, ('infile must be a filename,'
                ' StringIO instance, or a file as a list.')
        #
        # strip trailing '\n' from lines
        infile = [line.rstrip('\n') for line in infile]
        #
        # remove the UTF8 BOM if it is there
        # FIXME: support other BOM
        if infile and infile[0].startswith(BOM_UTF8):
            infile[0] = infile[0][3:]
            self.BOM = BOM_UTF8
        else:
            self.BOM = None
        #
        self._parse(infile)
        # if we had any errors, now is the time to raise them
        if self._errors:
            error = ConfigObjError("Parsing failed.")
            # set the errors attribute; it's a list of tuples:
            # (error_type, message, line_number)
            error.errors = self._errors
            # set the config attribute
            error.config = self
            raise error
        # delete private attributes
        del self._errors
        #
        if defaults['configspec'] is None:
            self.configspec = None
        else:
            self._handle_configspec(defaults['configspec'])

    def _parse(self, infile):
        """
        Actually parse the config file
        
        Testing Interpolation
        
        >>> c = ConfigObj()
        >>> c['DEFAULT'] = {
        ...     'b': 'goodbye',
        ...     'userdir': 'c:\\\\home',
        ...     'c': '%(d)s',
        ...     'd': '%(c)s'
        ... }
        >>> c['section'] = {
        ...     'a': '%(datadir)s\\\\some path\\\\file.py',
        ...     'b': '%(userdir)s\\\\some path\\\\file.py',
        ...     'c': 'Yo %(a)s',
        ...     'd': '%(not_here)s',
        ...     'e': '%(c)s',
        ... }
        >>> c['section']['DEFAULT'] = {
        ...     'datadir': 'c:\\\\silly_test',
        ...     'a': 'hello - %(b)s',
        ... }
        >>> c['section']['a'] == 'c:\\\\silly_test\\\\some path\\\\file.py'
        1
        >>> c['section']['b'] == 'c:\\\\home\\\\some path\\\\file.py'
        1
        >>> c['section']['c'] == 'Yo hello - goodbye'
        1
        
        Switching Interpolation Off
        
        >>> c.interpolation = False
        >>> c['section']['a'] == '%(datadir)s\\\\some path\\\\file.py'
        1
        >>> c['section']['b'] == '%(userdir)s\\\\some path\\\\file.py'
        1
        >>> c['section']['c'] == 'Yo %(a)s'
        1
        
        Testing the interpolation errors.
        
        >>> c.interpolation = True
        >>> c['section']['d']
        Traceback (most recent call last):
        MissingInterpolationOption: missing option "not_here" in interpolation.
        >>> c['section']['e']
        Traceback (most recent call last):
        InterpolationDepthError: max interpolation depth exceeded in value "%(c)s".
        
        Testing our quoting.
        
        >>> i._quote('\"""\'\'\'')
        Traceback (most recent call last):
        SyntaxError: EOF while scanning triple-quoted string
        >>> try:
        ...     i._quote('\\n', multiline=False)
        ... except ConfigObjError, e:
        ...    e.msg
        'Value "\\n" cannot be safely quoted.'
        >>> k._quote(' "\' ', multiline=False)
        Traceback (most recent call last):
        SyntaxError: EOL while scanning single-quoted string
        
        Testing with "stringify" off.
        >>> c.stringify = False
        >>> c['test'] = 1
        Traceback (most recent call last):
        TypeError: Value is not a string "1".
        """
        comment_list = []
        done_start = False
        this_section = self
        maxline = len(infile) - 1
        cur_index = -1
        reset_comment = False
        while cur_index < maxline:
            if reset_comment:
                comment_list = []
            cur_index += 1
            line = infile[cur_index]
            sline = line.strip()
            # do we have anything on the line ?
            if not sline or sline.startswith('#'):
                reset_comment = False
                comment_list.append(line)
                continue
            if not done_start:
                # preserve initial comment
                self.initial_comment = comment_list
                comment_list = []
                done_start = True
            reset_comment = True
            # first we check if it's a section marker
            mat = self._sectionmarker.match(line)
##            print >> sys.stderr, sline, mat
            if mat is not None:
                # is a section line
                (indent, sect_open, sect_name, sect_close, comment) = (
                    mat.groups())
                if indent and (self.indent_type is None):
                    self.indent_type = indent[0]
                cur_depth = sect_open.count('[')
                if cur_depth != sect_close.count(']'):
                    self._handle_error(
                        "Cannot compute the section depth at line %s.",
                        NestingError, infile, cur_index)
                    continue
                if cur_depth < this_section.depth:
                    # the new section is dropping back to a previous level
                    try:
                        parent = self._match_depth(
                            this_section,
                            cur_depth).parent
                    except SyntaxError:
                        self._handle_error(
                            "Cannot compute nesting level at line %s.",
                            NestingError, infile, cur_index)
                        continue
                elif cur_depth == this_section.depth:
                    # the new section is a sibling of the current section
                    parent = this_section.parent
                elif cur_depth == this_section.depth + 1:
                    # the new section is a child the current section
                    parent = this_section
                else:
                    self._handle_error(
                        "Section too nested at line %s.",
                        NestingError, infile, cur_index)
                #
                sect_name = self._unquote(sect_name)
                if parent.has_key(sect_name):
##                    print >> sys.stderr, sect_name
                    self._handle_error(
                        'Duplicate section name at line %s.',
                        DuplicateError, infile, cur_index)
                    continue
                # create the new section
                this_section = Section(
                    parent,
                    cur_depth,
                    self,
                    name=sect_name)
                parent[sect_name] = this_section
                parent.inline_comments[sect_name] = comment
                parent.comments[sect_name] = comment_list
##                print >> sys.stderr, parent[sect_name] is this_section
                continue
            #
            # it's not a section marker,
            # so it should be a valid ``key = value`` line
            mat = self._keyword.match(line)
##            print >> sys.stderr, sline, mat
            if mat is not None:
                # is a keyword value
                # value will include any inline comment
                (indent, key, value) = mat.groups()
                if indent and (self.indent_type is None):
                    self.indent_type = indent[0]
                # check for a multiline value
                if value[:3] in ['"""', "'''"]:
                    try:
                        (value, comment, cur_index) = self._multiline(
                            value, infile, cur_index, maxline)
                    except SyntaxError:
                        self._handle_error(
                            'Parse error in value at line %s.',
                            ParseError, infile, cur_index)
                        continue
                else:
                    # extract comment and lists
                    try:
                        (value, comment) = self._handle_value(value)
                    except SyntaxError:
                        self._handle_error(
                            'Parse error in value at line %s.',
                            ParseError, infile, cur_index)
                        continue
                #
##                print >> sys.stderr, sline
                key = self._unquote(key)
                if this_section.has_key(key):
                    self._handle_error(
                        'Duplicate keyword name at line %s.',
                        DuplicateError, infile, cur_index)
                    continue
                # add the key
##                print >> sys.stderr, this_section.name
                this_section[key] = value
                this_section.inline_comments[key] = comment
                this_section.comments[key] = comment_list
##                print >> sys.stderr, key, this_section[key]
##                if this_section.name is not None:
##                    print >> sys.stderr, this_section
##                    print >> sys.stderr, this_section.parent
##                    print >> sys.stderr, this_section.parent[this_section.name]
                continue
            #
            # it neither matched as a keyword
            # or a section marker
            self._handle_error(
                'Invalid line at line "%s".',
                ParseError, infile, cur_index)
        if self.indent_type is None:
            # no indentation used, set the type accordingly
            self.indent_type = ''
        # preserve the final comment
        self.final_comment = comment_list

    def _match_depth(self, sect, depth):
        """
        Given a section and a depth level, walk back through the sections
        parents to see if the depth level matches a previous section.
        
        Return a reference to the right section,
        or raise a SyntaxError.
        """
        while depth < sect.depth:
            if sect is sect.parent:
                # we've reached the top level already
                raise SyntaxError
            sect = sect.parent
        if sect.depth == depth:
            return sect
        # shouldn't get here
        raise SyntaxError

    def _handle_error(self, text, ErrorClass, infile, cur_index):
        """
        Handle an error according to the error settings.
        
        Either raise the error or store it.
        The error will have occured at ``cur_index``
        """
        line = infile[cur_index]
        message = text % cur_index
        error = ErrorClass(message, cur_index, line)
        if self.raise_errors:
            # raise the error - parsing stops here
            raise error
        # store the error
        # reraise when parsing has finished
        self._errors.append(error)

    def _unquote(self, value):
        """Return an unquoted version of a value"""
        if (value[0] == value[-1]) and (value[0] in ('"', "'")):
            value = value[1:-1]
        return value

    def _quote(self, value, multiline=True):
        """
        Return a safely quoted version of a value.
        
        Raise a ConfigObjError if the value cannot be safely quoted.
        If multiline is ``True`` (default) then use triple quotes
        if necessary.
        
        Don't quote values that don't need it.
        Recursively quote members of a list and return a comma joined list.
        Multiline is ``False`` for lists.
        Obey list syntax for empty and single member lists.
        """
        if isinstance(value, (list, tuple)):
            if not value:
                return ','
            elif len(value) == 1:
                return self._quote(value[0], multiline=False) + ','
            return ','.join([self._quote(val, multiline=False)
                for val in value])
        if not isinstance(value, StringTypes):
            if self.stringify:
                value = str(value)
            else:
                raise TypeError, 'Value "%s" is not a string.' % value
        squot = "'%s'"
        dquot = '"%s"'
        noquot = "%s"
        wspace_plus = ' \r\t\n\v\t\'"'
        tsquot = '"""%s"""'
        tdquot = "'''%s'''"
        if not value:
            return '""'
        if not (multiline and
                ((("'" in value) and ('"' in value)) or ('\n' in value))):
            # for normal values either single or double quotes will do
            if '\n' in value:
                raise ConfigObjError, ('Value "%s" cannot be safely quoted.' %
                    value)
            if ((value[0] not in wspace_plus) and
                    (value[-1] not in wspace_plus) and
                    (',' not in value)):
                quot = noquot
            else:
                if ("'" in value) and ('"' in value):
                    raise ConfigObjError, (
                        'Value "%s" cannot be safely quoted.' % value)
                elif '"' in value:
                    quot = squot
                else:
                    quot = dquot
        else:
            # if value has '\n' or "'" *and* '"', it will need triple quotes
            if (value.find('"""') != -1) and (value.find("'''") != -1):
                raise ConfigObjError, (
                    'Value "%s" cannot be safely quoted.' % value)
            if value.find('"""') == -1:
                quot = tdquot
            else:
                quot = tsquot
        return quot % value

    def _handle_value(self, value):
        """
        Given a value string, unquote, remove comment,
        handle lists. (including empty and single member lists)
        
        Testing list values.
        
        >>> testconfig3 = '''
        ... a = ,
        ... b = test,
        ... c = test1, test2   , test3
        ... d = test1, test2, test3,
        ... '''
        >>> d = ConfigObj(testconfig3.split('\\n'), raise_errors=True)
        >>> d['a'] == []
        1
        >>> d['b'] == ['test']
        1
        >>> d['c'] == ['test1', 'test2', 'test3']
        1
        >>> d['d'] == ['test1', 'test2', 'test3']
        1
        
        Testing with list values off.
        
        >>> e = ConfigObj(
        ...     testconfig3.split('\\n'),
        ...     raise_errors=True,
        ...     list_values=False)
        >>> e['a'] == ','
        1
        >>> e['b'] == 'test,'
        1
        >>> e['c'] == 'test1, test2   , test3'
        1
        >>> e['d'] == 'test1, test2, test3,'
        1
        
        Testing creating from a dictionary.
        
        >>> f = {
        ...     'key1': 'val1',
        ...     'key2': 'val2',
        ...     'section 1': {
        ...         'key1': 'val1',
        ...         'key2': 'val2',
        ...         'section 1b': {
        ...             'key1': 'val1',
        ...             'key2': 'val2',
        ...         },
        ...     },
        ...     'section 2': {
        ...         'key1': 'val1',
        ...         'key2': 'val2',
        ...         'section 2b': {
        ...             'key1': 'val1',
        ...             'key2': 'val2',
        ...         },
        ...     },
        ...      'key3': 'val3',
        ... }
        >>> g = ConfigObj(f)
        >>> f == g
        1
        
        Testing we correctly detect badly built list values (4 of them).
        
        >>> testconfig4 = '''
        ... config = 3,4,,
        ... test = 3,,4
        ... fish = ,,
        ... dummy = ,,hello, goodbye
        ... '''
        >>> try:
        ...     ConfigObj(testconfig4.split('\\n'))
        ... except ConfigObjError, e:
        ...     len(e.errors)
        4
        
        Testing we correctly detect badly quoted values (4 of them).
        
        >>> testconfig5 = '''
        ... config = "hello   # comment
        ... test = 'goodbye
        ... fish = 'goodbye   # comment
        ... dummy = "hello again
        ... '''
        >>> try:
        ...     ConfigObj(testconfig5.split('\\n'))
        ... except ConfigObjError, e:
        ...     len(e.errors)
        4
        """
        # do we look for lists in values ?
        if not self.list_values:
            mat = self._nolistvalue.match(value)
            if mat is None:
                raise SyntaxError
            (value, comment) = mat.groups()
            # FIXME: unquoting here can be a source of error
            return (self._unquote(value), comment)
        mat = self._valueexp.match(value)
        if mat is None:
            # the value is badly constructed, probably badly quoted,
            # or an invalid list
            raise SyntaxError
        (list_values, single, empty_list, comment) = mat.groups()
        if (list_values == '') and (single is None):
            # change this if you want to accept empty values
            raise SyntaxError
        # NOTE: note there is no error handling from here if the regex
        # is wrong: then incorrect values will slip through
        if empty_list is not None:
            # the single comma - meaning an empty list
            return ([], comment)
        if single is not None:
            single = self._unquote(single)
        if list_values == '':
            # not a list value
            return (single, comment)
        the_list = self._listvalueexp.findall(list_values)
        the_list = [self._unquote(val) for val in the_list]
        if single is not None:
            the_list += [single]
        return (the_list, comment)

    def _multiline(self, value, infile, cur_index, maxline):
        """
        Extract the value, where we are in a multiline situation
        
        Testing multiline values.
        
        >>> i == {
        ...     'name4': ' another single line value ',
        ...     'multi section': {
        ...         'name4': '\\n        Well, this is a\\n        multiline '
        ...             'value\\n        ',
        ...         'name2': '\\n        Well, this is a\\n        multiline '
        ...             'value\\n        ',
        ...         'name3': '\\n        Well, this is a\\n        multiline '
        ...             'value\\n        ',
        ...         'name1': '\\n        Well, this is a\\n        multiline '
        ...             'value\\n        ',
        ...     },
        ...     'name2': ' another single line value ',
        ...     'name3': ' a single line value ',
        ...     'name1': ' a single line value ',
        ... }
        1
        """
        quot = value[:3]
        newvalue = value[3:]
        single_line = self._triple_quote[quot][0]
        multi_line = self._triple_quote[quot][1]
        mat = single_line.match(value)
        if mat is not None:
            retval = list(mat.groups())
            retval.append(cur_index)
            return retval
        elif newvalue.find(quot) != -1:
            # somehow the triple quote is missing
            raise SyntaxError
        #
        while cur_index < maxline:
            cur_index += 1
            newvalue += '\n'
            line = infile[cur_index]
            if line.find(quot) == -1:
                newvalue += line
            else:
                # end of multiline, process it
                break
        else:
            # we've got to the end of the config, oops...
            raise SyntaxError
        mat = multi_line.match(line)
        if mat is None:
            # a badly formed line
            raise SyntaxError
        (value, comment) = mat.groups()
        return (newvalue + value, comment, cur_index)

    def _handle_configspec(self, configspec):
        """Parse the configspec."""
        try:
            configspec = ConfigObj(
                configspec,
                raise_errors=True,
                file_error=True,
                list_values=False)
        except ConfigObjError, e:
            # FIXME: Should these errors have a reference
            # to the already parsed ConfigObj ?
            raise ConfigspecError('Parsing configspec failed: %s' % e)
        except IOError, e:
            raise IOError('Reading configspec failed: %s' % e)
        self._set_configspec_value(configspec, self)

    def _set_configspec_value(self, configspec, section):
        """Used to recursively set configspec values."""
        if '__many__' in configspec.sections:
            section.configspec['__many__'] = configspec['__many__']
            if len(configspec.sections) > 1:
                # FIXME: can we supply any useful information here ?
                raise RepeatSectionError
        for entry in configspec.scalars:
            section.configspec[entry] = configspec[entry]
        for entry in configspec.sections:
            if entry == '__many__':
                continue
            if not section.has_key(entry):
                section[entry] = {}
            self._set_configspec_value(configspec[entry], section[entry])

    def _handle_repeat(self, section, configspec):
        """Dynamically assign configspec for repeated section."""
        try:
            section_keys = configspec.sections
            scalar_keys = configspec.scalars
        except AttributeError:
            section_keys = [entry for entry in configspec 
                                if isinstance(configspec[entry], dict)]
            scalar_keys = [entry for entry in configspec 
                                if not isinstance(configspec[entry], dict)]
        if '__many__' in section_keys and len(section_keys) > 1:
            # FIXME: can we supply any useful information here ?
            raise RepeatSectionError
        scalars = {}
        sections = {}
        for entry in scalar_keys:
            val = configspec[entry]
            scalars[entry] = val
        for entry in section_keys:
            val = configspec[entry]
            if entry == '__many__':
                scalars[entry] = val
                continue
            sections[entry] = val
        #
        section.configspec = scalars
        for entry in sections:
            if not section.has_key(entry):
                section[entry] = {}
            self._handle_repeat(section[entry], sections[entry])

    def _write_line(self, indent_string, entry, this_entry, comment):
        """Write an individual line, for the write method"""
        return '%s%s = %s%s' % (
            indent_string,
            self._quote(entry, multiline=False),
            self._quote(this_entry),
            comment)

    def _write_marker(self, indent_string, depth, entry, comment):
        """Write a section marker line"""
        return '%s%s%s%s%s' % (
            indent_string,
            '[' * depth,
            self._quote(entry, multiline=False),
            ']' * depth,
            comment)

    def _handle_comment(self, comment):
        """
        Deal with a comment.
        
        >>> filename = a.filename
        >>> a.filename = None
        >>> values = a.write()
        >>> index = 0
        >>> while index < 23:
        ...     index += 1
        ...     line = values[index-1]
        ...     assert line.endswith('# comment ' + str(index))
        >>> a.filename = filename
        
        >>> start_comment = ['# Initial Comment', '', '#']
        >>> end_comment = ['', '#', '# Final Comment']
        >>> newconfig = start_comment + testconfig1.split('\\n') + end_comment
        >>> nc = ConfigObj(newconfig)
        >>> nc.initial_comment
        ['# Initial Comment', '', '#']
        >>> nc.final_comment
        ['', '#', '# Final Comment']
        >>> nc.initial_comment == start_comment
        1
        >>> nc.final_comment == end_comment
        1
        """
        if not comment:
            return ''
        if self.indent_type == '\t':
            start = '\t'
        else:
            start = ' ' * NUM_INDENT_SPACES
        if not comment.startswith('#'):
            start += '# '
        return (start + comment)

    def _compute_indent_string(self, depth):
        """
        Compute the indent string, according to current indent_type and depth
        """
        if self.indent_type == '':
            # no indentation at all
            return ''
        if self.indent_type == '\t':
            return '\t' * depth
        if self.indent_type == ' ':
            return ' ' * NUM_INDENT_SPACES * depth
        raise SyntaxError

    # Public methods

    def write(self, section=None):
        """
        Write the current ConfigObj as a file
        
        tekNico: FIXME: use StringIO instead of real files
        
        >>> filename = a.filename
        >>> a.filename = 'test.ini'
        >>> a.write()
        >>> a.filename = filename
        >>> a == ConfigObj('test.ini', raise_errors=True)
        1
        >>> os.remove('test.ini')
        >>> b.filename = 'test.ini'
        >>> b.write()
        >>> b == ConfigObj('test.ini', raise_errors=True)
        1
        >>> os.remove('test.ini')
        >>> i.filename = 'test.ini'
        >>> i.write()
        >>> i == ConfigObj('test.ini', raise_errors=True)
        1
        >>> os.remove('test.ini')
        >>> a = ConfigObj()
        >>> a['DEFAULT'] = {'a' : 'fish'}
        >>> a['a'] = '%(a)s'
        >>> a.write()
        ['a = %(a)s', '[DEFAULT]', 'a = fish']
        """
        int_val = 'test'
        if self.indent_type is None:
            # this can be true if initialised from a dictionary
            self.indent_type = DEFAULT_INDENT_TYPE
        #
        out = []
        return_list = True
        if section is None:
            int_val = self.interpolation
            self.interpolation = False
            section = self
            return_list = False
            for line in self.initial_comment:
                stripped_line = line.strip()
                if stripped_line and not stripped_line.startswith('#'):
                    line = '# ' + line
                out.append(line)
        #
        indent_string = self._compute_indent_string(section.depth)
        for entry in (section.scalars + section.sections):
            if entry in section.defaults:
                # don't write out default values
                continue
            for comment_line in section.comments[entry]:
                comment_line = comment_line.lstrip()
                if comment_line and not comment_line.startswith('#'):
                    comment_line = '#' + comment_line
                out.append(indent_string + comment_line)
            this_entry = section[entry]
            comment = self._handle_comment(section.inline_comments[entry])
            #
            if isinstance(this_entry, dict):
                # a section
                out.append(self._write_marker(
                    indent_string,
                    this_entry.depth,
                    entry,
                    comment))
                out.extend(self.write(this_entry))
            else:
                out.append(self._write_line(
                    indent_string,
                    entry,
                    this_entry,
                    comment))
        #
        if not return_list:
            for line in self.final_comment:
                stripped_line = line.strip()
                if stripped_line and not stripped_line.startswith('#'):
                    line = '# ' + line
                out.append(line)
        #
        if int_val != 'test':
            self.interpolation = int_val
        #
        if (return_list) or (self.filename is None):
            return out
        #
        if isinstance(self.filename, StringTypes):
            h = open(self.filename, 'w')
            h.write(self.BOM or '')
            h.write('\n'.join(out))
            h.close()
        else:
            self.filename.seek(0)
            self.filename.write(self.BOM or '')
            self.filename.write('\n'.join(out))
            # if we have a stored file object (or StringIO)
            # we *don't* close it

    def validate(self, validator, section=None):
        """
        Test the ConfigObj against a configspec.
        
        It uses the ``validator`` object from *validate.py*.
        
        To run ``validate`` on the current ConfigObj, call: ::
        
            test = config.validate(validator)
        
        (Normally having previously passed in the configspec when the ConfigObj
        was created - you can dynamically assign a dictionary of checks to the
        ``configspec`` attribute of a section though).
        
        It returns ``True`` if everything passes, or a dictionary of
        pass/fails (True/False). If every member of a subsection passes, it
        will just have the value ``True``. (It also returns ``False`` if all
        members fail).
        
        In addition, it converts the values from strings to their native
        types if their checks pass (and ``stringify`` is set).
        
        >>> try:
        ...     from validate import Validator
        ... except ImportError:
        ...     print >> sys.stderr, 'Cannot import the Validator object, skipping the realted tests'
        ... else:
        ...     config = '''
        ...     test1=40
        ...     test2=hello
        ...     test3=3
        ...     test4=5.0
        ...     [section]
        ...         test1=40
        ...         test2=hello
        ...         test3=3
        ...         test4=5.0
        ...         [[sub section]]
        ...             test1=40
        ...             test2=hello
        ...             test3=3
        ...             test4=5.0
        ... '''.split('\\n')
        ...     configspec = '''
        ...     test1='integer(30,50)'
        ...     test2='string'
        ...     test3='integer'
        ...     test4='float(6.0)'
        ...     [section ]
        ...         test1='integer(30,50)'
        ...         test2='string'
        ...         test3='integer'
        ...         test4='float(6.0)'
        ...         [[sub section]]
        ...             test1='integer(30,50)'
        ...             test2='string'
        ...             test3='integer'
        ...             test4='float(6.0)'
        ...     '''.split('\\n')
        ...     val = Validator()
        ...     c1 = ConfigObj(config, configspec=configspec)
        ...     test = c1.validate(val)
        ...     test == {
        ...         'test1': True,
        ...         'test2': True,
        ...         'test3': True,
        ...         'test4': False,
        ...         'section': {
        ...             'test1': True,
        ...             'test2': True,
        ...             'test3': True,
        ...             'test4': False,
        ...             'sub section': {
        ...                 'test1': True,
        ...                 'test2': True,
        ...                 'test3': True,
        ...                 'test4': False,
        ...             },
        ...         },
        ...     }
        1
        >>> val.check(c1.configspec['test4'], c1['test4'])
        Traceback (most recent call last):
        VdtValueTooSmallError: the value "5.0" is too small.
        
        >>> val_test_config = '''
        ...     key = 0
        ...     key2 = 1.1
        ...     [section]
        ...     key = some text
        ...     key2 = 1.1, 3.0, 17, 6.8
        ...         [[sub-section]]
        ...         key = option1
        ...         key2 = True'''.split('\\n')
        >>> val_test_configspec = '''
        ...     key = integer
        ...     key2 = float
        ...     [section]
        ...     key = string
        ...     key2 = float_list(4)
        ...        [[sub-section]]
        ...        key = option(option1, option2)
        ...        key2 = boolean'''.split('\\n')
        >>> val_test = ConfigObj(val_test_config, configspec=val_test_configspec)
        >>> val_test.validate(val)
        1
        >>> val_test['key'] = 'text not a digit'
        >>> val_res = val_test.validate(val)
        >>> val_res == {'key2': True, 'section': True, 'key': False}
        1
        >>> configspec = '''
        ...     test1='integer(30,50, default=40)'
        ...     test2='string(default="hello")'
        ...     test3='integer(default=3)'
        ...     test4='float(6.0, default=6.0)'
        ...     [section ]
        ...         test1='integer(30,50, default=40)'
        ...         test2='string(default="hello")'
        ...         test3='integer(default=3)'
        ...         test4='float(6.0, default=6.0)'
        ...         [[sub section]]
        ...             test1='integer(30,50, default=40)'
        ...             test2='string(default="hello")'
        ...             test3='integer(default=3)'
        ...             test4='float(6.0, default=6.0)'
        ...     '''.split('\\n')
        >>> default_test = ConfigObj(['test1=30'], configspec=configspec)
        >>> default_test
        {'test1': '30', 'section': {'sub section': {}}}
        >>> default_test.validate(val)
        1
        >>> default_test == {
        ...     'test1': 30,
        ...     'test2': 'hello',
        ...     'test3': 3,
        ...     'test4': 6.0,
        ...     'section': {
        ...         'test1': 40,
        ...         'test2': 'hello',
        ...         'test3': 3,
        ...         'test4': 6.0,
        ...         'sub section': {
        ...             'test1': 40,
        ...             'test3': 3,
        ...             'test2': 'hello',
        ...             'test4': 6.0,
        ...         },
        ...     },
        ... }
        1
        
        Now testing with repeated sections : BIG TEST
        
        >>> repeated_1 = '''
        ... [dogs]
        ...     [[__many__]] # spec for a dog
        ...         fleas = boolean(default=True)
        ...         tail = option(long, short, default=long)
        ...         name = string(default=rover)
        ...         [[[__many__]]]  # spec for a puppy
        ...             name = string(default="son of rover")
        ...             age = float(default=0.0)
        ... [cats]
        ...     [[__many__]] # spec for a cat
        ...         fleas = boolean(default=True)
        ...         tail = option(long, short, default=short)
        ...         name = string(default=pussy)
        ...         [[[__many__]]] # spec for a kitten
        ...             name = string(default="son of pussy")
        ...             age = float(default=0.0)
        ...         '''.split('\\n')
        >>> repeated_2 = '''
        ... [dogs]
        ... 
        ...     # blank dogs with puppies
        ...     # should be filled in by the configspec
        ...     [[dog1]]
        ...         [[[puppy1]]]
        ...         [[[puppy2]]]
        ...         [[[puppy3]]]
        ...     [[dog2]]
        ...         [[[puppy1]]]
        ...         [[[puppy2]]]
        ...         [[[puppy3]]]
        ...     [[dog3]]
        ...         [[[puppy1]]]
        ...         [[[puppy2]]]
        ...         [[[puppy3]]]
        ... [cats]
        ... 
        ...     # blank cats with kittens
        ...     # should be filled in by the configspec
        ...     [[cat1]]
        ...         [[[kitten1]]]
        ...         [[[kitten2]]]
        ...         [[[kitten3]]]
        ...     [[cat2]]
        ...         [[[kitten1]]]
        ...         [[[kitten2]]]
        ...         [[[kitten3]]]
        ...     [[cat3]]
        ...         [[[kitten1]]]
        ...         [[[kitten2]]]
        ...         [[[kitten3]]]
        ... '''.split('\\n')
        >>> repeated_3 = '''
        ... [dogs]
        ... 
        ...     [[dog1]]
        ...     [[dog2]]
        ...     [[dog3]]
        ... [cats]
        ... 
        ...     [[cat1]]
        ...     [[cat2]]
        ...     [[cat3]]
        ... '''.split('\\n')
        >>> repeated_4 = '''
        ... [__many__]
        ... 
        ...     name = string(default=Michael)
        ...     age = float(default=0.0)
        ...     sex = option(m, f, default=m)
        ... '''.split('\\n')
        >>> repeated_5 = '''
        ... [cats]
        ... [[__many__]]
        ...     fleas = boolean(default=True)
        ...     tail = option(long, short, default=short)
        ...     name = string(default=pussy)
        ...     [[[description]]]
        ...         height = float(default=3.3)
        ...         weight = float(default=6)
        ...         [[[[coat]]]]
        ...             fur = option(black, grey, brown, "tortoise shell", default=black)
        ...             condition = integer(0,10, default=5)
        ... '''.split('\\n')
        >>> from validate import Validator
        >>> val= Validator()
        >>> repeater = ConfigObj(repeated_2, configspec=repeated_1)
        >>> repeater.validate(val)
        1
        >>> repeater == {
        ...     'dogs': {
        ...         'dog1': {
        ...             'fleas': True,
        ...             'tail': 'long',
        ...             'name': 'rover',
        ...             'puppy1': {'name': 'son of rover', 'age': 0.0},
        ...             'puppy2': {'name': 'son of rover', 'age': 0.0},
        ...             'puppy3': {'name': 'son of rover', 'age': 0.0},
        ...         },
        ...         'dog2': {
        ...             'fleas': True,
        ...             'tail': 'long',
        ...             'name': 'rover',
        ...             'puppy1': {'name': 'son of rover', 'age': 0.0},
        ...             'puppy2': {'name': 'son of rover', 'age': 0.0},
        ...             'puppy3': {'name': 'son of rover', 'age': 0.0},
        ...         },
        ...         'dog3': {
        ...             'fleas': True,
        ...             'tail': 'long',
        ...             'name': 'rover',
        ...             'puppy1': {'name': 'son of rover', 'age': 0.0},
        ...             'puppy2': {'name': 'son of rover', 'age': 0.0},
        ...             'puppy3': {'name': 'son of rover', 'age': 0.0},
        ...         },
        ...     },
        ...     'cats': {
        ...         'cat1': {
        ...             'fleas': True,
        ...             'tail': 'short',
        ...             'name': 'pussy',
        ...             'kitten1': {'name': 'son of pussy', 'age': 0.0},
        ...             'kitten2': {'name': 'son of pussy', 'age': 0.0},
        ...             'kitten3': {'name': 'son of pussy', 'age': 0.0},
        ...         },
        ...         'cat2': {
        ...             'fleas': True,
        ...             'tail': 'short',
        ...             'name': 'pussy',
        ...             'kitten1': {'name': 'son of pussy', 'age': 0.0},
        ...             'kitten2': {'name': 'son of pussy', 'age': 0.0},
        ...             'kitten3': {'name': 'son of pussy', 'age': 0.0},
        ...         },
        ...         'cat3': {
        ...             'fleas': True,
        ...             'tail': 'short',
        ...             'name': 'pussy',
        ...             'kitten1': {'name': 'son of pussy', 'age': 0.0},
        ...             'kitten2': {'name': 'son of pussy', 'age': 0.0},
        ...             'kitten3': {'name': 'son of pussy', 'age': 0.0},
        ...         },
        ...     },
        ... }
        1
        >>> repeater = ConfigObj(repeated_3, configspec=repeated_1)
        >>> repeater.validate(val)
        1
        >>> repeater == {
        ...     'cats': {
        ...         'cat1': {'fleas': True, 'tail': 'short', 'name': 'pussy'},
        ...         'cat2': {'fleas': True, 'tail': 'short', 'name': 'pussy'},
        ...         'cat3': {'fleas': True, 'tail': 'short', 'name': 'pussy'},
        ...     },
        ...     'dogs': {
        ...         'dog1': {'fleas': True, 'tail': 'long', 'name': 'rover'},
        ...         'dog2': {'fleas': True, 'tail': 'long', 'name': 'rover'},
        ...         'dog3': {'fleas': True, 'tail': 'long', 'name': 'rover'},
        ...     },
        ... }
        1
        >>> repeater = ConfigObj(configspec=repeated_4)
        >>> repeater['Michael'] = {}
        >>> repeater.validate(val)
        1
        >>> repeater == {
        ...     'Michael': {'age': 0.0, 'name': 'Michael', 'sex': 'm'},
        ... }
        1
        >>> repeater = ConfigObj(repeated_3, configspec=repeated_5)
        >>> repeater == {
        ...     'dogs': {'dog1': {}, 'dog2': {}, 'dog3': {}},
        ...     'cats': {'cat1': {}, 'cat2': {}, 'cat3': {}},
        ... }
        1
        >>> repeater.validate(val)
        1
        >>> repeater == {
        ...     'dogs': {'dog1': {}, 'dog2': {}, 'dog3': {}},
        ...     'cats': {
        ...         'cat1': {
        ...             'fleas': True,
        ...             'tail': 'short',
        ...             'name': 'pussy',
        ...             'description': {
        ...                 'weight': 6.0,
        ...                 'height': 3.2999999999999998,
        ...                 'coat': {'fur': 'black', 'condition': 5},
        ...             },
        ...         },
        ...         'cat2': {
        ...             'fleas': True,
        ...             'tail': 'short',
        ...             'name': 'pussy',
        ...             'description': {
        ...                 'weight': 6.0,
        ...                 'height': 3.2999999999999998,
        ...                 'coat': {'fur': 'black', 'condition': 5},
        ...             },
        ...         },
        ...         'cat3': {
        ...             'fleas': True,
        ...             'tail': 'short',
        ...             'name': 'pussy',
        ...             'description': {
        ...                 'weight': 6.0,
        ...                 'height': 3.2999999999999998,
        ...                 'coat': {'fur': 'black', 'condition': 5},
        ...             },
        ...         },
        ...     },
        ... }
        1
        
        Test that interpolation is preserved for validated string values.
        >>> t = ConfigObj()
        >>> t['DEFAULT'] = {}
        >>> t['DEFAULT']['test'] = 'a'
        >>> t['test'] = '%(test)s'
        >>> t['test']
        'a'
        >>> v = Validator()
        >>> t.configspec = {'test': 'string'}
        >>> t.validate(v)
        1
        >>> t.interpolation = False
        >>> t
        {'test': '%(test)s', 'DEFAULT': {'test': 'a'}}
        
        FIXME: Above tests will fail if we couldn't import Validator (the ones
        that don't raise errors will produce different output and still fail as
        tests)
        """
        if section is None:
            if self.configspec is None:
                raise ValueError, 'No configspec supplied.'
            section = self
        #
        spec_section = section.configspec
        if '__many__' in section.configspec:
            many = spec_section['__many__']
            # dynamically assign the configspecs
            # for the sections below
            for entry in section.sections:
                self._handle_repeat(section[entry], many)
        #
        out = {}
        ret_true = True
        ret_false = True
        for entry in spec_section:
            if entry == '__many__':
                continue
            if (not entry in section.scalars) or (entry in section.defaults):
                # missing entries
                # or entries from defaults
                missing = True
                val = None
            else:
                missing = False
                val = section[entry]
            try:
                check = validator.check(spec_section[entry],
                                        val,
                                        missing=missing)
            except validator.baseErrorClass:
                out[entry] = False
                ret_true = False
            # MIKE: we want to raise all other exceptions, not just print ?
##            except Exception, err:
##                print err
            else:
                ret_false = False
                out[entry] = True
                if self.stringify or missing:
                    # if we are doing type conversion
                    # or the value is a supplied default
                    if not self.stringify:
                        if isinstance(check, (list, tuple)):
                            # preserve lists
                            check = [str(item) for item in check]
                        elif missing and check is None:
                            # convert the None from a default to a ''
                            check = ''
                        else:
                            check = str(check)
                    if (check != val) or missing:
                        section[entry] = check
                if missing and entry not in section.defaults:
                    section.defaults.append(entry)
        #
        for entry in section.sections:
            check = self.validate(validator, section[entry])
            out[entry] = check
            if check == False:
                ret_true = False
            elif check == True:
                ret_false = False
            else:
                ret_true = False
                ret_false = False
        #
        if ret_true:
            return True
        elif ret_false:
            return False
        else:
            return out

class SimpleVal(object):
    """
    A simple validator.
    Can be used to check that all members expected are present.
    
    To use it, provide a configspec with all your members in (the value given
    will be ignored). Pass an instance of ``SimpleVal`` to the ``validate``
    method of your ``ConfigObj``. ``validate`` will return ``True`` if all
    members are present, or a dictionary with True/False meaning
    present/missing. (Whole missing sections will be replaced with ``False``)
    
    >>> val = SimpleVal()
    >>> config = '''
    ... test1=40
    ... test2=hello
    ... test3=3
    ... test4=5.0
    ... [section]
    ... test1=40
    ... test2=hello
    ... test3=3
    ... test4=5.0
    ...     [[sub section]]
    ...     test1=40
    ...     test2=hello
    ...     test3=3
    ...     test4=5.0
    ... '''.split('\\n')
    >>> configspec = '''
    ... test1=''
    ... test2=''
    ... test3=''
    ... test4=''
    ... [section]
    ... test1=''
    ... test2=''
    ... test3=''
    ... test4=''
    ...     [[sub section]]
    ...     test1=''
    ...     test2=''
    ...     test3=''
    ...     test4=''
    ... '''.split('\\n')
    >>> o = ConfigObj(config, configspec=configspec)
    >>> o.validate(val)
    1
    >>> o = ConfigObj(configspec=configspec)
    >>> o.validate(val)
    0
    """
    
    def __init__(self):
        self.baseErrorClass = ConfigObjError
    
    def check(self, check, member, missing=False):
        """A dummy check method, always returns the value unchanged."""
        if missing:
            raise self.baseErrorClass
        return member

# FIXME: test error code for badly built multiline values
# FIXME: test handling of StringIO
# FIXME: test interpolation with writing

def _doctest():
    """
    Dummy function to hold some of the doctests.
    
    >>> a.depth
    0
    >>> a == {
    ...     'key2': 'val',
    ...     'key1': 'val',
    ...     'lev1c': {
    ...         'lev2c': {
    ...             'lev3c': {
    ...                 'key1': 'val',
    ...             },
    ...         },
    ...     },
    ...     'lev1b': {
    ...         'key2': 'val',
    ...         'key1': 'val',
    ...         'lev2ba': {
    ...             'key1': 'val',
    ...         },
    ...         'lev2bb': {
    ...             'key1': 'val',
    ...         },
    ...     },
    ...     'lev1a': {
    ...         'key2': 'val',
    ...         'key1': 'val',
    ...     },
    ... }
    1
    >>> b.depth
    0
    >>> b == {
    ...     'key3': 'val3',
    ...     'key2': 'val2',
    ...     'key1': 'val1',
    ...     'section 1': {
    ...         'keys11': 'val1',
    ...         'keys13': 'val3',
    ...         'keys12': 'val2',
    ...     },
    ...     'section 2': {
    ...         'section 2 sub 1': {
    ...             'fish': '3',
    ...     },
    ...     'keys21': 'val1',
    ...     'keys22': 'val2',
    ...     'keys23': 'val3',
    ...     },
    ... }
    1
    >>> t = '''
    ... 'a' = b # !"$%^&*(),::;'@~#= 33
    ... "b" = b #= 6, 33
    ... ''' .split('\\n')
    >>> t2 = ConfigObj(t)
    >>> assert t2 == {'a': 'b', 'b': 'b'}
    >>> t2.inline_comments['b'] = ''
    >>> del t2['a']
    >>> assert t2.write() == ['','b = b', '']
    """

if __name__ == '__main__':
    # run the code tests in doctest format
    #
    testconfig1 = """\
    key1= val    # comment 1
    key2= val    # comment 2
    # comment 3
    [lev1a]     # comment 4
    key1= val    # comment 5
    key2= val    # comment 6
    # comment 7
    [lev1b]    # comment 8
    key1= val    # comment 9
    key2= val    # comment 10
    # comment 11
        [[lev2ba]]    # comment 12
        key1= val    # comment 13
        # comment 14
        [[lev2bb]]    # comment 15
        key1= val    # comment 16
    # comment 17
    [lev1c]    # comment 18
    # comment 19
        [[lev2c]]    # comment 20
        # comment 21
            [[[lev3c]]]    # comment 22
            key1 = val    # comment 23"""
    #
    testconfig2 = """\
                        key1 = 'val1'
                        key2 =   "val2"
                        key3 = val3
                        ["section 1"] # comment
                        keys11 = val1
                        keys12 = val2
                        keys13 = val3
                        [section 2]
                        keys21 = val1
                        keys22 = val2
                        keys23 = val3
                        
                            [['section 2 sub 1']]
                            fish = 3
    """
    #
    testconfig6 = '''
    name1 = """ a single line value """ # comment
    name2 = \''' another single line value \''' # comment
    name3 = """ a single line value """
    name4 = \''' another single line value \'''
        [ "multi section" ]
        name1 = """
        Well, this is a
        multiline value
        """
        name2 = \'''
        Well, this is a
        multiline value
        \'''
        name3 = """
        Well, this is a
        multiline value
        """     # a comment
        name4 = \'''
        Well, this is a
        multiline value
        \'''  # I guess this is a comment too
    '''
    #
    import doctest
    m = sys.modules.get('__main__')
    globs = m.__dict__.copy()
    a = ConfigObj(testconfig1.split('\n'), raise_errors=True)
    b = ConfigObj(testconfig2.split('\n'), raise_errors=True)
    i = ConfigObj(testconfig6.split('\n'), raise_errors=True)
    globs.update({
        'INTP_VER': INTP_VER,
        'a': a,
        'b': b,
        'i': i,
    })
    doctest.testmod(m, globs=globs)

"""
    BUGS
    ====
    
    With list values off, ConfigObj can incorrectly unquote values. (This makes
    it impossible to use listquote to handle your list values for you - for
    nested lists. Not handling quotes at all would be better for this)
    
    TODO
    ====
    
    A method to optionally remove uniform indentation from multiline values.
    (do as an example of using ``walk`` - along with string-escape)
    
    INCOMPATIBLE CHANGES
    ====================
    
    (I have removed a lot of needless complications - this list is probably not
    conclusive, many option/attribute/method names have changed)
    
    Case sensitive
    
    The only valid divider is '='
    
    We've removed line continuations with '\'
    
    No recursive lists in values
    
    No empty section
    
    No distinction between flatfiles and non flatfiles
    
    Change in list syntax - use commas to indicate list, not parentheses
    (square brackets and parentheses are no longer recognised as lists)
    
    ';' is no longer valid for comments and no multiline comments
    
    No attribute access
    
    We don't allow empty values - have to use '' or ""
    
    In ConfigObj 3 - setting a non-flatfile member to ``None`` would
    initialise it as an empty section.
    
    The escape entities '&mjf-lf;' and '&mjf-quot;' have gone
    replaced by triple quote, multiple line values.
    
    The ``newline``, ``force_return``, and ``default`` options have gone
    
    The ``encoding`` and ``backup_encoding`` methods have gone - replaced
    with the ``encode`` and ``decode`` methods.
    
    ``fileerror`` and ``createempty`` options have become ``file_error`` and
    ``create_empty``
    
    Partial configspecs (for specifying the order members should be written
    out and which should be present) have gone. The configspec is no longer
    used to specify order for the ``write`` method.
    
    Exceeding the maximum depth of recursion in string interpolation now
    raises an error ``InterpolationDepthError``.
    
    Specifying a value for interpolation which doesn't exist now raises an
    error ``MissingInterpolationOption`` (instead of merely being ignored).
    
    The ``writein`` method has been removed.
    
    The comments attribute is now a list (``inline_comments`` equates to the
    old comments attribute)
    
    ISSUES
    ======
    
    You can't have a keyword with the same name as a section (in the same
    section). They are both dictionary keys - so they would overlap.
    
    Interpolation checks first the 'DEFAULT' subsection of the current
    section, next it checks the 'DEFAULT' section of the parent section,
    last it checks the 'DEFAULT' section of the main section.
    
    Logically a 'DEFAULT' section should apply to all subsections of the *same
    parent* - this means that checking the 'DEFAULT' subsection in the
    *current section* is not necessarily logical ?
    
    In order to simplify unicode support (which is possibly of limited value
    in a config file) I have removed automatic support and added the
    ``encode`` and ``decode methods, which can be used to transform keys and
    entries. Because the regex looks for specific values on inital parsing
    (i.e. the quotes and the equals signs) it can only read ascii compatible
    encodings. For unicode use ``UTF8``, which is ASCII compatible.
    
    Does it matter that we don't support the ':' divider, which is supported
    by ``ConfigParser`` ?
    
    Following error with "list_values=False" : ::
    
        >>> a = ["a='hello', 'goodbye'"]
        >>>
        >>> c(a, list_values=False)
        {'a': "hello', 'goodbye"}
    
    The regular expression correctly removes the value -
    ``"'hello', 'goodbye'"`` and then unquote just removes the front and
    back quotes (called from ``_handle_value``). What should we do ??
    (*ought* to raise exception because it's an invalid value if lists are
    off *sigh*. This is not what you want if you want to do your own list
    processing - would be *better* in this case not to unquote.)
    
    String interpolation and validation don't play well together. When
    validation changes type it sets the value. This will correctly fetch the
    value using interpolation - but then overwrite the interpolation reference.
    If the value is unchanged by validation (it's a string) - but other types
    will be.
    
    List Value Syntax
    =================
    
    List values allow you to specify multiple values for a keyword. This
    maps to a list as the resulting Python object when parsed.
    
    The syntax for lists is easy. A list is a comma separated set of values.
    If these values contain quotes, the hash mark, or commas, then the values
    can be surrounded by quotes. e.g. : ::
    
        keyword = value1, 'value 2', "value 3"
    
    If a value needs to be a list, but only has one member, then you indicate
    this with a trailing comma. e.g. : ::
    
        keyword = "single value",
    
    If a value needs to be a list, but it has no members, then you indicate
    this with a single comma. e.g. : ::
    
        keyword = ,     # an empty list
    
    Using triple quotes it will be possible for single values to contain
    newlines and *both* single quotes and double quotes. Triple quotes aren't
    allowed in list values. This means that the members of list values can't
    contain carriage returns (or line feeds :-) or both quote values.
      
    CHANGELOG
    =========
    
    2005/10/09
    ----------
    
    Fixed typo in ``write`` method. (Testing for the wrong value when resetting
    ``interpolation``).
    
    2005/09/16
    ----------
    
    Fixed bug in ``setdefault`` - creating a new section *wouldn't* return
    a reference to the new section.
    
    2005/09/09
    ----------
    
    Removed ``PositionError``.
    
    Allowed quotes around keys as documented.
    
    Fixed bug with commas in comments. (matched as a list value)
    
    Beta 5
    
    2005/09/07
    ----------
    
    Fixed bug in initialising ConfigObj from a ConfigObj.
    
    Changed the mailing list address.
    
    Beta 4
    
    2005/09/03
    ----------
    
    Fixed bug in ``Section__delitem__`` oops.
    
    2005/08/28
    ----------
    
    Interpolation is switched off before writing out files.
    
    Fixed bug in handling ``StringIO`` instances. (Thanks to report from
    "Gustavo Niemeyer" <gustavo@niemeyer.net>)
    
    Moved the doctests from the ``__init__`` method to a separate function.
    (For the sake of IDE calltips).
    
    Beta 3
    
    2005/08/26
    ----------
    
    String values unchanged by validation *aren't* reset. This preserves
    interpolation in string values.
    
    2005/08/18
    ----------
    
    None from a default is turned to '' if stringify is off - because setting 
    a value to None raises an error.
    
    Version 4.0.0-beta2
    
    2005/08/16
    ----------
    
    By Nicola Larosa
    
    Actually added the RepeatSectionError class ;-)
    
    2005/08/15
    ----------
    
    If ``stringify`` is off - list values are preserved by the ``validate``
    method. (Bugfix)
    
    2005/08/14
    ----------
    
    By Michael Foord
    
    Fixed ``simpleVal``.
    
    Added ``RepeatSectionError`` error if you have additional sections in a
    section with a ``__many__`` (repeated) section.
    
    By Nicola Larosa
    
    Reworked the ConfigObj._parse, _handle_error and _multiline methods:
    mutated the self._infile, self._index and self._maxline attributes into
    local variables and method parameters
    
    Reshaped the ConfigObj._multiline method to better reflect its semantics
    
    Changed the "default_test" test in ConfigObj.validate to check the fix for
    the bug in validate.Validator.check
    
    2005/08/13
    ----------
    
    By Nicola Larosa
    
    Updated comments at top
    
    2005/08/11
    ----------
    
    By Michael Foord
    
    Implemented repeated sections.
    
    By Nicola Larosa
    
    Added test for interpreter version: raises RuntimeError if earlier than
    2.2
    
    2005/08/10
    ----------
   
    By Michael Foord
     
    Implemented default values in configspecs.
    
    By Nicola Larosa
    
    Fixed naked except: clause in validate that was silencing the fact
    that Python2.2 does not have dict.pop
    
    2005/08/08
    ----------
    
    By Michael Foord
    
    Bug fix causing error if file didn't exist.
    
    2005/08/07
    ----------
    
    By Nicola Larosa
    
    Adjusted doctests for Python 2.2.3 compatibility
    
    2005/08/04
    ----------
    
    By Michael Foord
    
    Added the inline_comments attribute
    
    We now preserve and rewrite all comments in the config file
    
    configspec is now a section attribute
    
    The validate method changes values in place
    
    Added InterpolationError
    
    The errors now have line number, line, and message attributes. This
    simplifies error handling
    
    Added __docformat__
    
    2005/08/03
    ----------
    
    By Michael Foord
    
    Fixed bug in Section.pop (now doesn't raise KeyError if a default value
    is specified)
    
    Replaced ``basestring`` with ``types.StringTypes``
    
    Removed the ``writein`` method
    
    Added __version__
    
    2005/07/29
    ----------
    
    By Nicola Larosa
    
    Indentation in config file is not significant anymore, subsections are
    designated by repeating square brackets
    
    Adapted all tests and docs to the new format
    
    2005/07/28
    ----------
    
    By Nicola Larosa
    
    Added more tests
    
    2005/07/23
    ----------
    
    By Nicola Larosa
    
    Reformatted final docstring in ReST format, indented it for easier folding
    
    Code tests converted to doctest format, and scattered them around
    in various docstrings
    
    Walk method rewritten using scalars and sections attributes
    
    2005/07/22
    ----------
    
    By Nicola Larosa
    
    Changed Validator and SimpleVal "test" methods to "check"
    
    More code cleanup
    
    2005/07/21
    ----------
    
    Changed Section.sequence to Section.scalars and Section.sections
    
    Added Section.configspec
    
    Sections in the root section now have no extra indentation
    
    Comments now better supported in Section and preserved by ConfigObj
    
    Comments also written out
    
    Implemented initial_comment and final_comment
    
    A scalar value after a section will now raise an error
    
    2005/07/20
    ----------
    
    Fixed a couple of bugs
    
    Can now pass a tuple instead of a list
    
    Simplified dict and walk methods
    
    Added __str__ to Section
    
    2005/07/10
    ----------
    
    By Nicola Larosa
    
    More code cleanup
    
    2005/07/08
    ----------
    
    The stringify option implemented. On by default.
    
    2005/07/07
    ----------
    
    Renamed private attributes with a single underscore prefix.
    
    Changes to interpolation - exceeding recursion depth, or specifying a
    missing value, now raise errors.
    
    Changes for Python 2.2 compatibility. (changed boolean tests - removed
    ``is True`` and ``is False``)
    
    Added test for duplicate section and member (and fixed bug)
    
    2005/07/06
    ----------
    
    By Nicola Larosa
    
    Code cleanup
    
    2005/07/02
    ----------
    
    Version 0.1.0
    
    Now properly handles values including comments and lists.
    
    Better error handling.
    
    String interpolation.
    
    Some options implemented.
    
    You can pass a Section a dictionary to initialise it.
    
    Setting a Section member to a dictionary will create a Section instance.
    
    2005/06/26
    ----------
    
    Version 0.0.1
    
    Experimental reader.
    
    A reasonably elegant implementation - a basic reader in 160 lines of code.
    
    *A programming language is a medium of expression.* - Paul Graham
"""


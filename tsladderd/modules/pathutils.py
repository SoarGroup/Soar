# 2005/08/28
# Version 0.2.1
# pathutils.py
# Functions useful for working with files and paths.
# http://www.voidspace.org.uk/python/recipebook.shtml#utils

# Copyright Michael Foord 2004
# Released subject to the BSD License
# Please see http://www.voidspace.org.uk/documents/BSD-LICENSE.txt

# For information about bugfixes, updates and support, please join the Pythonutils mailing list.
# http://voidspace.org.uk/mailman/listinfo/pythonutils_voidspace.org.uk
# Comments, suggestions and bug reports welcome.
# Scripts maintained at http://www.voidspace.org.uk/python/index.shtml
# E-mail fuzzyman@voidspace.org.uk

"""
This module contains convenience functions for working with files and paths.
"""

from __future__ import generators
import os
import sys

__all__ = (
    'readlines',
    'writelines',
    'readbinary',
    'writebinary',
    'readfile',
    'writefile',
    'tslash',
    'relpath',
    'splitall',
    'walkfiles',
    'walkdirs',
    'walkemptydirs',
    'formatbytes',
    'fullcopy',
    'import_path',
    )

######################################
# Functions to read and write files in text and binary mode.

def readlines(filename):
    """Passed a filename, it reads it, and returns a list of lines. (Read in text mode)"""
    filehandle = open(filename, 'r')
    outfile = filehandle.readlines()
    filehandle.close()
    return outfile

def writelines(filename, infile, newline=False):
    """
    Given a filename and a list of lines it writes the file. (In text mode)
    
    If ``newline`` is ``True`` (default is ``False``) it adds a newline to each
    line.
    """
    filehandle = open(filename, 'w')
    if newline:
        infile = [line + '\n' for line in infile]
    filehandle.writelines(infile)
    filehandle.close()

def readbinary(filename):
    """Given a filename, read a file in binary mode. It returns a single string."""
    filehandle = open(filename, 'rb')
    thisfile = filehandle.read()
    filehandle.close()
    return thisfile

def writebinary(filename, infile):
    """Given a filename and a string, write the file in binary mode. """
    filehandle = open(filename, 'wb')
    filehandle.write(infile)
    filehandle.close()

def readfile(filename):
    """Given a filename, read a file in text mode. It returns a single string."""
    filehandle = open(filename, 'r')
    outfile = filehandle.read()
    filehandle.close()
    return outfile

def writefile(filename, infile):
    """Given a filename and a string, write the file in text mode."""
    filehandle = open(filename, 'w')
    filehandle.write(infile)
    filehandle.close()
    
####################################################################
# Some functions for dealing with paths

def tslash(apath):
    """
    Add a trailing slash (``/``) to a path if it lacks one.
    
    It doesn't use ``os.sep`` because you end up in trouble on windoze, when you
    want separators for URLs.
    """
    if apath and apath != '.' and not apath.endswith('/') and not apath.endswith('\\'):
        return apath + '/'
    else:
        return apath

def relpath(origin, dest):
    """
    Return the relative path between origin and dest.
    
    If it's not possible return dest.
    
    
    If they are identical return ``os.curdir``
    
    Adapted from `path.py`_ by Jason Orendorff.
    
    
    .. _path.py: http://www.jorendorff.com/articles/python/path/
    """
    origin = os.path.abspath(origin).replace('\\', '/')
    dest = os.path.abspath(dest).replace('\\', '/')

    orig_list = splitall(os.path.normcase(origin))
    # Don't normcase dest!  We want to preserve the case.
    dest_list = splitall(dest)

    if orig_list[0] != os.path.normcase(dest_list[0]):
        # Can't get here from there.
        return dest

    # Find the location where the two paths start to differ.
    i = 0
    for start_seg, dest_seg in zip(orig_list, dest_list):
        if start_seg != os.path.normcase(dest_seg):
            break
        i += 1

    # Now i is the point where the two paths diverge.
    # Need a certain number of "os.pardir"s to work up
    # from the origin to the point of divergence.
    segments = [os.pardir] * (len(orig_list) - i)
    # Need to add the diverging part of dest_list.
    segments += dest_list[i:]
    if len(segments) == 0:
        # If they happen to be identical, use os.curdir.
        return os.curdir
    else:
        return os.path.join(*segments).replace('\\', '/')

def splitall(loc):
    """
    Return a list of the path components in loc. (Used by relpath_).
    
    The first item in the list will be  either ``os.curdir``, ``os.pardir``, empty,
    or the root directory of loc (for example, ``/`` or ``C:\\).
    
    The other items in the list will be strings.
        
    Adapted from path.py by Jason Orendorff.
    """
    parts = []
    while loc != os.curdir and loc != os.pardir:
        prev = loc
        loc, child = os.path.split(prev)
        if loc == prev:
            break
        parts.append(child)
    parts.append(loc)
    parts.reverse()
    return parts

#######################################################################
# a pre 2.3 walkfiles function - adapted from the path module by Jason Orendorff

join = os.path.join
isdir = os.path.isdir
isfile = os.path.isfile

def walkfiles(thisdir):
    """
    walkfiles(D) -> iterator over files in D, recursively. Yields full file paths.
    
    Adapted from path.py by Jason Orendorff.
    """
    for child in os.listdir(thisdir):
        thischild = join(thisdir, child)
        if isfile(thischild):
            yield thischild
        elif isdir(thischild):
            for f in walkfiles(thischild):
                yield f
                
def walkdirs(thisdir):
    """
    Walk through all the subdirectories in a tree. Recursively yields directory
    names (full paths).
    """
    for child in os.listdir(thisdir):
        thischild = join(thisdir, child)
        if isfile(thischild):
            continue
        elif isdir(thischild):
            for f in walkdirs(thischild):
                yield f
            yield thischild
            
def walkemptydirs(thisdir):
    """
    Recursively yield names of *empty* directories.
    
    These are the only paths omitted when using ``walkfiles``.
    """
    if not os.listdir(thisdir):       # if the directory is empty.. then yield it
        yield thisdir   
    for child in os.listdir(thisdir):
        thischild = join(thisdir, child)
        if isdir(thischild):
            for emptydir in walkemptydirs(thischild):
                yield emptydir

###############################################################
# formatbytes takes a filesize (as returned by os.getsize() )
# and formats it for display in one of two ways !!

def formatbytes(sizeint, configdict=None, **configs):
    """
    Given a file size as an integer, return a nicely formatted string that
    represents the size. Has various options to control it's output.
    
    You can pass in a dictionary of arguments or keyword arguments. Keyword
    arguments override the dictionary and there are sensible defaults for options
    you don't set.
    
    Options and defaults are as follows :
    
    *    ``forcekb = False`` -         If set this forces the output to be in terms
    of kilobytes and bytes only.
    
    *    ``largestonly = True`` -    If set, instead of outputting 
        ``1 Mbytes, 307 Kbytes, 478 bytes`` it outputs using only the largest 
        denominator - e.g. ``1.3 Mbytes`` or ``17.2 Kbytes``
    
    *    ``kiloname = 'Kbytes'`` -    The string to use for kilobytes
    
    *    ``meganame = 'Mbytes'`` - The string to use for Megabytes
    
    *    ``bytename = 'bytes'`` -     The string to use for bytes
    
    *    ``nospace = True`` -        If set it outputs ``1Mbytes, 307Kbytes``, 
        notice there is no space.
    
    Example outputs : ::
    
        19Mbytes, 75Kbytes, 255bytes
        2Kbytes, 0bytes
        23.8Mbytes
    
    .. note::
    
        It currently uses the plural form even for singular.
    """
    defaultconfigs = {  'forcekb' : False,
                        'largestonly' : True,
                        'kiloname' : 'Kbytes',
                        'meganame' : 'Mbytes',
                        'bytename' : 'bytes',
                        'nospace' : True}
    if configdict is None:
        configdict = {}
    for entry in configs:
        # keyword parameters override the dictionary passed in
        configdict[entry] = configs[entry]
    #
    for keyword in defaultconfigs:
        if not configdict.has_key(keyword):
            configdict[keyword] = defaultconfigs[keyword]
    #
    if configdict['nospace']:
        space = ''
    else:
        space = ' '
    #
    mb, kb, rb = bytedivider(sizeint)
    if configdict['largestonly']:
        if mb and not configdict['forcekb']:
            return stringround(mb, kb)+ space + configdict['meganame']
        elif kb or configdict['forcekb']:
            if mb and configdict['forcekb']:
                kb += 1024*mb
            return stringround(kb, rb) + space+ configdict['kiloname']
        else:
            return str(rb) + space + configdict['bytename']
    else:
        outstr = ''
        if mb and not configdict['forcekb']:
            outstr = str(mb) + space + configdict['meganame'] +', '
        if kb or configdict['forcekb'] or mb:
            if configdict['forcekb']:
                kb += 1024*mb
            outstr += str(kb) + space + configdict['kiloname'] +', '
        return outstr + str(rb) + space + configdict['bytename']

def stringround(main, rest):
    """
    Given a file size in either (mb, kb) or (kb, bytes) - round it
    appropriately.
    """
    # divide an int by a float... get a float
    value = main + rest/1024.0
    return str(round(value, 1))

def bytedivider(nbytes):
    """
    Given an integer (probably a long integer returned by os.getsize() )
    it returns a tuple of (megabytes, kilobytes, bytes).
    
    This can be more easily converted into a formatted string to display the
    size of the file.
    """
    mb, remainder = divmod(nbytes, 1048576)
    kb, rb = divmod(remainder, 1024)
    return (mb, kb, rb)

########################################

def fullcopy(src, dst):
    """
    Copy file from src to dst.
    
    If the dst directory doesn't exist, we will attempt to create it using makedirs.
    """
    import shutil
    if not os.path.isdir(os.path.dirname(dst)):
        os.makedirs(os.path.dirname(dst))
    shutil.copy(src, dst)    

#######################################

def import_path(fullpath, strict=True):
    """
    Import a file from the full path. Allows you to import from anywhere,
    something ``__import__`` does not do.
    
    If strict is ``True`` (the default), raise an ``ImportError`` if the module
    is found in the "wrong" directory.
    
    Taken from firedrop2_ by `Hans Nowak`_
    
    .. _firedrop2: http://www.voidspace.org.uk/python/firedrop2/
    .. _Hans Nowak: http://zephyrfalcon.org
    """
    path, filename = os.path.split(fullpath)
    filename, ext = os.path.splitext(filename)
    sys.path.insert(0, path)
    try:
        module = __import__(filename)
    except ImportError:
        del sys.path[0]
        raise
    del sys.path[0]
    #
    if strict:
        path = os.path.split(module.__file__)[0]
        # FIXME: doesn't *startswith* allow room for errors ?
        if not fullpath.startswith(path):
            raise ImportError, "Module '%s' found, but not in '%s'" % (
                  filename, fullpath)
    #
    return module

"""

Changelog
==========
2005/08/28      Version 0.2.1
Added import_path
Added __all__
Code cleanup

2005/06/01      Version 0.2.0
Added walkdirs generator.


2005/03/11      Version 0.1.1
Added rounding to formatbytes and improved bytedivider with divmod.
Now explicit keyword parameters override the configdict in formatbytes.

2005/02/18      Version 0.1.0
The first numbered version.
"""

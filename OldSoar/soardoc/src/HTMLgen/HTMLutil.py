#!/usr/bin/env python
"""This module provides utility functions/classes associated with HTMLgen.

This is functionality use by HTMLgen script writers rather than by HTMLgen
itself. (i.e. HTMLgen.py is not dependant on this module.)
"""
# HTMLutil.py
# COPYRIGHT (C) 1996-7  ROBIN FRIEDRICH
# Permission to  use, copy, modify, and distribute this software and its
# documentation  for  any  purpose  and  without fee  is hereby granted,
# provided that the above copyright notice appear in all copies and that
# both that copyright notice and this permission notice appear in
# supporting documentation.
# THE  AUTHOR  DISCLAIMS  ALL  WARRANTIES WITH  REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
# EVENT  SHALL THE  AUTHOR  BE  LIABLE  FOR  ANY  SPECIAL,   INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
# USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
__version__ = '$Id$'
import string, regex, os
import HTMLgen, HTMLcolors
from types import *

##### Escape diacritically marked text with HTML standard encoding #####
def latin1_escape(s):
    """Return string with special characters encoded to HTML standard.

    Works on ASCII characters in range 160-255.
    """
    slist = map(None, s)
    for i in range(len(slist)):
        ordinal = ord(slist[i])
        if 159 < ordinal < 256:
            slist[i] = "&#%d;" % ordinal
    return string.join(slist, '')

#############  Colorize Python Source  ###############
#           Ted Turner would love this:-)

def markup(filename):
    """Colorize Python source code.

    Pass filename.  Returns instance of the HTMLgen.Pre class ready
    for insertion into a document object.
    
    Strings are marked as green, while comments are made red.
    doc strings are made Blue. Functions are made Bold and
    classes Copper.
    """
    source = open(filename).read()
    # it's important to mark the strings first as the HTML tags
    # will be used to detect if key characters (like #) occur in
    # string literals.
    #t0 = time.clock()
    source = global_substitute(find_string_literal, Green, source)
    source = global_substitute(find_comment, Red, source)
    source = global_substitute(find_docstring, Blue, source)
    source = global_substitute(find_function, Copper, source)
    source = global_substitute(find_class, Bold, source)
    #print time.clock() - t0
    return HTMLgen.Pre(source)

def test_markup():
    filename = 'HTMLgen.py'
    d = HTMLgen.SimpleDocument(bgcolor=HTMLcolors.WHITE, title=filename)
    d.append(markup(filename))
    d.write('html/HTMLgen_src.html')

# Series of simple functions (class instances) which wrap text in HTML 
Red = HTMLgen.Font(color=HTMLcolors.RED3)
Blue = HTMLgen.Font(color=HTMLcolors.BLUE3)
Green = HTMLgen.Font(color=HTMLcolors.GREEN6)
Copper = HTMLgen.Font(color=HTMLcolors.COPPER)
Bold = HTMLgen.Strong()

def been_marked(text): 
    """Determine if the text have been marked by a previous gsub.
    (ugly hack but it works)
    """
    if regex.search('\(</?FONT\)\|\(</?STRONG\)', text) > -1:
        return 1
    else: 
        return 0

def global_substitute(search_func, repl_func, s):
    """Global substitution function.

    Returns a new string after having the repl_func process the text
    found by the search_func.

    search_func -- A function which takes a string and a starting index
                   and returns a tuple of the beginning and ending index
                   where a text pattern is found. Should return (None, None)
                   if nothing is found.
    repl_func   -- A function which takes a string and returns a new string
                   which has been processed in some way (ie.,marked up).
    s           -- The string to act on.
    """
    import array
    # I use character array here because it's faster when concatenating
    # longish strings. (I saved 40% in this case.)
    new = array.array('c')
    start = 0
    while 1:
        begin, end = search_func(s, start)
        if begin != None:
            changestring = s[begin:end]
            # want to escape the special characters ONLY if it has
            # had a previous FONT/STRONG tag applied.
            if not been_marked(changestring):
                changestring = HTMLgen.escape(changestring) 
            # Wrap changestring in a RawText object to prevent it from
            # being re-escaped by the Font class.
            new.fromstring( s[start:begin] )
            new.fromstring( repl_func(HTMLgen.RawText(changestring)) )
            start = end
        else:
            new.fromstring( s[start:] )
            break
    return new.tostring()

# Stuff used by find_docstring()
not_backslash = "[^\\\\]"
triple_single = "'''"
triple_double = '"""'
_doc_start_re = regex.compile(
    "\(^\|" + not_backslash + "\)" # bol or not backslash
    + "\(" + triple_single + "\|" + triple_double + "\)" )
single_re = not_backslash + triple_single
double_re = not_backslash + triple_double
_triple_re = { triple_single : regex.compile(single_re),
               triple_double : regex.compile(double_re) }

del not_backslash, triple_single, triple_double, \
    single_re, double_re

def find_docstring( s, begin=0):
    """return (b,e) s.t. s[b:e] is the leftmost triple-quoted
    string in s (including the quotes), or (None, None) if s
    doesn't contain a triple-quoted string
    """
    if _doc_start_re.search( s, begin ) < 0:
        return (None, None)
    startquote, endquote = _doc_start_re.regs[2]
    quotestring = s[startquote:endquote] # """ or '''
    quotefinder = _triple_re[quotestring]
    if quotefinder.search( s, endquote ) < 0:
        return (None, None)
    return startquote, quotefinder.regs[0][1]

string_re = regex.compile('\(\(\'[^\'\n]*\'\)\|\("[^"\n]"\)\)')
def find_string_literal( s, begin=0 ):
    if string_re.search(s, begin) > -1:
        return string_re.regs[1]
    return (None, None)

comment_re = regex.compile('#.*$')
def find_comment( s, begin=0 ):
    while comment_re.search(s, begin) > -1:
        if been_marked(comment_re.group(0)):
            begin = comment_re.regs[0][1]
            continue
        return comment_re.regs[0]
    return (None, None)

Name = '[a-zA-Z_][a-zA-Z0-9_]*'
func_re = regex.compile('\(^[ \t]*def[ \t]+' +Name+ '\)[ \t]*(') 
def find_function( s, begin=0 ):
    if func_re.search(s, begin) > -1:
        return func_re.regs[1]
    return (None, None)

class_re = regex.compile('\(^[ \t]*class[ \t]+' +Name+ '\)[ \t]*[(:]')
def find_class( s, begin=0 ):
    if class_re.search(s, begin) > -1:
        return class_re.regs[1]
    return (None, None)



########## stuff associated with converting imagemap specs ######
shape_names = {'rect':'rect', 'circ':'circle', 'poly':'polygon'}

def imap_convert(filename, format='NCSA'):
    """Convert server side imagemap format to client side HTML format.

    filename -- ASCII file containing the server imagemap specification
    format   -- Either 'NCSA'(default) or 'CERN'
    """
    f = open(filename)
    lines = f.readlines()
    f.close
    if format == 'NCSA':
        parse_function = parse_ncsa_line
    elif format == 'CERN':
        parse_function = parse_cern_line
    else:
        raise ValueError, 'Second arg has to be either NCSA or CERN'
    print 'Parsing', filename, 'assuming the', format, 'imagemap format'
    for line in lines:
        line = string.strip(line)
        print parse_function(line)

def parse_ncsa_line(line):
    """NCSA format looks like:
    default 
    poly blue.html 205,106 153,106 152,118 204,135 205,106 
    circ hole.html 103,96 132,121
    rect recta.html 51,47 201,75
    """
    line_elements = string.split(line, ' ')
    shape = line_elements[0]
    url = line_elements[1]
    if shape in ('rect', 'poly'):
        coords = string.join(line_elements[2:], ',')
    elif shape == 'circ':
        [x1, y1] = string.split(line_elements[2], ',')
        [x2, y2] = string.split(line_elements[3], ',')
        [x1,y1,x2,y2] = map(string.atoi, [x1,y1,x2,y2])
        r = (x2-x1)/2
        x0 = r + x1 
        y0 = r + y1 
        coords = "%d,%d,%d" % (x0,y0,r)
    return HTMLgen.Area(shape=shape_names[shape], href=url, coords=coords)

def parse_cern_line(line):
    """CERN format looks like:
    rect (51,47) (201,75) recta.html
    circ (117,110) 14 hole.html
    poly (205,106) (153,106) (152,118) (204,135) (205,106) blue.html
    """
    line_elements = string.split(line, ' ')
    shape = line_elements[0]
    url = line_elements[-1]
    if shape in ('rect', 'poly'):
        coord_list = map(lambda x: x[1:-1], line_elements[1:-1])
        coords = string.join(coord_list, ',')
    elif shape == 'circ':
        coords = line_elements[1][1:-1]
        coords = coords + ',' + line_elements[2]
    return HTMLgen.Area(shape=shape_names[shape], href=url, coords=coords)

def test_imap_convert():
    print 'testing NCSA format parsing'
    lol = string.split(parse_ncsa_line.__doc__, '\n')
    for line in lol:
        line = string.strip(line)
        if line[:4] in ('rect','circ','poly'):
            print parse_ncsa_line(line)
    print 'testing CERN format parsing'
    lol = string.split(parse_cern_line.__doc__, '\n')
    for line in lol:
        line = string.strip(line)
        if line[:4] in ('rect','circ','poly'):
            print parse_cern_line(line)


#########################################            
from UserList import UserList

class Directory(UserList):
    """Object representing a directory tree structure.

    Instances of this class can be populated with objects
    at the end of a path list. The path list is a list of
    strings representing directory names leading to the
    target value. The data structure can be emitted in a
    format condusive to processing by HTMLgen.List and
    friends.

    For example:

      D = Directory()
      D.add_object(['path','to','directory'], object)
      [would be analogous to creating a file "/path/to/directory/object"]
      ... repeated for any number of objects ...
      LoL = D.tree()
      html_list = HTMLgen.List(LoL)

    Unlike a file system which requires that a directory
    to exist before a file can be added; Directory will
    automatically create new directories in the path as
    needed.
    """
    def __init__(self, name='root', data=None):
        UserList.__init__(self, data)
        self.name = name

    def __cmp__(self, item):
        return cmp(self.name, item)

    def add_object(self, pathlist, object):
        """Add a new object into the directory structure.

        *pathlist* is a list of strings to be used as directory
        names leading to the object. If a subdirectory name does
        not exist one will be created automatically.

        *object* can be any python object. 
        """
        if pathlist:
            name = pathlist[0]
            del pathlist[0]
            try:
                i = self.index(name)
                self.data[i].add_object(pathlist, object)
            except ValueError:
                self.append(Directory(name))
                self.data[-1].add_object(pathlist, object)
        else:
            self.append(object)

    def tree(self):
        """Return the Directory object as a list of items and
        nested lists, aka tree, suitable for use in a HTMLgen.List
        class.
        """
        list = []
        for item in self.data:
            if type(item) is InstanceType:
                try:
                    list = list + item.tree()
                except AttributeError:
                    list.append(repr(item))
            else:
                list.append(item)
        return [self.name, list]

    def ls(self, pad=''):
        """Print an indented representation of the entire directory
        contents.
        """
        print pad, self.name+'/'
        pad = pad + '   '
        for item in self.data:
            if type(item) is InstanceType:
                try:
                    item.ls(pad)
                except AttributeError:
                    print id(item), '[non-directory object]'
            else:
                print pad, item

def test_Directory():
    import string
    p1 = '/usr/local/bin/gnutar'
    p2 = '/usr/local/bin/guitar'
    p3 = '/usr/local/lib/libmath.so'
    p4 = '/usr/local/lib/libC.so'
    p5 = '/usr/lib/libXm.so'
    p6 = '/usr/lib/libXt.so'
    p7 = '/var/adm/Log_78'
    p8 = '/etc/rc2.d/Start'
    D = Directory()
    for fullpath in (p1,p2,p3,p4,p5,p6,p7,p8):
        fullpath = string.split(fullpath, '/')[1:]
        object = fullpath[-1]
        path = fullpath[:-1]
        D.add_object(path, object)
    T = D.tree()
    print T
    D.ls()
    import HTMLgen
    print HTMLgen.List(T)
    print 'HOW IS THIS?'
    
###################
# stuff to test performance idea. Looks like character
# arrays are only faster when strings get large otherwise
# they are slower. (Un)fortunately HTMLgen strings tend
# to be short.
def make_string(iterations = 10 ):
    s = ''
    for i in range(iterations):
        s = s + 'A string to be contatenated. '
        s = s + 'A string to be contatenated. '
        s = s + 'A string to be contatenated. '
        s = s + 'A string to be contatenated. '
        s = s + 'A string to be contatenated. '
    return s
import array
def make_array(iterations = 10):
    s = array.array('c')
    for i in range(iterations):
        s.fromstring('A string to be contatenated. ')
        s.fromstring('A string to be contatenated. ')
        s.fromstring('A string to be contatenated. ')
        s.fromstring('A string to be contatenated. ')
        s.fromstring('A string to be contatenated. ')
    return s.tostring()
def make_list(iterations = 10): #This is what we'll use
    s = []
    for i in range(iterations):
        s.append('A string to be contatenated. ')
        s.append('A string to be contatenated. ')
        s.append('A string to be contatenated. ')
        s.append('A string to be contatenated. ')
        s.append('A string to be contatenated. ')
    return string.join(s, '')
import time
def test_perf(iter=10):
    t0 = time.clock()
    x = make_string(iter)
    print time.clock() - t0, 'String', len(x)
    t0 = time.clock()
    y = make_array(iter)
    print time.clock() - t0, 'Array', len(y)
    t0 = time.clock()
    y = make_list(iter)
    print time.clock() - t0, 'List', len(y)

####
# getting those damned ..'s out of file paths
def expand_dir_path(path):
    """Return fully qualified pathname for the given directory path.
    """
    savecwd = os.getcwd()
    os.chdir(path)
    cwd = os.getcwd()
    os.chdir(savecwd)
    return cwd

def expand_file_path(path):
    """Return fully qualified pathname for the given file path.
    """
    directory, filename = os.path.split(path)
    return os.path.join(expand_dir_path(directory), filename)

def expandpath(path):
    """Expand (fully qualify) an arbitrary path to an existing file or directory.

    If path does not map to an existing file the pathname is returned
    unchanged.
    """
    if os.isdir(path):
        return expand_dir_path(path)
    elif os.isfile(path):
        return expand_file_path(path)
    else:
        return path
    
###########  Organizing your images  ###########

def image_inventory(directory=os.curdir, prefix=None, flatten=None):
    """Return a dictionary containing Image objects derived from all
    GIF/JPEG/PNG files contained in the given directory.

    Arguments

       directory -- Directory containing image files.
       prefix    -- If known, furnish a file prefix for each Image object.
                    See Image class for further details.
       flatten   -- If not None, repr the Image objects out to strings.
    """
    import glob, os
    savedir = os.getcwd()
    os.chdir(directory)
    cwd = os.getcwd()
    pix = glob.glob('*.gif') + glob.glob('*.GIF')
    pix = pix + glob.glob('*.jpg') + glob.glob('*.JPG')
    pix = pix + glob.glob('*.png') + glob.glob('*.PNG')
    inventory = {}
    for file in pix:
        altname = os.path.splitext(file)[0]
        inventory[file] = HTMLgen.Image(file,
                                        alt = altname,
                                        prefix = prefix,
                                        absolute = cwd)
        if flatten: inventory[file] = str(inventory[file])
    os.chdir(savedir)
    return inventory

def store_image_inventory(directory=os.curdir, file='images.pkl', prefix=None):
    """Save a dictionary listing all image objects to a file.
    
    The dictionary containing all Image objects derived from a
    given directory can be saved either as a pickle file to preserve
    the Image objects or as string HTML representations savable in a
    marshaled form for performance.  Which form happens is a function
    of the file suffix given as the second argument. If it ends in ".mar"
    then the Image objects are flattened into plain strings and saved
    as a marshal file. Any other suffix, including the default ".pkl"
    will save the dictionary of objects as a pickle file. Note: the
    marshal form is about two orders of magnitude faster when fetching
    this dictionary.

    Arguments

       directory -- Directory to generate Image objects from and
                    store the resulting dictionary into.
       file -- Optional name of file to save into. default is "images.pkl"
               If the file suffix is '.mar' the Image objects will be
               flattened into regular strings and saved as a marshalled
               file, (a performance route when you know the SRC prefix
               ahead of time).
       prefix -- Optional specification of the Image prefix. Typically
                 used when you what to specify a known URL path for
                 your image repository, eg. http:/www.phunni.com/image.
    """
    import os
    if file[-3:] == 'mar':
        import marshal
        dump = marshal.dump
        flatten = 'Yes'
    else:
        import pickle
        dump = pickle.dump
        flatten = None
    images = image_inventory(directory, prefix, flatten)
    dumpfile = os.path.join(directory, file)
    f = open(dumpfile, 'w')
    dump(images, f)
    f.close()
    print len(images), 'Image references saved in', dumpfile

def fetch_image_inventory(directory, file='images.pkl'):
    """Return a dict of Image objects from a pickle or marshal file

    Optional file argument to specify a file other than
    'images.pkl'. If the file ends in '.mar', it's assumed to be a
    marshalled file containing a single dictionary.  """

    import os
    if file[-3:] == 'mar':
        import marshal
        load = marshal.load
    else:
        import pickle
        load = pickle.load
    file = os.path.join(directory, file)
    #t0 = time.clock()
    f = open(file)
    dict = load(f)
    f.close()
    #print file, time.clock() - t0, 'seconds'
    return dict

def test_image_inventory():
    # make image/images.pkl
    store_image_inventory('./image')
    # Make a marshal file of preprocessed image references
    store_image_inventory('./image', 'images.mar', 'http://www.python.org/image')
    # Read them back into memory
    D1 = fetch_image_inventory('./image') # unpickle
    D2 = fetch_image_inventory('./image', 'images.mar') # unmarshal
    if len(D1) == len(D2):
        print 'Tests OK'
        os.unlink('./image/images.pkl')
        os.unlink('./image/images.mar')
    else:
        print 'Something is amiss.'

def print_image_sizes(dir=os.curdir):
    """Calculate image sizes.
    """
    import ImageH, glob
    os.chdir(dir)
    filelist = []
    suffixlist = ('*.gif','*.GIF','*.jpg','*.JPG','*.jpeg','*.JPEG','*.png','*.PNG')
    for suffix in suffixlist:
        filelist = filelist + glob.glob1('', suffix)
    for file in filelist:
        (width, height) = ImageH.open(file).size
        print "('%s', %d, %d)" % (file, width, height)

def test_template():
    'Test TemplateDocument class.'
    X = {'substitutions':HTMLgen.Bold('substitutions'),
         'TemplateDocument': HTMLgen.Emphasis('TemplateDocument'),
         'std_disclaimer': """The Agency disavows any knowledge of your actions, Mr. Phelps."""}
    T = HTMLgen.TemplateDocument('data/templatefile.html')
    T.substitutions = X
    T.write('html/templatetest.html')
    
###########################
def test_driver():
    things2test = ( ('Test the Python source colorizer?', test_markup),
                    ('Test the imagemap format converter?', test_imap_convert),
                    ('Test the Directory class?', test_Directory),
                    ('Test Image inventory tools?', test_image_inventory),
                    ('Test TemplateDocument class?', test_template) )
    print '    Select one or more tests to perform, just hit return if none.'
    i = 0
    for (question, function) in things2test:
        print '[%d] %s' % (i+1, question)
        i = i + 1
    answer = raw_input('Type numbers separated by commas: ')
    if answer:
        try:
            answers = map(string.atoi, map(string.strip, string.split(answer, ',')))
            for item in answers:
                things2test[item-1][1]()
        except (ValueError, IndexError):
            print "Input error."
    
if __name__ == '__main__':
    import sys
    if sys.argv[-1] == '-s':
        test_markup()
        test_template()
    else:
        test_driver()
        
    

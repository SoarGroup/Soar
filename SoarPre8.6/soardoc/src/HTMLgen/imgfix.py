#!/usr/bin/env python

"""Examine and fix the height/width attributes on IMG tags in the given files.
"""
#'$Id$'

# COPYRIGHT (C) 1998  ROBIN FRIEDRICH  email:Robin.Friedrich@pdq.net
# Permission to use, copy, modify, and distribute this software and
# its documentation for any purpose and without fee is hereby granted,
# provided that the above copyright notice appear in all copies and
# that both that copyright notice and this permission notice appear in
# supporting documentation.
# THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
# ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL THE
# AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
# DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

import re, sys, getopt, os
from string import lower
from imgsize import imgsize

USAGE = """imgfix.py [-c] [-n] [-d /docrootdir] file [more files]
    -c Conservative mode will assume all existing img tag properties are
       correct and will only add width/height where they do not exist.
    -n No-Op mode. Prints what imgfix would do. No files touched.
    -d /docrootdir  provide a directory path to use when img src tags use
       absolute paths (i.e. start with /). Used to locate image files only.
    file(s) containing HTML IMG tags to be fixed.
    """
cwd = os.getcwd()
def main(filename):
    os.chdir(cwd)
    dirname, filename = os.path.split(filename)
    os.chdir(dirname or cwd)
    
    f = open(filename)
    s = f.read()
    f.close()
    s2, n = imgprog.subn(fixsize, s)
    if not NOOP:
        g = open(filename+'_', 'w')
        g.write(s2)
        g.close()
        os.rename(filename, filename+'~')
        os.rename(filename+'_', filename)
    print "%s, %d image tag(s)" % (filename, n)
    
def fixsize(imgmatch):
    """fixsize(imgmatch) imgmatch is a match object containing an IMG tag.
    Will return a new IMG tag containing verified height and width properties.
    All other properties are left alone.
    """
    imgcontent = imgmatch.group(1)
    if NOOP: print imgmatch.group(0), "<<< Original"
    attrs = ['src', 'width', 'height']
    dict = {}
    i = 0
    while 1:
        m = atrprog.search(imgcontent, i)
        if m is None: break
        name, value = lower(m.group(1)), unquote(m.group(2))
        i = m.end(1)
        if not attrs.count(name):
            attrs.append(name)
        dict[name] = value
    # CONSERV set true means if the IMG has an existing width property trust it.
    if CONSERV and dict.has_key('width')and dict.has_key('height'):
        return imgmatch.group(0)
    srcpath = dict['src']
    if srcpath[0] == '/': #SRC path starting with / will be relative to docroot
        srcpath = os.path.join(DOCROOT, srcpath[1:])
    try:
        f = open(srcpath)
        dict['width'], dict['height'] = imgsize(srcpath)
    except IOError: # if file not found or not a valid graphic format
        return imgmatch.group(0)  #leave it unchanged

    s = '<img'
    for name in attrs:
        s = s + ' %s="%s"' % (name, dict[name])
    s = s + '>'
    if NOOP: print s, "<<< New Value"
    return s

def unquote(s):
    if not s: return s
    if (s[0] == '"' and s[-1] == '"') or \
       (s[0] == "'" and s[-1] == "'"):
        s = s[1:-1]
    return s

#I placed these regexes at the end because they mess up Xemacs fontlock mode.
imgprog = re.compile(r'<\s*IMG([^>]+)\s*>', re.IGNORECASE)
atrprog = re.compile(r'''\s*(\w+)\s*=\s*("[^"]*"|'[^']*'|\S+)''')

NOOP = 0
CONSERV = 0
DOCROOT = os.curdir
if __name__ == '__main__':
    try:
        opts, args = getopt.getopt(sys.argv[1:], "ncd:")
    except getopt.error:
        print USAGE
        sys.exit(1)
    for opt, optarg in opts:
        if opt == '-n':
            NOOP = 1
        elif opt == '-c':
            CONSERV = 1
        elif opt == '-d':
            DOCROOT = optarg
    for filename in args:
        main(filename)

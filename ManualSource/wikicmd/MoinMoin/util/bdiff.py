"""
    MoinMoin - Binary patching and diffing

    @copyright: 2005 Matt Mackall <mpm@selenic.com>,
                2006 MoinMoin:AlexanderSchremmer

    Algorithm taken from mercurial's mdiff.py

    @license: GNU GPL, see COPYING for details.
"""

import zlib, difflib, struct

BDIFF_PATT = ">lll"
BDIFF_PATT_SIZE = struct.calcsize(BDIFF_PATT)

def compress(text):
    return zlib.compress(text) # here we could tune the compression level

def decompress(bin):
    return zlib.decompress(bin)

def diff(a, b):
    """ Generates a binary diff of the passed strings.
        Note that you can pass arrays of strings as well.
        This might give you better results for text files. """
    if not a:
        s = "".join(b)
        return s and (struct.pack(BDIFF_PATT, 0, 0, len(s)) + s)

    bin = []
    la = lb = 0

    p = [0]
    for i in a: p.append(p[-1] + len(i))

    for am, bm, size in difflib.SequenceMatcher(None, a, b).get_matching_blocks():
        s = "".join(b[lb:bm])
        if am > la or s:
            bin.append(struct.pack(BDIFF_PATT, p[la], p[am], len(s)) + s)
        la = am + size
        lb = bm + size

    return "".join(bin)

def textdiff(a, b):
    """ A diff function optimised for text files. Works with binary files as well. """
    return diff(a.splitlines(1), b.splitlines(1))

def patchtext(bin):
    """ Returns the new hunks that are contained in a binary diff."""
    pos = 0
    t = []
    while pos < len(bin):
        p1, p2, l = struct.unpack(BDIFF_PATT, bin[pos:pos + BDIFF_PATT_SIZE])
        pos += BDIFF_PATT_SIZE
        t.append(bin[pos:pos + l])
        pos += l
    return "".join(t)

def patch(a, bin):
    """ Patches the string a with the binary patch bin. """
    c = last = pos = 0
    r = []

    while pos < len(bin):
        p1, p2, l = struct.unpack(BDIFF_PATT, bin[pos:pos + BDIFF_PATT_SIZE])
        pos += BDIFF_PATT_SIZE
        r.append(a[last:p1])
        r.append(bin[pos:pos + l])
        pos += l
        last = p2
        c += 1
    r.append(a[last:])

    return "".join(r)

def test():
    a = ("foo\n" * 30)
    b = ("  fao" * 30)

    a = file(r"test.1").read()
    b = file(r"test.2").read()
    a = a.splitlines(1)
    b = b.splitlines(1)

    d = diff(a, b)
    z = compress(d)
    print repr(patchtext(d))
    print repr(d)
    print "".join(b) == patch("".join(a), d)
    print len(d), len(z)


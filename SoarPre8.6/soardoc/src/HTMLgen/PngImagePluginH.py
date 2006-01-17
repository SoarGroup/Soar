#
# The Python Imaging Library.
# $Id$
#
# PNG support code
#
# See "PNG (Portable Network Graphics) Specification, version 1.0;
# W3C Recommendation", 1996-10-01, Thomas Boutell (ed.).
#
# history:
# 0.1	96-05-06 fl	Created (couldn't resist it)
# 0.2	96-12-14 fl	Upgraded, added read and verify support
#	96-12-15 fl	Separate PNG stream parser
# 	96-12-29 fl	Added write support, added getchunks
# 0.3	96-12-30 fl	Eliminated circular references in decoder
#
# Copyright (c) Secret Labs AB 1997.
# Copyright (c) Fredrik Lundh 1996.
#
# See the README file for information on usage and redistribution.
#

__version__ = "0.3"

import string

import ImageH, ImageFileH, ImagePaletteH


def i16(c):
    return ord(c[1]) + (ord(c[0])<<8)
def i32(c):
    return ord(c[3]) + (ord(c[2])<<8) + (ord(c[1])<<16) + (ord(c[0])<<24)


_MAGIC = "\211PNG\r\n\032\n"


_MODES = {
    # supported bits/color combinations, and corresponding modes/rawmodes
    (1, 0): ("1", "1"),
    (2, 0): ("L", "L;2"),
    (4, 0): ("L", "L;4"),
    (8, 0): ("L", "L"),
    (16,0): ("L", "L;16B"),
    (8, 2): ("RGB", "RGB"),
    (16,2): ("RGB", "RGB;16B"),
    (1, 3): ("P", "P;1"),
    (2, 3): ("P", "P;2"),
    (4, 3): ("P", "P;4"),
    (8, 3): ("P", "P"),
    (8, 4): ("RGBA", "LA"),
    (16,4): ("RGBA", "LA;16B"),
    (8, 6): ("RGBA", "RGBA"),
    (16,6): ("RGBA", "RGBA;16B"),
}


# --------------------------------------------------------------------
# Support classes.  Suitable for PNG and related formats like MNG etc.

class ChunkStream:

    def __init__(self, fp):

	self.fp = fp
	self.queue = []

	if not hasattr(ImageH.core, "crc32"):
	    self.crc = self.crc_skip

    def read(self):
	"Fetch a new chunk. Returns header information."

	if self.queue:
	    cid, pos, len = self.queue[-1]
	    del self.queue[-1]
	    self.fp.seek(pos)
	else:
	    s = self.fp.read(8)
	    cid = s[4:]
	    pos = self.fp.tell()
	    len = i32(s)

	return cid, pos, len

    def close(self):
	del self.queue
	self.fp = None

    def push(self, cid, pos, len):

	self.queue.append((cid, pos, len))

    def call(self, cid, pos, len):
	"Call the appropriate chunk handler"

	if ImageH.DEBUG:
	    print "STREAM", cid, pos, len
	return getattr(self, "chunk_" + cid)(pos, len)

    def crc(self, cid, data):
	"Read and verify checksum"

	crc1 = ImageH.core.crc32(data, ImageH.core.crc32(cid))
	crc2 = i16(self.fp.read(2)), i16(self.fp.read(2))
	if crc1 != crc2:
	    raise SyntaxError, "broken PNG file"\
		"(bad header checksum in %s)" % cid

    def crc_skip(self, cid, data):
	"Read checksum.  Used of the C module is not present"

	self.fp.read(4)

    def verify(self, endchunk = "IEND"):

	# Simple approach; just calculate checksum for all remaining
	# blocks.  Must be called directly after open.

	cids = []

	while 1:
	    cid, pos, len = self.read()
	    if cid == endchunk:
		break
	    self.crc(cid, self.fp.read(len))
	    cids.append(cid)

	return cids


# --------------------------------------------------------------------
# PNG image stream (IHDR/IEND)

class PngStream(ChunkStream):

    def __init__(self, fp):

	ChunkStream.__init__(self, fp)

	# local copies of Image attributes
	self.im_info = {}
	self.im_size = (0,0)
	self.im_mode = None
	self.im_tile = None
	self.im_palette = None

    def chunk_IHDR(self, pos, len):
	
	# image header
	s = self.fp.read(len)
	self.im_size = i32(s), i32(s[4:])
	try:
	    self.im_mode, self.im_rawmode = _MODES[(ord(s[8]), ord(s[9]))]
	except:
	    pass
	if ord(s[12]):
	    self.im_info["interlace"] = 1
	if ord(s[11]):
	    raise SyntaxError, "unknown filter category"
	return s

    def chunk_IDAT(self, pos, len):

	# image data
	self.im_tile = [("zip", (0,0)+self.im_size, pos, self.im_rawmode)]
	self.im_idat = len
	raise EOFError

    def chunk_IEND(self, pos, len):

	# end of PNG image
	raise EOFError

    def chunk_PLTE(self, pos, len):
	
	# palette
	s = self.fp.read(len)
	if self.im_mode == "P":
	    self.im_palette = "RGB", s
	return s

    def chunk_gAMA(self, pos, len):

	# gamma setting
	s = self.fp.read(len)
	self.im_info["gamma"] = i32(s) / 100000.0
	return s

    def chunk_tEXt(self, pos, len):

	# text
	s = self.fp.read(len)
	[k, v] = string.split(s, "\0")
	self.im_info[k] = v
	return s


# --------------------------------------------------------------------
# PNG reader

def _accept(prefix):
    return prefix[:8] == _MAGIC

class PngImageFile(ImageFileH.ImageFile):

    format = "PNG"
    format_description = "Portable network graphics"

    def _open(self):

	if self.fp.read(8) != _MAGIC:
	    raise SyntaxError, "not a PNG file"

	#
	# Parse headers, up until the first IDAT chunk

	self.png = PngStream(self.fp)

	while 1:

	    #
	    # get next chunk

	    cid, pos, len = self.png.read()

	    try:
	        s = self.png.call(cid, pos, len)
	    except EOFError:
		break

	    except AttributeError:
		if ImageH.DEBUG:
		    print cid, pos, len, "(unknown)"
		s = self.fp.read(len)

	    self.png.crc(cid, s)

	#
	# Copy relevant attributes from the PngStream.  An alternative
	# would be to let the PngStream class modify these attributes
	# directly, but that introduces circular references which are
	# difficult to break no matter what happens in the decoders.
	# (believe me, I've tried ;-)

	self.mode = self.png.im_mode
	self.size = self.png.im_size
	self.info = self.png.im_info
	self.tile = self.png.im_tile
	if self.png.im_palette:
	    rawmode, data = self.png.im_palette
	    self.palette = ImagePaletteH.raw(rawmode, data)

	self.__idat = len # used by load_read()


    def verify(self):
	"Verify PNG file"

	# back up to beginning of IDAT block
	self.fp.seek(self.tile[0][2] - 8)

	self.png.verify()
	self.png.close()

	self.fp = None


    def load_read(self, bytes):
	"Read more data from chunks (used by ImageFile.load)"

	while self.__idat == 0:
	    # end of chunk, skip forward to next one

	    self.fp.read(4) # CRC

	    cid, pos, len = self.png.read()

	    if cid not in ["IDAT", "DDAT"]:
		self.png.push(cid, pos, len)
		return ""

	    self.__idat = len # empty chunks are allowed

	# read more data from this chunk
	if bytes <= 0:
	    bytes = self.__idat
	else:
	    bytes = min(bytes, self.__idat)

	self.__idat = self.__idat - bytes

	return self.fp.read(bytes)

#
# --------------------------------------------------------------------
# PNG writer

def o16(i):
    return chr(i>>8&255) + chr(i&255)

def o32(i):
    return chr(i>>24&255) + chr(i>>16&255) + chr(i>>8&255) + chr(i&255)

_OUTMODES = {
    # supported bits/color combinations, and corresponding modes/rawmodes
    "1":   ("1", chr(1)+chr(0)),
    "L;1": ("L;1", chr(1)+chr(0)),
    "L;2": ("L;2", chr(2)+chr(0)),
    "L;4": ("L;4", chr(4)+chr(0)),
    "L":   ("L", chr(8)+chr(0)),
    "P;1": ("P;1", chr(1)+chr(3)),
    "P;2": ("P;2", chr(2)+chr(3)),
    "P;4": ("P;4", chr(4)+chr(3)),
    "P":   ("P", chr(8)+chr(3)),
    "RGB": ("RGB", chr(8)+chr(2)),
    "RGBA":("RGBA", chr(8)+chr(6)),
}

def putchunk(fp, cid, *data):
    "Write a PNG chunk (including CRC field)"

    data = string.join(data, "")

    fp.write(o32(len(data)) + cid)
    fp.write(data)
    hi, lo = ImageH.core.crc32(data, ImageH.core.crc32(cid))
    fp.write(o16(hi) + o16(lo))

class _idat:
    # wrap output from the encoder in IDAT chunks

    def __init__(self, fp, chunk):
	self.fp = fp
	self.chunk = chunk
    def write(self, data):
	self.chunk(self.fp, "IDAT", data)

def _save(im, fp, filename, chunk = putchunk):
    # save an image to disk (called by the save method)

    mode = im.mode

    if mode == "P":

	#
	# attempt to minimize storage requirements for palette images

	if im.encoderinfo.has_key("bits"):

	    # number of bits specified by user
	    n = 1 << im.encoderinfo["bits"]

	else:

	    # check palette contents
	    n = 256 # FIXME

	if n <= 2:
	    bits = 1
	elif n <= 4:
	    bits = 2
	elif n <= 16:
	    bits = 4
	else:
	    bits = 8

	if bits != 8:
	    mode = "%s;%d" % (mode, bits)

    # encoder options
    if im.encoderinfo.has_key("dictionary"):
	dictionary = im.encoderinfo["dictionary"]
    else:
	dictionary = ""

    im.encoderconfig = (im.encoderinfo.has_key("optimize"), dictionary)

    # get the corresponding PNG mode
    try:
	rawmode, mode = _OUTMODES[mode]
    except KeyError:
	raise IOError, "cannot write mode %s as PNG" % mode

    #
    # write minimal PNG file

    fp.write(_MAGIC)

    chunk(fp, "IHDR",
	  o32(im.size[0]), o32(im.size[1]),	#  0: size
	  mode,					#  8: depth/type
	  chr(0),				# 10: compression
	  chr(0),				# 11: filter category
	  chr(0))				# 12: interlace flag

    if im.mode == "P":
	chunk(fp, "PLTE", im.im.getpalette("RGB"))

    if 0:
	# FIXME: to be supported some day
	chunk(fp, "gAMA", o32(int(gamma * 100000.0)))

    ImageFileH._save(im, _idat(fp, chunk), [("zip", (0,0)+im.size, 0, rawmode)])

    chunk(fp, "IEND", "")

    try:
        fp.flush()
    except: pass


# --------------------------------------------------------------------
# PNG chunk converter

def getchunks(im, **params):
    """Return a list of PNG chunks representing this image."""

    class collector:
        data = []
        def write(self, data):
	    pass
	def append(self, chunk):
	    self.data.append(chunk)

    def append(fp, cid, *data):
	data = string.join(data, "")
	hi, lo = ImageH.core.crc32(data, ImageH.core.crc32(cid))
	crc = o16(hi) + o16(lo)
	fp.append((cid, data, crc))

    fp = collector()

    try:
	im.encoderinfo = params
	_save(im, fp, None, append)
    finally:
	del im.encoderinfo

    return fp.data


# --------------------------------------------------------------------
# Registry

ImageH.register_open("PNG", PngImageFile, _accept)
ImageH.register_save("PNG", _save)

ImageH.register_extension("PNG", ".png")

ImageH.register_mime("PNG", "image/png")

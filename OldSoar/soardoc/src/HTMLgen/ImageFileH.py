#
# The Python Imaging Library.
# $Id$
#
# base class for image file handlers
#
# History:
#	95-09-09 fl	Created
#	96-03-11 fl	Fixed load mechanism.
#	96-04-15 fl	Added pcx/xbm decoders.
#	96-04-30 fl	Added encoders.
#	96-12-14 fl	Added load helpers
#	97-01-11 fl	Use encode_to_file where possible
#	97-08-27 fl	Flush output in _save
#	98-03-05 fl	Use memory mapping where possible
#
# Copyright (c) Secret Labs AB 1997-98.
# Copyright (c) Fredrik Lundh 1995-97.
#
# See the README file for information on usage and redistribution.
#


import ImageH
import traceback, sys

MAXBLOCK = 65536

#
# --------------------------------------------------------------------
# Helpers

def _tilesort(t1, t2):
    # sort on offset
    return cmp(t1[2], t2[2])

#
# --------------------------------------------------------------------
# ImageFile base class

class ImageFile(ImageH.Image):
    "Base class for image file format handlers."

    def __init__(self, fp = None, filename = None):

	self.im = None
	self.mode = ""
	self.size = (0, 0)
	self.palette = None
	self.tile = None
	self.info = {}

	self.decoderconfig = ()
	self.decodermaxblock = MAXBLOCK

        if type(fp) == type(""):
	    self.fp = open(fp, "rb")
	    self.filename = fp
	else:
	    self.fp = fp
	    self.filename = filename

	try:
	    self._open()
	except IndexError, v: # end of data
	    if ImageH.DEBUG > 1:
		traceback.print_exc()
	    raise SyntaxError, v
	except TypeError, v: # end of data (ord)
	    if ImageH.DEBUG > 1:
		traceback.print_exc()
	    raise SyntaxError, v
	except KeyError, v: # unsupported mode
	    if ImageH.DEBUG > 1:
		traceback.print_exc()
	    raise SyntaxError, v

	if not self.mode or self.size[0] <= 0:
	    raise SyntaxError, "not identified by this driver"

    def draft(self, mode, size):
	"Set draft mode"

	pass

    def verify(self):
	"Check file integrity"

	# raise exception if something's wrong.  must be called
	# directly after open, and closes file when finished.
	self.fp = None

    def load(self):
	"Load image data based on tile list"

	ImageH.Image.load(self)

	if self.tile == None:
	    raise IOError, "cannot load this image"
	if not self.tile:
	    return

	# ------------------------------------------------------------
	# memory mapping.  

	self.map = None

	if self.filename and len(self.tile) == 1:
	    d, e, o, a = self.tile[0]
	    if d == "raw" and a[0] in ("L", "P", "RGBX", "RGBA"):
		# FIXME: add support for "F" and "I" too
		try:
		    self.map = ImageH.core.map(self.filename)
		    self.map.seek(o)
		    self.im = self.map.readimage(
			self.mode, self.size, a[1], a[2]
			)
		except (AttributeError, IOError):
		    self.map = None

	# create image if necessary
	if not self.im or\
	   self.im.mode != self.mode or self.im.size != self.size:
	    self.im = ImageH.core.new(self.mode, self.size)

	# create palette (optional)
        if self.mode == "P":
            ImageH.Image.load(self)

	if not self.map:

	    # process tiles in file order
	    self.tile.sort(_tilesort)

	    for d, e, o, a in self.tile:
		d = ImageH._getdecoder(d, e, a, self.decoderconfig)
		self.load_seek(o)
		try:
		    d.setimage(self.im, e)
		except ValueError:
		    continue
		try:
		    # FIXME: This is a hack to handle TIFF's JpegTables tag.
		    b = self.tile_prefix
		except AttributeError:
		    b = ""
		t = len(b)
		while 1:
		    s = self.load_read(self.decodermaxblock)
		    if not s:
			self.tile = []
			raise IOError, "image file is truncated, %d bytes left" % len(b)
		    b = b + s
		    n, e = d.decode(b)
		    if n < 0:
			break
		    b = b[n:]
		    t = t + n

	self.tile = []
	self.fp = None # might be shared

	if not self.map and e < 0:
	    raise IOError, "decoder error %d when reading image file" % e

	# post processing
	if hasattr(self, "tile_post_rotate"):
	    # FIXME: This is a hack to handle rotated PCD's
	    self.im = self.im.rotate(self.tile_post_rotate)
	    self.size = self.im.size

	self.load_end()

    #
    # FIXME: don't like these.  make them go away...

    def load_end(self):
	# may be overridden
	pass

    def load_seek(self, pos):
	# may be overridden for contained formats
	self.fp.seek(pos)

    def load_read(self, bytes):
	# may be overridden for blocked formats (e.g. PNG)
	return self.fp.read(bytes)


#
# --------------------------------------------------------------------
# Save image body

def _save(im, fp, tile):
    "Helper to save image based on tile list"

    im.load()
    if not hasattr(im, "encoderconfig"):
	im.encoderconfig = ()
    tile.sort(_tilesort)
    bufsize = max(MAXBLOCK, im.size[0] * 4) # see RawEncode.c
    try:
	fh = fp.fileno()
	fp.flush()
    except AttributeError:
	# compress to Python file-compatible object
	for e, b, o, a in tile:
	    e = ImageH._getencoder(im.mode, e, a, im.encoderconfig)
	    if o > 0:
		fp.seek(o, 0)
	    e.setimage(im.im, b)
	    while 1:
		l, s, d = e.encode(bufsize)
		fp.write(d)
		if s:
		    break
	    if s < 0:
		raise IOError, "encoder error %d when writing image file" % s
    else:
	# slight speedup: compress to real file object
	for e, b, o, a in tile:
	    e = ImageH._getencoder(im.mode, e, a, im.encoderconfig)
	    if o > 0:
		fp.seek(o, 0)
	    e.setimage(im.im, b)
	    s = e.encode_to_file(fh, bufsize)
	    if s < 0:
		raise IOError, "encoder error %d when writing image file" % s
    try:
        fp.flush()
    except: pass

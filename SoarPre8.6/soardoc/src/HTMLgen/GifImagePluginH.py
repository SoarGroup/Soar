#
# The Python Imaging Library.
# $Id$
#
# GIF file handling
#
# History:
#	95-09-01 fl	Created
#	96-12-14 fl	Added interlace support
#	96-12-30 fl	Added animation support
#	97-01-05 fl	Added write support, fixed local colour map bug
#	97-02-23 fl	Make sure to load raster data in getdata()
#	97-07-05 fl	Support external decoder
#
# Copyright (c) Secret Labs AB 1997.
# Copyright (c) Fredrik Lundh 1995-97.
#
# See the README file for information on usage and redistribution.
#


__version__ = "0.4"


import array
import ImageH, ImageFileH, ImagePaletteH


# --------------------------------------------------------------------
# Helpers

def i16(c):
    return ord(c[0]) + (ord(c[1])<<8)

def o16(i):
    return chr(i&255) + chr(i>>8&255)


# --------------------------------------------------------------------
# Identify/read GIF files

def _accept(prefix):
    return prefix[:6] in ["GIF87a", "GIF89a"]

class GifImageFile(ImageFileH.ImageFile):

    format = "GIF"
    format_description = "Compuserve GIF"

    def data(self):
	s = self.fp.read(1)
	if s and ord(s):
	    return self.fp.read(ord(s))
	return None

    def _open(self):

	# Screen
	s = self.fp.read(13)
	if s[:6] not in ["GIF87a", "GIF89a"]:
	    raise SyntaxError, "not a GIF file"

	self.info["version"] = s[:6]

	self.size = i16(s[6:]), i16(s[8:])

	self.tile = []

	flags = ord(s[10])

	bits = (flags & 7) + 1

	if flags & 128:
	    # get global palette
	    self.info["background"] = ord(s[11])
	    self.global_palette = self.palette =\
		ImagePaletteH.raw("RGB", self.fp.read(3<<bits))

	self.fp2 = self.fp # FIXME: hack
	self.offset = 0
	self.dispose = None

	self.frame = -1
	self.seek(0) # get ready to read first frame

    def seek(self, frame):

	# FIXME: can only seek to next frame; should add rewind capability
	if frame != self.frame + 1:
	    raise ValueError, "cannot seek to frame %d" % frame
	self.frame = frame

	self.tile = []

	self.fp = self.fp2 # FIXME: hack
	if self.offset:
	    # backup to last frame
	    self.fp.seek(self.offset)
	    while self.data():
		pass
	    self.offset = 0

	if self.dispose:
	    self.im = self.dispose
	    self.dispose = None

	self.palette = self.global_palette

	while 1:

	    s = self.fp.read(1)
	    if not s or s == ";":
		break

	    elif s == "!":
		#
		# extensions
		# 
		s = self.fp.read(1)
		block = self.data()
		if ord(s) == 249:
		    #
		    # graphic control extension
		    #
		    flags = ord(block[0])
		    if flags & 1:
			self.info["transparency"] = ord(block[3])
		    self.info["duration"] = i16(block[1:3]) * 10
		    try:
			# disposal methods
			if flags & 8:
			    # replace with background colour
			    self.dispose = ImageH.core.fill("P", self.size,
				self.info["background"])
			elif flags & 16:
			    # replace with previous contents
			    self.dispose = self.im.copy()
		    except (AttributeError, KeyError):
			pass
		elif ord(s) == 255:
		    #
		    # application extension
		    #
		    self.info["extension"] = block, self.fp.tell()
		    if block[:11] == "NETSCAPE2.0":
			self.info["loop"] = 1 # FIXME
		while self.data():
		    pass

	    elif s == ",":
		#
		# local image
		#
		s = self.fp.read(9)

		# extent
		x0, y0 = i16(s[0:]), i16(s[2:])
		x1, y1 = x0 + i16(s[4:]), y0 + i16(s[6:])
		flags = ord(s[8])

		interlace = (flags & 64) != 0

		if flags & 128:
		    bits = (flags & 7) + 1
		    self.palette =\
			ImagePaletteH.raw("RGB", self.fp.read(3<<bits))

		# image data
		bits = ord(self.fp.read(1))
		self.offset = self.fp.tell()
		self.tile = [("gif",
			     (x0, y0, x1, y1),
			     self.offset,
			     (bits, interlace))]
		break

	    else:
		pass
		# raise IOError, "illegal GIF tag `%x`" % ord(s)

	if not self.tile:
	    self.fp2 = None
	    raise EOFError, "no more images in GIF file"

	self.mode = "L"
	if self.palette:
	    self.mode = "P"

    def tell(self):
	return self.frame


# --------------------------------------------------------------------
# Write GIF files

try:
    import _imaging_gif
except ImportError:
    _imaging_gif = None

RAWMODE = {
    "1": "L",
    "L": "L",
    "P": "P",
}

def _save(im, fp, filename):

    if _imaging_gif:
        # call external driver
        try:
            _imaging_gif.save(im, fp, filename)
            return
        except IOError:
            pass # write uncompressed file

    # header
    for s in getheader(im):
	fp.write(s)

    # local image header
    fp.write("," +
	     o16(0) + o16(0) +		# bounding box
	     o16(im.size[0]) +		# size
	     o16(im.size[1]) +
	     chr(0) +			# flags
	     chr(8))			# bits

    ImageFileH._save(im, fp, [("gif", (0,0)+im.size, 0, RAWMODE[im.mode])])

    fp.write("\0") # end of image data

    fp.write(";") # end of file

    try:
        fp.flush()
    except: pass

def _save_netpbm(im, fp, filename):

    #
    # If you need real GIF compression and/or RGB quantization, you
    # can use the external NETPBM/PBMPLUS utilities.  See comments
    # below for information on how to enable this.

    import os
    file = im._dump()
    if im.mode != "RGB":
	os.system("ppmtogif %s >%s" % (file, filename))
    else:
	os.system("ppmquant 256 %s | ppmtogif >%s" % (file, filename))
    try: os.unlink(file)
    except: pass


# --------------------------------------------------------------------
# GIF utilities

def getheader(im):
    """Return a list of strings representing a GIF header"""

    try:
        rawmode = RAWMODE[im.mode]
    except KeyError:
	raise IOError, "cannot save mode %s as GIF" % im.mode

    s = [
	"GIF87a" +		# magic
	o16(im.size[0]) +	# size
	o16(im.size[1]) +
	chr(7 + 128) +		# flags: bits + palette
	chr(0) +		# background
	chr(0)			# reserved/aspect
    ]

    # global palette
    if im.mode == "P":
	# colour palette
	s.append(im.im.getpalette("RGB"))
    else:
	# greyscale
	for i in range(256):
	    s.append(chr(i) * 3)

    return s

def getdata(im, offset = (0, 0), **params):
    """Return a list of strings representing this image.
       The first string is a local image header, the rest contains
       encoded image data."""

    class collector:
        data = []
        def write(self, data):
	    self.data.append(data)

    im.load() # make sure raster data is available

    fp = collector()

    try:
	im.encoderinfo = params

	# local image header
	fp.write("," +
		 o16(offset[0]) +	# offset
		 o16(offset[1]) +
		 o16(im.size[0]) +	# size
		 o16(im.size[1]) +
		 chr(0) +		# flags
		 chr(8))		# bits

	ImageFileH._save(im, fp, [("gif", (0,0)+im.size, 0, RAWMODE[im.mode])])

	fp.write("\0") # end of image data

    finally:
	del im.encoderinfo

    return fp.data


# --------------------------------------------------------------------
# Registry

ImageH.register_open(GifImageFile.format, GifImageFile, _accept)
ImageH.register_save(GifImageFile.format, _save)
ImageH.register_extension(GifImageFile.format, ".gif")
ImageH.register_mime(GifImageFile.format, "image/gif")

#
# Uncomment the following line if you wish to use NETPBM/PBMPLUS
# instead of the built-in "uncompressed" GIF encoder

# Image.register_save(GifImageFile.format, _save_netpbm)

#
# The Python Imaging Library.
# $Id$
#
# the Image class wrapper
#
# history:
#	95-09-09 fl	Created
#	96-03-11 fl	PIL release 0.0
#	96-04-30 fl	PIL release 0.1b1
#	96-05-27 fl	PIL release 0.1b2
#	96-10-04 fl	PIL release 0.2a1
#	96-11-04 fl	PIL release 0.2b1
#	96-12-08 fl	PIL release 0.2b2
#	96-12-16 fl	PIL release 0.2b3
#	97-01-14 fl	PIL release 0.2b4
#	97-06-02 fl	PIL release 0.3a1
#	97-08-27 fl	PIL release 0.3a2
#	98-02-02 fl	PIL release 0.3a3
#	98-03-09 fl	PIL release 0.3b1
#
# Copyright (c) Secret Labs AB 1997-98.
# Copyright (c) Fredrik Lundh 1995-97.
#
# See the README file for information on usage and redistribution.
#

VERSION = "0.3a3"

class _imaging_not_installed:
    def __getattr__(self, id):
	raise ImportError, "The _imaging C module is not installed"

try:
    # If the _imaging C module is not present, you can only use the
    # "open" function to identify files.  Most other operations will
    # fail.  Note that other modules should not refer to _imaging
    # directly; import Image and use the core variable instead.
    import _imaging
    core = _imaging
    del _imaging
except ImportError:
    core = _imaging_not_installed()

import ImagePaletteH
import os, string

# type stuff
from types import IntType, StringType, TupleType, ListType, FloatType, LongType
isStringType = lambda t: type(t) == StringType
isTupleType  = lambda t: type(t) == TupleType
isImageType  = lambda t: hasattr(t, "im")
#from operator import isNumberType, isSequenceType # not in JPython 1.0.3
def isNumberType(t):
    type(t) in (IntType, FloatType, LongType)
    
def isSequenceType(t):
    type(t) in (TupleType, ListType, StringType)

#
# Debug level

DEBUG = 0

#
# Constants (defined in _imagingmodule.c)

# transpose
FLIP_LEFT_RIGHT = 0
FLIP_TOP_BOTTOM = 1
ROTATE_90 = 2
ROTATE_180 = 3
ROTATE_270 = 4

# transforms
AFFINE = 0
EXTENT = 1
PERSPECTIVE = 2 # Not yet implemented

# resampling
NEAREST = 0
ANTIALIAS = 1

# categories
NORMAL = 0
SEQUENCE = 1
CONTAINER = 2

#
# Registries

ID = []
OPEN = {}
MIME = {}
SAVE = {}
EXTENSION = {}

#
# Helpers

_initialized = 0

def preinit():
    "Load standard file format drivers."

    global _initialized
    if _initialized >= 1:
	return

    for m in ["BmpImagePlugin", "GifImagePluginH", "JpegImagePluginH",
              "PpmImagePlugin", "TiffImagePlugin"]:
        try:
            __import__(m)
        except ImportError:
            pass # ignore missing driver for now

    _initialized = 1

def init():
    "Load all file format drivers."

    global _initialized
    if _initialized >= 2:
	return

    import os, sys

    # only check directories (including current, if present in the path)
    for path in filter(os.path.isdir, sys.path):
	for file in os.listdir(path):
	    if file[-15:] == "ImagePluginH.py":
		p, f = os.path.split(file)
		f, e = os.path.splitext(f)
		try:
		    sys.path.insert(0, path)
		    try:
			__import__(f)
		    finally:
			del sys.path[0]
		except ImportError:
		    if DEBUG:
			print "Image: failed to import",
			print f, ":", sys.exc_value

    if OPEN or SAVE:
	_initialized = 2


# --------------------------------------------------------------------
# Codec factories (used by tostring/fromstring and ImageFile.load)

def _getdecoder(d, e, a, ac = ()):

    # tweak arguments
    if a == None:
	a = ()
    elif type(a) != TupleType:
	a = (a,)

    try:
	# get decoder
	decoder = getattr(core, d + "_decoder")
	return apply(decoder, a + ac)
    except AttributeError:
	raise IOError, "decoder %s not available" % d

def _getencoder(mode, encoder_name, args, extra = ()):

    # tweak arguments
    if args == None:
	args = ()
    elif type(args) != TupleType:
	args = (args,)

    try:
	# get encoder
	encoder = getattr(core, encoder_name + "_encoder")
	return apply(encoder, (mode,) + args + extra)
    except AttributeError:
	raise IOError, "encoder %s not available" % encoder_name


# --------------------------------------------------------------------
# Simple expression analyzer

class _E:
    def __init__(self, data): self.data = data
    def __coerce__(self, other): return self, _E(other)
    def __add__(self, other): return _E((self.data, "__add__", other.data))
    def __mul__(self, other): return _E((self.data, "__mul__", other.data))

def _getscaleoffset(expr):
    stub = ["stub"]
    data = expr(_E(stub)).data
    try:
        (a, b, c) = data # simplified syntax
        if (a is stub and b == "__mul__" and isNumberType(c)):
            return c, 0.0
        if (a is stub and b == "__add__" and isNumberType(c)):
            return 1.0, c
    except TypeError: pass
    try:
        ((a, b, c), d, e) = data # full syntax
        if (a is stub and b == "__mul__" and isNumberType(c) and
            d == "__add__" and isNumberType(e)):
            return c, e
    except TypeError: pass
    raise ValueError, "illegal expression"


# --------------------------------------------------------------------
# Implementation wrapper

import os

class Image:

    format = None
    format_description = None

    def __init__(self):
	self.im = None
	self.mode = ""
	self.size = (0,0)
	self.palette = None
	self.info = {}
	self.category = NORMAL

    def __setattr__(self, id, value):
	if id == "palette":
	    pass # print "set", id, value
	self.__dict__[id] = value

    def _makeself(self, im):
	new = Image()
	new.im = im
	new.mode = im.mode
	new.size = im.size
	new.palette = self.palette
	new.info = self.info
	return new

    def _dump(self, file = None):
        import tempfile
	if not file:
	    file = tempfile.mktemp()
	self.load()
	self.im.save_ppm(file)
	return file

    def tostring(self, encoder_name = "raw", *args):
	"Return image as a binary string"

        # may pass tuple instead of argument list
        if len(args) == 1 and isTupleType(args[0]):
            args = args[0]

        # compatibility mode
        if encoder_name == "raw" and args == ():
            mode = self.mode
            if mode == "RGB":
                mode = "RGBX"
            args = (mode, 0, -1)
	else:
            mode = self.mode

        self.load()

        # unpack data
        e = _getencoder(mode, encoder_name, args)
        e.setimage(self.im)

        data = []
        while 1:
            l, s, d = e.encode(65536)
            data.append(d)
            if s:
                break
        if s < 0:
            raise RuntimeError, "encoder error %d in tostring" % s

        return string.join(data, "")

    def tobitmap(self, name = "image"):
	"Return image as an XBM bitmap"

	self.load()
	if self.mode != "1":
	    raise ValueError, "not a bitmap"
        data = self.tostring("xbm")
	return string.join(["#define %s_width %d\n" % (name, self.size[0]),
		"#define %s_height %d\n"% (name, self.size[1]),
		"static char %s_bits[] = {\n" % name, data, "};"], "")

    def fromstring(self, data, decoder = "raw", *args):
        "Load data to image from binary string"

        # may pass tuple instead of argument list
        if len(args) == 1 and isTupleType(args[0]):
            args = args[0]

        # compatibility mode
        if decoder == "raw" and args == ():
            mode = self.mode
            if mode == "RGB":
                mode = "RGBX"
            args = (mode, 0, -1)

        # unpack data
        d = _getdecoder(decoder, None, args)
        d.setimage(self.im)
        s = d.decode(data)

        if s != (-1, 0):
            raise ValueError, "cannot decode image data"

    def load(self):
	if self.im and self.palette and self.palette.rawmode:
	    self.im.putpalette(self.palette.rawmode, self.palette.data)
            self.palette.mode = "RGB"
            self.palette.rawmode = None
            if self.info.has_key("transparency"):
                self.im.putpalettealpha(self.info["transparency"], 0)
                self.palette.mode = "RGBA"

    #
    # function wrappers

    def convert(self, mode = None, data = None):
        "Convert to other pixel format"

	if not mode:
	    if self.mode == "P":
		mode = self.palette.mode
	    else:
		return self.copy()
	self.load()
	if data:
	    if mode in ["L", "RGB"]:
		im = self.im.convert_matrix(mode, data)
	    elif mode == "P":
		im = self.im.convert(mode) # FIXME
	else:
	    im = self.im.convert(mode)
	return self._makeself(im)

    def copy(self):
        "Copy raster data"

	self.load()
	im = self.im.copy()
	return self._makeself(im)

    def crop(self, box = None):
        "Crop region from image"

	self.load()
	if box == None:
	    return self.copy()

        # delayed operation
        return _ImageCrop(self, box)

    def draft(self, mode, size):
        "Configure image decoder"

	pass

    def filter(self, kernel):
        "Apply environment filter to image"

	self.load()
	id = kernel.id
	if len(self.mode) == 1:
	    return self._makeself(self.im.filter(id))
	# fix to handle multiband images since _imaging doesn't
	ims = []
	for c in range(self.im.bands):
	    ims.append(self._makeself(self.im.getband(c).filter(id)))
	return merge(self.mode, ims)

    def getbbox(self):
	"Get bounding box of actual data (non-zero pixels) in image"

    	self.load()
	return self.im.getbbox()

    def getdata(self, band = None):
	"Get image data as sequence object."

    	self.load()
	if band != None:
	    return self.im.getband(band)
	return self.im # could be misused

    def getpixel(self, (x, y)):
	"Get pixel value"

    	self.load()
        if 0 <= x < self.size[0] and 0 <= y <= self.size[1]:
            return self.im[int(x + y * self.size[0])]
        raise IndexError

    def getprojection(self):
	"Get projection to x and y axes"

    	self.load()
	x, y = self.im.getprojection()
	return map(ord, x), map(ord, y)

    def histogram(self, mask = None):
        "Take histogram of image"

	self.load()
	if mask:
	    mask.load()
	    return self.im.histogram(mask.im)
	return self.im.histogram()

    def offset(self, xoffset, yoffset = None):
        "Offset image in horizontal and/or vertical direction"

	if yoffset == None:
	    yoffset = xoffset
	self.load()
	return self._makeself(self.im.offset(xoffset, yoffset))

    def paste(self, im, box = None, mask = None):
        "Paste other image into region"

	if box == None:
	    # all of image
	    box = (0, 0) + self.size

        if not isImageType(im):
            if len(box) == 2:
                box = box + self.size
            im = new(self.mode, (box[2]-box[0], box[3]-box[1]), im)

	elif len(box) == 2:
	    # lower left corner given
	    box = box + (box[0]+im.size[0], box[1]+im.size[1])

        im.load()
	self.load()

	# fix to handle conversion when pasting
	if self.mode != im.mode:
	    im = im.convert(self.mode)

	if mask:
	    mask.load()
	    self.im.paste(im.im, box, mask.im)
	else:
	    self.im.paste(im.im, box)

    def point(self, lut, mode = None):
        "Map image through lookup table"

        if self.mode == "F":
            # floating point; lut must be a valid expression
            scale, offset = _getscaleoffset(lut)
            self.load()
            im = self.im.point_transform(scale, offset);
        else:
            # integer image; use lut and mode
            if not isSequenceType(lut):
                # if it isn't a list, it should be a function
                lut = map(lut, range(256)) * len(self.mode)
            self.load()
            im = self.im.point(lut, mode)

	return self._makeself(im)

    def putalpha(self, im):
        "Set alpha layer"

	if self.mode != "RGBA" or im.mode not in ["1", "L"]:
	    raise ValueError, "illegal image mode"

	im.load()
	self.load()

	if im.mode == "1":
	    im = im.convert("L")

	self.im.putband(im.im, 3)

    def putdata(self, data, scale = 1.0, offset = 0.0):
	"Put data from a sequence object into an image."

	self.load() # hmm...
	self.im.putdata(data, scale, offset)

    def putpalette(self, data, rawmode = "RGB"):
	"Put palette data into an image."

        if self.mode not in ("L", "P"):
            raise ValueError, "illegal image mode"
        if type(data) != StringType:
            data = string.join(map(chr, data), "")
        self.mode = "P"
        self.palette = ImagePaletteH.raw(rawmode, data)
        self.palette.mode = "RGB"

    def resize(self, size, resample = NEAREST):
        "Resize image"

	if resample not in [NEAREST, ANTIALIAS]:
	    raise ValueError, "unknown resampling method"

	self.load()
	if resample == NEAREST:
	    im = self.im.resize(size)
	else:
	    im = self.im.resize_antialias(size)
	return self._makeself(im)

    def rotate(self, angle, resample = NEAREST):
        "Rotate image.  Angle given as degrees counter-clockwise."

	if resample != NEAREST:
	    raise ValueError, "unknown resampling method"

	self.load()
	im = self.im.rotate(angle)
	return self._makeself(im)

    def save(self, fp, format = None, **params):
        "Save image to file or stream"

	if isStringType(fp):
	    import __builtin__
	    filename = fp
            fp = __builtin__.open(fp, "wb")
            close = 1
	else:
	    filename = ""
            close = 0

	self.encoderinfo = params
	self.encoderconfig = ()

	self.load()

	preinit()

        ext = string.lower(os.path.splitext(filename)[1])

        try:

            if not format:
                format = EXTENSION[ext]

	    SAVE[string.upper(format)](self, fp, filename)

	except KeyError, v:

            init()

            if not format:
                format = EXTENSION[ext]

            SAVE[string.upper(format)](self, fp, filename)

        if close:
            fp.close()

    def seek(self, frame):
        "Seek to given frame in sequence file"

        if frame != 0:
            raise EOFError

    def show(self, title = None):
        "Display image (for debug purposes only)"

	try:
	    import ImageTk
	    ImageTk._show(self, title)
	    # note: caller must enter mainloop
	except:
	    _showxv(self, title)

    def split(self):
        "Split image into bands"

        ims = []
	self.load()
	for i in range(self.im.bands):
	    ims.append(self._makeself(self.im.getband(i)))
	return tuple(ims)

    def tell(self):
        "Return current frame number"

        return 0

    def thumbnail(self, size):
	"Create thumbnail representation (modifies image in place)"

	# preserve aspect ratio
	x, y = self.size
	if x > size[0]: y = y * size[0] / x; x = size[0]
	if y > size[1]: x = x * size[1] / y; y = size[1]
	size = x, y

	if size == self.size:
	    return

	self.draft(None, size)

	im = self.resize(size)

	self.im = im.im
	self.mode = im.mode
	self.size = size

    def transform(self, size, method, data, resample = NEAREST):
        "Transform image"

	if method == EXTENT:
	    x0, y0, x1, y1 = data
	    xs = float(x1 - x0) / size[0]
	    ys = float(y1 - y0) / size[1]
	    data = (xs, 0, x0 + xs/2, 0, ys, y0 + ys/2)
	elif method != AFFINE:
	    raise ValueError, "unknown transformation method"
	if resample != NEAREST:
	    raise ValueError, "unknown resampling method"

	self.load()
	im = self.im.transform(size, data)
	return self._makeself(im)

    def transpose(self, method):
        "Transpose image (flip or rotate in 90 degree steps)"

	self.load()
	im = self.im.transpose(method)
	return self._makeself(im)


# --------------------------------------------------------------------
# Delayed operations

class _ImageCrop(Image):

    def __init__(self, im, box):

	Image.__init__(self)

        self.mode = im.mode
        self.size = box[2]-box[0], box[3]-box[1]

        self.__crop = box

        self.im = im.im

    def load(self):
        
        # delayed evaluation of crop operation

        if self.__crop is None:
            return

	# FIXME: the C implementation of crop is broken, so we
        # implement it by pasting into empty image instead.

	# im = self.im.__crop(self.__crop)

	im = core.new(self.mode, self.size)

	im.paste(self.im, (-self.__crop[0], -self.__crop[1],
		 self.im.size[0]-self.__crop[0],
		 self.im.size[1]-self.__crop[1]))

	if self.mode == "P":
	    im.putpalette("RGB", self.im.getpalette("RGB", "RGB"))

        self.im = im

        self.__crop = None


# --------------------------------------------------------------------
# Factories

#
# Debugging

def _wedge():
    "Create greyscale wedge (for debugging only)"

    return Image()._makeself(core.wedge("L"))

#
# Create/open images.

def new(mode, size, color = 0):
    "Create a new image"

    if color == None:
	# don't initialize
	return Image()._makeself(core.new(mode, size))

    if isTupleType(color):
	# convert colour tuple to integer value (0xAABBGGRR)
	if len(color) == 3:
	    color = color[0] + (color[1]<<8) + (color[2]<<16)
	elif len(color) == 4:
	    color = color[0] + (color[1]<<8) + (color[2]<<16) + (color[3]<<24)
	else:
	    color = color[0]

    return Image()._makeself(core.fill(mode, size, color))

def fromstring(mode, size, data, decoder = "raw", *args):
    "Load image from string"

    # may pass tuple instead of argument list
    if len(args) == 1 and isTupleType(args[0]):
        args = args[0]

    if decoder == "raw" and args == ():
        args = mode

    im = new(mode, size)
    im.fromstring(data, decoder, args)
    return im

def open(fp, mode = "r"):
    "Open an image file, without loading the raster data"

    if mode != "r":
        raise ValueError, "bad mode"

    if isStringType(fp):
        import __builtin__
	filename = fp
        fp = __builtin__.open(fp, "rb")
    else:
        filename = ""

    prefix = fp.read(16)

    preinit()

    for i in ID:
	try:
	    factory, accept = OPEN[i]
	    if not accept or accept(prefix):
		fp.seek(0)
		return factory(fp, filename)
	except SyntaxError:
	    pass

    init()

    for i in ID:
	try:
	    factory, accept = OPEN[i]
	    if not accept or accept(prefix):
		fp.seek(0)
		return factory(fp, filename)
	except SyntaxError:
	    pass

    raise IOError, "cannot identify image file"

#
# Image processing.

def blend(im1, im2, alpha):
    "Interpolate between images."

    if alpha == 0.0:
	return im1
    elif alpha == 1.0:
	return im2
    return Image()._makeself(core.blend(im1.im, im2.im, alpha))

def composite(image1, image2, mask):
    "Create composite image by blending images using a transparency mask"

    image = image2.copy()
    image.paste(image1, None, mask)
    return image

def eval(image, *args):
    "Evaluate image expression"

    return image.point(args[0])

def merge(mode, bands):
    "Merge a set of single band images into a new multiband image."

    if len(mode) != len(bands) or "*" in mode:
        raise ValueError, "wrong number of bands"
    for im in bands[1:]:
        if len(im.mode) != 1 or im.size != bands[0].size:
            raise ValueError, "wrong number of bands"
    im = core.new(mode, bands[0].size)
    for i in range(len(mode)):
	bands[i].load()
	im.putband(bands[i].im, i)
    return Image()._makeself(im)


# --------------------------------------------------------------------
# Plugin registry

def register_open(id, factory, accept = None):
    id = string.upper(id)
    ID.append(id)
    OPEN[id] = factory, accept

def register_mime(id, mimetype):
    MIME[string.upper(id)] = mimetype

def register_save(id, driver):
    SAVE[string.upper(id)] = driver

def register_extension(id, extension):
    EXTENSION[string.lower(extension)] = string.upper(id)


# --------------------------------------------------------------------
# Unix display support

def _showxv(self, title = None):

    if self.mode == "P":
	file = self.convert("RGB")._dump()
    else:
	file = self._dump()

    if title:
	opt = "-name \"%s\"" % title
    else:
	opt = ""

    os.system("(xv %s %s; rm -f %s)&" % (opt, file, file))

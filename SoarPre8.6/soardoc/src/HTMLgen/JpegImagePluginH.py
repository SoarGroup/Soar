#
# The Python Imaging Library.
# $Id$
#
# JPEG (JFIF) file handling
#
# See "Digital Compression and Coding of Continous-Tone Still Images,
# Part 1, Requirements and Guidelines" (CCITT T.81 / ISO 10918-1)
#
# History:
#	95-09-09 fl	Created
#	95-09-13 fl	Added full parser
#	96-03-25 fl	Added hack to use the IJG command line utilities
#	96-05-05 fl	Workaround Photoshop 2.5 CMYK polarity bug
# 0.1	96-05-28 fl	Added draft support, JFIF version
# 0.2	96-12-30 fl	Added encoder options, added progression property
# 0.3	97-08-27 fl	Save mode 1 images as BW
#
# Copyright (c) Secret Labs AB 1997.
# Copyright (c) Fredrik Lundh 1995-96.
#
# See the README file for information on usage and redistribution.
#

__version__ = "0.3"

import array, string
import ImageH, ImageFileH


def i16(c):
    return ord(c[1]) + (ord(c[0])<<8)

def i32(c):
    return ord(c[3]) + (ord(c[2])<<8) + (ord(c[1])<<16) + (ord(c[0])<<24)

#
# Parser

def Skip(self, marker):
    self.fp.read(i16(self.fp.read(2))-2)

def APP(self, marker):
    #
    # Application marker.  Store these in the APP dictionary.
    # Also look for well-known application markers.

    s = self.fp.read(i16(self.fp.read(2))-2)
    self.app["APP%d" % (marker&15)] = s

    if marker == 0xFFE0 and s[:4] == "JFIF":
        self.info["jfif"] = i16(s[5:])
    if marker == 0xFFEE and s[:5] == "Adobe":
        self.info["adobe"] = i16(s[5:])
	self.info["adobe_transform"] = ord(s[11])

def SOF(self, marker):
    #
    # Start of frame marker.  Defines the size and mode of the
    # image.  JPEG is colour blind, so we use some simple
    # heuristics to map the number of layers to an appropriate
    # mode.  Note that this could be made a bit brighter, by
    # looking for JFIF and Adobe APP markers.

    s = self.fp.read(i16(self.fp.read(2))-2)
    self.size = i16(s[3:]), i16(s[1:])

    self.bits = ord(s[0])
    if self.bits != 8:
	raise SyntaxError, "cannot handle %d-bit layers" % self.bits

    self.layers = ord(s[5])
    if self.layers == 1:
	self.mode = "L"
    elif self.layers == 3:
	self.mode = "RGB"
    elif self.layers == 4:
	self.mode = "CMYK"
    else:
	raise SyntaxError, "cannot handle %d-layer images" % self.layers

    if marker in [0xFFC2, 0xFFC6, 0xFFCA, 0xFFCE]:
	self.info["progression"] = 1

    for i in range(6, len(s), 3):
	t = s[i:i+3]
	# 4-tuples: id, vsamp, hsamp, qtable
	self.layer.append(t[0], ord(t[1])/16, ord(t[1])&15, ord(t[2]))

def DQT(self, marker):
    #
    # Define quantization table.  Support baseline 8-bit tables
    # only.  Note that there might be more than one table in
    # each marker.

    # FIXME: The quantization tables can be used to estimate the
    # compression quality.

    s = self.fp.read(i16(self.fp.read(2))-2)
    while len(s):
	if len(s) < 65:
	    raise SyntaxError, "bad quantization table marker"
	v = ord(s[0])
	if v/16 == 0:
	    self.quantization[v&15] = array.array("b", s[1:65])
	    s = s[65:]
	else:
	    pass
	    # raise SyntaxError, "bad quantization table element size"


#
# JPEG marker table

MARKER = {
    0xFFC0: ("SOF0", "Baseline DCT", SOF),
    0xFFC1: ("SOF1", "Extended Sequential DCT", SOF),
    0xFFC2: ("SOF2", "Progressive DCT", SOF),
    0xFFC3: ("SOF3", "Spatial lossless", SOF),
    0xFFC4: ("DHT", "Define Huffman table", Skip),
    0xFFC5: ("SOF5", "Differential sequential DCT", SOF),
    0xFFC6: ("SOF6", "Differential progressive DCT", SOF),
    0xFFC7: ("SOF7", "Differential spatial", SOF),
    0xFFC8: ("JPG", "Extension", None),
    0xFFC9: ("SOF9", "Extended sequential DCT (AC)", SOF),
    0xFFCA: ("SOF10", "Progressive DCT (AC)", SOF),
    0xFFCB: ("SOF11", "Spatial lossless DCT (AC)", SOF),
    0xFFCC: ("DAC", "Define arithmetic coding conditioning", Skip),
    0xFFCD: ("SOF13", "Differential sequential DCT (AC)", SOF),
    0xFFCE: ("SOF14", "Differential progressive DCT (AC)", SOF),
    0xFFCF: ("SOF15", "Differential spatial (AC)", SOF),
    0xFFD0: ("RST0", "Restart 0", None),
    0xFFD1: ("RST1", "Restart 1", None),
    0xFFD2: ("RST2", "Restart 2", None),
    0xFFD3: ("RST3", "Restart 3", None),
    0xFFD4: ("RST4", "Restart 4", None),
    0xFFD5: ("RST5", "Restart 5", None),
    0xFFD6: ("RST6", "Restart 6", None),
    0xFFD7: ("RST7", "Restart 7", None),
    0xFFD8: ("SOI", "Start of image", None),
    0xFFD9: ("EOI", "End of image", None),
    0xFFDA: ("SOS", "Start of scan", Skip),
    0xFFDB: ("DQT", "Define quantization table", DQT),
    0xFFDC: ("DNL", "Define number of lines", Skip),
    0xFFDD: ("DRI", "Define restart interval", Skip),
    0xFFDE: ("DHP", "Define hierarchical progression", SOF),
    0xFFDF: ("EXP", "Expand reference component", Skip),
    0xFFE0: ("APP0", "Application segment 0", APP),
    0xFFE1: ("APP1", "Application segment 1", APP),
    0xFFE2: ("APP2", "Application segment 2", APP),
    0xFFE3: ("APP3", "Application segment 3", APP),
    0xFFE4: ("APP4", "Application segment 4", APP),
    0xFFE5: ("APP5", "Application segment 5", APP),
    0xFFE6: ("APP6", "Application segment 6", APP),
    0xFFE7: ("APP7", "Application segment 7", APP),
    0xFFE8: ("APP8", "Application segment 8", APP),
    0xFFE9: ("APP9", "Application segment 9", APP),
    0xFFEA: ("APP10", "Application segment 10", APP),
    0xFFEB: ("APP11", "Application segment 11", APP),
    0xFFEC: ("APP12", "Application segment 12", APP),
    0xFFED: ("APP13", "Application segment 13", APP),
    0xFFEE: ("APP14", "Application segment 14", APP),
    0xFFEF: ("APP15", "Application segment 15", APP),
    0xFFF0: ("JPG0", "Extension 0", None),
    0xFFF1: ("JPG1", "Extension 1", None),
    0xFFF2: ("JPG2", "Extension 2", None),
    0xFFF3: ("JPG3", "Extension 3", None),
    0xFFF4: ("JPG4", "Extension 4", None),
    0xFFF5: ("JPG5", "Extension 5", None),
    0xFFF6: ("JPG6", "Extension 6", None),
    0xFFF7: ("JPG7", "Extension 7", None),
    0xFFF8: ("JPG8", "Extension 8", None),
    0xFFF9: ("JPG9", "Extension 9", None),
    0xFFFA: ("JPG10", "Extension 10", None),
    0xFFFB: ("JPG11", "Extension 11", None),
    0xFFFC: ("JPG12", "Extension 12", None),
    0xFFFD: ("JPG13", "Extension 13", None),
    0xFFFE: ("COM", "Comment", Skip)
}


def _accept(prefix):
    return prefix[0] == "\377"

class JpegImageFile(ImageFileH.ImageFile):

    format = "JPEG"
    format_description = "JPEG (ISO 10918)"

    def _open(self):

	s = self.fp.read(1)

	if ord(s[0]) != 255:
	    raise SyntaxError, "not an JPEG file"

	# Create attributes
	self.bits = self.layers = 0

	# JPEG specifics (internal)
	self.layer = []
	self.huffman_dc = {}
	self.huffman_ac = {}
	self.quantization = {}
	self.app = {}

	while 1:

	    s = s + self.fp.read(1)

	    i = i16(s)

	    if MARKER.has_key(i):
		name, description, handler = MARKER[i]
		# print hex(i), name, description
		if handler != None:
		    handler(self, i)
		if i == 0xFFDA: # start of scan
		    rawmode = self.mode
		    if self.mode == "CMYK" and self.info.has_key("adobe"):
			rawmode = "CMYK;I" # Photoshop 2.5 is broken!
		    self.tile = [("jpeg", (0,0) + self.size, 0, (rawmode, ""))]
		    # self.offset = self.fp.tell()
		    break
		s = self.fp.read(1)
	    else:
	        raise SyntaxError, "no marker found"

    def draft(self, mode, size):

	if len(self.tile) != 1:
	    return

	d, e, o, a = self.tile[0]
	scale = 0

	if a == "RGB" and mode in ["L", "YCC"]:
	    self.mode = a = mode

	if size:
	    scale = max(self.size[0] / size[0], self.size[1] / size[1])
	    for s in [8, 4, 2, 1]:
		if scale >= s:
		    break
	    e = e[0], e[1], (e[2]-e[0]+s-1)/s+e[0], (e[3]-e[1]+s-1)/s+e[1]
	    self.size = ((self.size[0]+s-1)/s, (self.size[1]+s-1)/s)
	    scale = s

	self.tile = [(d, e, o, a)]
	self.decoderconfig = (scale, 1)

	return self

    def load_hack(self):

	# ALTERNATIVE: handle JPEGs via the IJG command line utilities

	import tempfile, os
	file = tempfile.mktemp()
	os.system("djpeg %s >%s" % (self.filename, file))

	try:
	    self.im = ImageH.core.open_ppm(file)
	finally:
	    try: os.unlink(file)
	    except: pass

	self.mode = self.im.mode
	self.size = self.im.size

	self.tile = []


def _fetch(dict, key, default = 0):
    try:
	return dict[key]
    except KeyError:
	return default

RAWMODE = {
    "1": "L",
    "L": "L",
    "RGB": "RGB",
    "RGBA": "RGB",
    "CMYK": "CMYK",
}

def _save(im, fp, filename):
    try:
        rawmode = RAWMODE[im.mode]
    except KeyError:
	raise IOError, "cannot write mode %s as JPEG" % im.mode
    # get keyword arguments
    im.encoderconfig = (_fetch(im.encoderinfo, "quality", 0),
			im.encoderinfo.has_key("progressive"),
			_fetch(im.encoderinfo, "smooth", 0),
			im.encoderinfo.has_key("optimize"),
			_fetch(im.encoderinfo, "streamtype", 0))
    ImageFileH._save(im, fp, [("jpeg", (0,0)+im.size, 0, rawmode)])

def _save_hack(im, fp, filename):
    # ALTERNATIVE: handle JPEGs via the IJG command line utilities.
    import os
    file = im._dump()
    os.system("cjpeg %s >%s" % (file, filename))
    try: os.unlink(file)
    except: pass

# -------------------------------------------------------------------q-
# Registry stuff

ImageH.register_open("JPEG", JpegImageFile, _accept)
ImageH.register_save("JPEG", _save)

ImageH.register_extension("JPEG", ".jfif")
ImageH.register_extension("JPEG", ".jpe")
ImageH.register_extension("JPEG", ".jpg")
ImageH.register_extension("JPEG", ".jpeg")

ImageH.register_mime("JPEG", "image/jpeg")

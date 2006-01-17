#
# The Python Imaging Library.
# $Id$
#
# image palette object
#
# History:
#	96-03-11 fl	Rewritten.
#	97-01-03 fl	Up and running.
#	97-08-23 fl	Added load hack
#
# Copyright (c) Secret Labs AB 1997.
# Copyright (c) Fredrik Lundh 1996-97.
#
# See the README file for information on usage and redistribution.
#
try:
    import array
    jpython = 0
except ImportError:
    import jarray
    jpython = 1
import ImageH

class ImagePalette:
    "Colour palette for palette mapped images"

    def __init__(self, mode = "RGB", palette = None):
        self.mode = mode
        self.palette = palette or range(256)*len(self.mode)
	if len(self.mode)*256 != len(self.palette):
	    raise ValueError, "wrong palette size"

    def tostring(self):
        if jpython == 0:
            return array.array("b", self.palette).tostring()
        else:
            return jarray.array(self.palette, "b").tostring()


    def save(self, fp):
        if type(fp) == type(""):
	    fp = open(fp, "w")
	fp.write("# Palette\n")
	fp.write("# Mode: %s\n" % self.mode)
	for i in range(256):
	    fp.write("%d" % i)
	    for j in range(i, len(self.palette), 256):
	        fp.write(" %d" % self.palette[j])
	    fp.write("\n")
        fp.close()

# --------------------------------------------------------------------
# Internal

class raw:
    def __init__(self, rawmode, data):
	self.rawmode = rawmode
	self.data = data

# --------------------------------------------------------------------
# Factories

def new(mode, data):
    ImageH.core.new_palette(mode, data)

def negative(mode = "RGB"):
    palette = range(256)
    palette.reverse()
    return ImagePalette(mode, palette * len(mode))

def random(mode = "RGB"):
    from whrandom import randint
    palette = map(lambda a: randint(0, 255), [0]*256*len(mode))
    return ImagePalette(mode, palette)

def wedge(mode = "RGB"):
    return ImagePalette(mode, range(256) * len(mode))

def load(filename):

    # FIXME: supports GIMP gradients only

    fp = open(filename, "rb")

    lut = None

    if not lut:
        try:
            import GimpPaletteFile
            fp.seek(0)
            p = GimpPaletteFile.GimpPaletteFile(fp)
            lut = p.getpalette()
        except (SyntaxError, ValueError):
            pass

    if not lut:
        try:
            import GimpGradientFile
            fp.seek(0)
            p = GimpGradientFile.GimpGradientFile(fp)
            lut = p.getpalette()
        except (SyntaxError, ValueError):
            pass

    if not lut:
        try:
            import PaletteFile
            fp.seek(0)
            p = PaletteFile.PaletteFile(fp)
            lut = p.getpalette()
        except (SyntaxError, ValueError):
            pass

    if not lut:
        raise IOError, "cannot load palette"

    return lut # data, rawmode


# add some psuedocolour palettes as well

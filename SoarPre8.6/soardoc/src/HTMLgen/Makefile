#'$Id$'
# Makefile for HTMLgen (requires gnu make)
#

VERSION = 2_2

PACKAGE = HTMLgen

DOCS=        README Makefile ChangeLog HTML.rc HTMLgen.rc appt.txt 

MODULES=     HTMLgen.py HTMLcolors.py HTMLutil.py HTMLcalendar.py \
		barchart.py colorcube.py imgsize.py NavLinks.py Formtools.py

MODULESC=    HTMLgen.pyc HTMLcolors.pyc HTMLutil.pyc HTMLcalendar.pyc \
		barchart.pyc colorcube.pyc imgsize.pyc NavLinks.pyc Formtools.pyc

PIL=	     ImageH.py ImageFileH.py ImagePaletteH.py \
		GifImagePluginH.py JpegImagePluginH.py PngImagePluginH.py

PILC=	     ImageH.pyc ImageFileH.pyc ImagePaletteH.pyc \
		GifImagePluginH.pyc JpegImagePluginH.pyc PngImagePluginH.pyc

EXTRAS=	     StickyForm.py cgiapp.py installp.py imgfix.py 

SUBDIRS=     image html data

TEST=        HTMLtest.py

DIST_DOCS =    $(addprefix HTMLgen/, $(DOCS))
DIST_MODULES = $(addprefix HTMLgen/, $(MODULES) $(PIL) $(EXTRAS))
DIST_SUBDIRS = $(addprefix HTMLgen/, $(SUBDIRS))
DIST_TEST =    $(addprefix HTMLgen/, $(TEST))
DIST_FILES=    $(DIST_DOCS) $(DIST_MODULES) $(DIST_TEST) $(DIST_SUBDIRS)

all:
	@echo Type \"make install\" to install HTMLgen.
	@echo The modules will be put in site-packages.
	@echo
	@echo Other targets include: test gendoc compileall dist bigdist 

dist:	test gendoc bigdist

bigdist:
	(cd ..; tar chf - $(DIST_FILES)) | gzip --best > $(PACKAGE).tgz
	uuencode $(PACKAGE).tgz $(PACKAGE).tgz > $(PACKAGE).tgz.uu

smalldist:
	(cd ..; tar chf -  $(DIST_MODULES) ) | gzip --best > small$(PACKAGE).tgz
	uuencode small$(PACKAGE).tgz small$(PACKAGE).tgz > small$(PACKAGE).tgz.uu

test:
	python $(TEST)

gendoc:
	gendoc -i -d html -f HTMLg -h HTMLgen $(MODULES) $(EXTRAS)
	python HTMLutil.py -s

compileall:
	python -c "import compileall; compileall.compile_dir('.',0)"

install: compileall
	python installp.py -f $(MODULES) $(MODULESC) $(PIL) $(PILC)
	@echo Installation of $(PACKAGE) done.

checkin:
	ci -u $(MODULES) $(PIL) $(EXTRAS) $(TEST) Makefile

release:
	rcs -N$(VERSION): $(MODULES) $(PIL) $(EXTRAS) $(TEST) Makefile


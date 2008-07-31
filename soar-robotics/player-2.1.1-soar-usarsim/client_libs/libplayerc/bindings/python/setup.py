from distutils.core import setup, Extension

from srcbuilddirs import *
if srcdir == '':
  srcdir = '.'
if top_srcdir == '':
  top_srcdir = '.'
if builddir == '':
  builddir = '.'
if top_builddir == '':
  top_builddir = '.'

#TODO: handle linking to libjpeg conditionally, depending on whether or not
#      it was found during configuration.  The easiest way to do this may
#      be to use libplayerpacket's pkgconfig info.  Until then, I'm
#      disabling the use of camera decompression in the libplayerc python
#      bindings.
module = Extension('_playerc',
                   sources = ['playerc.i'],
                   include_dirs = [srcdir + '/../..', top_srcdir, top_builddir, top_builddir + '/libplayercore', top_builddir + '/client_libs'],
                   library_dirs = [builddir + '/../../.libs', 
                                   top_builddir + '/libplayerxdr/.libs',
                                   top_builddir + '/libplayercore/.libs',
                                   top_builddir + '/libplayerjpeg/.libs'],
                   libraries = ['playerxdr', 'playerc', 'playerjpeg', 'jpeg', 'playererror'])


setup(name = 'playerc',
      version = 'X.X',
      description = 'Bindings for playerc',
      py_modules = ['playerc'],
      ext_modules = [module])

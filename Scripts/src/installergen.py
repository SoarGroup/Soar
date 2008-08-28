#!/usr/bin/python
#
# Author: Jonathan Voigt, University of Michigan
# Date: September 2008

import logging
import getopt
import sys
import os
import os.path
import fnmatch
import stat
import shutil
import time
import distutils.file_util
import distutils.dir_util
import re
import subprocess
import urllib
import md5
import zipfile
import zlib
import sys

# swt.jar 3.3 digests
WIN_DIGEST = '33ac049c1f70126f5fe190da2bd9ff77'

####
# Configuration
generatorConfig = {}

# Specific directories to remove from source tree
if os.name != 'posix':
    generatorConfig[ 'remove' ] = [ ]
    generatorConfig[ 'removedirs' ] = [ ]
    generatorConfig[ 'copyglobs' ] = [ '*.pdf', '*.dll', '*.lib', '*.exe', '*.jar', 
                                      'Python_sml_ClientInterface.py', '*.pyd', 'Tcl_sml_ClientInterface', 
                                      'Tcl_sml_ClientInterface_wrap.cxx', 'Java_sml_ClientInterface_wrap.cxx', 
                                      'CSharp_sml_ClientInterface_wrap.cxx', 'Python_sml_ClientInterface_wrap.cxx', ]

    generatorConfig[ 'top-level-files' ] =[ 'Visual Soar.bat', 'Eaters.bat', 'Missionaries and Cannibals.bat', 'SoarJavaDebugger.bat',
                                       'TankSoar.bat', 'Towers of Hanoi.bat', 'announce.txt', 'readme.txt', 'license.txt', ] 

    # File globs to completely remove from the tree
    generatorConfig['remove'] = [ '.project', '.classpath', '.settings', '.pydevproject', '.svn', '*.so', '*.so.1', '*.so.2', '*.a', 
                   '*.jnilib', 'java_swt', '*.sh', '*.doc', '*.docx', '*.ppt', '*.pptx', '*.pl', 'crossmingw.py', 'SoarSCons.py',
                   'ManualSource', '*.tex', 'Scripts', 'installergen.py', 'obj', 'Doxyfile', 'SConstruct', 'SConscript', 'Resources', ]
elif sys.platform == 'darwin':
    generatorConfig[ 'remove' ] = [ ]
    generatorConfig[ 'removedirs' ] = [ ]
    generatorConfig[ 'copyglobs' ] = [ '*.pdf', '*.dylib', '*.jar', '*.a', '*.jnilib', 
                                       'TestSoarPerformance',  'TestConnectionSML', 'FilterC', 'SoarTextIO', 'TestMultiAgent', 'TOHSML', 'Tests', 'QuickLink', 'TestCLI', 'TestSMLEvents', 'TestClientSML', 'TestSMLPerformance', 
                                      'Python_sml_ClientInterface.py', '*.pyd', 'Tcl_sml_ClientInterface', 
                                      'Tcl_sml_ClientInterface_wrap.cxx', 'Java_sml_ClientInterface_wrap.cxx', 
                                      'CSharp_sml_ClientInterface_wrap.cxx', 'Python_sml_ClientInterface_wrap.cxx', ]

    generatorConfig[ 'top-level-files' ] =[ 'run-CommandLineInterface.sh', 'run-Debugger.sh', 'run-Eaters.sh', 'run-Soar2D.sh', 'run-TankSoar.sh', 'run-VisualSoar.sh', 
                                           'announce.txt', 'readme.txt', 'license.txt', ] 

    # File globs to completely remove from the tree
    generatorConfig['remove'] = [ '.project', '.classpath', '.settings', '.pydevproject', '.svn', '*.so', '*.so.1', '*.so.2', 
                   'java_swt', '*.plist', '*.doc', '*.docx', '*.ppt', '*.pptx', '*.pl', 'crossmingw.py',
                   'ManualSource', '*.tex', 'Scripts', 'installergen.py', 'obj', 'Doxyfile', 'Resources', ]
else:
    generatorConfig[ 'remove' ] = [ ]
    generatorConfig[ 'removedirs' ] = [ ]
    generatorConfig[ 'copyglobs' ] = [ '*.pdf', '*.so', '*.jar', '*.a',
                                       'TestSoarPerformance',  'TestConnectionSML', 'FilterC', 'SoarTextIO', 'TestMultiAgent', 'TOHSML', 'Tests', 'QuickLink', 'TestCLI', 'TestSMLEvents', 'TestClientSML', 'TestSMLPerformance', 
                                      'Python_sml_ClientInterface.py', '*.pyd', 'Tcl_sml_ClientInterface', 
                                      'Tcl_sml_ClientInterface_wrap.cxx', 'Java_sml_ClientInterface_wrap.cxx', 
                                      'CSharp_sml_ClientInterface_wrap.cxx', 'Python_sml_ClientInterface_wrap.cxx', ]

    generatorConfig[ 'top-level-files' ] =[ 'run-CommandLineInterface.sh', 'run-Debugger.sh', 'run-Eaters.sh', 'run-Soar2D.sh', 'run-TankSoar.sh', 'run-VisualSoar.sh', 
                                           'announce.txt', 'readme.txt', 'license.txt', ] 

    # File globs to completely remove from the tree
    generatorConfig['remove'] = [ '.project', '.classpath', '.settings', '.pydevproject', '.svn', '*.dylib', '*.so.1', '*.so.2', 
                   'java_swt', '*.plist', '*.doc', '*.docx', '*.ppt', '*.pptx', '*.pl', 'crossmingw.py',
                   'ManualSource', '*.tex', 'Scripts', 'installergen.py', 'obj', 'Doxyfile', 'Resources', ]

generatorConfig[ 'baseurl' ] = 'https://winter.eecs.umich.edu/svn/soar/tags/'

####
class Generator:
    "This thing builds Soar packages."
    config = None
    
    def __init__( self, generatorConfig ):
        self.config = generatorConfig
        
        self.config[ 'target-basename' ] = "Soar-Suite-%s" % ( self.config[ 'version' ], )

        self.config[ 'target-parent' ] = os.path.join( "..", self.config[ 'target-basename' ] )
        self.config[ 'target-path' ] = os.path.join( self.config[ 'target-parent' ], 'SoarSuite', )
            
        self.config[ 'target-url' ] = "%s%s" % ( self.config[ 'baseurl' ], self.config[ 'target-basename' ], )

        if os.name != 'posix':
            self.config[ 'target-file' ] = "%s-windows.zip" % ( self.config[ 'target-basename' ], )
        else:
            if sys.platform == 'darwin':
                self.config[ 'target-file' ] = "%s-osx.tar.gz" % ( self.config[ 'target-basename' ], )
            else:
                self.config[ 'target-file' ] = "%s-linux.tar.gz" % ( self.config[ 'target-basename' ], )
            
        for x in self.config:
            logging.debug( 'config: %s: %s' % ( x, repr( self.config[x] ), ) )
            
    def checkJarmd5( self ):
        # open the swt.jar file
        try:
            f = file( os.path.join( "SoarLibrary", "bin", "swt.jar" ), 'rb')
        except:
            return False
        
        # compute digest
        m = md5.new()
        while True:
            d = f.read(8096)
            if not d:
                break
            m.update(d)
        
        return WIN_DIGEST == m.hexdigest()
                       
    def checkForSWTJar( self ):
        jarPath = os.path.join('SoarLibrary', 'bin', 'swt.jar')
        if os.path.exists( jarPath ):
            if self.checkJarmd5():
                return True
            else:
                logging.info( "md5 of swt.jar failed, removing old jar." )
                os.remove( jarPath )
            
        try:
            urllib.urlretrieve( 'http://ai.eecs.umich.edu/~soar/sitemaker/misc/jars/windows/swt.jar', jarPath )
        except IOError:
            logging.critical( "Error downloading swt.jar to SoarLibrary/bin: IOError" )
            return False
        except ContentTooShortError:
            logging.critical( "Error downloading swt.jar to SoarLibrary/bin: IOError" )
            return False
            
        if not self.checkJarmd5():
            logging.critical( "Error downloading swt.jar to SoarLibrary/bin, md5 failed again." )
            return False
        
        logging.info( "Successfully downloaded swt.jar to SoarLibrary/bin." )
        return True

    def build( self ):
        if os.name != 'posix':
            if not self.checkForSWTJar():
                sys.exit(1)
            
        logging.info( 'Building everything' )
        if os.name != 'posix':
            retcode = subprocess.call( ["rebuild-all.bat", ] )
        else:
            retcode = subprocess.call( ["scons", "-c", "Core/ClientSMLSWIG" ] )
            
            if retcode == 0:
                retcode = subprocess.call( ["scons", "debug=no", "Core/ClientSMLSWIG/Java" ] )
                
                if retcode == 0:
                    retcode = subprocess.call( ["scons", "debug=no", "Core/ClientSMLSWIG/Python" ] )
                    
                    if retcode == 0:
                        retcode = subprocess.call( ["scons", "debug=no" ] )
        
        if retcode != 0:
            logging.critical( "build failed" )
            sys.exit(1)
    
    def source( self ):
            
        # remove old stuff
        if os.path.exists( self.config[ 'target-file' ] ):
            logging.info('Removing old package')
            os.remove( self.config[ 'target-file' ] )
            
        if os.path.exists( self.config[ 'target-parent' ] ):
            logging.info('Removing %s' % ( self.config[ 'target-parent' ], ) )
            shutil.rmtree( self.config[ 'target-parent' ] )
            
        # export soar suite
        logging.debug( 'Making dir %s' % ( self.config[ 'target-parent' ], ) )
        os.makedirs( self.config[ 'target-parent' ] )
    
        logging.info('Exporting source')
        args = []
        #if self.config[ 'log-level' ] < logging.INFO:
        if False:
            args = [ "svn", "export", self.config[ 'target-url' ], self.config[ 'target-path' ], ]
        else:
            args = [ "svn", "export", "-q", self.config[ 'target-url' ], self.config[ 'target-path' ], ]
            
        retcode = subprocess.call( args )
        if retcode != 0:
            logging.critical( "svn export failed" )
            sys.exit(1)

        # remove unnecessary files and folders
        logging.info( 'Removing globs from source that are not to be distributed with the release' )
        for root, dirs, files in os.walk( self.config[ 'target-path' ] ):
            for glob in self.config[ 'remove' ]:
                matched = [ n for n in dirs if fnmatch.fnmatchcase( n, glob ) ]
                for x in matched:
                    dirs.remove( x )
                    logging.debug( 'Removing dir %s' % os.path.join(root, x) )
                    shutil.rmtree( os.path.join(root, x) )

                matched = [ n for n in files if fnmatch.fnmatchcase( n, glob ) ]
                for x in matched:
                    logging.debug( 'Removing file %s' % os.path.join( root, x ) )
                    os.remove( os.path.join( root, x ) )

        # copy binaries over
        logging.info( 'Copying globs from working tree' )
        for root, dirs, files in os.walk('.'):
            for glob in self.config[ 'copyglobs' ]:
                #matched = [ n for n in dirs if fnmatch.fnmatchcase( n, glob ) ]
                #for x in matched:
                #    dirs.remove( x )
                #    src = os.path.join( root, x )
                #    dst = os.path.join( self.config['target-path'], src )
                #    
                #    # Make sure parent directory exists
                #    dstdir = os.path.join( self.config['target-path'], root )
                #    if not os.path.exists( dstdir ):
                #        logging.debug( 'Making dir %s' % ( dstdir, ) )
                #        os.makedirs( dstdir )

                #   logging.debug( '%s -dir-> %s' % ( src, dst ) )
                #    shutil.copytree( src, dst )

                matched = [ n for n in files if fnmatch.fnmatchcase( n, glob ) ]
                for x in matched:
                    src = os.path.join( root, x )
                    dst = os.path.join( self.config[ 'target-path' ], src )

                    # Make sure parent directory exists
                    dstdir = os.path.join( self.config[ 'target-path' ], root )
                    if not os.path.exists( dstdir ):
                        logging.debug( 'Making dir %s' % ( dstdir, ) )
                        os.makedirs( dstdir )

                    logging.debug( '%s -file-> %s' % ( src, dst ) )
                    shutil.copyfile( src, dst )
                    shutil.copymode( src, dst )

        logging.info( 'Copying SWIG java files' )
        for f in fnmatch.filter( os.listdir( os.path.join( 'Core', 'ClientSMLSWIG', 'Java', 'build' ) ), "*.java" ):
            srcdir = os.path.join( 'Core', 'ClientSMLSWIG', 'Java', 'build' )
            dstdir = os.path.join( self.config['target-path'], srcdir )
            if not os.path.exists( dstdir ):
                logging.debug( 'Making dir %s' % ( dstdir, ) )
                os.makedirs( dstdir )
            
            src = os.path.join( srcdir, f )
            dst = os.path.join( dstdir, f )
            logging.debug( '%s -file-> %s' % ( src, dst, ) )
            shutil.copyfile( src, dst )
            shutil.copymode( src, dst )
        
        logging.info( 'Copying top-level files' )
        for root, dirs, files in os.walk('.'):
            for glob in self.config[ 'top-level-files' ]:
                matched = [ n for n in files if fnmatch.fnmatchcase( n, glob ) ]
                for x in matched:
                    src = os.path.join( root, x )
                    dst = os.path.join( self.config[ 'target-parent' ], x )

                    logging.debug( '%s -file-> %s' % ( src, dst ) )
                    shutil.copyfile( src, dst )
                    shutil.copymode( src, dst )

        logging.info( 'Creating archive' )
        if os.name != 'posix':
            zip = zipfile.ZipFile( os.path.join( "..", self.config[ 'target-file' ] ), "w" )
            for root, dirs, files in os.walk( self.config['target-parent'] ):
                ( dontcare1, dontcare2, zipTargetRoot ) = root.partition( os.sep )
                for name in files:
                    zipTarget = os.path.join( zipTargetRoot, name )
                    logging.debug( 'Adding %s' % ( zipTarget, ) )
                    zip.write( os.path.join( root, name ), zipTarget, zipfile.ZIP_DEFLATED )
            zip.close()
        else:
            retcode = subprocess.call( ["tar", "cfzp", os.path.join( "..", self.config[ 'target-file' ] ), self.config['target-parent'] ] )
            

def usage():
    print sys.argv[0], "usage:"
    print "\t-t, --tag: Target base version (required)"
    print "\t-h, --help: This message."
    print "\t-q, --quiet: Decrease logger verbosity, use multiple times for greater effect."
    print "\t-v, --verbose: Increase logger verbosity, use multiple times for greater effect."
    print "\t-b, --build: Build everything before generating package."
    
def main():
    
    loglevel = logging.INFO
    
    try:
        opts, args = getopt.getopt( sys.argv[1:], "hqvbt:", [ "help", "quiet", "verbose", "build", "tag=", ] )
    except getopt.GetoptError:
        logging.critical( "Unrecognized option." )
        usage()
        sys.exit(1)

    build = False
    tag = None
    
    for o, a in opts:
        if o in ( "-h", "--help" ):
            usage()
            sys.exit(0)
        if o in ( "-q", "--quiet" ):
            loglevel += 10
        if o in ( "-v", "--verbose" ):
            loglevel -= 10
        if o in ( "-b", "--build" ):
            build = True
        if o in ( "-t", "--tag" ):
            tag = a
    
    if tag == None:
        logging.critical( 'Version tag required (option: -t|--tag)' )
        sys.exit(1)
    generatorConfig[ 'version' ] = tag
    
    if loglevel < logging.DEBUG:
        loglevel = logging.DEBUG
    if loglevel > logging.CRITICAL:
        loglevel = logging.CRITICAL

    generatorConfig[ 'log-level' ] = loglevel
    logging.basicConfig( level=loglevel, format='%(asctime)s %(levelname)s %(message)s' )
    
    logging.info( 'Starting generator in %s' % os.getcwd() )
    generator = Generator( generatorConfig )
    
    if build:
        generator.build()
    generator.source()

    logging.info('Done')
    
if __name__ == "__main__":
    main()

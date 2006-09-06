#!/usr/bin/python
#
# Author: Jonathan Voigt, University of Michigan
# Date: September 2006


###
# Variables
# These should be in a separate configuration file in the future.
# Using c as an alias to save typing
c = generatorConfig = {}

c['soarurl'] = 'https://winter.eecs.umich.edu/svn/soar/trunk/SoarSuite'

c['nameandversion'] = 'Soar Suite 8.6.3'

# File globs to completely remove from the tree
c['remove'] = ['Makefile.in', '*.nsi.in', 'INSTALL', 'README', '.project', 
               '.cvsignore', '.svn', '*.xcodeproj', '*.so', '*.so.1', '*.so.2', 
               '*.jnilib', 'java_swt', '*.sh', '*.plist', '*.doc', '*.ppt', '*.pl',
               '*.am', '*.ac', '*.m4', 'ManualSource', 'Old', '*.tex', 'Scripts',
               'installergen.py']

# Globs to copy from working copy to Core component
# WORKING --copy-to-> CORE
c['copycoreglobs'] = ['*.pdf', '*.dll', '*.exe', '*.jar',]

# Globs to copy from working copy to Source component
# WORKING --copy-to-> SOURCE
c['copysourceglobs'] = ['ClientSML.lib', 'ElementXML.lib', 'ConnectionSML.lib', 
                        'Tcl_sml_ClientInterface', 'Tcl_sml_ClientInterface_wrap.cxx', 
                        'Java_sml_ClientInterface_wrap.cxx', 'CSharp_sml_ClientInterface_wrap.cxx',]

# Globs to MOVE from Source component to Core component
# SOURCE --move-to-> CORE
c['moveglobs'] = ['COPYING', 'Documentation', 'docs', 'Icons', 'SoarLibrary', 'agents', 'maps', 
                  'templates', 'tcl', 'TSI', 'TclEaters', 'run-*.bat', 'TestTclSML.tcl', 
                  'pkgIndex.tcl', 'mac.soar', 'FilterTcl', 'towers-of-hanoi-SML.soar',]

# Nullsoft installer script input file
c['nsiinput'] = "8.6.3.nsi.in"

# Nullsoft installer script output file
c['nsioutput'] = "Soar-Suite-8.6.3.nsi"

# Location of NSIS executable (makensis.exe)
c['makensis'] = "\"\Program Files (x86)\NSIS\makensis.exe\""
###

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

class Generator:
    "This thing builds Soar installers."
    config = None
    files_to_delete = None
    dirs_to_delete = None
    
    def __init__(self, generatorConfig):
        self.config = generatorConfig
        self.config['namedashes'] = generatorConfig['nameandversion'].replace(' ', '-')
        self.config['core'] = os.path.join('..', '%s-c' % self.config['namedashes'])
        self.config['source'] = os.path.join('..', '%s-s' % self.config['namedashes'])
        self.config['msprogramsname'] = generatorConfig['nameandversion'].replace('Suite ', '')
        for x in self.config:
            logging.debug('config: %s: %s' % (x, repr(self.config[x])))
    
    def build(self):
        logging.info('Rebuilding everything.')
        os.system('rebuild-all.bat')
    
    def do_files(self, top, nsioutput):
        for root, dirs, files in os.walk(top):
            outputdir = root.replace(top, "")
            nsioutput.write("\n\tSetOutPath \"$INSTDIR%s\"\n" % outputdir)
            self.dirs_to_delete.append("$INSTDIR%s" % outputdir)
            
            for f in files:
                nsioutput.write("\tFile \"%s\"\n" % os.path.join(root, f))
                self.dirs_to_delete.append("$INSTDIR%s\\%s" % (outputdir, f))

    def source(self):
        if os.path.exists(self.config['source']):
            logging.debug('Removing old source tree: %s' % self.config['source'])
            shutil.rmtree(self.config['source'])
        
        logging.info('Checking out source tree.')
        os.system('svn export -q %s %s' % (self.config['soarurl'], self.config['source']))
        
        logging.info('Removing globs from source that are not to be distributed with the release.')
        for root, dirs, files in os.walk(self.config['source']):
            for glob in self.config['remove']:
                #matched = fnmatch.filter(dirs, glob)
                matched = [n for n in dirs if fnmatch.fnmatchcase(n, glob)]
                for x in matched:
                    dirs.remove(x)
                    logging.debug('Removing %s' % os.path.join(root, x))
                    shutil.rmtree(os.path.join(root, x))
                #matched = fnmatch.filter(files, glob)
                matched = [n for n in files if fnmatch.fnmatchcase(n, glob)]
                for x in matched:
                    logging.debug('Removing %s' % os.path.join(root, x))
                    os.remove(os.path.join(root, x))
            
        if os.path.exists(self.config['core']):
            logging.debug('Removing old core tree: %s' % self.config['core'])
            shutil.rmtree(self.config['core'])
            
        logging.info('Copying globs from working tree to core.')
        for root, dirs, files in os.walk('.'):
            for glob in self.config['copycoreglobs']:
                #matched = fnmatch.filter(dirs, glob)
                matched = [n for n in dirs if fnmatch.fnmatchcase(n, glob)]
                for x in matched:
                    dirs.remove(x)
                    src = os.path.join(root, x)
                    dst = os.path.join(self.config['core'], src)
                    logging.debug('%s -dir-> %s' % (src, dst))
                    
                    # Make sure parent directory exists
                    dstdir = os.path.join(self.config['core'], root)
                    if not os.path.exists(dstdir):
                        os.makedirs(dstdir)
                    shutil.copytree(src,dst)
                #matched = fnmatch.filter(files, glob)
                matched = [n for n in files if fnmatch.fnmatchcase(n, glob)]
                for x in matched:
                    src = os.path.join(root, x)
                    dst = os.path.join(self.config['core'], src)
                    logging.debug('%s -file-> %s' % (src, dst))

                    # Make sure parent directory exists
                    dstdir = os.path.join(self.config['core'], root)
                    if not os.path.exists(dstdir):
                        os.makedirs(dstdir)
                    shutil.copyfile(src, dst)

        logging.info('Copying SWIG java files to source.')
        for f in fnmatch.filter(os.listdir('Core\\ClientSMLSWIG\\Java\\build'), "*.java"):
            srcdir = 'Core\\ClientSMLSWIG\\Java\\build'
            dstdir = os.path.join(self.config['source'], srcdir)
            if not os.path.exists(dstdir):
                os.makedirs(dstdir)
            
            src = os.path.join(srcdir, f)
            dst = os.path.join(dstdir, f)
            logging.debug('%s -file-> %s' % (src, dst))
            shutil.copyfile(src, dst)
        
        logging.info('Moving globs from source tree to core.')
        for root, dirs, files in os.walk(self.config['source']):
            for glob in self.config['moveglobs']:
                #matched = fnmatch.filter(dirs, glob)
                matched = [n for n in dirs if fnmatch.fnmatchcase(n, glob)]
                for x in matched:
                    dirs.remove(x)
                    src = os.path.join(root, x)
                    dst = os.path.join(self.config['core'], src.replace(self.config['source'], '.'))
                    logging.debug('%s -dir->> %s' % (src, dst))

                    # This not necessary with distutils.dir_util.copy_tree
                    #dstdir = os.path.join(self.config['core'], root)
                    #if not os.path.exists(dstdir):
                    #    os.makedirs(dstdir)
                    distutils.dir_util.copy_tree(src, dst, update=True)
                    shutil.rmtree(src)
                #matched = fnmatch.filter(files, glob)
                matched = [n for n in files if fnmatch.fnmatchcase(n, glob)]
                for x in matched:
                    src = os.path.join(root, x)
                    dst = os.path.join(self.config['core'], src.replace(self.config['source'], '.'))
                    logging.debug('%s -file->> %s' % (src, dst))
        
                    # Make sure parent directory exists
                    dstdir = os.path.join(self.config['core'], root.replace(self.config['source'], '.'))
                    if not os.path.exists(dstdir):
                        os.makedirs(dstdir)
                    shutil.move(src,dst)

        logging.info('Copying globs from working tree to source.')
        for root, dirs, files in os.walk('.'):
            for glob in self.config['copysourceglobs']:
                #matched = fnmatch.filter(dirs, glob)
                matched = [n for n in dirs if fnmatch.fnmatchcase(n, glob)]
                for x in matched:
                    dirs.remove(x)
                    src = os.path.join(root, x)
                    dst = os.path.join(self.config['source'], src)
                    logging.debug('%s -dir-> %s' % (src, dst))
                    
                    # Make sure parent directory exists
                    dstdir = os.path.join(self.config['source'], root)
                    if not os.path.exists(dstdir):
                        os.makedirs(dstdir)
                    shutil.copytree(src,dst)
                #matched = fnmatch.filter(files, glob)
                matched = [n for n in files if fnmatch.fnmatchcase(n, glob)]
                for x in matched:
                    src = os.path.join(root, x)
                    dst = os.path.join(self.config['source'], src)
                    logging.debug('%s -file-> %s' % (src, dst))

                    # Make sure parent directory exists
                    dstdir = os.path.join(self.config['source'], root)
                    if not os.path.exists(dstdir):
                        os.makedirs(dstdir)
                    shutil.copyfile(src, dst)
        
        logging.info('Renaming COPYING to License.txt')
        shutil.move(os.path.join(self.config['core'], 'COPYING'), os.path.join(self.config['core'], 'License.txt'))

        logging.info('Removing svn dirs from core.')
        for root, dirs, files in os.walk(self.config['core']):
            for x in dirs:
                if x == ".svn":
                    dirs.remove(x)
                    shutil.rmtree(x)
        
        logging.info('Removing svn dirs from source.')
        for root, dirs, files in os.walk(self.config['source']):
            for x in dirs:
                if x == ".svn":
                    dirs.remove(x)
                    shutil.rmtree(x)
        
        logging.info('Removing globs from source again.')
        for root, dirs, files in os.walk(self.config['source']):
            for glob in self.config['remove']:
                #matched = fnmatch.filter(dirs, glob)
                matched = [n for n in dirs if fnmatch.fnmatchcase(n, glob)]
                for x in matched:
                    dirs.remove(x)
                    logging.debug('Removing %s' % os.path.join(root, x))
                    shutil.rmtree(os.path.join(root, x))
                #matched = fnmatch.filter(files, glob)
                matched = [n for n in files if fnmatch.fnmatchcase(n, glob)]
                for x in matched:
                    logging.debug('Removing %s' % os.path.join(root, x))
                    os.remove(os.path.join(root, x))
        
        logging.info('Removing core Documentation\\ManualSource.')
        shutil.rmtree(os.path.join(self.config['core'], 'Documentation\\ManualSource'))

        logging.info('Removing core SoarLibrary\\lib')
        shutil.rmtree(os.path.join(self.config['core'], 'SoarLibrary\\lib'))
        
        logging.info('Copying working SoarLibrary\\bin\\tcl_sml_clientinterface\\pkgIndex.tcl to core')
        #os.makedirs(os.path.join(self.config['core'], 'SoarLibrary\\bin\\tcl_sml_clientinterface'))
        shutil.copyfile('SoarLibrary\\bin\\tcl_sml_clientinterface\\pkgIndex.tcl',
                        os.path.join(self.config['core'], 'SoarLibrary\\bin\\tcl_sml_clientinterface\\pkgIndex.tcl'))

        logging.info('Moving core Tools\\VisualSoar\\Source to source')
        distutils.dir_util.copy_tree(os.path.join(self.config['core'], 'Tools\\VisualSoar\\Source'), 
                                     os.path.join(self.config['source'], 'Tools\\VisualSoar\\Source'), update=True)
        shutil.rmtree(os.path.join(self.config['core'], 'Tools\\VisualSoar\\Source'))

    def nsi(self):
        logging.info('Generating NSI installer script.')
        
        nsiinput = file(self.config['nsiinput'])
        nsioutput = file(self.config['nsioutput'], 'w')
        
        self.files_to_delete = []
        self.dirs_to_delete = []

        for line in nsiinput:
            match = re.match(r"(.*)nameandversion(.*)", line)
            if match != None:
                nsioutput.write("%s%s%s\n" % (match.group(1), self.config['nameandversion'], match.group(2)))
                continue
            
            match = re.match(r"(.*)installdir(.*)", line)
            if match != None:
                nsioutput.write("%sSoar/%s%s\n" % (match.group(1), self.config['namedashes'], match.group(2)))
                continue

            match = re.match(r"(.*)outfile(.*)", line)
            if match != None:
                nsioutput.write("%s%s.exe%s\n" % (match.group(1), self.config['namedashes'], match.group(2)))
                continue

            match = re.match(r"(.*)msprogramsname(.*)", line)
            if match != None:
                nsioutput.write("%s%s%s\n" % (match.group(1), self.config['msprogramsname'], match.group(2)))
                continue

            match = re.match(r"(.*)corefiles(.*)", line)
            if match != None:
                self.do_files(self.config['core'], nsioutput)
                nsioutput.write("" % ())
                continue

            match = re.match(r"(.*)sourcefiles(.*)", line)
            if match != None:
                self.do_files(self.config['source'], nsioutput)
                nsioutput.write("" % ())
                continue

            match = re.match(r"\s*CreateShortCut\s*\"(.+?)\"", line)
            if match != None:
                self.files_to_delete.append(match.group(1))
                nsioutput.write(line)
                continue

            match = re.match(r"\s*CreateDirectory\s*\"(.+?)\"", line)
            if match != None:
                self.dirs_to_delete.append(match.group(1))
                nsioutput.write(line)
                continue

            match = re.match(r"(.*)deletefiles(.*)", line)
            if match != None:
                for f in self.files_to_delete:
                    nsioutput.write("\tDelete \"%s\"\n" % (f))
                continue

            match = re.match(r"(.*)deletedirs(.*)", line)
            if match != None:
                self.dirs_to_delete.reverse()
                for dir in self.dirs_to_delete:
                    nsioutput.write("\tRMDir \"%s\"\n" % (dir))
                continue

            nsioutput.write(line)
        
        self.files_to_delete = None
        self.dirs_to_delete = None
        
        nsiinput.close()
        nsioutput.close()
        
        logging.info('Converting forward slashes to backslashes.')

        logging.debug('Reading NSI script into memory')
        nsiscript = file(self.config['nsioutput'])
        nsiscriptlines = nsiscript.readlines()
        nsiscript.close()
        
        logging.debug('Writing new script.')
        nsioutput = file(self.config['nsioutput'], 'w')
        
        for line in nsiscriptlines:
            match = re.match(r"http:\/\/", line)
            if match == None:
                match = re.match(r"([^\"]*)\"(.*)\"(.*)", line)
                if match != None:
                    slashes = match.group(2)
                    slashes = slashes.replace("/", "\\")
                    line = "%s\"%s\"%s\n" % (match.group(1), slashes, match.group(3))
            nsioutput.write(line)
        nsioutput.close()
        
        logging.info('Creating installer.')
        os.system('%s %s' % (self.config['makensis'], self.config['nsioutput']))

def usage():
    print "installergen.py usage:"
    print "\t-h, --help: This message."
    print "\t-q, --quiet: Decrease logger verbosity, use multiple times for greater effect."
    print "\t-v, --verbose: Increase logger verbosity, use multiple times for greater effect."
    print "\tModes:"
    print "\t-a, --all: Do everything."
    print "\t-b, --build: Build everything."
    print "\t-s, --source: Check out source, generate -core and -source dirs."
    print "\t-n, --nsi: Generate NSI script and run NSIS."

def main():
    loglevel = logging.INFO
    build = False
    source = False
    nsi = False
    
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hqvbsna", ["help", "quiet", "verbose", "build", "source", "nsi", "all"])
    except getopt.GetoptError:
        print "Unrecognized option."
        usage()
        sys.exit(1)
        
    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        if o in ("-q", "--quiet"):
            loglevel += 10
        if o in ("-v", "--verbose"):
            loglevel -= 10
        if o in ("-a", "--all"):
            build = True;
            source = True;
            nsi = True;
        if o in ("-b", "--build"):
            build = True;
        if o in ("-s", "--source"):
            source = True;
        if o in ("-n", "--nsi"):
            nsi = True;
    
    if loglevel < logging.DEBUG:
        loglevel = logging.DEBUG
    if loglevel > logging.CRITICAL:
        loglevel = logging.CRITICAL

    logging.basicConfig(level=loglevel,format='%(asctime)s %(levelname)s %(message)s')
    
    if not build and not source and not nsi:
        logging.critical('One of the three modes (build, source, nsi) must be chosen on the command line.')
    
    logging.info('Starting generator in %s' % os.getcwd())
    generator = Generator(generatorConfig)
    if build:
        generator.build()
    
    if source:
        generator.source()
        
    if nsi:
        generator.nsi()
        
    logging.info('Done')
    time.sleep(2)
    
if __name__ == "__main__":
    main()

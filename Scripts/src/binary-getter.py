#!/usr/bin/python
#
# Author: Jonathan Voigt, Nate Derbinsky, University of Michigan
# Date: September 2008

import logging
import sys
import os
import os.path
import urllib2
import xml.dom.minidom
import xml.sax.saxutils
import htmlentitydefs
import re
import cgi

logging.info( 'Copying binaries from winter' )
index = urllib2.urlopen( "http://winter.eecs.umich.edu/soar/release-binaries/Soar-Suite-9.0.0-rc2" )
dom = xml.dom.minidom.parseString( index.read() )

def getText( nodelist ):
    rc = ""
    for node in nodelist:
        if node.nodeType == node.TEXT_NODE:
            rc = rc + node.data
    return rc

for li in dom.getElementsByTagName("li"):
    href = xml.sax.saxutils.unescape( li.getElementsByTagName("a")[0].getAttribute("href") )
    linktext = getText( li.getElementsByTagName("a")[0].childNodes )
    dst = os.path.join( "target-parent/", href )
    srcurl = "%s/%s" % ( "http://winter.eecs.umich.edu/soar/release-binaries/Soar-Suite-9.0.0-rc2", href, )
    print '%s -> %s' % ( srcurl, dst, )
    
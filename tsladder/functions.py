#!/usr/bin/python

from configobj import ConfigObj
from pathutils import *
from cgiutils import *

def welcome_page():
    newwelcomepage = readfile('templates/header.txt')
    newwelcomepage += "<p>Don't touch anything, winter could explode.</p>"
    newwelcomepage = readfile('templates/footer.txt')
    print serverline + "\n" + newwelcomepage
    sys.exit()

def upload_page(action, userconfig):
    print "<form method='POST' enctype='multipart/form-data' action='index.cgi'>"
    print "User ID: <input type='text' name='userid' /><br />"
    print "Tank ID: <input type='text' name='tankid' /><br />"
    print "Tank zip: <input type='file' name='upfile' /><br />"
    print "<input type='submit' value='Upload Tank' />"
    print "<input type='hidden' name='action' value='upload' />"
    print "</form>"
    return

def save_tank():
    f = open("tanks/" + form["upfile"].filename, 'w')
    f.write(form["upfile"].value)
    f.close()
    return

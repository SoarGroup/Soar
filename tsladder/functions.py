#!/usr/bin/python

import sys
from configobj import ConfigObj
from pathutils import *
from cgiutils import *

def account_links(userconfig):
	ret = "<p><a href='/tsladder/index.cgi?login=logout'>Log out</a> | <a href='/tsladder/index.cgi?login=editaccount'>Edit account</a>"
	if (userconfig != None) & (userconfig['admin'] == "3"):
        	ret += " | <a href='/tsladder/index.cgi?login=admin'>Admin</a>"
	ret += "</p>\n"
	return ret
	
def welcome_page(action, userconfig):
    page = readfile('templates/header.txt')
    page += account_links(userconfig)
    page += "<p>Don't touch anything, winter could explode.</p>"
    page += readfile('templates/menu.txt')
    page += readfile('templates/footer.txt')
    
    page = page.replace('**manage tanks link**', thisscript+'?action=managetanks')
    
    print "Content-type: text/html\n"
    print page
    sys.exit()

def managetanks_page(action, userconfig):
    page = readfile('templates/header.txt')
    page += account_links(userconfig)
    page += "<p>Don't touch anything, winter could explode.</p>"
    page += readfile('templates/managetanks.txt')
    page += readfile('templates/footer.txt')

    tanktable = "<table border='0'>\n"
    tanktable += "<tr><td></td></tr>\n"
    tanktable += "\n</table>"

    page = page.replace('**tank table**', tanktable)
	
    print "Content-type: text/html\n"
    print page
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

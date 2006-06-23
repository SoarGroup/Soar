#!/usr/bin/python

import sys
import os
from configobj import ConfigObj
from pathutils import *
from cgiutils import *

thisscript = os.environ['SCRIPT_NAME']

def account_links(userconfig):
	ret = "<p><a href='/tsladder/index.cgi?login=logout'>Log out</a> | <a href='/tsladder/index.cgi?login=editaccount'>Edit account</a>"
	if (userconfig != None) & (userconfig['admin'] == "3"):
        	ret += " | <a href='/tsladder/index.cgi?login=admin'>Admin</a>"
	ret += "</p>\n"
	return ret
	
def welcome_page(action, userconfig):
    page = readfile('templates/header.html')
    page += account_links(userconfig)
    page += "<p>Don't touch anything, winter could explode.</p>"
    page += readfile('templates/menu.html')
    page += readfile('templates/footer.html')
    
    page = page.replace('**manage tanks link**', thisscript+'?action=managetanks')
    
    print "Content-type: text/html\n"
    print page
    sys.exit()

def managetanks_page(action, userconfig):
	page = readfile('templates/header.html')
	page += account_links(userconfig)
	page += "<p>Don't touch anything, winter could explode.</p>"
	page += readfile('templates/managetanks.html')
	page += readfile('templates/footer.html')

	tanktable = "<table border='1'>\n"
	if (not userconfig.has_key('tanks')) | (len(userconfig['tanks']) == 0):
		tanktable += "<tr><td>No tanks.</td></tr>\n"
	else:
		for tank in userconfig['tanks']:
			tanktable += "<tr><td>"
			tanktable += tank + " (" + userconfig[tank]['source'] + ")"
			tanktable += "</td><td><a href=''>Delete</a></td></tr>\n"
	tanktable += "\n</table>"
	tanktable += "<p>" + repr(userconfig) + "</p>"

	page = page.replace('**tank table**', tanktable)
	page = page.replace('**upload tank link**', thisscript+'?action=upload')

	print "Content-type: text/html\n"
	print page
	sys.exit()
	
def upload_page(action, userconfig):
	page = readfile('templates/header.html')
	page += account_links(userconfig)
	page += readfile('templates/uploadform.html')
	page += readfile('templates/footer.html')

	print "Content-type: text/html\n"
	print page
	sys.exit();

def save_tank():
	f = open("tanks/" + form["upfile"].filename, 'w')
	f.write(form["upfile"].value)
	f.close()
	sys.exit();

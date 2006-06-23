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
	page += "<p>" + repr(userconfig) + "</p>"
	page += readfile('templates/managetanks.html')
	page += readfile('templates/footer.html')

	tanktable = "<table border='1'>\n"
	if (not userconfig.has_key('tanks')) | (len(userconfig['tanks']) == 0):
		tanktable += "<tr><td>No tanks.</td></tr>\n"
	else:
		for tank in userconfig['tanks']:
			tanktable += "<tr><td>"
			tanktable += tank + " (" + userconfig[tank]['source'] + ")"
			tanktable += "</td><td><a href='" + thisscript + "?action=deletetank&tank=" + tank + "'>Delete</a></td></tr>\n"
	tanktable += "\n</table>"

	page = page.replace('**tank table**', tanktable)
	page = page.replace('**upload form**', readfile('templates/uploadform.html'))
	page = page.replace('**this script**', thisscript)

	print "Content-type: text/html\n"
	print page
	sys.exit()
	
def delete_tank(action, userconfig, tank):
	userconfig['tanks'].remove(tank)
	#FIXME: currenly no way of removing tanks from file!
	#userconfig.remove(tank)
	top = "tanks/" + userconfig['username'] + "/" + tank
	for root, dirs, files in os.walk(top, topdown=False):
		for name in files:
			os.remove(os.path.join(root, name))
		for name in dirs:
			os.rmdir(os.path.join(root, name))
	os.rmdir(top)
	userconfig.write()
	
def upload_page(action, userconfig, tankname, tankfilename, tankfile, source):
	#verify no duplicate tank name exists
	duplicate = True
	try:
		userconfig['tanks'].index(tankname)
	except ValueError:
		duplicate = False

	page = readfile('templates/header.html')
	page += account_links(userconfig)
	
	if duplicate:
		page += "<p><font color='red'>Tank name '" + tankname + "' already exists, hit back and select a new tank name.</font></p>"
	else:
		#create the tank directory
		os.makedirs("tanks/" + userconfig['username'] + "/" + tankname)

		#save the zip file
		file = open("tanks/" + userconfig['username'] + "/" + tankname + "/" + tankfilename, 'w')
		file.write(tankfile)
		file.close()

		#update the user config
		userconfig['tanks'].append(tankname)
		userconfig[tankname] = {}
		userconfig[tankname]['source'] = source
		userconfig.write()

		page += "<p>Tank name: " + tankname + "</p>"
		page += "<p>Tank filename: " + tankfilename + "</p>"
		
	page += readfile('templates/footer.html')

	print "Content-type: text/html\n"
	print page
	sys.exit();

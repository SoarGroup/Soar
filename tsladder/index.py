#!/usr/bin/python -u

import cgi
import cgitb; cgitb.enable()
#import cgitb; cgitb.enable(display=0, logdir="/tmp")

import sys
import os
sys.path.append(os.path.abspath('modules'))
sys.path.append(os.path.abspath('modules/logintools'))

from configobj import ConfigObj
from pathutils import *
from cgiutils import *

userdir = 'users/'
theform = cgi.FieldStorage()
from logintools import login
action, userconfig = login(theform, userdir)

from functions import *

import MySQLdb

def get_userid(cursor, screwed=False):
	cursor.execute("SELECT userid FROM users WHERE username=%s", (userconfig['username'],))
	if int(cursor.rowcount) < 1:
		if screwed:
			raise RuntimeError("Username addition silently failed, preventing recursion.")
		cursor.execute("INSERT INTO users (username) VALUES(%s)", (userconfig['username'],))
		return get_userid(cursor, True)
		
	result, = cursor.fetchone()
	return int(result)

db = MySQLdb.connect(host="localhost", user="tsladder", passwd="75EbRL", db="tsladder")
cursor = db.cursor()
userid = get_userid(cursor)
if userid == 0:
	cursor.execute("INSERT INTO users (username) VALUES(%s)", (userconfig['username'],))
	userid = get_userid(cursor)

if action == None:
	welcome_page(action, userid, cursor)

if action == "managetanks":
	managetanks_page(action, userid, cursor)

if action == "upload":
	tankname = theform['tankname'].value
	tankfile = theform['tankfile'].value
	tankfilename = theform['tankfile'].filename
	source = theform['source'].value
	upload_page(action, userid, cursor, tankname, tankfilename, tankfile, source)

if action == "deletetank":
	tankid = int(theform['tankid'].value)
	delete_tank(action, userid, cursor, tankid)
	managetanks_page(action, userid, cursor)
	
if action == "viewtank":
	tankid = int(theform['tankid'].value)
	view_tank_page(action, userid, cursor, tankid)

if action == "viewfile":
	tankid = int(theform['tankid'].value)
	file = theform['file'].value
	view_file_page(action, userid, cursor, tankid, file)

if action == "mirrormatch":
	tankid = int(theform['tankid'].value)
	mirrormatch_page(action, userid, cursor, tankid)

if action == "resetstats":
	resetstats(cursor)
	welcome_page(action, userid, cursor)

if action == "viewmatches":
	tankid = int(theform['tankid'].value)
	view_matches_page(action, userid, cursor, tankid)

if action == "viewmatchlog":
	matchid = int(theform['matchid'].value)
	view_match_log_page(action, userid, cursor, matchid)

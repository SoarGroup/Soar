#!/usr/bin/python

import cgi
import cgitb; cgitb.enable()
import smtplib

form = cgi.FieldStorage()

if not form.has_key("")

server = smtplib.SMTP('localhost')
server.sendmail(from_addr="tsladder@winter.eecs.umich.edu", )
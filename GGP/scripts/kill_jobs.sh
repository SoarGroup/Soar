#!/usr/bin/python
import os

m = ["bahamut", "smaug", "grapes", "auk", "badboy", "flamingo", "winter", "wyrm"]
me = os.popen('hostname').read().strip()
m.remove(me)
user = os.environ["USER"]

for i in m:
	print i
	print 'ssh %s "kill -9 $(ps h -u %s | grep python | awk \'{print \$1}\')"' % (i, user)
	#os.system('ssh %s "kill -9 $(ps h -u %s | grep python | awk \'{print \$1}\')"' % (i, user))

print me
os.system('kill -9 $(ps h -u %s | grep python | awk \'{print \$1}\')' % user)

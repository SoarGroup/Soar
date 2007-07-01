import os
import time
import sys

server = os.spawnl(os.P_NOWAIT, "TestClientSML", "TestClientSML", "-shortlistener")
time.sleep(3)
client = os.spawnl(os.P_NOWAIT, "TestClientSML", "TestClientSML", "-remote")

ignored, exitcode = os.waitpid(client, 0)
clientSuccess = False
print "Client",
if exitcode:
    print "failed."
else:
    print "successful."
    clientSuccess = True

ignored, exitcode = os.waitpid(server, 0)
serverSuccess = False
print "Server",
if exitcode:
    print "failed."
else:
    print "successful."
    serverSuccess = True

if clientSuccess and serverSuccess:
    sys.exit(0)
sys.exit(1)
#!/usr/bin/python

# Jonathan Voigt, July 2007

# invoke without args to run all tests
# invoke with test name to run a specific test:
#   tester.py q14

import os
import time
import sys
import types

class TestDefinition:
    success = False
    def __init__(self, name, args):
        self.name = name
        self.args = args
        if len(self.args) == 1:
            self.single = True
        else:
            self.single = False

def defineTests(port):
    tests = []
    tests.append(TestDefinition('q1', [["TestConnectionSML",],]))
    tests.append(TestDefinition('q2', [["TestClientSML",],]))
    tests.append(TestDefinition('q3', [["TestClientSML", "-shortlistener", "%d" % port,], ["TestClientSML", "-remote", "%d" % port,],]))
    tests.append(TestDefinition('q4', [["TestMultiAgent",],]))
    tests.append(TestDefinition('q13', [["TOHSML",],]))
    tests.append(TestDefinition('q14', [["TestClientSML", "-shortlistener", "%d" % port,], ["TOHSML", "-remote", "%d" % port,],]))
    tests.append(TestDefinition('q15', [["TestSoarPerformance",],]))
    tests.append(TestDefinition('q16', [["TestSMLPerformance",],]))
    return tests

port = 12121
parsePort = False
singleTest = None
for arg in sys.argv[1:]:
    if parsePort == True:
        port = int(arg)
        parsePort = False
    elif arg == '-port':
        parsePort = True
    else:
        singleTest = arg
        
print 'singleTest', singleTest
print 'port', port

tests = defineTests(port)
for test in tests:
    if singleTest != None:
        if singleTest != test.name:
            continue
    
    if test.single:
        #single test
        print "***"
        print test.name
        print "***"
        exitcode = os.spawnv(os.P_WAIT, test.args[0][0], test.args[0])
        if exitcode == 0:
            print "%s: Success" % (test.name,)
            test.success = True
        else: 
            print "%s: Failure" % (test.name,)
            
    else:
        #multi test
        print "***" 
        print test.name
        print "***"
        server = os.spawnv(os.P_NOWAIT, test.args[0][0], test.args[0])
        time.sleep(3)
        client = os.spawnv(os.P_NOWAIT, test.args[1][0], test.args[1])
        
        ignored, exitcode = os.waitpid(client, 0)
        print "%s: Client" % (test.name,),
        failed = False
        if exitcode:
            print "failed."
            failed = True
        else:
            print "successful."
        
        ignored, exitcode = os.waitpid(server, 0)
        print "%s: Server" % (test.name,),
        if exitcode:
            print "failed."
            failed = True
        else:
            print "successful."

        if not failed:
            test.success = True

print "Tests complete."
print

failure = False
for test in tests:
    if singleTest != None:
        if singleTest != test.name:
            continue
    if test.success:
        status = "Success"
    else:
        status = "Failure"
        failure = True
        
    print "%s: %s" % (test.name, status)

if failure:
    print "Some tests failed."
    sys.exit(1)

print "All tests successful."
sys.exit(0)        
        


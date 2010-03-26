#!/usr/bin/python
#
# Simple Soar code obfuscator
# This does simple find and replaces to quickly (but weakly) obfuscate Soar code.

import sys
import Python_sml_ClientInterface
import sre
import random

psci = Python_sml_ClientInterface

class IdentifierMap:
    _identifierMap = {}
    _first = 'abcdefghijklmnopqrstuvwxyz'
    _rest = _first + '1234567890'
    
    def getIdentifier(self, actualIdentifier):
        if actualIdentifier in self._identifierMap:
            return self._identifierMap[actualIdentifier]
        
        identifier = str()
        
        while True:
            identifier = random.sample(self._first, 1)[0]
            for character in random.sample(self._rest, 7):
                identifier += character
            if identifier not in self._identifierMap:
                break
        
        self._identifierMap[actualIdentifier] = identifier
        return identifier

if len(sys.argv) < 2:
    print 'usage: %s FILE [GLOBAL_REPLACE]' % sys.argv[0]
    print '  FILE: The main .soar file of the agent to obfuscate'
    print '  GLOBAL_REPLACE: A space separated list of strings to substitute in the file'
    print
    print 'Examples:'
    print '  %s ../Demos/eight-puzzle/eight-puzzle.soar' % sys.argv[0]
    print '  %s agents/tanksoar/simple-bot/simple-bot.soar "avoid-direction missiles-energy side-direction"' % sys.argv[0]
    sys.exit(0)
    
idMap = IdentifierMap()

globalReplace = str()
if len(sys.argv) > 2:
    globalReplace = sys.argv[2]
globalReplace = globalReplace.split()
print globalReplace

kernel = psci.Kernel.CreateKernelInCurrentThread()
agent = kernel.CreateAgent('obfuscator')
kernel.ExecuteCommandLine('source "%s"' % sys.argv[1], 'obfuscator')
original = kernel.ExecuteCommandLine('print -f', 'obfuscator')
original = original.split('\n')
for line in original:
    match = sre.match(r'^\s*sp\s*\{(.+)\s*$', line)
    if match is not None:
        line = line.replace(match.group(1), idMap.getIdentifier(match.group(1)))
    else:
        vars = sre.findall(r'<(\S+)>', line)
        for var in vars:
            line = line.replace('<' + var + '>', '<' + idMap.getIdentifier(var) + '>')

        match = sre.match(r'.*\^name ([\w-]+)', line)
        if match is not None:
            line = line.replace('^name %s' % match.group(1), '^name %s' % idMap.getIdentifier(match.group(1)))

    for word in globalReplace:
        line = line.replace(word, idMap.getIdentifier(word))
        
    print line
        
import os
import time
import sys

print os.getcwd()
filename = raw_input("filename:")

file = open(filename, "r")

count = 0
newRow = False;
for line in file:
    line = line.strip()
    
    if len(line) == 0:
        continue;
    if line.find(r'<row') != -1:
    	newRow = True;
    	print "];"
    	print count, "=","[",
    	count += 1
    elif line.find(r'<cell') != -1:
    	if not newRow:
	    	sys.stdout.write(',')
    elif line.find(r'<object') != -1:
    	newRow = False
        if line.find('trees') != -1:
	    	sys.stdout.write('t')
        elif line.find('rocks') != -1:
        	sys.stdout.write('r')
        elif line.find('ground') != -1:
        	sys.stdout.write('g')

print "];"
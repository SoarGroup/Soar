import os
import time

print os.getcwd()
filename = raw_input("filename:")

file = open(filename, "r")

for line in file:
    line = line.strip()
    if len(line) == 0:
        continue;
    found_row = line.find(r'<row')
    found_cell = line.find(r'<cell>')
    found_object = line.find(r'<object')
    
    if found_row != -1 or found_cell != -1 or found_object != -1:
        print line

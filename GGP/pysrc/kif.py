# functions for extracting information from kif files

import re

INDEX_PAT = re.compile('\\(index\\s+(\\d+)\\s*\\)')
WALL_PAT  = re.compile('\\(init\\s+\\(wall\\s+(\\d+)\\s+(\\d+)\\s+(\\w+)\\s*\\)\\s*\\)')
LOC_PAT   = re.compile('\\(init\\s+\\(location\\s+([-\\w]+)\\s+(\\d+)\\s+(\\d+)\\s*\\)\\s*\\)')

def GetMapSize(file_name):
	file = open(file_name, 'r')
	# find the largest index
	indexes = []
	for line in file.readlines():
		m = INDEX_PAT.match(line)
		if m != None:
			indexes.append(int(m.group(1)))
	file.close()

	indexes.sort()
	return indexes[-1]

def GetWalls(file_name):
	walls = []
	file = open(file_name, 'r')
	for line in file.readlines():
		m = WALL_PAT.match(line)
		if m != None:
			walls.append((int(m.group(1)), int(m.group(2)), m.group(3)))
	file.close()
	return walls

def GetLocations(file_name):
	locations = dict()
	file = open(file_name, 'r')
	for line in file.readlines():
		m = LOC_PAT.match(line)
		if m != None:
			locations[m.group(1)] = (int(m.group(2)), int(m.group(3)))
	file.close()
	return locations

def SetLocations(file_name, new_file_name, locs):
	file = open(file_name, 'r')
	new_file = open(new_file_name, 'w')

	for line in file.readlines():
		m = LOC_PAT.match(line)
		if m != None:
			if m.group(1) in locs:
				np = locs[m.group(1)]
				new_file.write('(init (location %s %d %d))\n' % (m.group(1), np[0], np[1]))
				continue
		new_file.write(line)
	
	file.close()
	new_file.close()


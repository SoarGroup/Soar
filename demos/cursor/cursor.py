from __future__ import print_function
import sys

def bound(v, low, high):
	if v < low:
		return low
	if v > high:
		return high
	return v
	
cp = [0, 0, 0]
v = [0, 0, 0]

inputs = ['vx', 'vy', 'vz']

print('''a cursor world v 0 0 0 0 0 1 0 1 0 0 1 1 1 0 0 1 0 1 1 1 0 1 1 1
         a target world v 0 0 0 0 0 1 0 1 0 0 1 1 1 0 0 1 0 1 1 1 0 1 1 1 p -20 -20 -20
***''')
sys.stdout.flush()

while True:
	while True:
		line = sys.stdin.readline()
		if line == "":
			sys.exit(0)
		if line.strip() == "***":
			break
		fs = line.split()
		if len(fs) == 0:
			continue
		try:
			v[inputs.index(fs[0])] = bound(float(fs[1]), -3, 3)
		except ValueError:
			print("Incorrect input: " + line, file=sys.stderr)
			sys.exit(1)

	cp = [ sum(pv) for pv in zip(cp, v) ]
	print("INPUT: ", v, file=sys.stderr)
	print("POS:   ", cp, file=sys.stderr)
	print('c cursor p {} {} {}\n***'.format(*cp))
	sys.stdout.flush()

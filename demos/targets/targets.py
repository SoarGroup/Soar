from __future__ import print_function
import sys, os
import random
import fcntl

DELTA = 1

def cube_verts(h, w, d):
	h2 = h / 2
	w2 = w / 2
	d2 = d / 2
	
	return [ -h2, -w2, -d2,
	          h2, -w2, -d2,
	          h2,  w2, -d2,
	         -h2,  w2, -d2,
	         -h2, -w2,  d2,
	          h2, -w2,  d2,
	          h2,  w2,  d2,
	         -h2,  w2,  d2 ]

def overlap(a1, a2, b1, b2):
	return a1 <= b1 < a2 or b1 <= a1 < b2

def dist(p1, p2):
	return sum((x1 - x2) ** 2 for x1, x2 in zip(p1, p2))
	
class Cube:
	def __init__(self, name, canvas, h = 1.0, w = 1.0, d = 1.0, solid=True):
		self.name = name
		self.canvas = canvas
		self.added = False;
		self.verts = cube_verts(h, w, d)
		self.dims = (h, w, d)
		self.pos = [0.0, 0.0, 0.0]
		self.solid = solid
		self.dirty = True
	
	def print_sgel(self):
		if not self.dirty:
			return
		
		if not self.added:
			sgel = "a {} world v {} ".format(self.name, ' '.join(str(x) for x in self.verts))
			self.added = True
		else:
			sgel = "c {} ".format(self.name)
		
		sgel += "p {}".format(' '.join(str(x) for x in self.pos))
		print(sgel)
		self.dirty = False
	
	def move(self, offsets):
		for i in range(3):
			self.pos[i] += offsets[i]
		self.dirty = True
	
	def move_to(self, pos):
		self.pos = pos
		self.dirty = True

	def draw(self):
		w2 = self.dims[0] / 2
		h2 = self.dims[1] / 2
		bbox = (self.pos[0] - w2, self.pos[1] - h2, self.pos[0] + w2, self.pos[1] + h2)
		self.rid = self.canvas.create_rectangle(bbox)
		tx = self.pos[0]
		ty = self.pos[1]
		self.tid = self.canvas.create_text(tx, ty, text = self.name, justify = tk.CENTER)
		
	def update_canvas(self):
		if self.canvas == None:
			return
		
		if hasattr(self, 'rid'):
			self.canvas.delete(self.rid)
			self.canvas.delete(self.tid)
		self.draw()
	
	def min(self):
		return [ self.pos[i] - self.dims[i] / 2 for i in range(3) ]
	
	def max(self):
		return [ self.pos[i] + self.dims[i] / 2 for i in range(3) ]
		
	def intersects(self, b):
		amin = self.min()
		amax = self.max()
		bmin = b.min()
		bmax = b.max()
		for i in range(3):
			if not overlap(amin[i], amax[i], bmin[i], bmax[i]):
				return False
		return True
		
# This world requires the agent to "lock" a cube to the cursor to move it. No collisions occur.
class World1:
	def __init__(self, cursor, cubes):
		self.cursor = cursor
		self.cubes = cubes
		self.print_sgel()
	
	def print_sgel(self):
		self.cursor.print_sgel()
		for c in self.cubes:
			c.print_sgel()
		print('***')
		sys.stdout.flush()
	
	def input(self, locked, d):
		if locked:
			for c in self.cubes:
				if self.cursor.intersects(c):
					c.move(d)
		self.cursor.move(d)
		self.print_sgel()

# This world requires the agent to push the cubes with the cursor. No locking required.
class World2:
	def __init__(self, cursor, cubes):
		self.cursor = cursor
		self.cubes = cubes
		self.print_sgel()
	
	def print_sgel(self):
		self.cursor.print_sgel()
		for c in self.cubes:
			c.print_sgel()
		print('***')
		sys.stdout.flush()
	
	def input(self, locked, d):
		self.cursor.move(d)
		self.cursor.update_canvas()
		for c in self.cubes:
			if c.solid and self.cursor.intersects(c):
				# There are six possible positions for the cube that resolve the collision,
				# 2 for each dimension. Move it to the closest one.
				positions = []
				for i in range(3):
					p1 = c.pos[:]; p2 = c.pos[:]
					p1[i] = self.cursor.pos[i] - self.cursor.dims[i] / 2 - c.dims[i] / 2
					p2[i] = self.cursor.pos[i] + self.cursor.dims[i] / 2 + c.dims[i] / 2
					positions.append(p1)
					positions.append(p2)
				
				dists = [ dist(c.pos, p) for p in positions ]
				closest = min(zip(dists, positions))[1]
				dpos = [x1 - x2 for x1, x2 in zip(closest, c.pos)]
				c.move(dpos)
				c.update_canvas()
				#assert not self.cursor.intersects(c)
		
		self.print_sgel()
	
class Input:
	def __init__(self, world):
		self.world = world
		self.readbuf = ""
	
	def gui_input(self, f, m):
		input = sys.stdin.read()
		if input == '':
			sys.exit(0)
		self.proc_input(input)
	
	def proc_input(self, input):
		self.readbuf += input
		cmd, sep, rest = self.readbuf.partition('\n***\n')
		if sep == '':
			self.readbuf = cmd
		else:
			self.readbuf = rest
			d = [0, 0, 0]
			valid = False
			for line in cmd.split('\n'):
				fields = line.split()
				if len(fields) < 2:
					continue
				
				name = fields[0]
				val = float(fields[1])
				if name == 'dx':
					d[0] = val
					valid = True
				elif name == 'dy':
					d[1] = val
					valid = True
				elif name == 'dz':
					d[2] = val
					valid = True
			
			if valid:
				self.world.input(False, d)

# Argument list has the following format:
#
# [random seed] [num cubes] [touching which cube] [touching which side]

def make_cubes(canvas, args):
	minpos = 0
	maxpos = 300
	
	cubesize = 30
	rseed = 1
	ncubes = 2
	ntargets = 0
	touching = -1
	side = 'x'
	
	try:
		rseed = int(args[0])
		ncubes = int(args[1])
		ntargets = int(args[2])
		touching = int(args[3])
		side = args[4]
	except IndexError:
		pass
	
	random.seed(rseed)
	
	while True:
		cubes = []
		for i in range(ncubes + ntargets):
			if len(cubes) == 0:
				name = 'cur'
			else:
				name = 'c{}'.format(len(cubes))
			
			if len(cubes) < ncubes:
				c = Cube(name, canvas, cubesize, cubesize, cubesize, True)
			else:
				c = Cube(name, canvas, cubesize, cubesize, cubesize, False)
				
			bad = True
			while bad:
				x = random.randint(minpos, maxpos)
				y = random.randint(minpos, maxpos)
				c.move_to([x, y, 0])
			
				bad = False
				for c1 in cubes:
					if c.intersects(c1):
						bad = True
						break
			
			cubes.append(c)
		
		if touching >= 1 and side in 'lrtb':
			cur = cubes[0]
			tc = cubes[touching]
			if side == 'l':
				x = tc.min()[0] - (cubesize / 2)
				y = random.randint(tc.min()[1], tc.max()[1])
			elif side == 'r':
				x = tc.max()[0] + (cubesize / 2)
				y = random.randint(tc.min()[1], tc.max()[1])
			elif side == 't':
				x = random.randint(tc.min()[0], tc.max()[0])
				y = tc.min()[1] - (cubesize / 2)
			elif side == 'b':
				x = random.randint(tc.min()[0], tc.max()[0])
				y = tc.max()[1] + (cubesize / 2)
			
			cur.move_to([x, y, 0])
			
			# check for intersections
			
			bad = False
			for c in cubes[1:]:
				if cur.intersects(c):
					bad = True
					break
			
			if not bad:
				return cubes
		else:
			return cubes
			
if __name__ == '__main__':
	if len(sys.argv) > 1 and sys.argv[1] == '-h':
		headless = True
		cubes = make_cubes(None, sys.argv[2:])
	else:
		headless = False
		import Tkinter as tk
		win = tk.Tk()
		canvas = tk.Canvas(win)
		canvas.pack(fill = tk.BOTH, expand = 1)
		canvas.focus_set()
	
		cubes = make_cubes(canvas, sys.argv[1:])
		for c in cubes:
			c.update_canvas()
	
	w = World2(cubes[0], cubes[1:])
	input = Input(w)
	
	if headless:
		while True:
			line = sys.stdin.readline()
			if len(line) == 0:
				sys.exit(0)
			input.proc_input(line)
	else:
		# set stdin to non-blocking read
		fd = sys.stdin.fileno()
		fl = fcntl.fcntl(fd, fcntl.F_GETFL)
		fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)
	
		win.tk.createfilehandler(sys.stdin, tk.READABLE, input.gui_input)
		tk.mainloop()
	
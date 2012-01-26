from __future__ import print_function
import sys, os
import random
import fcntl

DELTA = 1
MINPOS = 0
MAXPOS = 200
CUBESIZE = 30
HALFCUBE = CUBESIZE / 2

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

def check_bounds(a, b):
	return MINPOS <= a <= b <= MAXPOS

class Cube:
	def __init__(self, name, canvas, solid=True, h = CUBESIZE, w = CUBESIZE, d = CUBESIZE):
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
		while True:
			cmd, sep, rest = self.readbuf.partition('\n***\n')
			if sep == '':
				self.readbuf = cmd
				return
			
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
# [num cubes] [num targets] [touching which cube] [touching which side]

def experiment1_layout(canvas, args):
	ncubes = 2
	ntargets = 0
	touching = -1
	side = 'x'
	
	try:
		ncubes = int(args[0])
		ntargets = int(args[1])
		touching = int(args[2])
		side = args[3]
	except IndexError:
		pass
	
	while True:
		cubes = []
		for i in range(ncubes + ntargets):
			if i == 0:
				name = 'cur'
			elif len(cubes) < ncubes:
				name = 'c{}'.format(i)
			else:
				name = 't{}'.format(i)
			
			if i < ncubes:
				c = Cube(name, canvas, True)
			else:
				c = Cube(name, canvas, False)
				
			bad = True
			while bad:
				x = random.randint(MINPOS, MAXPOS)
				y = random.randint(MINPOS, MAXPOS)
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
				x = tc.min()[0] - HALFCUBE
				y = random.randint(tc.min()[1], tc.max()[1])
			elif side == 'r':
				x = tc.max()[0] + HALFCUBE
				y = random.randint(tc.min()[1], tc.max()[1])
			elif side == 't':
				x = random.randint(tc.min()[0], tc.max()[0])
				y = tc.min()[1] - HALFCUBE
			elif side == 'b':
				x = random.randint(tc.min()[0], tc.max()[0])
				y = tc.max()[1] + HALFCUBE
			
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

def experiment2_layout(canvas, args):
	ntargets = 1
	cur_c1_xrel = 'e'
	cur_c1_yrel = 'n'
	c1_t1_xrel = 'e'
	c1_t1_yrel = 'n'
	
	try:
		ntargets = int(args[0])
		cur_c1_xrel = args[1]
		cur_c1_yrel = args[2]
		c1_t1_xrel = args[3]
		c1_t1_yrel = args[4]
	except IndexError:
		pass
	
	while True:
		cur = Cube('cur', canvas, True)
		c1 = Cube('c1', canvas, True)
		t1 = Cube('t1', canvas, False)
		cubes = [cur, c1, t1]
		
		curx = random.randint(MINPOS, MAXPOS)
		cury = random.randint(MINPOS, MAXPOS)
		cur.move_to([curx, cury, 0])
		#print('cur - {} {}'.format(curx, cury), file=sys.stderr)
		
		c1_xbounds = [MINPOS, MAXPOS]
		c1_ybounds = [MINPOS, MAXPOS]
		if cur_c1_xrel == 'e':
			c1_xbounds[0] = curx + CUBESIZE
		elif cur_c1_xrel == 'w':
			c1_xbounds[1] = curx - CUBESIZE
			
		if cur_c1_yrel == 'n':
			c1_ybounds[1] = cury - CUBESIZE
		elif cur_c1_yrel == 's':
			c1_ybounds[0] = cury + CUBESIZE
		
		if not check_bounds(*c1_xbounds) or not check_bounds(*c1_ybounds):
			continue
		
		c1x = random.randint(*c1_xbounds)
		c1y = random.randint(*c1_ybounds)
		c1.move_to([c1x, c1y, 0])
		#print('c1 - {} {}'.format(c1x, c1y), file=sys.stderr)

		t1_ybounds = [MINPOS, MAXPOS]
		t1_xbounds = [MINPOS, MAXPOS]
		if c1_t1_xrel == 'e':
			t1_xbounds[0] = c1x + CUBESIZE
		elif c1_t1_xrel == 'w':
			t1_xbounds[1] = c1x - CUBESIZE
		
		if c1_t1_yrel == 'n':
			t1_ybounds[1] = c1y - CUBESIZE
		elif c1_t1_yrel == 's':
			t1_ybounds[0] = c1y + CUBESIZE
		
		if not check_bounds(*t1_xbounds) or not check_bounds(*t1_ybounds):
			continue
		
		t1x = random.randint(*t1_xbounds)
		t1y = random.randint(*t1_ybounds)
		t1.move_to([t1x, t1y, 0])
		#print('t1 - {} {}'.format(t1x, t1y), file=sys.stderr)
		break
		
	for i in range(ntargets - 1):
		c = Cube('t{}'.format(i+2), canvas, False)
			
		bad = True
		while bad:
			x = random.randint(MINPOS, MAXPOS)
			y = random.randint(MINPOS, MAXPOS)
			c.move_to([x, y, 0])
		
			bad = False
			for c1 in cubes:
				if c.intersects(c1):
					bad = True
					break
		
		cubes.append(c)
	
	return cubes

def layout(n, canvas, args):
	if n == 1:
		return experiment1_layout(canvas, args)
	else:
		return experiment2_layout(canvas, args)
	
if __name__ == '__main__':
	random.seed(1)
	headless = False
	experiment = 1
	
	args = sys.argv[1:]
	if len(args) > 0 and args[0] == '-h':
		headless = True
		args.pop(0)
	
	if len(args) < 2:
		print('usage: {} [-h] experiment rand_seed [...]'.format(sys.argv[0]))
		sys.exit(1)
	
	experiment = int(args[0])
	random.seed(args[1])
	args = args[2:]
	
	if headless:
		cubes = layout(experiment, None, args)
	else:
		import Tkinter as tk
		win = tk.Tk()
		canvas = tk.Canvas(win)
		canvas.pack(fill = tk.BOTH, expand = 1)
		canvas.focus_set()
	
		cubes = layout(experiment, canvas, args)
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
	
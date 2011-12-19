from __future__ import print_function
import sys, os
import fcntl
import Tkinter as tk

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

def intersects(amin, amax, bmin, bmax):
	for i in range(3):
		if not overlap(amin[i], amax[i], bmin[i], bmax[i]):
			return False
	return True

def dist(p1, p2):
	return sum((x1 - x2) ** 2 for x1, x2 in zip(p1, p2))
	
class Cube:
	def __init__(self, name, canvas, h = 1.0, w = 1.0, d = 1.0):
		self.name = name
		self.canvas = canvas
		self.added = False;
		self.verts = cube_verts(h, w, d)
		self.dims = (h, w, d)
		self.pos = [0.0, 0.0, 0.0]
		self.draw()
		self.dirty = True
	
	def draw(self):
		w2 = self.dims[0] / 2
		h2 = self.dims[1] / 2
		bbox = (self.pos[0] - w2, self.pos[1] - h2, self.pos[0] + w2, self.pos[1] + h2)
		self.rid = self.canvas.create_rectangle(bbox)
		tx = self.pos[0]
		ty = self.pos[1]
		self.tid = self.canvas.create_text(tx, ty, text = self.name, justify = tk.CENTER)
		
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
		self.update_canvas()
		self.dirty = True

	def update_canvas(self):
		self.canvas.delete(self.rid)
		self.canvas.delete(self.tid)
		self.draw()
	
	def min(self):
		return [ self.pos[i] - self.dims[i] / 2 for i in range(3) ]
	
	def max(self):
		return [ self.pos[i] + self.dims[i] / 2 for i in range(3) ]
		
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
		cursormin = self.cursor.min()
		cursormax = self.cursor.max()
		for c in self.cubes:
			cubemin = c.min()
			cubemax = c.max()
			if intersects(cursormin, cursormax, cubemin, cubemax):
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
				assert not intersects(cursormin, cursormax, c.min(), c.max())
		
		self.print_sgel()
	
class Input:
	def __init__(self, world):
		self.world = world
		self.readbuf = ""
	
	def handle_gui(self, evt):
		locked = evt.state & 0x0004
		if evt.keysym == 'Left':
			self.world.input(locked, (-DELTA, 0., 0.))
		elif evt.keysym == 'Right':
			self.world.input(locked, (DELTA, 0., 0.))
		elif evt.keysym == 'Up':
			self.world.input(locked, (0., -DELTA, 0.))
		elif evt.keysym == 'Down':
			self.world.input(locked, (0., DELTA, 0.))
		else:
			return
		
	def handle_stdin(self, f, m):
		input = sys.stdin.read()
		if input == '':
			sys.exit(0)
		
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
			
if __name__ == '__main__':
	win = tk.Tk()
	canvas = tk.Canvas(win)
	canvas.pack(fill = tk.BOTH, expand = 1)
	
	cursor = Cube("cur", canvas, 20, 20, 20)
	c1 = Cube("c1", canvas, 30, 30, 30)
	c1.move([30, 0, 0])
	w = World2(cursor, [c1])
	input = Input(w)
	
	canvas.focus_set()
	# If we send output directly, then Soar can't learn over it
	#canvas.bind('<Key>', input.handle_gui)
	
	# set stdin to non-blocking read
	fd = sys.stdin.fileno()
	fl = fcntl.fcntl(fd, fcntl.F_GETFL)
	fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)
	
	win.tk.createfilehandler(sys.stdin, tk.READABLE, input.handle_stdin)
	tk.mainloop()
	
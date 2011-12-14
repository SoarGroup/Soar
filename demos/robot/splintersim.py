from __future__ import print_function
import sys, os, socket, time
sys.path.append('..')
import sock
import random, math
import numpy as num
import pymunk as munk
import Tkinter as tk

def rotate(rot, v):
	halfroll = rot[0] / 2
	halfpitch = rot[1] / 2
	halfyaw = rot[2] / 2

	sin_r2 = math.sin(halfroll)
	sin_p2 = math.sin(halfpitch)
	sin_y2 = math.sin(halfyaw)

	cos_r2 = math.cos(halfroll)
	cos_p2 = math.cos(halfpitch)
	cos_y2 = math.cos(halfyaw)

	a = cos_r2 * cos_p2 * cos_y2 + sin_r2 * sin_p2 * sin_y2
	b = sin_r2 * cos_p2 * cos_y2 - cos_r2 * sin_p2 * sin_y2
	c = cos_r2 * sin_p2 * cos_y2 + sin_r2 * cos_p2 * sin_y2
	d = cos_r2 * cos_p2 * sin_y2 - sin_r2 * sin_p2 * cos_y2
	
	t2 = a*b;
	t3 = a*c;
	t4 = a*d;
	t5 = -b*b;
	t6 = b*c;
	t7 = b*d;
	t8 = -c*b;
	t9 = c*d;
	t10 = -d*d;

	r0 = 2*((t8+t10)*v[0] + (t6-t4)*v[1]  + (t3+t7)*v[2]) + v[0];
	r1 = 2*((t4+t6)*v[0]  + (t5+t10)*v[1] + (t9-t2)*v[2]) + v[1];
	r2 = 2*((t7-t3)*v[0]  + (t2+t9)*v[1]  + (t5+t8)*v[2]) + v[2];
	
	return num.array([r0, r1, r2])

def close(a, b, tol):
	t = abs(a * tol)
	return a - t <= b <= a + t
	
class Splinter(object):
	WHEEL_DIAMETER     = 0.25
	BASELINE           = 0.35
	TORQUE_CONSTANT    = 3.0   # torque (Nm) per amp
	EMF_CONSTANT       = 2.0   # volts per rad_per_sec
	WINDING_RESISTANCE = 5.5   # ohms
	INERTIA            = 0.5   # kg*m^2
	DRAG_CONSTANT      = 1.0   # drag (Nm per rad_per_sec) ( >= 0)
	DT                 = 0.016 # need a better way to figure this out
	MASS               = 1.0
	LENGTH             = 0.5
	
	def __init__(self, space, name, initpos):
		self.space = space
		self.name = name
		self.body = munk.Body(Splinter.MASS, munk.moment_for_box(Splinter.MASS, Splinter.LENGTH, Splinter.BASELINE))
		l2 = Splinter.LENGTH / 2
		b2 = Splinter.BASELINE / 2
		self.verts = [(-l2, -b2), (-l2, b2), (l2, b2), (l2, -b2)]
		self.shape = munk.Poly(self.body, self.verts)
		self.space.add(self.body, self.shape)
		self.body.position = initpos

		self.lrps = 0.
		self.rrps = 0.
	
	def calc_rps(self, rps, volts):
		volts_emf = rps * Splinter.EMF_CONSTANT
		amps = (volts - volts_emf) / Splinter.WINDING_RESISTANCE
		torque0 = amps * Splinter.TORQUE_CONSTANT
		torque_drag = rps * Splinter.DRAG_CONSTANT
		torque_net = torque0 - torque_drag
		acceleration = torque_net / Splinter.INERTIA
		return rps + acceleration * Splinter.DT
		
	def update(self, inputs):
		if 'left' not in inputs or 'right' not in inputs:
			return
			
		lvolt = inputs['left']
		rvolt = inputs['right']
		self.lrps = self.calc_rps(self.lrps, lvolt * 12)
		self.rrps = self.calc_rps(self.rrps, rvolt * 12)
		
		dleft  = Splinter.DT * self.lrps * Splinter.WHEEL_DIAMETER;
		dright = Splinter.DT * self.rrps * Splinter.WHEEL_DIAMETER;
	
		rot = (0., 0., self.body.angle)
		self.body.velocity = munk.Vec2d(rotate(rot, num.array([(dleft + dright) / 2.0, 0.0, 0.0]))[:2])
		self.body.angular_velocity = (dright - dleft) / Splinter.BASELINE
	
	def get_sgel(self, first):
		lines = []
		ps = ' '.join(str(x) for x in self.body.position) + ' 0'
		rs = '0 0 {}'.format(self.body.angle)
		vs1 = ' '.join('{} {} 0'.format(x, y) for x, y in self.verts)
		vs2 = ' '.join('{} {} 1'.format(x, y) for x, y in self.verts)
		vs = vs1 + ' ' + vs2
		#wvs = ' '.join('{} {} 0'.format(x, y) for x, y in self.shape.get_points())
		#print('splinter: ' + ps + ' ; ' + rs + ' ; ' + wvs)
		if first:
			lines.append('a {} world v {} p {} r {}'.format(self.name, vs, ps, rs))
		else:
			lines.append('c {} p {} r {}'.format(self.name, ps, rs))
			
		lines.append('p {} left_rads_per_sec {}'.format(self.name, self.lrps))
		lines.append('p {} right_rads_per_sec {}'.format(self.name, self.rrps))
		for d in range(2):
			lines.append('p {} vel_{} {}'.format(self.name, d, self.body.velocity[d]))
		lines.append('p {} vel_2 0.0'.format(self.name))
		lines.append('p {} rotation_rate_0 0.'.format(self.name))
		lines.append('p {} rotation_rate_1 0.'.format(self.name))
		lines.append('p {} rotation_rate_2 {}'.format(self.name, self.body.angular_velocity))
		return '\n'.join(lines)
	
	def get_name(self):
		return self.name
	
	def get_pos(self):
		return self.body.position
	
	def get_points(self):
		return self.shape.get_points()
	
	def get_color(self):
		return 'blue'

class Block(object):
	MASS = 2.0
	WIDTH = 1.0
	
	def __init__(self, space, name, initpos):
		self.name = name
		self.space = space
		self.body = munk.Body(Block.MASS, munk.moment_for_box(Block.MASS, Block.WIDTH, Block.WIDTH))
		w2 = Block.WIDTH / 2
		self.verts = [(-w2, -w2), (-w2, w2), (w2, w2), (w2, -w2)]
		self.shape = munk.Poly(self.body, self.verts)
		self.space.add(self.body, self.shape)
		self.body.position = initpos
	
	def update(self, inputs):
		vx, vy = self.body.velocity
		self.body.velocity = (vx * 0.5, vy * 0.5)
		self.body.angular_velocity *= 0.5
	
	def get_sgel(self, first):
		ps = '{} {} 0'.format(*self.body.position)
		rs = '0 0 {}'.format(self.body.angle)
		vs1 = ' '.join('{} {} 0'.format(x, y) for x, y in self.verts)
		vs2 = ' '.join('{} {} 1'.format(x, y) for x, y in self.verts)
		vs = vs1 + ' ' + vs2
		#wvs = ' '.join('{} {} 0'.format(x, y) for x, y in self.shape.get_points())
		#print('block: ' + ps + ' ; ' + rs + ' ; ' + wvs)
		if first:
			return 'a {} world v {} p {} r {}'.format(self.name, vs, ps, rs)
		else:
			return 'c {} p {} r {}'.format(self.name, ps, rs)
	
	def get_name(self):
		return self.name
		
	def get_pos(self):
		return self.body.position
	
	def get_points(self):
		return self.shape.get_points()
	
	def get_color(self):
		return 'yellow'
		
class Display(object):
	SCALE = 50.0
	
	def __init__(self, root):
		self.canvas = tk.Canvas(root)
		self.canvas.bind('<Button-1>', lambda evt: self.canvas.scan_mark(evt.x, evt.y))
		self.canvas.bind('<B1-Motion>', lambda evt: self.canvas.scan_dragto(evt.x, evt.y))
		self.canvas.create_oval((0,0,5,5))
		self.canvas.pack(fill=tk.BOTH, expand=True)
		self.objs = {}  # name -> (display polygon, prev position)
	
	def update_obj(self, name, pos, verts, color):
		p = (pos[0] * Display.SCALE, pos[1] * Display.SCALE)
		if name in self.objs:
			poly, oldpos = self.objs[name]
			self.canvas.delete(poly)
			self.canvas.create_line(oldpos, p, fill=color)
		
		sverts = [ (x * Display.SCALE, y * Display.SCALE) for x, y in verts ]
		poly = self.canvas.create_polygon(sverts, fill='', outline=color)
		self.objs[name] = (poly, p)

class World(object):
	def __init__(self, space, display, sockfile):
		self.space = space
		self.objects = []
		self.inputs = {}
		self.display = display
		self.sock = sock.Sock()
		self.sock.connect(sockfile)
		self.display.canvas.tk.createfilehandler(self.sock.sock, tk.READABLE, self.read)
		self.first = True

	def update(self):
		for obj in self.objects:
			obj.update(self.inputs)
		
		self.space.step(1.0)
		
		msg = []
		for obj in self.objects:
			self.display.update_obj(obj.get_name(), obj.get_pos(), obj.get_points(), obj.get_color())
			msg.append(obj.get_sgel(self.first))
			
		self.sock.send('\n'.join(msg))
		self.first = False
		
	def read(self, dummy, dummy2):
		msg = self.sock.receive()
		if msg == None:
			sys.exit(0)
		
		#print('received\n-----\n{}\n-----'.format(msg))
		self.inputs = {}
		for line in msg.split('\n'):
			if len(line) == 0:
				continue
				
			f = line.split()
			if len(f) != 2:
				print('Ignoring input "{}"'.format(line), file=sys.stderr)
			else:
				self.inputs[f[0]] = float(f[1])
		
		self.update()
	
	def add_object(self, obj):
		self.objects.append(obj)
			
if __name__ == '__main__':
	munk.init_pymunk()
	disp = Display(tk.Tk())
	space = munk.Space(iterations = 100, elastic_iterations = 100)
	world = World(space, disp, 'env')
	world.add_object(Splinter(space, 'splinter', (0., 0.)))
	world.add_object(Block(space, 'block', (1., 1.)))
	world.update()
	tk.mainloop()

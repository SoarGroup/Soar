#!/usr/bin/env python

from __future__ import print_function
import os, sys
import Tkinter as tk
from sock import Sock

IPC=os.path.join(os.environ.get('SVSNAMESPACE', ''), 'disp')
AXIS_LEN=100
AXIS_NAMES =  { 0 : 'x',   1 : 'y',     2 : 'z' }
AXIS_COLORS = { 0 : 'red', 1 : 'green', 2 : 'blue' }
SCALE = 50.0
POINTSIZE = 2

class Object3D(object):
	def __init__(self, name, scene, ptlist):
		self.name = name
		self.scene = scene
		self.canvas = scene.canvas
		self.ptlist = ptlist
	
	def update(self, ptlist):
		self.ptlist = ptlist
		
	def draw(self, axes):
		self.canvas.delete(self.name)
		if len(self.ptlist) == 1:
			self.draw_point(axes)
		elif len(self.ptlist) == 2:
			self.draw_line(axes)
		else:
			self.draw_poly(axes)
	
	def draw_point(self, axes):
		c1, c2 = self.ptlist[0][axes[0]] * SCALE, self.ptlist[0][axes[1]] * SCALE
		bbox = (c1 - POINTSIZE, c2 - POINTSIZE, c1 + POINTSIZE, c2 + POINTSIZE)
		self.canvas.create_oval(bbox, tags=self.name)
		self.canvas.create_text((c1, c2), anchor=tk.NW, text=self.name, tags=self.name)
	
	def draw_line(self, axes):
		a1, a2 = self.ptlist[0][axes[0]] * SCALE, self.ptlist[0][axes[1]] * SCALE
		b1, b2 = self.ptlist[1][axes[0]] * SCALE, self.ptlist[1][axes[1]] * SCALE
		self.canvas.create_line(a1, a2, b1, b2, tags=self.name)
		self.canvas.create_text((a1, a2), anchor=tk.NW, text=self.name, tags=self.name)
		
	def draw_bbox(self, axes):
		c1 = [ x[axes[0]] for x in self.ptlist ]
		c2 = [ x[axes[1]] for x in self.ptlist ]
		min1 = min(c1) * SCALE; min2 = min(c2) * SCALE
		max1 = max(c1) * SCALE; max2 = max(c2) * SCALE
		self.canvas.create_rectangle((min1,min2,max1,max2), tags=self.name)
		self.canvas.create_text((min1,min2), anchor=tk.NW, text=self.name, tags=self.name)
	
	def draw_poly(self, axes):
		pts = [ (p[axes[0]] * SCALE, p[axes[1]] * SCALE) for p in self.ptlist ]
		self.canvas.create_polygon(pts, tags=self.name, fill="", outline="black")	
		self.canvas.create_text(pts[0], anchor=tk.NW, text=self.name, tags=self.name)
		
class Scene(object):
	def __init__(self, name, root):
		self.name = name
		self.root = root
		self.curraxis = (0,1)
		self.offset = (0, 0)
		self.canvas = tk.Canvas(self.root, background='white')
		self.canvas.create_text((0,0), anchor=tk.NW, text=self.name)
		self.objects = {}
		self.menu = tk.Menu(self.canvas, tearoff=0)
		
		for n, a in [('xy',(0,1)), ('xz',(0,2)), ('yz',(1,2))]:
			self.menu.add_command(label=n, command=lambda a=a: self.draw(a))
		
		self.canvas.bind('<Button-3>', lambda e: self.menu.post(e.x_root, e.y_root))
		self.canvas.bind('<ButtonPress-1>', lambda e: self.startdrag(e.x, e.y))
		self.canvas.bind('<B1-Motion>', lambda e: self.dragto(e.x, e.y))
		self.canvas.bind('<ButtonRelease-1>', lambda e: self.enddrag(e.x, e.y))
		self.draw(self.curraxis)
	
	def startdrag(self, x, y):
		self.canvas.scan_mark(x, y)
		self.anchor = (x, y)

	def dragto(self, x, y):
		self.canvas.scan_dragto(x, y, 1)
	
	def enddrag(self, x, y):
		self.offset = (self.offset[0] + x - self.anchor[0], self.offset[1] + y - self.anchor[1])
		
	def update_object(self, name, ptlist):
		o = self.objects.get(name, None)
		if o == None:
			o = Object3D(name, self, ptlist)
			self.objects[name] = o
		else:
			o.update(ptlist)
			
		o.draw(self.curraxis)
	
	def delete_object(self, name):
		self.canvas.delete(name)
		self.objects.pop(name)
	
	def draw(self, axes):
		self.canvas.delete('axis')
		n1 = AXIS_NAMES[axes[0]]
		c1 = AXIS_COLORS[axes[0]]
		n2 = AXIS_NAMES[axes[1]]
		c2 = AXIS_COLORS[axes[1]]
		self.canvas.create_text((AXIS_LEN, 0), fill=c1, anchor=tk.W, text=n1, tags='axis')
		self.canvas.create_line((0,0,AXIS_LEN,0), fill=c1, tags='axis')
		self.canvas.create_text((0,AXIS_LEN), fill=c2, anchor=tk.N, text=n2, tags='axis')
		self.canvas.create_line((0,0,0,AXIS_LEN), fill=c2, tags='axis')
		self.curraxis = axes
		
		for o in self.objects.values():
			o.draw(axes)
	
class Display(object):
	def __init__(self, root):
		self.root = root
		self.scenes = {}
		self.scenestack = []
		self.currscene = None
		self.menu = tk.Menu(self.root, tearoff=0)
		self.root.bind('<Button-2>', self.display_menu)
	
	def display_menu(self, evt):
		self.menu.post(evt.x_root, evt.y_root)
		
	def add_scene(self, name):
		s = Scene(name, self.root)
		self.scenes[name] = s
		self.scenestack.append(name)
		self.menu.add_command(label=name, command=lambda n=name: self.switch_scene(n))
		if self.currscene != None:
			s.offset = self.currscene.offset
			s.canvas.scan_mark(0, 0)
			s.canvas.scan_dragto(s.offset[0], s.offset[1], 1)
		self.switch_scene(name)
		return s
	
	def delete_scene(self, name):
		for i in range(self.menu.index(tk.END)+1):
			if self.menu.entrycget(i, 'label') == name:
				self.menu.delete(i)
				break
		s = self.scenes.pop(name)
		self.scenestack.pop(i)
		if self.currscene == s:
			self.switch_scene(self.scenestack[i-1])
		
	def switch_scene(self, name):
		if self.currscene:
			self.currscene.canvas.pack_forget()
		self.currscene = self.scenes[name]
		self.currscene.canvas.pack(expand=True, fill=tk.BOTH)
	
	def get_scene(self, name):
		return self.scenes[name]
	
class Connection(object):
	def __init__(self, path_or_host, port=None):
		self.root = tk.Tk()
		self.display = Display(self.root)
		self.sock = Sock()
		self.sock.connect(path_or_host, port)
		self.root.tk.createfilehandler(self.sock.sock, tk.READABLE, self.dispatch)
	
	def do_newscene(self, msg):
		self.display.add_scene(msg.strip())
	
	def do_delscene(self, msg):
		self.display.delete_scene(msg.strip())
	
	def do_updateobject(self, msg):
		lines = list(filter(None, msg.split('\n')))
		scene = lines[0]
		objname = lines[1]
		pts = [ tuple(float(f) for f in l.split()) for l in lines[2:] ]
		self.display.get_scene(scene).update_object(objname, pts)
	
	def do_delobject(self, msg):
		lines = list(filter(None, msg.split('\n')))
		scene = lines[0]
		objname = lines[1]
		self.display.get_scene(scene).delete_object(objname)
	
	def dispatch(self, sock, mask):
		while True:
			msg = self.sock.receive()
			header, _, rest = msg.partition('\n')
			if header == None:
				sys.exit(0)
			handler = getattr(self, 'do_' + header, None)
			if handler == None:
				print('unhandled message', header)
			else:
				handler(rest)
			
			if not self.sock.has_buffered():
				break;
			
	def run(self):
		tk.mainloop()

if __name__ == '__main__':
	if len(sys.argv) > 1 and sys.argv[1] == '-r':
		c = Connection('wyrm', 8888)
	else:
		c = Connection(IPC)
	c.run()

from __future__ import print_function
import os, sys, fcntl
import Tkinter as tk

SCALE = 50.0

def handle_line(line):
	global objects
	global currobj
	global canvas
	
	fields = line.split()
	if len(fields) == 1:
		if fields[0] == "end":
			if currobj == "":
				return
			canvas.delete(currobj)
			canvas.create_polygon(objects[currobj], tags = currobj, fill = "", outline="red")
			canvas.update_idletasks()
			currobj = ""
		else:
			currobj = fields[0]
			objects[currobj] = []
			
	elif len(fields) == 2:
		if currobj == "":
			return
		try:
			x = float(fields[0]) * SCALE
			y = float(fields[1]) * SCALE
		except ValueError:
			return
		
		objects[currobj].append((x, y))

def stdin_handler(f, m):
	global buf
	
	input = sys.stdin.read()
	if len(input) == 0:
		sys.exit(0)
	
	buf += input
	while '\n' in buf:
		line, _, buf = buf.partition('\n')
		handle_line(line)

buf = ""
objects = {}
currobj = ""
root = tk.Tk()
canvas = tk.Canvas(root)
canvas.bind('<Button-1>', lambda evt: canvas.scan_mark(evt.x, evt.y))
canvas.bind('<B1-Motion>', lambda evt: canvas.scan_dragto(evt.x, evt.y))
canvas.create_oval((0,0,5,5))
canvas.pack(fill=tk.BOTH, expand=True)

fcntl.fcntl(0, fcntl.F_SETFL, os.O_NONBLOCK)
root.tk.createfilehandler(sys.stdin, tk.READABLE, stdin_handler)
tk.mainloop()

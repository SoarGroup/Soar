from __future__ import print_function
import sys, os, time
import fcntl
import struct
import Tkinter as tk

DELAY = 50

class remote:
	def __init__(self, path):
		self.win = tk.Tk()
		self.var = tk.StringVar()
		self.var.set('stop')
		for d in 'left right forward backward stop'.split():
			btn = tk.Radiobutton(self.win, text = d, variable = self.var, value = d, command=self.press)
			btn.dir = d
			btn.pack(anchor = 'w')

		self.file = open(path, 'wb')
		self.currcmd = (0.0, 0.0)
		self.press()

	def press(self):
		self.currcmd = {
			'stop'     : (   0,    0),
			'left'     : (-0.5,  0.5),
			'right'    : ( 0.5, -0.5),
			'forward'  : ( 1.0,  1.0),
			'backward' : (-1.0, -1.0)
		}.get(self.var.get(), (0.0, 0.0))
		self.write_cmd()
	
	def write_cmd(self):
		fcntl.flock(self.file.fileno(), fcntl.LOCK_EX)
		self.file.seek(0, 0)
		self.file.write(struct.pack('dd', *self.currcmd))
		self.file.flush()
		fcntl.flock(self.file.fileno(), fcntl.LOCK_UN)

if len(sys.argv[1]) == 0:
	print('usage: {} <file>'.format(sys.argv[0]))
	sys.exit(1)

r = remote(sys.argv[1])
tk.mainloop()

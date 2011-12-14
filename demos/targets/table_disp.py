from __future__ import print_function
import sys, os
import fcntl
import Tkinter as tk


class KVTable:
	def __init__(self, parent):
		self.panes = tk.PanedWindow(parent, orient = tk.HORIZONTAL)
		self.panes.pack(fill = tk.BOTH, expand = 1)
		
		self.keyframe = tk.Frame(self.panes, relief = tk.FLAT)
		self.valframe = tk.Frame(self.panes, relief = tk.FLAT)
		self.panes.add(self.keyframe)
		self.panes.add(self.valframe)
		
		self.keys = []
		self.vals = []
		self.key_label = {}
		self.readbuf = ''
	
	def add_item(self, key, val):
		keylabel = tk.Label(self.keyframe, text = key, relief = tk.FLAT)
		vallabel = tk.Label(self.valframe, text = val, relief = tk.FLAT)
		
	def update(self, key, val):
		if key not in self.key_inds:
			self.keylist.insert(tk.END, key)
			self.vallist.insert(tk.END, val)
			ind = self.keylist.size() - 1
			self.key_inds[key] = ind
			if ind % 2 == 0:
				self.keylist.itemconfig(ind, bg = 'lightgray')
				self.vallist.itemconfig(ind, bg = 'lightgray')
		else:
			self.vallist.itemconfig(self.key_inds[key], text=val)

	def handle_stdin(self, f, m):
		input = sys.stdin.read()
		if input == '':
			sys.exit(0)
		
		self.readbuf += input
		line, sep, rest = self.readbuf.partition('\n')
		if sep == '':
			self.readbuf = line
		else:
			self.readbuf = rest
			fields = line.split()
			if len(fields) == 2:
				self.update(fields[0], fields[1])

if __name__ == '__main__':
	# set stdin to non-blocking read
	fd = sys.stdin.fileno()
	fl = fcntl.fcntl(fd, fcntl.F_GETFL)
	fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)
	
	win = tk.Tk()
	tbl = KVTable(win)
	
	win.tk.createfilehandler(sys.stdin, tk.READABLE, tbl.handle_stdin)
	tk.mainloop()

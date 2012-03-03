#!/usr/bin/python

import sys, os, fcntl
import tkinter as tk
import tkinter.ttk as ttk

# single quotes (') delimit fields. Literal single quotes can be represented by ''
def split_with_quotes(line):
	fields = []
	while True:
		line = line.strip()
		if len(line) == 0:
			break
		
		if line[0] == "'":
			i = 0
			while True:
				i = line.find("'", i + 1)
				if i == -1 or line[i-1] != "\\":
					break
			
			if i == -1:
				fields.append(line[1:])
				break
			else:
				fields.append(line[1:i])
				line = line[i+1:]
		else:
			x = line.split(None, 1)
			fields.append(x[0])
			if len(x) == 2:
				line = x[1]
			else:
				line = ""
	
	return [ f.replace("\\'", "'") for f in fields ]

class DataTree:
	def __init__(self):
		self.context = ()
		self.readbuf = ''
		self.hist = {}
		self.make_tree()
		self.make_menu()
	
	def make_tree(self):
		self.win = tk.Tk()
		self.win.selection_handle(self.handle_selection)
		self.tree = ttk.Treeview(self.win, columns = ('val', 'prev'))
		self.tree.pack(expand = 1, fill = tk.BOTH)
		self.tree.bind('<Button-3>', self.show_menu)
		
	def make_menu(self):
		self.menu = tk.Menu(self.tree, tearoff = 0)
		self.menu.add_command(label = 'copy current', command = self.copy_curr)
		self.menu.add_command(label = 'copy history', command = self.copy_hist)
		
	def read(self, f, mask):
		try:
			input = sys.stdin.read()
		except IOError:
			return
		
		if input == '':
			sys.exit(0)
		
		self.readbuf += input
		
		while '\n' in self.readbuf:
			line, sep, rest = self.readbuf.partition('\n')
			self.readbuf = rest
			self.parse_line(line)
	
	def parse_line(self, line):
		fields = split_with_quotes(line)
		if len(fields) == 0:
			return
		
		if fields[0] == 'BEGIN':
			if len(fields) < 2:
				print('syntax error: {}'.format(line), file=sys.stderr)
				return
			self.context += tuple(fields[1:])
			self.proc_item(self.context, None)
		elif fields[0] == 'END':
			self.context = self.context[:-1]
		elif fields[0] == 'ENDALL':
			self.context = ()
		elif fields[0] == 'CLEAR':
			# clear the entire tree
			self.tree.set_children('')
		else:
			if len(fields) == 1:
				self.proc_item(self.context + tuple(fields), None)
			elif len(fields) > 1:
				self.proc_item(self.context + tuple(fields[:-1]), fields[-1])
	
	def proc_item(self, key, val):
		name = '.'.join(key)
		if not self.tree.exists(name):
			if len(key) > 1:
				self.proc_item(key[:-1], None)
				parent = '.'.join(key[:-1])
			else:
				parent = ''
			
			assert(self.tree.exists(parent))
			self.tree.insert(parent, 'end', name, text = key[-1])
		
		if val == None:
			return
		
		assert(self.tree.exists(name))
		
		h = self.hist.setdefault(name, [])
		if len(h) == 0:
			prev = ''
		else:
			prev = h[-1]
		
		if not val.startswith('%'):
			self.tree.item(name, values = (val, prev))
			h.append(val)
		else:
			cmd = val[1:]
			if cmd.startswith('+') or cmd.startswith('-'):
				# inc/dec
				if isinstance(prev, str):
					if len(prev) == 0:
						prev = 0.0
					else:
						prev = float(prev)
					
				newval = prev + float(cmd)
				self.tree.item(name, values = (newval, prev))
				h.append(newval)
			elif cmd == 'CLEAR':
				self.tree.set_children(name)
			elif cmd == 'DELETE':
				self.tree.delete(name)
	
	def show_menu(self, e):
		self.menu_target = self.tree.identify_row(e.y)
		self.menu.tk_popup(e.x_root, e.y_root, 0)
	
	def copy_curr(self):
		self.selbuf = self.hist.get(self.menu_target, [''])[-1]
		self.win.clipboard_clear()
		self.win.clipboard_append(self.selbuf, type = 'STRING')
		self.win.selection_own()

	def copy_hist(self):
		self.selbuf = '\n'.join(self.hist.get(self.menu_target, []))
		self.win.clipboard_clear()
		self.win.clipboard_append(self.selbuf, type = 'STRING')
		self.win.selection_own()
	
	def handle_selection(self, offset, length):
		o = int(offset)
		l = int(length)
		return self.selbuf[o:o + l]
		
if __name__ == '__main__':
	# set stdin to non-blocking read
	fd = sys.stdin.fileno()
	fl = fcntl.fcntl(fd, fcntl.F_GETFL)
	fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)
	
	t = DataTree()
	t.win.tk.createfilehandler(sys.stdin, tk.READABLE, t.read)
	tk.mainloop()
	
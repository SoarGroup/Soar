#!/usr/bin/env python

from __future__ import print_function
import sys, os, fcntl
import wx
from wx.gizmos import TreeListCtrl

STYLE = wx.TR_HIDE_ROOT | wx.TR_ROW_LINES | wx.TR_DEFAULT_STYLE

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
	
class MainFrame(wx.Frame):
	def __init__(self):
		wx.Frame.__init__(self, None, title='Data Tree', size=(200,100))
		self.readbuf = ""
		self.tree = TreeListCtrl(self, style = STYLE)
		self.tree.AddColumn("key")
		self.tree.AddColumn("val")
		self.tree.AddColumn("prev")
		self.root = self.tree.AddRoot('')
		self.items = { () : self.root }
		self.context = []
		
		self.input_timer = wx.Timer(self, 1)
		self.input_timer.Start(100)
		wx.EVT_TIMER(self, 1, self.get_input)
		
		wx.EVT_CLOSE(self, self.on_close)
		
		self.Show(True)
		
	def proc_item(self, key, val):
		if key in self.items:
			item = self.items[key]
		else:
			p = self.proc_item(key[:-1], None)
			item = self.tree.AppendItem(p, key[-1])
			self.items[key] = item
		
		if val == 'CLEAR':
			self.tree.DeleteChildren(item)
			for k in self.items.keys():
				if k != key and k[:len(key)] == key:
					self.items.pop(k)
		elif val == 'DELETE':
			self.tree.Delete(item)
			for k in self.items.keys():
				if k[:len(key)] == key:
					self.items.pop(k)
		elif val != None:
			self.tree.SetItemText(item, self.tree.GetItemText(item, 1), 2)
			self.tree.SetItemText(item, val, 1)

		return item
	
	def get_input(self, evt):
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
				print('syntax error: {}'.format(line))
				return
			self.context.extend(fields[1:])
			if tuple(self.context) not in self.items:
				self.proc_item(tuple(self.context), None)
		elif fields[0] == 'END':
			self.context = self.context[:-1]
		elif fields[0] == 'ENDALL':
			self.context.clear()
		elif fields[0] == 'CLEAR':
			self.tree.DeleteChildren(self.root)
			self.items.clear()
			self.items[()] = self.root
			self.context = []
		else:
			if len(fields) == 1:
				self.proc_item(tuple(self.context + fields), None)
			elif len(fields) > 1:
				self.proc_item(tuple(self.context + fields[:-1]), fields[-1])
		
	def on_close(self, evt):
		self.input_timer.Stop()
		self.Destroy()
	
if __name__ == '__main__':
	# set stdin to non-blocking read
	fd = sys.stdin.fileno()
	fl = fcntl.fcntl(fd, fcntl.F_GETFL)
	fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)
	
	app = wx.App(False)
	frame = MainFrame()
	app.MainLoop()

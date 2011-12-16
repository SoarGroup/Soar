#!/usr/bin/env python3

# Originally adapted from
# http://blog.rburchell.com/2010/02/pyside-tutorial-model-view-programming.html

import sys, os
from PyQt4 import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *

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

class StdinThread(QThread):
	line_read = pyqtSignal(str, name = 'line_read')
	
	def __init__(self, model):
		QThread.__init__(self)
		self.model = model
		self.line_read.connect(self.model.parse_line)
	
	def run(self):
		for line in sys.stdin:
			self.line_read.emit(line)
		
		sys.exit(0)
		
class TreeItem:
	def __init__(self, parent, name, val):
		self.parent = parent
		self.name = name
		self.val = val
		self.history = []
		
		if self.parent == None:
			self.path = '/'
		else:
			self.path = self.parent.path + '/' + self.name
		
		self.children = []
		self.data = [name, val]
	
	def get_child(self, i):
		return self.children[i]
	
	def remove_child(self, i):
		self.children.pop(i)
		
	def get_row(self):
		if self.parent == None:
			return 0
		return self.parent.children.index(self)
	
	def get_data(self, i):
		if i == 0:
			return self.name
		elif i == 1:
			return self.val
		elif i == 2:
			if len(self.history) > 0:
				return self.history[-1]
			else:
				return None

	def set_value(self, val):
		self.history.append(self.val)
		self.val = val
	
	def add_child(self, name):
		i = TreeItem(self, name, None)
		self.children.append(i)
		return i
	
	def num_children(self):
		return len(self.children)
		
	def clear(self):
		del self.children[:]
	
	def get_parent(self):
		return self.parent
	
	def increment(self, amount):
		self.history.append(self.val)
		if self.val == None:
			self.val = amount
		else:
			if type(self.val) != float:
				try:
					self.val = float(self.val)
				except ValueError:
					self.val = 0.0
			
			self.val += amount
	
class TreeModel(QAbstractItemModel):

	def __init__(self):
		QAbstractItemModel.__init__(self)
		
		self.root = TreeItem(None, 'root', 'root')
		self.context = []
		self.items = {}
		self.items[()] = self.root
		self.readbuf = ''
	
	def proc_item(self, key, val):
		if key in self.items:
			item = self.items[key]
		else:
			pitem = self.proc_item(key[:-1], None)
			if pitem is self.root:
				pind = QModelIndex()
			else:
				pind = self.createIndex(pitem.get_row(), 0, pitem)
			
			row = pitem.num_children()
			self.beginInsertRows(pind, row, row)
			item = pitem.add_child(key[-1])
			self.endInsertRows()
			self.items[key] = item
		
		if val == None:
			return item
		
		assert(item is not self.root)
		itemrow = item.get_row()
		ind = self.createIndex(itemrow, 0, item)
		indend = self.createIndex(itemrow, 2, item)
		assert(ind.isValid())
		
		if not val.startswith('%'):
			item.set_value(val)
			self.dataChanged.emit(ind, indend)
		else:
			cmd = val[1:]
			if cmd.startswith('+') or cmd.startswith('-'):
				item.increment(float(cmd))
				self.dataChanged.emit(ind, indend)
			elif cmd == 'CLEAR':
				self.beginRemoveRows(ind, 0, self.rowCount(ind))
				item.clear()
				for k in self.items.keys():
					if k != key and k[:len(key)] == key:
						self.items.pop(k)
				self.endRemoveRows()
			elif cmd == 'DELETE':
				pitem = item.get_parent()
				pind = self.createIndex(itemrow, 0, pitem)
				self.beginRemoveRows(pind, itemrow, itemrow)
				pitem.remove_child(item.get_row())
				self.endRemoveRows()
				for k in self.items.keys():
					if k[:len(key)] == key:
						self.items.pop(k)
	
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
			del self.context[:]
		elif fields[0] == 'CLEAR':
			# clear the entire tree
			self.beginRemoveRows(QModelIndex(), 0, self.root.num_children())
			self.root.clear()
			self.items.clear()
			self.items[()] = self.root
			self.context = []
			self.endRemoveRows()
		else:
			if len(fields) == 1:
				self.proc_item(tuple(self.context + fields), None)
			elif len(fields) > 1:
				self.proc_item(tuple(self.context + fields[:-1]), fields[-1])
	
	def rowCount(self, parent = QModelIndex()):
		if parent.column() > 0:
			return 0;
		
		if parent.isValid():
			return parent.internalPointer().num_children()
		
		return self.root.num_children()
	
	def columnCount(self, index):
		return 3
	
	def data(self, index, role):
		if not index.isValid():
			return None
		
		if role == Qt.DisplayRole:
			return index.internalPointer().get_data(index.column())
		
		return None

	def index(self, row, column, parent):
		if not self.hasIndex(row, column, parent):
			return QModelIndex()
		
		if not parent.isValid():
			i = self.root
		else:
			i = parent.internalPointer()
		
		if row < i.num_children():
			return self.createIndex(row, column, i.get_child(row))
		
		return QModelIndex()
	
	def parent(self, index):
		if not index.isValid():
			return QModelIndex()
		
		p = index.internalPointer().get_parent()
		
		if p is self.root:
			return QModelIndex()
		
		return self.createIndex(p.get_row(), 0, p)
	
# set things up, and run it. :)
if __name__ == '__main__':
	app = QApplication(sys.argv)
	model = TreeModel()
	tree = QTreeView()
	
	tree.setStyleSheet('''
		QTreeView::item {
			border-right: 1px solid lightgray;
			border-bottom: 1px solid lightgray;
		}
		QTreeView::item:selected {
			background: gray;
		}
	''')
	
	tree.setModel(model)
	tree.show()
	
	thread = StdinThread(model)
	thread.start()
	app.exec_()

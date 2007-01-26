#!/usr/bin/env python

import sys

def Combine(children):
	result = ElementGGP()
	result._ElementGGP__children = children
	return result

class ElementGGPIterator:
	__children = None
	__index = None
	
	def __init__(self, children):
		self.__children = children
		self.__index = -1
		
	def __iter__(self):
		return self
	
	def next(self):
		self.__index += 1
		if self.__index >= len(self.__children):
			raise StopIteration
		return self.__children[self.__index]

class ElementGGP:
	"""Used to represent the lisp-like code of GGP."""

	__children = None

	def __init__(self, string = None):
		"""Create using an optional string of code. If no string
		is given, an empty, valid element "()" is created. Raises
		ValueError if there is a problem with the passed string."""
		
		self.__children = []

		if string == None:
			return

		depth = 0
		start = -1
		current = None
		for x in range(len(string)):
			if depth == 0:
				# At the top level
				if string[x] == '(':
					# Start of top level element
					depth += 1
				else:
					# Paren must start top level element
					raise ValueError("No opening paren: %s" % string)
			elif depth == 1:
				# Inside the top level
				if string[x] == '(':
					# Start of sub element
					start = x
					depth += 1

				elif string[x] == ')':
					if current != None:
						# Currently parsing a string, ends the string
						self.__children.append(current)
						current = None
						
					# End of top level element
					if not x == (len(string) - 1):
						raise ValueError("Extra characters at end: %s" % string)

				elif string[x] == ' ':
					# Space inside top level
					if current != None:
						# Currently parsing a string, ends the string
						self.__children.append(current)
						current = None
						
				else:
					# Some other character inside top level
					if current != None:
						# Currently parsing a string, append it
						current += string[x]
					else:
						# Not currently parsing a string, start one
						current = string[x]

			else:
				# Inside sub element, keep track of parens
				if string[x] == '(':
					depth += 1
				elif string[x] == ')':
					depth -= 1
					if depth == 1:
						# End of sub element, append it
						self.__children.append(ElementGGP(string[start:x+1]))
						start = -1

	def __check_key(self, key):
		if not isinstance(key, int):
			raise TypeError("key must be integer")
			
		if key < 0:
			return key % len(self.__children)
			
		if key >= len(self.__children):
			raise IndexError
		
		return key
		
	def __getitem__(self, key):
		key = self.__check_key(key)
		return self.__children[key]

	def __setitem__(self, key, value):
		key = self.__check_key(key)

		if (not isinstance(value, str)) and (not isinstance(value, ElementGGP)):
			raise TypeError("value must be string or ElementGGP")
			
		self.__children[key] = value
	
	def remove(self, key):
		self.__delitem__(key)

	def __delitem__(self, key):
		key = self.__check_key(key)
		self.__children[key:key+1] = []
	
	def __str__(self):
		if len(self.__children) < 1:
			return "()"
		
		str = "("
		for element in self.__children:
			if isinstance(element, ElementGGP):
				str = str + element.__str__() + " "
			else:
				str = str + element + " "
		
		str = str[:-1]
		str = str + ")"
		return str
	
	def __len__(self):
		return len(self.__children)

	def __iter__(self):
		return ElementGGPIterator(self.__children)

	def __eq__(self, other):
		if not isinstance(other, ElementGGP):
			return False

		if len(self.__children) != len(other.__children):
			return False

		for i in range(len(self.__children)):
			if isinstance(self.__children[i], str):
				# because rules are case-insensitive
				c1 = self.__children[i].lower()
			else:
				c1 = self.__children[i]

			if isinstance(other.__children[i], str):
				# because rules are case-insensitive
				c2 = other.__children[i].lower()
			else:
				c2 = other.__children[i]

			if c1 != c2:
				return False

		return True

	def __ne__(self, other):
		return not (self == other)

	def __hash__(self):
		if isinstance(self, str):
			return self.__hash__()
		else:
			h = 0
			for c in self.__children:
				h ^= c.__hash__()
			return h
	
	# not commutative
	def __add__(a, b):
		result = a.deep_copy()
		result.__children.append(b)
		return result

	def __iadd__(self, other):
		self.__children.append(other)

	def __contains__(self, other):
		return other in self.__children

	def get_vars(self):
		vars = set()
		for c in self.__children:
			if isinstance(c, str):
				if c[0] == '?':
					vars.add(c)
			else:
				# recurse
				vars = vars.union(c.get_vars())
		return vars

	def deep_copy(self):
		copy = ElementGGP()
		for c in self.__children:
			if isinstance(c, str):
				copy.__children.append(c)
			else:
				copy.__children.append(c.deep_copy())
		return copy

	def or_child(self):
		for c in self.__children:
			if isinstance(c, ElementGGP):
				ret = c.or_child()
				if ret != None:
					return ret
			elif c.lower() == "or":
				return self
		return None
	
	def max_depth(self):
		cdepths = [c.max_depth() for c in self.__children if isinstance(c, ElementGGP)]
		if len(cdepths) == 0:
			return 1
		else:
			return sorted(cdepths).pop()


if __name__ == '__main__':
	tests = [None, '()', 
		'(A)', '(B C)',
		'((D) E)', '(F (G))',
		'(H I J)','((K) L M)','(N (O) P)','(Q R (S))',
		'((T U) V)', '(W (X Y))', '((Z) A (B))',
		'((C D E))', '(F (G H (I J K)))',]

	print "Constructor tests"
	for test in tests:
		print "Test:\t\t%s" % test
		element = ElementGGP(test)
		print "Result:\t\t%s" % element
		if len(element) > 0:
			print "e[0]:\t\t%s" % element[0]
			if len(element[0]) > 0:
				print "e[0][0]:\t%s" % element[0][0]
	
	print "Iterator test for: (a (b c) d (e f g) (h) i)"
	element = ElementGGP("(a (b c) d (e f g) (h) i)")
	for item in element:
		print "\t%s" % item
	
	print "Index test for: (a (b c) d (e f g) (h) i)"
	for x in range(len(element)):
		print "\t%s" % element[x]
	
	print "Setting all indexes to '(z a)'"
	for x in range(len(element)):
		element[x] = ElementGGP("(z a)")
		print "\t%s" % element[x]
	
	print element
	
	print "Tests complete."

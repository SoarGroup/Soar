class EquivalenceClass:

	def __init__(self):
		# maps members to the class (index) they're in
		self.__mem2class = {}
		# maps class indexes to sets of members
		self.__class2mem = {}

		self.__count = 0
	
	def make_equivalent(self, m1, m2):
		if m1 not in self.__mem2class and m2 not in self.__mem2class:
			self.__mem2class[m1] = self.__count
			self.__mem2class[m2] = self.__count
			self.__class2mem[self.__count] = set([m1, m2])
			self.__count += 1

		elif m1 in self.__mem2class and m2 in self.__mem2class:
			# both equivalence classes exist, so they have to be merged into one
			mc = self.__count
			self.__count += 1

			c1 = self.__mem2class[m1]
			c2 = self.__mem2class[m2]
			if c1 == c2:
				# nothing to do
				return
			self.__class2mem[mc] = self.__class2mem[c1] | self.__class2mem[c2]
			del self.__class2mem[c1]
			del self.__class2mem[c2]
			for m in self.__class2mem[mc]:
				self.__mem2class[m] = mc

		elif m1 in self.__mem2class:
			c = self.__mem2class[m1]
			self.__class2mem[c].add(m2)
			self.__mem2class[m2] = c
		else:
			c = self.__mem2class[m2]
			self.__class2mem[c].add(m1)
			self.__mem2class[m1] = c

	def has_member(self, m):
		return m in self.__mem2class

	def get_classes(self):
		return self.__class2mem.keys()

	def get_class_index(self, m):
		return self.__mem2class[m]

	def get_classes_as_sets(self):
		return self.__class2mem.values()

	def get_all_members(self):
		return self.__mem3class.keys()

	def __str__(self):
		return str(self.__mem2class)

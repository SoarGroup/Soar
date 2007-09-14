import ggp_utils
import pdb

class BestList:
	def __init__(self, proj, l=[]):
		self.__list = l
		self.__proj = proj
		self.__dirty = True

	def __undirty(self):
		if self.__dirty:
			self.__best_pos = ggp_utils.find_max(self.__list, self.__proj)[0]
			self.__best = self.__list[self.__best_pos]
			self.__dirty = False

	def get_best_pos(self):
		self.__undirty()
		return self.__best_pos

	def get_best(self):
		self.__undirty()
		assert self.__best == self.__list[self.__best_pos]
		return self.__best
	
	def pop_best(self):
		self.__undirty()
		del self.__list[self.__best_pos]
		self.__dirty = True
		return self.__best

	def __getitem__(self, i):
		return self.__list[i]

	def __setitem__(self, i, v):
		self.__list[i] = v
		if i == self.__best_pos:
			self.__dirty = True
		elif not self.__dirty and self.__proj(v) > self.__proj(self.__best):
			self.__best_pos = i
			self.__best = v
	
	def __len__(self):
		return len(self.__list)

	def __delitem__(self, i):
		if not self.__dirty:
			if i < self.__best_pos:
				self.__best_pos -= 1
				self.__best = self.__list[self.__best_pos]
			elif i == self.__best_pos:
				self.__dirty = True

		del self.__list[i]
	
	def copy(self):
		c = BestList(self.__proj, self.__list[:])
		if not self.__dirty:
			c.__dirty = False
			c.__best, c.__best_pos = self.__best, self.__best_pos
		return c

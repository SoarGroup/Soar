from cache_hash import CacheHash

class PositionIndex(CacheHash):
	def __init__(self, index):
		self.__index = tuple(index)
		CacheHash.__init__(self)
	
	def __str__(self):
		return str(self.__index)

	def __eq__(self, other):
		if not isinstance(other, PositionIndex): return False
		return self.__index == other.__index
	
	def __ne__(self, other):
		return not self == other

	def __cmp__(self, other):
		return cmp(self.__index, other.__index)

	def get_hash(self):
		return hash(self.__index)
	
	@staticmethod
	def get_all_positions(sentence):
		pos = []
		for i in range(sentence.arity()):
			if sentence.get_term(i).is_complex():
				pos.extend([PositionIndex([i] + rec_pos) for rec_pos in PositionIndex.get_all_positions_rec(sentence.get_term(i))])
			else:
				pos.append(PositionIndex([i]))
		return pos
	
	@staticmethod
	def get_all_positions_rec(complex):
		pos = []
		for i in range(complex.arity()):
			if complex.get_term(i).is_complex():
				pos.extend([[i] + rec_pos for rec_pos in PositionIndex.get_all_positions_rec(complex.get_term(i))])
			else:
				pos.append([i])
		return pos

	@staticmethod
	def get_term_positions(sentence, term):
		positions = PositionIndex.__get_term_positions_rec(sentence, term)
		return [PositionIndex(p) for p in positions]
	
	
	@staticmethod
	def __get_term_positions_rec(complex, term):
		positions = []
		for i in range(complex.arity()):
			if complex.get_term(i) == term:
				positions.append([i])
			elif complex.get_term(i).is_complex():
				rec_pos = PositionIndex.__get_term_positions_rec(complex.get_term(i), term)
				positions.extend([[i] + pos for pos in rec_pos])
		return positions

	def fetch(self, sentence):
		complex = sentence
		for i in self.__index:
			complex = complex.get_term(i)
		return complex

	def set(self, sentence, val):
		complex = sentence
		for i in self.__index[:-1]:
			complex = complex.get_term(i)
		complex.set_term(self.__index[-1], val)
		self.recalc_hash()

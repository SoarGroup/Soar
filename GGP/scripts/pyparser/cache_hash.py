class CacheHash:
	def __init__(self):
		self.recalc_hash()
	
	def recalc_hash(self):
		self.__hashval = self.get_hash()

	def __hash__(self):
		return self.__hashval

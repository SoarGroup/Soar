class CacheHash:
	def __init__(self):
		self.__hashvalid = False
	
	def mark_hash_old(self):
		self.__hashvalid = False

	def __hash__(self):
		if not self.__hashvalid:
			self.__hashval = self.get_hash()
		return self.__hashval

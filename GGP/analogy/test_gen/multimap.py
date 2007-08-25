class MultiMap:
	def __init__(self):
		self.__m = {}
	
	def __getitem__(self, k):
		if k in self.__m:
			return self.__m[k]
		else:
			return []
	
	def __setitem__(self, k, item):
		self.__m.setdefault(k, []).append(item)

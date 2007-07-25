
class UniqueNameGenerator:
	def __init__(self):
		self.var_indices = dict()

	def get_name(self, preferred):
		if len(preferred) == 0:
			preferred = "a"
		
		name = preferred
		while self.var_indices.has_key(name):
			self.var_indices[name] += 1
			name = name + str(self.var_indices[name])
		self.var_indices[name] = 0
		return name
	
	def copy(self):
		c = UniqueNameGenerator()
		c.var_indices = dict(self.var_indices.items())
		return c

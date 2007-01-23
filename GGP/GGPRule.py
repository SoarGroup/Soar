from GGPSentence import GGPSentence

class GGPRule:
	def __init__(self, elementGGP):
		if isinstance(elementGGP[0], str):
			if elementGGP[0] == "<=":
				self.__head = GGPSentence(elementGGP[1])
				self.__body = [ GGPSentence(i) for i in list(elementGGP)[2:]]
			else: # head with no body
				self.__head = GGPSentence(elementGGP)
				self.__body = []
		else:
			# undefined
			assert False, "Unrecognized GGP: %s" % str(elementGGP)
	
	def head(self):
		return self.__head
	
	def body(self):
		return self.__body
	
	def has_body(self):
		return len(self.__body) > 0
	
	def __str__(self):
		if len(self.__body) == 0:
			return "(%s)" % str(self.__head)
		else:
			bodystr = ""
			for b in self.__body:
				bodystr += str(b)
			return "(<= %s %s)" % (str(self.__head), bodystr)
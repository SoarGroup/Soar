import RuleParser

class Comparison:
	@staticmethod
	def make_from_GGP_sentence(sentence):
		if sentence.term(1).type() == 'variable':
			rhs_is_var = True
		else:
			rhs_is_var = False

		if sentence.name() == 'distinct':
			comp = Comparison(sentence.term(0), '<>', sentence.term(1), rhs_is_var)
		else:
			comp = Comparison(sentence.term(0), sentence.name(), sentence.term(1), rhs_is_var)

		return comp

	def __init__(self, lhs, comparison, rhs, rhs_is_var):
		self.__lhs = lhs
		self.__rhs = rhs
		self.__comp = comparison
		self.__rhs_is_var = rhs_is_var

	def complement(self):
		if self.__comp == '<>':
			self.__comp = '='
		if self.__comp == '=':
			self.__comp = '<>'
		if self.__comp == '<':
			self.__comp = '>='
		if self.__comp == '<=':
			self.__comp = '>'
		if self.__comp == '>':
			self.__comp = '<='
		if self.__comp == '>=':
			self.__comp = '<'
	
	def get_comparison(self):
		return self.__comp

	def make_soar_condition(self, sp):
		assert self.__comp != '=', "Equality can't be explicitly expressed in Soar"

#		var_map = RuleParser.GDLSoarVarMapper(sp.get_name_gen())
#		lhs_id = var_map.get_var(self.__lhs)
		lhs_id = self.__lhs
		if self.__rhs_is_var:
			sp.add_id_predicate(lhs_id, self.__comp, self.__rhs)
		else:
			sp.add_predicate(lhs_id, self.__comp, self.__rhs)
	
	def __str__(self):
		return '%s %s %s' % (self.__lhs, self.__comp, self.__rhs)

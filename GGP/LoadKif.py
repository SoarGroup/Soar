import RuleParser
from ElementGGP import ElementGGP

kif = open("mummymaze1p-horiz.kif")

description = ""
line = kif.readline()
while line != "":
	if len(line.strip()) > 0 and line.strip()[0] != ';':
		description += " %s " % line.strip()
	line = kif.readline()


RuleParser.TranslateDescription("game", ElementGGP("(%s)" % description), "generated.soar")
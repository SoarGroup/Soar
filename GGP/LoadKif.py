import RuleParser
from ElementGGP import ElementGGP

kifdir = "..\\kif"
#kiffile = "mummymaze1p-horiz.kif"
kiffile = "buttons.kif"
kif = open("%s\\%s" % (kifdir, kiffile))

description = ""
line = kif.readline()
while line != "":
	if len(line.strip()) > 0 and line.strip()[0] != ';':
		description += " %s " % line.strip()
	line = kif.readline()


RuleParser.TranslateDescription("game", ElementGGP("(%s)" % description), "generated.soar")
import RuleParser
import tempfile
from ElementGGP import ElementGGP

kifdir = "..\\kif"
kiffile = "mummymaze1p-horiz.kif"
#kiffile = "buttons.kif"
soarfile = tempfile.mkstemp(".soar", kiffile[:-3], ".")[1]
print soarfile
kif = open("%s\\%s" % (kifdir, kiffile))

description = ""
line = kif.readline()
while line != "":
	if len(line.strip()) > 0 and line.strip()[0] != ';':
		description += " %s " % line.strip()
	line = kif.readline()


RuleParser.TranslateDescription("game", ElementGGP("(%s)" % description), soarfile)
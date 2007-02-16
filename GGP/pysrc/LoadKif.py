import os
import RuleParser
from ElementGGP import ElementGGP

kifdir = os.path.join('..', 'kif', 'R02_mod')
agentdir = os.path.join('..', 'agents')

#kiffile = "mummymaze1p-horiz.kif"
kiffile = "mummymaze1p-horiz-easy.kif"
#kiffile = "mummymaze1p-vert.kif"
#kiffile = "mummymaze1p-hv.kif"
#kiffile = "mummymaze1p-nosouth.kif"
#kiffile = "mummymaze1p-unreachable.kif"
#kiffile = "mummymaze1p-separate.kif"
#kiffile = "buttons.kif"

soarfile = os.path.join(agentdir, '%s.soar' % (kiffile[:-4]))
n = 0
while os.path.exists(soarfile):
	soarfile = os.path.join(agentdir, '%s.%d.soar' % (kiffile[:-4], n))
	n += 1

print soarfile
kif = open(os.path.join(kifdir, kiffile))

description = ""
line = kif.readline()
while line != "":
	if len(line.strip()) > 0 and line.strip()[0] != ';':
		description += " %s " % line.strip()
	line = kif.readline()


RuleParser.TranslateDescription("game", ElementGGP("(%s)" % description), soarfile)
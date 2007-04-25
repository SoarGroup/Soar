import os, sys
import RuleParser
from ElementGGP import ElementGGP

agentdir = os.path.join('..', 'agents')

if len(sys.argv) > 1:
	kiffile = os.path.split(sys.argv[1])[1]
	kif = open(sys.argv[1])
else:
	#kifdir = os.path.join('..', 'kif', 'R03_mod')
	kifdir = os.path.join('..', 'kif', 'escape')
	kiffile = "escape.kif"
	#kiffile = "mm-r03-composition-source1.kif"
	#kiffile = "mm-r03-abstraction-source.kif"
	#kiffile = "mummymaze1p-horiz.kif"
	#kiffile = "mummymaze1p-horiz-gun-new.kif"
	#kiffile = "mummymaze1p-vert.kif"
	#kiffile = "mummymaze1p-hv.kif"
	#kiffile = "mummymaze1p-nosouth.kif"
	#kiffile = "mummymaze1p-unreachable.kif"
	#kiffile = "mummymaze1p-separate.kif"
	#kiffile = "buttons.kif"

	kif = open(os.path.join(kifdir, kiffile))

soarfile = os.path.join(agentdir, '%s.soar' % (kiffile[:-4]))
n = 0
while os.path.exists(soarfile):
	soarfile = os.path.join(agentdir, '%s.%d.soar' % (kiffile[:-4], n))
	n += 1

print soarfile

description = ""
line = kif.readline()
while line != "":
	commentPos = line.strip().find(';')
	if commentPos >= 0:
		line = line.strip()[0:commentPos]
	if len(line) > 0:
		description += " %s " % line.strip()
	line = kif.readline()


RuleParser.TranslateDescription("game", ElementGGP("(%s)" % description), soarfile)

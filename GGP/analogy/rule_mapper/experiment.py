import os, sys

base_dir = os.path.join('..')
rule_gen_dir = os.path.join(base_dir, 'test_gen')
mapper_dir = os.path.join(base_dir, 'rule_mapper')
src_rules = os.path.join(rule_gen_dir, 'src_rules.kif')
tgt_rules = os.path.join(rule_gen_dir, 'tgt_rules.kif')

rule_gen_cmd = 'python test_gen.py >> /dev/null'
mapper_cmd = 'python rule_mapper2.py %s %s %d'

for bins in range(1,11):
	matches = 0
	for i in range(20):
		# make the rules
		os.chdir(rule_gen_dir)
		os.system(rule_gen_cmd)
		# run the mapper
		os.chdir(mapper_dir)
		result = os.popen(mapper_cmd % (src_rules, tgt_rules, bins)).read()
		matches += int(result)
		print '.',
		sys.stdout.flush()

	print '\n%d bins: %d matches avg' % (bins, matches / 10.0)

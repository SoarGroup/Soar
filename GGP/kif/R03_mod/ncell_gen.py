#!/usr/bin/python

r = range(8)
for x in r:
	var_name = '<ncellx%d>' % x
	print '(<facts> ^ncellx %s)' % var_name
	print '(%s ^p1 %d)' % (var_name, x)
	for y in r:
		for x1 in r:
			if x < x1:
				print '(%s ^rest <ncellxrest_%d_%d_%d_%d>)' % (var_name, x, y, x1, y)
				print '(<ncellxrest_%d_%d_%d_%d> ^p2 %d ^p3 east ^p4 %d ^p5 %d ^p6 %d)' % (x, y, x1, y, y, x1-x, x1, y)
			elif x > x1:
				print '(%s ^rest <ncellxrest_%d_%d_%d_%d>)' % (var_name, x, y, x1, y)
				print '(<ncellxrest_%d_%d_%d_%d> ^p2 %d ^p3 west ^p4 %d ^p5 %d ^p6 %d)' % (x, y, x1, y, y, x-x1, x1, y)
		for y1 in r:
			if y < y1:
				print '(%s ^rest <ncellxrest_%d_%d_%d_%d>)' % (var_name, x, y, x, y1)
				print '(<ncellxrest_%d_%d_%d_%d> ^p2 %d ^p3 south ^p4 %d ^p5 %d ^p6 %d)' % (x, y, x, y1, y, y1-y,  x, y1)
			elif y > y1:
				print '(%s ^rest <ncellxrest_%d_%d_%d_%d>)' % (var_name, x, y, x, y1)
				print '(<ncellxrest_%d_%d_%d_%d> ^p2 %d ^p3 north ^p4 %d ^p5 %d ^p6 %d)' % (x, y, x, y1, y, y-y1,  x, y1)

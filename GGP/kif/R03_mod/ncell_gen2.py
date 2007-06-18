#!/usr/bin/python

r = range(8)
for x in r:
	var1 = '<ncellx%d>' % x
	print '(<facts> ^ncellx %s)' % var1
	print '(%s ^p1 %d)' % (var1, x)
	for y in r:
		var2 = '<rest_%d_%d>' % (x, y)
		print '(%s ^rest1 %s)' % (var1, var2)
		print '(%s ^p2 %d)' % (var2, y)
		for x1 in r:
			var3 = '<rest_%d_%d_%d_%d>' % (x, y, x1, y)
			if x < x1:
				print '(%s ^rest2 %s)' % (var2, var3)
				print '(%s ^p3 east ^p4 %d ^p5 %d ^p6 %d)' % (var3, x1-x, x1, y)
			elif x > x1:
				print '(%s ^rest2 %s)' % (var2, var3)
				print '(%s ^p3 west ^p4 %d ^p5 %d ^p6 %d)' % (var3, x-x1, x1, y)
		for y1 in r:
			var3 = '<rest_%d_%d_%d_%d>' % (x, y, x, y1)
			if y < y1:
				print '(%s ^rest2 %s)' % (var2, var3)
				print '(%s ^p3 south ^p4 %d ^p5 %d ^p6 %d)' % (var3, y1-y, x, y1)
			elif y > y1:
				print '(%s ^rest2 %s)' % (var2, var3)
				print '(%s ^p3 north ^p4 %d ^p5 %d ^p6 %d)' % (var3, y-y1, x, y1)

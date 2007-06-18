r = range(8)
for x in r:
	for y in r:
		for x1 in r:
			if x < x1:
				print '(nCell %d east %d %d %d %d)' % (x1-x, x, y, x1, y)
			elif x > x1:
				print '(nCell %d west %d %d %d %d)' % (x-x1, x, y, x1, y)
		for y1 in r:
			if y < y1:
				print '(nCell %d south %d %d %d %d)' % (y1-y, x, y, x, y1)
			elif y > y1:
				print '(nCell %d north %d %d %d %d)' % (y-y1, x, y, x, y1)

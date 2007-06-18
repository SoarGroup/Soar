r = range(12)
for x1 in r:
	for y1 in r:
		for x2 in r:
			for y2 in r:

				# orthogonals
				if x1 == x2:
					if y1 > y2:
						print "(nCell %d north %d %d %d %d)" % (y1 - y2, x1, y1, x2, y2)
					if y1 < y2:
						print "(nCell %d south %d %d %d %d)" % (y2 - y1, x1, y1, x2, y2)
				if y1 == y2:
					if x1 > x2:
						print "(nCell %d west %d %d %d %d)" % (x1 - x2, x1, y1, x2, y2)
					if x1 < x2:
						print "(nCell %d east %d %d %d %d)" % (x2 - x1, x1, y1, x2, y2)

				# diagonals
				if x1 > x2 and y1 > y2 and x1 - x2 == y1 - y2:
					print "(nCell %d nw %d %d %d %d)" % (x1 - x2, x1, y1, x2, y2)
				if x1 < x2 and y1 > y2 and x2 - x1 == y1 - y2:
					print "(nCell %d ne %d %d %d %d)" % (x2 - x1, x1, y1, x2, y2)
				if x1 > x2 and y1 < y2 and x1 - x2 == y2 - y1:
					print "(nCell %d sw %d %d %d %d)" % (x1 - x2, x1, y1, x2, y2)
				if x1 < x2 and y1 < y2 and x2 - x1 == y2 - y1:
					print "(nCell %d se %d %d %d %d)" % (x2 - x1, x1, y1, x2, y2)

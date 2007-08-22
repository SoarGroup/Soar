def find_max(seq, func):
	if len(seq) == 0:
		return (-1, -1)
	max = func(seq[0])
	mpos = 0
	for i in range(1,len(seq)):
		v = func(seq[i])
		if max < v:
			max = v
			mpos = i
	return (mpos, max)

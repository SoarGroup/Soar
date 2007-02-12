import sys, re
import xml.dom.minidom as dom
import xml.dom.ext

def CreateSVGDoc(x1, y1, x2, y2):
	doc = dom.Document()
	root = doc.createElementNS("http://www.w3.org/2000/svg", "svg")
	root.setAttribute('viewBox', '%d %d %d %d' % (x1, y1, x2, y2))
	root.setAttribute('width', '100%')
	root.setAttribute('height', '100%')
	doc.appendChild(root)
	layer = doc.createElement('g')
	layer.setAttribute('id', 'mainlayer')
	root.appendChild(layer)
	return (doc, layer)

def DrawLine(doc, layer, x1, y1, x2, y2, color='black', width=1):
	group = doc.createElement('g')
	group.setAttribute('stroke', color)
	group.setAttribute('fill', 'none')
	line = doc.createElement('line')
	line.setAttribute('x1', str(x1))
	line.setAttribute('x2', str(x2))
	line.setAttribute('y1', str(y1))
	line.setAttribute('y2', str(y2))
	line.setAttribute('stroke-width', str(width))
	group.appendChild(line)
	layer.appendChild(group)

def DrawCircle(doc, layer, x, y, r, stroke='black', fill='none'):
	circle = doc.createElement('circle')
	circle.setAttribute('cx', str(x))
	circle.setAttribute('cy', str(y))
	circle.setAttribute('r', str(r))
	circle.setAttribute('stroke', stroke)
	circle.setAttribute('fill', fill)
	layer.appendChild(circle)

def DrawText(doc, layer, x, y, t):
	group = doc.createElement('g')
	group.setAttribute('font-size','10px')
	elem = doc.createElement('text')
	elem.setAttribute('x', str(x))
	elem.setAttribute('y', str(y))
	text = doc.createTextNode(t)
	elem.appendChild(text)
	group.appendChild(elem)
	layer.appendChild(group)

def TransformCoords(x, y, y_max, horiz_offset, vert_offset, htweak=0, vtweak=0):
	border = 20
	res = 100
	horiz_offset_val = {'l':0, 'm':res/2, 'r':res}
	vert_offset_val = {'l':res, 'm':res/2, 'h':0}

	nx = border + x * res + horiz_offset_val[horiz_offset] + htweak
	ny = border + (y_max - y) * res + vert_offset_val[vert_offset] + vtweak

	return (nx, ny)

if __name__ == '__main__':

	wall_pat = re.compile('\\(init\\s+\\(wall\\s+(\\d+)\\s+(\\d+)\\s+(\\w+)\\s*\\)\\s*\\)')
	index_pat = re.compile('\\(index\\s+(\\d+)\\s*\\)')
	loc_pat = re.compile('\\(init\\s+\\(location\\s+([-\\w]+)\\s+(\\d+)\\s+(\\d+)\\s*\\)\\s*\\)')

	# find the largest index
	file = open(sys.argv[1], 'r')
	indexes = []
	for line in file.readlines():
		m = index_pat.match(line)
		if m != None:
			indexes.append(int(m.group(1)))
	file.close()

	indexes.sort()
	max_index = indexes[-1]

	# create the document with the correct scale
	lower_right = TransformCoords(max_index, 0, max_index, 'r', 'l')
	doc, layer = CreateSVGDoc(0, 0, lower_right[0], lower_right[1])
	
	# draw a grid
	for i in range(max_index+1):
		left = TransformCoords(0, i, max_index, 'r', 'l')
		right = TransformCoords(max_index, i, max_index, 'r', 'l')
		DrawLine(doc, layer, left[0], left[1], right[0], right[1])
	
	for i in range(max_index+1):
		bot = TransformCoords(i, 0, max_index, 'r', 'h')
		top = TransformCoords(i, max_index, max_index, 'r', 'h')
		DrawLine(doc, layer, bot[0], bot[1], top[0], top[1])

	# go through the kif again and draw the walls
	file = open(sys.argv[1], 'r')
	for line in file.readlines():
		m = wall_pat.match(line)
		if m != None:
			x = int(m.group(1))
			y = int(m.group(2))
			dir = m.group(3)
			if dir == 'north':
				coord1 = TransformCoords(x, y, max_index, 'l', 'h')
				coord2 = TransformCoords(x, y, max_index, 'r', 'h')
			else:
				coord1 = TransformCoords(x, y, max_index, 'r', 'l')
				coord2 = TransformCoords(x, y, max_index, 'r', 'h')

			DrawLine(doc, layer, coord1[0], coord1[1], coord2[0], coord2[1], width=7)
			continue

		m = loc_pat.match(line)
		if m != None:
			obj = m.group(1)
			x = int(m.group(2))
			y = int(m.group(3))
			coord = TransformCoords(x, y, max_index, 'l', 'h', 5, 15)
			#DrawCircle(doc, layer, coord[0], coord[1], 10, 'none', 'red')
			DrawText(doc, layer, coord[0], coord[1], obj)

	xml.dom.ext.PrettyPrint(doc)

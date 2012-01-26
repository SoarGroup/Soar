from __future__ import print_function
import sys, os, time
import Tkinter as tk

#while not os.path.exists('ctrl'):
#	time.sleep(0.5)
#pipe = open('ctrl', 'w')

log = open('remote.log', 'w')

def handle_input(evt):
	#global pipe
	
	if evt.keysym == 'Left':
		vals = (-1, 0)
	elif evt.keysym == 'Right':
		vals = (1, 0)
	elif evt.keysym == 'Up':
		vals = (0, -1)
	elif evt.keysym == 'Down':
		vals = (0, 1)
	else:
		return

	out = '{} {}'.format(*vals)
	
	#pipe.write(out + '\n')
	#pipe.flush()
	
	if sys.stdout.closed:
		print('ASDF', file=sys.stderr)
		sys.exit(0)
	
	print(out)
	sys.stdout.flush()
	log.write(out + '\n')
	log.flush()
	
win = tk.Tk()
lbl = tk.Label(win, text = 'REMOTE', height = 20, width = 50)
lbl.pack(fill = tk.BOTH, expand = 1)
win.bind("<Key>", handle_input)
tk.mainloop()

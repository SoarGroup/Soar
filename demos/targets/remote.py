from __future__ import print_function
import sys, os, time
import Tkinter as tk

while not os.path.exists('ctrl'):
	time.sleep(0.5)

pipe = open('ctrl', 'w')
log = open('remote.log', 'w')

def handle_input(evt):
	global pipe
	
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

	pipe.write('{} {}\n'.format(*vals))
	pipe.flush()
	log.write('{} {}\n'.format(*vals))
	log.flush()
	
win = tk.Tk()
lbl = tk.Label(win, text = 'REMOTE', height = 20, width = 50)
lbl.pack(fill = tk.BOTH, expand = 1)
win.bind("<Key>", handle_input)
tk.mainloop()

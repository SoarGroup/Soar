from __future__ import print_function
import sys, os, time
import Tkinter as tk

sys.path.append('../')
import sock

DELAY = 50

if len(sys.argv[1]) == 0:
	print('usage: {} <socket path>'.format(sys.argv[0]))
	sys.exit(1)
	
sck = sock.Sock()
sck.connect(sys.argv[1])

currcmd = (0.0, 0.0)

def press(dir):
	global currcmd
	print('press ' + dir)
	currcmd = {
		'L' : (-0.5,  0.5),
		'R' : ( 0.5, -0.5),
		'F' : ( 1.0,  1.0),
		'B' : (-1.0, -1.0)
	}.get(dir, (0.0, 0.0))
	

def release():
	global currcmd
	currcmd = (0.0, 0.0)
	#sck.send('left {}\nright {}'.format(*cmd))
	
def send_cmd(): # (file, mask):
	global win
	global sck
	sck.receive()
	sck.send('left {}\nright {}'.format(*currcmd))
	win.after(DELAY, send_cmd)
	
win = tk.Tk()
for b in 'LFBR':
	btn = tk.Button(win, text = b)
	btn.bind('<Button-1>', lambda e, b=b: press(b))
	btn.bind('<ButtonRelease-1>', lambda e: release())
	btn.pack(side = tk.LEFT)
	
#win.tk.createfilehandler(sck.sock, tk.READABLE, handle_msg)
win.after(DELAY, send_cmd)
tk.mainloop()

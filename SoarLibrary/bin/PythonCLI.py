#!/usr/bin/env python
#############
# Configuration
c = {}
c['agent'] = 'soar'

#############

import threading
import Queue
import sys
import Python_sml_ClientInterface
import curses
import curses.ascii

psci = Python_sml_ClientInterface

def PrintCallback(id, userData, agent, message):
	userData.addstr(message)
	userData.refresh()

def get_input(stdscr, agent):
	stdscr.scroll(1)
	stdscr.addstr(curses.LINES - 1, 0, agent + '> ')
	command = ''
	asc = curses.ascii
	while True:
		stdscr.refresh()
		c = stdscr.getch()
		if asc.isprint(c):
			ch = chr(c)
			stdscr.addch(ch)
			command = command + ch
		elif c == asc.NL:
			stdscr.addch('\n')
			return command
		elif c == asc.DEL:
			yx = stdscr.getyx()
			stdscr.delch(yx[0], yx[1] - 1)
			command = command[:-1]


def cli(stdscr):
	stdscr.scrollok(True)

	kernel = psci.Kernel.CreateKernelInNewThread()
	agent = kernel.CreateAgent(c['agent'])
	
	agent.RegisterForPrintEvent(psci.smlEVENT_PRINT, PrintCallback, stdscr)
	
	while True:
		input = get_input(stdscr, c['agent'])
		line = kernel.ExecuteCommandLine(input, c['agent'])
		stdscr.addstr(curses.LINES - 1, 0, line)
		stdscr.refresh()
		if input.startswith('quit'):
			break
		
	kernel.DestroyAgent(agent)
	kernel.Shutdown()
	del kernel

def main():
	curses.wrapper(cli)

if __name__ == "__main__":
	main()

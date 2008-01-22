#!/usr/bin/env python

import signal
import os, sys
import Python_sml_ClientInterface

psci = Python_sml_ClientInterface

kernel = None
agent = None

def PrintCallback(id, userData, agent, message):
	print message

def run_script(script):
	global kernel
	global agent

	for command in open(script, 'r'):
		if command.startswith('quit'):
			break
		line = kernel.ExecuteCommandLine(command, 'soar')
		if len(line.strip()) > 0:
			print line
		
	line = kernel.ExecuteCommandLine('stats', 'soar')
	if len(line.strip()) > 0:
		print line
	kernel.DestroyAgent(agent)
	kernel.Shutdown()
	del kernel

def run_agent(agent_file, commands):
	global kernel
	global agent

	kernel.ExecuteCommandLine('source %s' % agent_file, 'soar')
	for c in commands:
		line = kernel.ExecuteCommandLine(c, 'soar')
		print line
		
	line = kernel.ExecuteCommandLine('stats', 'soar')
	if len(line.strip()) > 0:
		print line
	kernel.DestroyAgent(agent)
	kernel.Shutdown()
	del kernel

def handle_args(args):
	global kernel

	commands = []

	i = 0
	while i < len(args):
		a = args[i]
		if a.startswith('-w'):
			watchlevel = a[2:]
			kernel.ExecuteCommandLine("watch %s" % watchlevel, 'soar')
		elif a == '-l':
			kernel.ExecuteCommandLine('learn --on', 'soar')
		elif a == '-f':
			kernel.ExecuteCommandLine('indifferent-selection --first', 'soar')
		elif a == '-c':
			commands.extend(args[i+1].split(';'))
			i += 1
		i += 1
	
	return commands

def sig_handler(signum, frame):
	global kernel

	kernel.ExecuteCommandLine('stop-soar', 'soar')
	kernel.DestroyAgent(agent)
	kernel.Shutdown()
	sys.exit(1)

def pick_port():
	p = 12121
	if sys.platform == 'win32':
		# don't know how to do this in windows
		return p;

	SOCKET_DIR=os.path.join(os.environ['HOME'], '.soartmp')
	used = os.listdir(SOCKET_DIR)
	while str(p) in used:
		p += 1
	return p

if __name__ == "__main__":
	signal.signal(signal.SIGINT, sig_handler)

	kernel = psci.Kernel.CreateKernelInNewThread(psci.Kernel.kDefaultLibraryName, pick_port())
	agent = kernel.CreateAgent('soar')
	agent.RegisterForPrintEvent(psci.smlEVENT_PRINT, PrintCallback, None)

	nargs = len(sys.argv)
	if len(sys.argv) >= 2:
		commands = handle_args(sys.argv[1:nargs-1])
		if sys.argv[nargs-1].endswith('.soar'):
			agent_file = sys.argv[nargs-1]
			if len(commands) > 0:
				run_agent(agent_file, commands)
			else:
				run_agent(agent_file, ['run 1700000'])
		else:
			run_script(sys.argv[nargs-1])
	else:
		print 'Usage: soar [-w<n>] [-l] [-f] <script file> | <agent file>'
		sys.exit(1)

#!/usr/bin/env python

import signal
import sys
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
		if len(strip(line)) > 0:
			print line
		
	kernel.DestroyAgent(agent)
	kernel.Shutdown()
	del kernel

def run_agent(agent_file):
	global kernel
	global agent

	kernel.ExecuteCommandLine('source %s' % agent_file, 'soar')
	kernel.ExecuteCommandLine('run', 'soar')
		
	kernel.DestroyAgent(agent)
	kernel.Shutdown()
	del kernel

def handle_args(args):
	global kernel

	for a in args:
		if a.startswith('-w'):
			watchlevel = a[2:]
			kernel.ExecuteCommandLine("watch %s" % watchlevel, 'soar')
		elif a == '-l':
			kernel.ExecuteCommandLine('learn --on', 'soar')

def sig_handler(signum, frame):
	global kernel

	kernel.Shutdown()

if __name__ == "__main__":
	signal.signal(signal.SIGINT, sig_handler)

	kernel = psci.Kernel.CreateKernelInNewThread()
	agent = kernel.CreateAgent('soar')
	agent.RegisterForPrintEvent(psci.smlEVENT_PRINT, PrintCallback, None)

	nargs = len(sys.argv)
	if len(sys.argv) > 2:
		handle_args(sys.argv[1:nargs-1])
		if sys.argv[nargs-1].endswith('.soar'):
			run_agent(sys.argv[nargs-1])
		else:
			run_script(sys.argv[nargs-1])
	else:
		print 'Usage: soar <script file> | <agent file>'
		sys.exit(1)

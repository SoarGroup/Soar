#!/usr/bin/env python

import sys
sys.path.append('../lib')
import Python_sml_ClientInterface as psci

PROMPT='% '

def PrintCallback(id, userData, agent, message):
	m = message.strip()
	if len(m) > 0:
		sys.stdout.write(m + '\n')
		sys.stdout.flush()

if __name__ == "__main__":
	kernel = psci.Kernel.CreateKernelInCurrentThread()
	agent = kernel.CreateAgent('soar')
	
	agent.RegisterForPrintEvent(psci.smlEVENT_PRINT, PrintCallback, None)
	
	while True:
		sys.stdout.write(PROMPT)
		input = sys.stdin.readline()
		if input == '' or input.startswith('quit'):
			break
		
		out = agent.ExecuteCommandLine(input.strip()).strip()
		if len(out) > 0:
			print out
		
	kernel.DestroyAgent(agent)
	kernel.Shutdown()
	del kernel

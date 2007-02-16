#!/usr/bin/python

import Python_sml_ClientInterface as sml
import sys

def print_callback(id, userData, agent, message):
	print message,

def file_output_callback(id, userData, agent, message):
  userData.write(message)

def RunSoar(file, output=False, output_file=None):
	cmd_file = open(file, 'r')
	kernel = sml.Kernel.CreateKernelInNewThread()
	if kernel == None:
		print "Kernel creation failed."
		cmd_file.close()
		return "Error"

	agent = kernel.CreateAgent('agent')
	if agent == None:
		print "Agent creation failed: %s" % kernel.GetLastErrorDescription()
		cmd_file.close()
		return "Error"

	if output:
		if output_file == None:
			agent.RegisterForPrintEvent(sml.smlEVENT_PRINT, print_callback, None)
		else:
			outfile = open(output_file, 'w')
			agent.RegisterForPrintEvent(sml.smlEVENT_PRINT, file_output_callback, outfile)

	retstrings = ""
	for cmd in cmd_file:
#			print "###pyrunsoar###: %s" % cmd,
		if cmd[0] == '!':
			retstrings += agent.ExecuteCommandLine(cmd[1:])
		else:
			agent.ExecuteCommandLine(cmd)

	cmd_file.close()
	if output and output_file != None:
		outfile.close()

	return retstrings

if __name__ == '__main__':
	output = False
	outfile = None
	filename = ""

	i = 1
	while i < len(sys.argv):
		if sys.argv[i] == "-o":
			output = True
		elif sys.argv[i] == '-f':
			outfile = sys.argv[i + 1]
			i += 1
		else:
			filename = sys.argv[i]
		i += 1
	
	RunSoar(filename, output, outfile)

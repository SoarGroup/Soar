
import sys
import Python_sml_ClientInterface

psci = Python_sml_ClientInterface

kernel = psci.Kernel.CreateKernelInCurrentThread()
agent = kernel.CreateAgent('obfuscator')
kernel.ExecuteCommandLine('source "%s"' % sys.argv[1], 'obfuscator')
original = kernel.ExecuteCommandLine('print -f', 'obfuscator')
original = original.split('\n')
for line in original:
    print line
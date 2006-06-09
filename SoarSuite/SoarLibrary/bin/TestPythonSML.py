import Python_sml_ClientInterface

def PrintCallback(id, userData, agent, message):
	print message

def ProductionExcisedCallback(id, userData, agent, prodName, instantiation):
	print "removing", prodName

def ProductionFiredCallback(id, userData, agent, prodName, instantiation):
	print "fired", prodName

def PhaseExecutedCallback(id, userData, agent, phase):
	print "phase", phase, "executed"
	
def StructuredTraceCallback(id, userData, agent, pXML):
	print "structured data:", pXML.GenerateXMLString(True)

kernel = Python_sml_ClientInterface.Kernel.CreateKernelInNewThread()
agent = kernel.CreateAgent('Soar1')
printcallbackid = agent.RegisterForPrintEvent(Python_sml_ClientInterface.smlEVENT_PRINT, PrintCallback, None)
productionCallbackId = agent.RegisterForProductionEvent(Python_sml_ClientInterface.smlEVENT_BEFORE_PRODUCTION_REMOVED, ProductionExcisedCallback, None)
productionCallbackId = agent.RegisterForProductionEvent(Python_sml_ClientInterface.smlEVENT_AFTER_PRODUCTION_FIRED, ProductionFiredCallback, None)
runCallbackId = agent.RegisterForRunEvent(Python_sml_ClientInterface.smlEVENT_AFTER_PHASE_EXECUTED, PhaseExecutedCallback, None)
structuredCallbackId = agent.RegisterForXMLEvent(Python_sml_ClientInterface.smlEVENT_XML_TRACE_OUTPUT, StructuredTraceCallback, None)

#load the TOH productions
result = agent.LoadProductions('../Demos/towers-of-hanoi/towers-of-hanoi.soar')
#loads a function to test the user-defined RHS function stuff
result = agent.LoadProductions('../Tests/TOHtest.soar')

agent.RunSelf(2, Python_sml_ClientInterface.sml_ELABORATION)

agent.UnregisterForProductionEvent(productionCallbackId)
agent.UnregisterForRunEvent(runCallbackId)

kernel.RunAllAgents(3)

#set the watch level to 0
result = kernel.ExecuteCommandLine("watch 0", "Soar1")

#excise the monitor production
result = kernel.ExecuteCommandLine("excise towers-of-hanoi*monitor*operator-execution*move-disk", "Soar1")

#run TOH the rest of the way and time it using Python's built-in timer
#from timeit import Timer
#Timer('result = agent.RunSelfForever()').timeit()

kernel.DestroyAgent(agent)

kernel.Shutdown()
del kernel

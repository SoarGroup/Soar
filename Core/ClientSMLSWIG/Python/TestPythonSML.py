#!/usr/bin/env python
#
# This is a test script which tests several aspects of the Python SML interface
#  including kernel and agent creation, running, registering and unregistering
#  several kinds of callbacks, reinitializing, agent destruction, and kernel
#  destruction (and maybe some other things, too).
#
# This file needs to be compatible with python 3.5, to run on CI jobs testing the lowest supported python version.
import atexit
from pathlib import Path
import sys
import os
import time

try:
    import Python_sml_ClientInterface
    from Python_sml_ClientInterface import *

    BASE_DIR = Path(Python_sml_ClientInterface.__file__).parent
except ImportError:
    from soar_sml import *

    BASE_DIR = Path(os.environ.get("SOAR_UNIT_TEST_BASE_DIR", os.path.abspath("out/")))


AGENT_DIR = BASE_DIR / 'SoarUnitTests'

class CalledSignal:
    called = False

# towers_of_hanoi_file = AGENT_DIR / 'test-towers-of-hanoi-SML.soar'
towers_of_hanoi_file = AGENT_DIR / 'Chunking' / 'tests' / 'towers-of-hanoi-recursive' / 'towers-of-hanoi-recursive' / 'towers-of-hanoi-recursive_source.soar'
toh_test_file = AGENT_DIR / 'TOHtest.soar'
for source_file in (towers_of_hanoi_file, toh_test_file):
    if not source_file.is_file():
        raise FileNotFoundError("Source file doesn't exist: %s" % source_file)

def PrintCallback(id, userData, agent, message):
    print(message)

def ProductionExcisedCallback(id, signal: CalledSignal, agent, prodName, instantiation):
    print("removing %s (%s)" % (prodName, instantiation))
    signal.called = True

def ProductionFiredCallback(id, signal: CalledSignal, agent, prodName, instantiation):
    print("fired", prodName)

def PhaseExecutedCallback(id, signal: CalledSignal, agent, phase):
    print("phase", phase, "executed")
    signal.called = True

def AgentCreatedCallback(id, signal: CalledSignal, agent):
    print(agent.GetAgentName(), "created")
    signal.called = True

def AgentReinitializedCallback(id, signal: CalledSignal, agent):
    print(agent.GetAgentName(), "reinitialized")
    signal.called = True

def AgentDestroyedCallback(id, signal: CalledSignal, agent):
    print("destroying agent", agent.GetAgentName())
    signal.called = True

def SystemShutdownCallback(id, signal: CalledSignal, kernel):
    print("Shutting down kernel ", kernel)
    signal.called = True

def RhsFunctionTest(id, userData, agent, functionName, argument):
    print("Agent", agent.GetAgentName(), "called RHS function", functionName, "with argument(s) '", argument, "' and userData '", userData, "'")
    assert argument == "this is a test"
    assert userData == {"foo": "bar"}
    return "success"

def StructuredTraceCallback(id, signal: CalledSignal, agent, pXML):
    print("structured data:", pXML.GenerateXMLString(True))
    signal.called = True

def UpdateEventCallback(id, signal: CalledSignal, kernel, runFlags):
    print("update event fired with flags", runFlags)
    signal.called = True

def UserMessageCallback(id, tester, agent, clientName, message):
    print("Agent", agent.GetAgentName(), "received usermessage event for clientName '", clientName, "' with message '", message, "'")
    assert tester(clientName, message), ("❌ UserMessageCallback called with unexpected clientName '%s' or message '%s'" % (clientName, message))
    return ""

kernel = Kernel.CreateKernelInNewThread()
if not kernel:
    print('❌ Kernel creation failed', file=sys.stderr)
    sys.exit(1)
else:
    print('✅ Kernel creation succeeded')

# Neglecting to shut down the kernel causes a segfault, so we use atexit to guarantee proper cleanup
# TODO: This should be handled by the Python SML interface automatically.
def __cleanup():
    global kernel
    try:
        if kernel:
            kernel.Shutdown()
            del kernel
    except NameError:
        pass

atexit.register(__cleanup)

agent_create_called = CalledSignal()
agentCallbackId0 = kernel.RegisterForAgentEvent(smlEVENT_AFTER_AGENT_CREATED, AgentCreatedCallback, agent_create_called)

agent_reinit_called = CalledSignal()
agentReinitCallback = kernel.RegisterForAgentEvent(smlEVENT_BEFORE_AGENT_REINITIALIZED, AgentReinitializedCallback, agent_reinit_called)

agent_destroy_called = CalledSignal()
agentCallbackId2 = kernel.RegisterForAgentEvent(smlEVENT_BEFORE_AGENT_DESTROYED, AgentDestroyedCallback, agent_destroy_called)

shutdown_handler_called = CalledSignal()
shutdownCallbackId = kernel.RegisterForSystemEvent(smlEVENT_BEFORE_SHUTDOWN, SystemShutdownCallback, shutdown_handler_called)

rhsCallbackId = kernel.AddRhsFunction("RhsFunctionTest", RhsFunctionTest, {"foo": "bar"})

update_handler_called = CalledSignal()
updateCallbackId = kernel.RegisterForUpdateEvent(smlEVENT_AFTER_ALL_OUTPUT_PHASES, UpdateEventCallback, update_handler_called)
messageCallbackId = kernel.RegisterForClientMessageEvent("TestMessage", UserMessageCallback, lambda clientName, message: clientName == "TestMessage" and message == "This is a \"quoted\"\" message")

def test_create_agent(kernel):
    assert not agent_create_called.called
    agent = kernel.CreateAgent('Soar1')
    if not agent:
        print('❌ Agent creation failed', file=sys.stderr)
        sys.exit(1)
    else:
        print('✅ Agent creation succeeded')
    assert agent_create_called.called
    return agent

agent = test_create_agent(kernel)

printcallbackid = agent.RegisterForPrintEvent(smlEVENT_PRINT, PrintCallback, None)

prod_removed_handler_signal = CalledSignal()
prod_removed_callback_id = agent.RegisterForProductionEvent(smlEVENT_BEFORE_PRODUCTION_REMOVED, ProductionExcisedCallback, prod_removed_handler_signal)

prod_fired_handler_signal = CalledSignal()
prod_fired_callback_id = agent.RegisterForProductionEvent(smlEVENT_AFTER_PRODUCTION_FIRED, ProductionFiredCallback, prod_fired_handler_signal)

phase_executed_handler_signal = CalledSignal()
runCallbackId = agent.RegisterForRunEvent(smlEVENT_AFTER_PHASE_EXECUTED, PhaseExecutedCallback, phase_executed_handler_signal)

xml_trace_handler_signal = CalledSignal()
structuredCallbackId = agent.RegisterForXMLEvent(smlEVENT_XML_TRACE_OUTPUT, StructuredTraceCallback, xml_trace_handler_signal)

# load the TOH productions
result = agent.LoadProductions(str(towers_of_hanoi_file))

#loads a function to test the user-defined RHS function stuff
result = agent.LoadProductions(str(toh_test_file))

kernel.SendClientMessage(agent, "TestMessage", "This is a \"quoted\"\" message")
kernel.UnregisterForClientMessageEvent(messageCallbackId)

agent.RunSelf(2, sml_ELABORATION)

kernel.RunAllAgents(3)

#set the watch level to 0
result = kernel.ExecuteCommandLine("watch 0", "Soar1")

# excise the monitor production
def test_excise(kernel):
    assert not prod_removed_handler_signal.called, "❌ Production excise handler called before excise"
    result = kernel.ExecuteCommandLine("excise towers-of-hanoi*monitor*operator-execution*move-disk", "Soar1")
    assert prod_removed_handler_signal.called, "❌ Production excise handler not called"
    print("✅ Production excise: %s" % result)

test_excise(kernel)

#run TOH the rest of the way and time it
start = time.time()
result = agent.RunSelfForever()
end = time.time()
print("\nTime in seconds:", end - start)

def check_rhs_handler_called(kernel):
    s1 = kernel.ExecuteCommandLine("print s1", "Soar1")
    if s1.find("^rhstest success") == -1:
        print("\n❌RHS test FAILED; s1 is %s" % s1, file=sys.stderr)
        sys.exit(1)
    else:
        print("\n✅RHS test PASSED")

check_rhs_handler_called(kernel)

def test_agent_reinit(agent):
    assert not agent_reinit_called.called, "❌ Agent reinit handler called before reinit"
    kernel.ExecuteCommandLine("init-soar", "Soar1")
    assert agent_reinit_called.called, "❌ Agent reinit handler not called"
    print("✅ Agent reinit")

# test unregistering callbacks (not required, just to test)
agent.UnregisterForProductionEvent(prod_removed_callback_id)
agent.UnregisterForProductionEvent(prod_fired_callback_id)
agent.UnregisterForRunEvent(runCallbackId)

test_agent_reinit(agent)

def test_catch_exception(agent):
    input_link = agent.GetInputLink()
    try:
        input_link.GetParameterValue("non-existent")
    except ValueError as e:
        print("✅ Correctly caught exception:", e)
    else:
        assert False, "❌ No exception caught"

test_catch_exception(agent)

def test_agent_destroy(agent):
    assert not agent_destroy_called.called, "❌ Agent destroy handler called before destroy"
    kernel.DestroyAgent(agent)
    assert agent_destroy_called.called, "❌ Agent destroy handler not called"
    print("✅ Agent destroy")

test_agent_destroy(agent)

# test unregistering callbacks (except shutdown) (not required, just to test)
print("Removing callbacks...")
kernel.UnregisterForAgentEvent(agentCallbackId0)
kernel.UnregisterForAgentEvent(agentReinitCallback)
kernel.UnregisterForAgentEvent(agentCallbackId2)
kernel.RemoveRhsFunction(rhsCallbackId)
kernel.UnregisterForUpdateEvent(updateCallbackId)

def test_shutdown(kernel):
    assert not shutdown_handler_called.called, "❌ Kernel shutdown handler called before shutdown"
    #shutdown the kernel; this makes sure agents are deleted and events fire correctly
    kernel.Shutdown()
    assert shutdown_handler_called.called, "❌ Kernel shutdown handler not called"
    print("✅ Kernel shutdown")

test_shutdown(kernel)
#delete kernel object
del kernel

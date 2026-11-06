#!/usr/bin/env python3
"""
Test script for Python SML interface that can be run via CTest.

This tests several aspects of the Python SML interface including:
- Kernel and agent creation
- Running agents
- Registering and unregistering callbacks
- Reinitializing agents
- Agent destruction and kernel shutdown

This file needs to be compatible with python 3.5+.
"""
import unittest
from pathlib import Path
import os
import re
import sys
import time

# Add the python build directory to the path
SCRIPT_DIR = Path(__file__).parent.absolute()
REPO_ROOT = SCRIPT_DIR.parent
OUT_DIR = REPO_ROOT / "out"

# Try to detect build directory from environment or common locations
BUILD_DIR = None
if "SOAR_BUILD_DIR" in os.environ:
    BUILD_DIR = Path(os.environ["SOAR_BUILD_DIR"])
else:
    # Try common build directory patterns
    for build_type in ["Release", "Debug"]:
        candidate = REPO_ROOT / "build" / build_type
        if candidate.exists():
            BUILD_DIR = candidate
            break
    if BUILD_DIR is None:
        BUILD_DIR = REPO_ROOT / "build" / "Debug"  # fallback

PYTHON_BUILD_DIR = BUILD_DIR / "Core" / "ClientSMLSWIG" / "python"
PYTHON_LIBRARY_DIR = BUILD_DIR / "Core" / "ClientSMLSWIG"

# Add out/ (where _Python_sml_ClientInterface.pyd/.abi3.so and Python_sml_ClientInterface.py live)
# as well as the CMake build subdirectories to sys.path
for p in (OUT_DIR, PYTHON_BUILD_DIR, PYTHON_LIBRARY_DIR):
    if p.exists() and str(p) not in sys.path:
        sys.path.insert(0, str(p))

try:
    import Python_sml_ClientInterface
    from Python_sml_ClientInterface import *
    print(f"[OK] Successfully imported Python_sml_ClientInterface")
    print(f"Script directory: {SCRIPT_DIR}")
    print(f"Build directory: {BUILD_DIR}")
    print(f"Out directory: {OUT_DIR}")
except ImportError as e:
    print(f"[ERROR] Failed to import Python_sml_ClientInterface: {e}")
    print(f"Python path: {sys.path}")
    sys.exit(1)

# Test data directories: prefer out/SoarUnitTests or source UnitTests/SoarTestAgents
if (OUT_DIR / "SoarUnitTests").exists():
    AGENT_DIR = OUT_DIR / "SoarUnitTests"
elif (SCRIPT_DIR / "SoarTestAgents").exists():
    AGENT_DIR = SCRIPT_DIR / "SoarTestAgents"
else:
    AGENT_DIR = BUILD_DIR / "UnitTests" / "SoarTestAgents"

class CalledSignal:
    """Helper class to track if callbacks were called."""
    def __init__(self):
        self.called = False
        self.reset()
    
    def reset(self):
        self.called = False


class TestPythonSML(unittest.TestCase):
    """Test cases for Python SML interface."""

    def setUp(self):
        """Set up test fixtures before each test method."""
        # Verify test files exist
        self.towers_of_hanoi_file = AGENT_DIR / 'Chunking' / 'tests' / 'towers-of-hanoi-recursive' / 'towers-of-hanoi-recursive' / 'towers-of-hanoi-recursive_source.soar'
        self.toh_test_file = AGENT_DIR / 'TOHtest.soar'
        
        for source_file in (self.towers_of_hanoi_file, self.toh_test_file):
            self.assertTrue(source_file.is_file(), f"Source file doesn't exist: {source_file}")
        
        # Create kernel
        self.kernel = Kernel.CreateKernelInNewThread()
        self.assertIsNotNone(self.kernel, "Kernel creation failed")
        print('✅ Kernel creation succeeded')
        
        # Set up callback signals
        self.agent_create_called = CalledSignal()
        self.agent_reinit_called = CalledSignal()
        self.agent_destroy_called = CalledSignal()
        self.shutdown_handler_called = CalledSignal()
        self.prod_removed_handler_signal = CalledSignal()
        self.prod_fired_handler_signal = CalledSignal()
        self.phase_executed_handler_signal = CalledSignal()
        self.xml_trace_handler_signal = CalledSignal()
        self.update_handler_called = CalledSignal()
        
        # Register kernel-level callbacks
        self.agentCallbackId0 = self.kernel.RegisterForAgentEvent(smlEVENT_AFTER_AGENT_CREATED, self.agent_created_callback, self.agent_create_called)
        self.agentReinitCallback = self.kernel.RegisterForAgentEvent(smlEVENT_BEFORE_AGENT_REINITIALIZED, self.agent_reinitialized_callback, self.agent_reinit_called)
        self.agentCallbackId2 = self.kernel.RegisterForAgentEvent(smlEVENT_BEFORE_AGENT_DESTROYED, self.agent_destroyed_callback, self.agent_destroy_called)
        self.shutdownCallbackId = self.kernel.RegisterForSystemEvent(smlEVENT_BEFORE_SHUTDOWN, self.system_shutdown_callback, self.shutdown_handler_called)
        self.rhsCallbackId = self.kernel.AddRhsFunction("RhsFunctionTest", self.rhs_function_test, {"foo": "bar"})
        self.updateCallbackId = self.kernel.RegisterForUpdateEvent(smlEVENT_AFTER_ALL_OUTPUT_PHASES, self.update_event_callback, self.update_handler_called)
        
        # Test client message functionality
        messageCallbackId = self.kernel.RegisterForClientMessageEvent("TestMessage", self.user_message_callback, lambda clientName, message: clientName == "TestMessage" and message == "This is a \"quoted\"\" message")
        self.kernel.SendClientMessage(None, "TestMessage", "This is a \"quoted\"\" message")
        self.kernel.UnregisterForClientMessageEvent(messageCallbackId)

    def tearDown(self):
        """Clean up after each test method."""
        if hasattr(self, 'kernel') and self.kernel:
            # Unregister callbacks
            self.kernel.UnregisterForAgentEvent(self.agentCallbackId0)
            self.kernel.UnregisterForAgentEvent(self.agentReinitCallback)
            self.kernel.UnregisterForAgentEvent(self.agentCallbackId2)
            self.kernel.RemoveRhsFunction(self.rhsCallbackId)
            self.kernel.UnregisterForUpdateEvent(self.updateCallbackId)
            
            # Shutdown kernel
            self.assertFalse(self.shutdown_handler_called.called, "❌ Kernel shutdown handler called before shutdown")
            self.kernel.Shutdown()
            self.assertTrue(self.shutdown_handler_called.called, "❌ Kernel shutdown handler not called")
            print("✅ Kernel shutdown")
            
            del self.kernel

    # Callback functions
    def print_callback(self, id, userData, agent, message):
        print(message)

    def production_excised_callback(self, id, signal, agent, prodName, instantiation):
        print(f"removing {prodName} ({instantiation})")
        signal.called = True

    def production_fired_callback(self, id, signal, agent, prodName, instantiation):
        print("fired", prodName)

    def phase_executed_callback(self, id, signal, agent, phase):
        print("phase", phase, "executed")
        signal.called = True

    def agent_created_callback(self, id, signal, agent):
        print(agent.GetAgentName(), "created")
        signal.called = True

    def agent_reinitialized_callback(self, id, signal, agent):
        print(agent.GetAgentName(), "reinitialized")
        signal.called = True

    def agent_destroyed_callback(self, id, signal, agent):
        print("destroying agent", agent.GetAgentName())
        signal.called = True

    def system_shutdown_callback(self, id, signal, kernel):
        print("Shutting down kernel", kernel)
        signal.called = True

    def rhs_function_test(self, id, userData, agent, functionName, argument):
        print(f"Agent {agent.GetAgentName()} called RHS function {functionName} with argument(s) '{argument}' and userData '{userData}'")
        self.assertEqual(argument, "this is a test")
        self.assertEqual(userData, {"foo": "bar"})
        return "success"

    def structured_trace_callback(self, id, signal, agent, pXML):
        print("structured data:", pXML.GenerateXMLString(True))
        signal.called = True

    def update_event_callback(self, id, signal, kernel, runFlags):
        print("update event fired with flags", runFlags)
        signal.called = True

    def user_message_callback(self, id, tester, agent, clientName, message):
        print(f"Agent received usermessage event for clientName '{clientName}' with message '{message}'")
        self.assertTrue(tester(clientName, message), f"❌ UserMessageCallback called with unexpected clientName '{clientName}' or message '{message}'")
        return ""

    def test_agent_creation(self):
        """Test agent creation and callback."""
        self.assertFalse(self.agent_create_called.called)
        agent = self.kernel.CreateAgent('Soar1')
        self.assertIsNotNone(agent, "Agent creation failed")
        self.assertTrue(self.agent_create_called.called)
        print('✅ Agent creation succeeded')
        
        # Clean up
        self.kernel.DestroyAgent(agent)

    def test_full_towers_of_hanoi(self):
        """Test full Towers of Hanoi execution with callbacks."""
        # Create agent
        self.agent_create_called.reset()
        agent = self.kernel.CreateAgent('Soar1')
        self.assertIsNotNone(agent, "Agent creation failed")
        self.assertTrue(self.agent_create_called.called)
        
        # Register agent-level callbacks
        printcallbackid = agent.RegisterForPrintEvent(smlEVENT_PRINT, self.print_callback, None)
        prod_removed_callback_id = agent.RegisterForProductionEvent(smlEVENT_BEFORE_PRODUCTION_REMOVED, self.production_excised_callback, self.prod_removed_handler_signal)
        prod_fired_callback_id = agent.RegisterForProductionEvent(smlEVENT_AFTER_PRODUCTION_FIRED, self.production_fired_callback, self.prod_fired_handler_signal)
        runCallbackId = agent.RegisterForRunEvent(smlEVENT_AFTER_PHASE_EXECUTED, self.phase_executed_callback, self.phase_executed_handler_signal)
        structuredCallbackId = agent.RegisterForXMLEvent(smlEVENT_XML_TRACE_OUTPUT, self.structured_trace_callback, self.xml_trace_handler_signal)
        
        # Load productions
        result = agent.LoadProductions(str(self.towers_of_hanoi_file))
        self.assertIsNotNone(result, "Failed to load towers of hanoi productions")
        
        result = agent.LoadProductions(str(self.toh_test_file))
        self.assertIsNotNone(result, "Failed to load TOH test productions")
        
        # Run some cycles
        agent.RunSelf(2, sml_ELABORATION)
        self.kernel.RunAllAgents(3)
        
        # Set watch level to 0
        result = self.kernel.ExecuteCommandLine("watch 0", "Soar1")
        
        # Test production excision
        self.assertFalse(self.prod_removed_handler_signal.called, "❌ Production excise handler called before excise")
        result = self.kernel.ExecuteCommandLine("excise towers-of-hanoi*monitor*operator-execution*move-disk", "Soar1")
        self.assertTrue(self.prod_removed_handler_signal.called, "❌ Production excise handler not called")
        print("✅ Production excise:", result)
        
        # Run TOH to completion and time it
        start = time.time()
        result = agent.RunSelfForever()
        end = time.time()
        print(f"\nTime in seconds: {end - start}")
        
        # Check RHS function was called
        s1 = self.kernel.ExecuteCommandLine("print s1", "Soar1")
        self.assertIn("^rhstest success", s1, f"❌RHS test FAILED; s1 is {s1}")
        print("\n✅RHS test PASSED")
        
        # Test agent reinitialization
        self.assertFalse(self.agent_reinit_called.called, "❌ Agent reinit handler called before reinit")
        
        # Unregister some callbacks before reinit
        agent.UnregisterForProductionEvent(prod_removed_callback_id)
        agent.UnregisterForProductionEvent(prod_fired_callback_id)
        agent.UnregisterForRunEvent(runCallbackId)
        
        self.kernel.ExecuteCommandLine("init-soar", "Soar1")
        self.assertTrue(self.agent_reinit_called.called, "❌ Agent reinit handler not called")
        print("✅ Agent reinit")
        
        # Test null WME constructors
        self._test_null_wme_constructors(agent)
        
        # Test agent destruction
        self.assertFalse(self.agent_destroy_called.called, "❌ Agent destroy handler called before destroy")
        self.kernel.DestroyAgent(agent)
        self.assertTrue(self.agent_destroy_called.called, "❌ Agent destroy handler not called")
        print("✅ Agent destroy")

    def _test_null_wme_constructors(self, agent):
        """Test passing null arguments to WME constructors."""
        # The logic for safely handling null arguments in the C++ code is not language-specific,
        # so this test does not need to be run in the other SWIG-binding languages
        link = agent.GetInputLink()

        id_wme = link.CreateIdWME("foo")
        link.CreateSharedIdWME(None, id_wme)
        link.CreateIdWME(None)
        link.CreateStringWME(None, None)
        link.CreateIntWME(None, 1)
        link.CreateFloatWME(None, 1.0)

        agent.ExecuteCommandLine("step")

        result_string = agent.ExecuteCommandLine("print I2")
        failure_message = lambda details: f"❌ Pass null to WME constructors: {details}\nResult string was: {result_string}\n"
        
        self.assertIn("^foo F1", result_string, failure_message("CreateIdWME('foo') failed"))
        self.assertIn("^nil F1", result_string, failure_message("CreateSharedIdWME(None, id_wme) failed"))
        self.assertIn("^nil N1", result_string, failure_message("CreateIdWME(None) failed"))
        self.assertIn("^nil nil", result_string, failure_message("CreateStringWME(None, None) failed"))
        self.assertIsNotNone(re.search(r"\^nil 1(?!\.)", result_string), failure_message("CreateIntWME(None) failed"))
        self.assertIn("^nil 1.0", result_string, failure_message("CreateFloatWME(None) failed"))

        print("✅ Pass null to WME constructors")


if __name__ == '__main__':
    # Set up test environment
    print(f"Script directory: {SCRIPT_DIR}")
    print(f"Build directory: {BUILD_DIR}")
    print(f"Agent directory: {AGENT_DIR}")
    print(f"Python build directory: {PYTHON_BUILD_DIR}")
    
    # Check that required directories exist
    if not BUILD_DIR.exists():
        print(f"❌ Build directory does not exist: {BUILD_DIR}")
        sys.exit(1)
    
    if not AGENT_DIR.exists():
        print(f"❌ Agent directory does not exist: {AGENT_DIR}")
        sys.exit(1)
    
    if not PYTHON_BUILD_DIR.exists():
        print(f"[ERROR] Python build directory does not exist: {PYTHON_BUILD_DIR}")
        sys.exit(1)
    
    # Run tests
    unittest.main(verbosity=2)
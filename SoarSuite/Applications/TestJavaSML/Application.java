import sml.Agent;
import sml.Identifier;
import sml.IntElement;
import sml.Kernel;
import sml.StringElement;

/********************************************************************************************
*
* Application.java
* 
* Description:	
* 
* Created on 	Nov 6, 2004
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/

import sml.* ;

/************************************************************************
 * 
 * Simple test app for working out SML (Soar Markup Language) interface through Java
 * 
 ************************************************************************/
public class Application
{
	private Kernel m_Kernel ;

	public static class EventListener implements Agent.RunEventInterface, Agent.PrintEventInterface, Agent.ProductionEventInterface,
												 Agent.xmlEventInterface, Agent.OutputEventInterface, Agent.OutputNotificationInterface,
												 Kernel.AgentEventInterface, Kernel.SystemEventInterface, Kernel.RhsFunctionInterface
	{
		// We'll test to make sure we can keep a ClientXML object that we're passed.
		public ClientXML m_Keep = null ;

		public void runEventHandler(int eventID, Object data, Agent agent, int phase)
		{
			System.out.println("Received run event in Java") ;
			Application me = (Application)data ;
		}

		// We pass back the agent's name because the Java Agent object may not yet
		// exist for this agent yet.  The underlying C++ object *will* exist by the 
		// time this method is called.  So instead we look up the Agent object
		// from the kernel with GetAgent().
		public void agentEventHandler(int eventID, Object data, String agentName)
		{
			System.out.println("Received agent event in Java") ;
			
			Application me = (Application)data ;
			Agent agent = me.m_Kernel.GetAgent(agentName) ;
			
			if (agent == null)
				throw new IllegalStateException("Error in agent event handler -- got event for agent that hasn't been created yet in ClientSML") ;
		}

		public void productionEventHandler(int eventID, Object data, Agent agent, String prodName, String instantiation)
		{
			System.out.println("Received production event in Java") ;
		}
		
		public void systemEventHandler(int eventID, Object data, Kernel kernel)
		{
			System.out.println("Received system event in Java") ;
		}

		public void printEventHandler(int eventID, Object data, Agent agent, String message)
		{
			System.out.println("Received print event in Java: " + message) ;
		}
		
		public void outputNotificationHandler(Object data, Agent agent)
		{
			System.out.println("Received an output notification in Java") ;
		}

		public void xmlEventHandler(int eventID, Object data, Agent agent, ClientXML xml)
		{
			String xmlText = xml.GenerateXMLString(true) ;
			System.out.println("Received xml trace event in Java: " + xmlText) ;

			String allChildren = "" ;
			
			if (xml.GetNumberChildren() > 0)
			{
				ClientXML child = new ClientXML() ;
				xml.GetChild(child, 0) ;
				
				String childText = child.GenerateXMLString(true) ;
				allChildren += childText ;
				
				child.delete() ;
			}
			
			if (m_Keep != null)
				m_Keep.delete() ;
			
			m_Keep = new ClientXML(xml) ;
			
			// No need to explicitly delete xml now.  The Java version is like C++
			// where you need to copy the object to keep it, which means you don't need to
			// explicitly delete it.
			//xml.delete() ;
		}

		public String rhsFunctionHandler(int eventID, Object data, String agentName, String functionName, String argument)
		{
			System.out.println("Received rhs function event in Java for function: " + functionName + "(" + argument + ")") ;
			return "My rhs result " + argument ;
		}
		
		public void outputEventHandler(Object data, String agentName, String attributeName, WMElement pWmeAdded)
		{
			System.out.println("Received output event in Java for wme: " + pWmeAdded.GetIdentifierName() + " ^" + pWmeAdded.GetAttribute() + " " + pWmeAdded.GetValueAsString()) ;
			
			// Retrieve the application class pointer (to test our user data was passed successfully) and then print out the tree of wmes from the wme added down.
			Application app = (Application)data ;
			app.printWMEs(pWmeAdded) ;
		}
	}
	
	public void printWMEs(WMElement pRoot)
	{
		if (pRoot.GetParent() == null)
			System.out.println("Top Identifier " + pRoot.GetValueAsString()) ;
		else
		{
			System.out.println("(" + pRoot.GetParent().GetIdentifierSymbol() + " ^" + pRoot.GetAttribute() + " " + pRoot.GetValueAsString() + ")") ;
		}

		if (pRoot.IsIdentifier())
		{
			// NOTE: Can't just cast to (Identifier) in Java because the underlying C++ object needs to be cast
			// so we use a conversion method.
			Identifier pID = pRoot.ConvertToIdentifier() ;
			int size = pID.GetNumberChildren() ;

			for (int i = 0 ; i < size ; i++)
			{
				WMElement pWME = pID.GetChild(i) ;

				printWMEs(pWME) ;
			}
		}
	}
	
	private void Test()
	{
		// Make sure the kernel was ok
		if (m_Kernel.HadError())
			throw new IllegalStateException("Error initializing kernel: " + m_Kernel.GetLastErrorDescription()) ;
		
		String version = m_Kernel.GetSoarKernelVersion() ;
		System.out.println("Soar version " + version) ;
		
		// Create an agent
		Agent agent = m_Kernel.CreateAgent("javatest") ;
		Agent pAgent = agent ;	// So it's easier copying existing C++ code here
		Kernel pKernel = m_Kernel ;

		// We test the kernel for an error after creating an agent as the agent
		// object may not be properly constructed if the create call failed so
		// we store errors in the kernel in this case.
		if (m_Kernel.HadError())
			throw new IllegalStateException("Error creating agent: " + m_Kernel.GetLastErrorDescription()) ;
		
		String cwd = agent.ExecuteCommandLine("pwd") ;
		String path = pKernel.GetLibraryLocation() ;
		path += "/../Tests/testjavasml.soar" ;
		
		// Load some productions
		boolean load = agent.LoadProductions(path) ;

		if (!load || agent.HadError())
			throw new IllegalStateException("Error loading productions from testjavasml.soar: " + agent.GetLastErrorDescription()) ;

		Identifier pInputLink = agent.GetInputLink() ;

		if (pInputLink == null)
			throw new IllegalStateException("Error getting the input link") ;

		// Some random adds
		Identifier pID = pAgent.CreateIdWME(pInputLink, "plane") ;
		StringElement pWME1 = pAgent.CreateStringWME(pID, "type", "Boeing747") ;
		IntElement pWME2    = pAgent.CreateIntWME(pID, "speed", 200) ;
		
		// Then add some tic tac toe stuff which should trigger output
		Identifier pSquare = pAgent.CreateIdWME(pInputLink, "square") ;
		StringElement pEmpty = pAgent.CreateStringWME(pSquare, "content", "RANDOM") ;
		IntElement pRow = pAgent.CreateIntWME(pSquare, "row", 1) ;
		IntElement pCol = pAgent.CreateIntWME(pSquare, "col", 2) ;
		
		boolean ok = pAgent.Commit() ;

		// Quick test of init-soar
		pAgent.InitSoar() ;

		// Update the square's value to be empty.  This ensures that the update
		// call is doing something.  Otherwise, when we run we won't get a match.
		pAgent.Update(pEmpty, "EMPTY") ;
		ok = pAgent.Commit() ;
		
		/*******************************************************
		// Register some event handlers
		// Each call takes an object from a class that implements a handler method
		// (E.g. listener has "runEventHandler").
		// The type for that method is fixed by SML (there are examples above for each)
		// I'm using one listener class for all events but each could be in its own class or they
		// could be methods in "this" class.
		// The last parameter is an arbitrary data object which will be passed back to us in the callback.
		// I'm just passing in "this" but it could be anything.
		// The integer we get back is only required when we unregister the handler.
		********************************************************/
		EventListener listener = new EventListener() ;
		int jRunCallback    = pAgent.RegisterForRunEvent(smlRunEventId.smlEVENT_AFTER_DECISION_CYCLE, listener, this) ;		
		int jProdCallback   = pAgent.RegisterForProductionEvent(smlProductionEventId.smlEVENT_AFTER_PRODUCTION_FIRED, listener, this) ;		
		int jPrintCallback  = pAgent.RegisterForPrintEvent(smlPrintEventId.smlEVENT_PRINT, listener, this) ;		
		int jSystemCallback = pKernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_AFTER_RESTART, listener, this) ;		
		int jSystemCallback2 = pKernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, listener, this) ;		
		int jAgentCallback  = pKernel.RegisterForAgentEvent(smlAgentEventId.smlEVENT_BEFORE_AGENT_REINITIALIZED, listener, this) ;		
		int jRhsCallback    = pKernel.AddRhsFunction("test-rhs", listener, this) ;
		int jTraceCallback  = pAgent.RegisterForXMLEvent(smlXMLEventId.smlEVENT_XML_TRACE_OUTPUT, listener, this) ;
		int jOutputCallback = pAgent.AddOutputHandler("move", listener, this) ;
		int jOutputNotification = pAgent.RegisterForOutputNotification(listener, this) ;		
		
		// Trigger an agent event by doing init-soar
		pAgent.InitSoar() ;

		//pAgent.ExecuteCommandLine("watch 5") ;
		
		// Now we should match (if we really loaded the tictactoe example rules) and so generate some real output
		String trace = pAgent.RunSelfTilOutput(20) ;	// Should just cause Soar to run a decision or two (this is a test that run til output works stops at output)

		System.out.println(trace) ;

		// Trigger an agent event by doing init-soar
		pAgent.InitSoar() ;

		// Having rolled everything back we should be able to run forward again
		// This tests that initSoar really works.
		String trace1 = pAgent.RunSelfTilOutput(20) ;

		System.out.println(trace1) ;

		// See if our RHS function worked
		String value = pAgent.GetOutputLink().GetParameterValue("test") ;
		
		if (value == null)
			throw new IllegalStateException("RHS function failed to set a value for ^test") ;
		
		System.out.println("Value of ^test (via RHS function) is: " + value) ;
		
		// Now stress things a little
		// String traceLong = pAgent.Run(500) ;

		if (listener.m_Keep == null)
			throw new IllegalStateException("The xmlEventHandler wasn't called (so m_Keep is null)") ;

		// Try to access m_Keep to make sure the underlying XML object's not been deleted
		String testXML = listener.m_Keep.GenerateXMLString(true) ;
		System.out.println("Last trace message: " + testXML) ;
		listener.m_Keep.delete() ;
		
		System.out.println("Unregister callbacks") ;
		
		// Unregister our callbacks
		// (This isn't required, I'm just testing that it works)
		pAgent.UnregisterForXMLEvent(jTraceCallback) ;
		pAgent.UnregisterForRunEvent(jRunCallback) ;
		pAgent.UnregisterForProductionEvent(jProdCallback) ;
		pAgent.UnregisterForPrintEvent(jPrintCallback) ;
		pAgent.UnregisterForOutputNotification(jOutputNotification) ;
		pAgent.RemoveOutputHandler(jOutputCallback) ;
		pKernel.UnregisterForSystemEvent(jSystemCallback) ;
		pKernel.UnregisterForSystemEvent(jSystemCallback2) ;
		pKernel.UnregisterForAgentEvent(jAgentCallback) ;
		pKernel.RemoveRhsFunction(jRhsCallback) ;
		
		//String trace2 = pAgent.RunSelfTilOutput(20) ;
		//System.out.println(trace2) ;
		
		// Clean up
		m_Kernel.Shutdown() ;
		
		// Delete the kernel, releasing all of the owned objects (hope this works ok in Java...)
		m_Kernel.delete() ;
	}
	
	//	 We create a file to say we succeeded or not, deleting any existing results beforehand
	//	 The filename shows if things succeeded or not and the contents can explain further.
	public void reportResult(String testName, boolean success, String msg)
	{
		try
		{
			// Decide on the filename to use for success/failure
			String kSuccess = testName + "-success.txt" ;
			String kFailure = testName + "-failure.txt" ;
	
			// Remove any existing result files
			java.io.File successFile = new java.io.File(kSuccess) ;
			java.io.File failFile = new java.io.File(kFailure) ;
			
			successFile.delete() ;
			failFile.delete() ;
	
			// Create the output file
			java.io.FileWriter outfile = new java.io.FileWriter(success ? successFile : failFile) ;
	
			if (success)
			{
				outfile.write("Tests SUCCEEDED") ;
				outfile.write(msg) ;
				System.out.println("Tests SUCCEEDED") ;
			}
			else
			{
				outfile.write("ERROR *** Tests FAILED");
				outfile.write(msg) ;
				System.out.println("ERROR *** Tests FAILED") ;
				System.out.println(msg) ;
			}
	
			outfile.close();
		}
		catch (Exception e)
		{
			System.out.println("Exception occurred when writing results to file") ;
		}
	}

	private void TestListener() throws Exception
	{
		// Sleep for 20 seconds so others can connect to us if they wish to test that
		// (for this part we'll use the default port)
		System.out.println("############ Listener Test ############") ;

		m_Kernel = Kernel.CreateKernelInCurrentThread("SoarKernelSML", false, Kernel.GetDefaultPort()) ;
		//m_Kernel.SetTraceCommunications(true) ;
		
		for (int i = 0 ; i < 200 ; i++)
		{
			//System.out.println("Checking " + i) ;
			m_Kernel.CheckForIncomingCommands() ;
			Thread.sleep(100) ;
		}
		
		m_Kernel.delete() ;		
	}
	
	private void TestRemote() throws Exception
	{
		// Now repeat the tests but with a kernel that's running in a different process (needs a listener to exist for this)
		System.out.println("############ Remote Test ############") ;

		// Initialize the remote kernel
		m_Kernel = Kernel.CreateRemoteConnection(true, null, Kernel.GetDefaultPort()) ;

		if (m_Kernel.HadError())
		{
			System.out.println("Failed to connect to the listener...was one running?") ;
			return ;
		}
		
		Test() ;		
	}
	
	public Application(String[] args)
	{
		boolean success = true ;
		String  msg = "" ;
		
		boolean remote = false ;
		boolean listener = false ;
		for (int arg = 0 ; arg < args.length ; arg++)
		{
			if (args[arg].equals("-remote"))
				remote = true ;
			if (args[arg].equals("-listener"))
				listener = true ;
		}
		
		try
		{
			boolean localTests = !remote && !listener ;
			
			if (localTests)
			{
				// Now repeat the tests but with a kernel that's running in a different thread
				System.out.println("############ Current Thread ############") ;

				// Initialize the kernel
				m_Kernel = Kernel.CreateKernelInCurrentThread("SoarKernelSML", false, 12345) ;
		
				Test() ;
				
				// Now repeat the tests but with a kernel that's running in a different thread
				System.out.println("############ New Thread ############") ;
				
				m_Kernel = Kernel.CreateKernelInNewThread("SoarKernelSML", 13131) ;

				Test() ;
			}
			
			if (remote)
				TestRemote() ;
			else if (listener)
				TestListener() ;
		}
		catch (Throwable t)
		{
			success = false ;
			msg = t.toString() ;
		}
		finally
		{
			reportResult("testjavasml", success, msg) ;
		}
	}
	
	public static void main(String[] args)
	{
		new Application(args);
	}
}

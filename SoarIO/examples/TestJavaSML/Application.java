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

	public static class EventListener
	{
		public void runEventHandler(int eventID, Object data, Agent agent, int phase)
		{
			System.out.println("Received run event in Java") ;
			Application me = (Application)data ;
			int x = 1 ;
		}

		public void agentEventHandler(int eventID, Object data, Agent agent)
		{
			System.out.println("Received agent event in Java") ;
		}

		public void productionEventHandler(int eventID, Object data, Agent agent, String prodName, String instantiation)
		{
			System.out.println("Received production event in Java") ;
		}
		
		public void kernelEventHandler(int eventID, Object data, Kernel kernel)
		{
			System.out.println("Received kernel event in Java") ;
		}
	}
	
	private void Test()
	{
		// Make sure the kernel was ok
		if (m_Kernel.HadError())
			throw new IllegalStateException("Error initializing kernel: " + m_Kernel.GetLastErrorDescription()) ;
		
		// Create an agent
		Agent agent = m_Kernel.CreateAgent("javatest") ;
		Agent pAgent = agent ;	// So it's easier copying existing C++ code here
		Kernel pKernel = m_Kernel ;

		if (m_Kernel.HadError())
			throw new IllegalStateException("Error creating agent: " + m_Kernel.GetLastErrorDescription()) ;

		// Load some productions
		boolean load = agent.LoadProductions("testsml.soar") ;

		if (!load || agent.HadError())
			throw new IllegalStateException("Error loading productions from testsml.soar: " + agent.GetLastErrorDescription()) ;

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

		// Update the square's value to be empty.  This ensures that the update
		// call is doing something.  Otherwise, when we run we won't get a match.
		pAgent.Update(pEmpty, "EMPTY") ;
		ok = pAgent.Commit() ;
		
		// Register some event handlers
		EventListener listener = new EventListener() ;
		int jRunCallback   = pAgent.RegisterForRunEventByHand(pAgent, smlEventId.smlEVENT_AFTER_DECISION_CYCLE, listener, "runEventHandler", this) ;		
		int jAgentCallback = pAgent.RegisterForAgentEventByHand(pAgent, smlEventId.smlEVENT_BEFORE_AGENT_REINITIALIZED, listener, "agentEventHandler", this) ;		
		int jProdCallback   = pAgent.RegisterForProductionEventByHand(pAgent, smlEventId.smlEVENT_AFTER_PRODUCTION_FIRED, listener, "productionEventHandler", this) ;		
		int jSystemCallback = pKernel.RegisterForSystemEventByHand(pKernel, smlEventId.smlEVENT_AFTER_RESTART, listener, "systemEventHandler", this) ;		

		// Now we should match (if we really loaded the tictactoe example rules) and so generate some real output
		String trace = pAgent.RunTilOutput(20) ;	// Should just cause Soar to run a decision or two (this is a test that run til output works stops at output)

		System.out.println(trace) ;

		String trace1 = pAgent.RunTilOutput(20) ;

		System.out.println(trace1) ;
		
		// Trigger an agent event by doing init-soar
		pAgent.InitSoar() ;

		System.out.println("Unregister callbacks") ;
		
		// Unregister our callbacks
		// (This isn't required, I'm just testing that it works)
		
		pAgent.UnregisterForRunEventByHand(smlEventId.smlEVENT_AFTER_DECISION_CYCLE, jRunCallback) ;
		pAgent.UnregisterForAgentEventByHand(smlEventId.smlEVENT_BEFORE_AGENT_REINITIALIZED, jAgentCallback) ;
		pAgent.UnregisterForProductionEventByHand(smlEventId.smlEVENT_AFTER_PRODUCTION_FIRED, jProdCallback) ;
		pKernel.UnregisterForSystemEventByHand(smlEventId.smlEVENT_AFTER_RESTART, jSystemCallback) ;
		
		String trace2 = pAgent.RunTilOutput(20) ;
		System.out.println(trace2) ;
		
		pAgent.InitSoar() ;

		// Clean up
		ok = m_Kernel.DestroyAgent(pAgent) ;
		
		// Delete the kernel, releasing all of the owned objects (hope this works ok in Java...)
		m_Kernel.delete() ;
	}
	
	public Application()
	{
		// It looks like SWIG requires us to explicitly load the library
		// before we call any methods, so we'll do that here.
        System.loadLibrary("Java_sml_ClientInterface");

        // Initialize the kernel
		m_Kernel = Kernel.CreateEmbeddedConnection("KernelSML", true, false, 0) ;
		
		Test() ;
	}
	
	public static void main(String[] args)
	{
		new Application();
	}
}

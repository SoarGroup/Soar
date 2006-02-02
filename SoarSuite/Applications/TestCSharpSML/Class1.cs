//////////////////////////////////////////////////////////////////////////
//
//	Sample C# Soar application
//
//  Douglas Pearson,  Jan 2006.
//
// In order to build a Soar C# application, the application needs a reference to smlCSharp.
// To add that reference, right click on the references folder (in solution view),
// select "add reference...", then "browse..." to locate the smlCSharp.dll located in SoarLibrary.
//
//////////////////////////////////////////////////////////////////////////

using System;
using System.Runtime.InteropServices;
using sml ;

namespace TestCSharpSML
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Class1
	{
		static void MyRunEventCallback(sml.smlRunEventId eventID, IntPtr callbackData, IntPtr agentPtr, sml.smlPhase phase)
		{
			// Retrieve the original object reference from the GCHandle which is used to pass the value safely to and from C++ (unsafe/unmanaged) code.
			sml.Agent agent = (sml.Agent)((GCHandle)agentPtr).Target ;

			// Retrieve arbitrary data from callback data object (note data here can be null, but the wrapper object callbackData won't be so this call is safe)
			// This field's usage is up to the user and passed in during registation call and passed back here.  Can be used to provide context.
			Object userData = (Object)((GCHandle)callbackData).Target ;

			String name = agent.GetAgentName() ;
			System.Console.Out.WriteLine(eventID + " agent " + name + " in phase " + phase + " with user data " + userData) ;
		}

		static void MyProductionEventCallback(sml.smlProductionEventId eventID, IntPtr callbackData, IntPtr agentPtr, String productionName, String instantiation)
		{
			// Retrieve the original object reference from the GCHandle which is used to pass the value safely to and from C++ (unsafe/unmanaged) code.
			sml.Agent agent = (sml.Agent)((GCHandle)agentPtr).Target ;

			// Retrieve arbitrary data from callback data object (note data here can be null, but the wrapper object callbackData won't be so this call is safe)
			// This field's usage is up to the user and passed in during registation call and passed back here.  Can be used to provide context.
			Object userData = (Object)((GCHandle)callbackData).Target ;

			String name = agent.GetAgentName() ;
			System.Console.Out.WriteLine(eventID + " agent " + name + " production " + productionName + " instantiation " + instantiation + " with user data " + userData) ;
		}

		static void MyAgentEventCallback(sml.smlAgentEventId eventID, IntPtr callbackData, IntPtr kernelPtr, String agentName)
		{
			// Retrieve the original object reference from the GCHandle which is used to pass the value safely to and from C++ (unsafe/unmanaged) code.
			sml.Kernel kernel = (sml.Kernel)((GCHandle)kernelPtr).Target ;

			// Retrieve arbitrary data from callback data object (note data here can be null, but the wrapper object callbackData won't be so this call is safe)
			// This field's usage is up to the user and passed in during registation call and passed back here.  Can be used to provide context.
			Object userData = (Object)((GCHandle)callbackData).Target ;

			// This callback returns the name of the agent as a string, to avoid having SWIG have to lookup the C# agent object
			// and pass that back.  We do something similar in Java for the same reasons.
			sml.Agent agent = kernel.GetAgent(agentName) ;
			
			if (agent == null)
				throw new Exception("Error looking up agent in callback") ;

			System.Console.Out.WriteLine(eventID + " agent " + agentName + " with user data " + userData) ;
		}

		static void MySystemEventCallback(sml.smlSystemEventId eventID, IntPtr callbackData, IntPtr kernelPtr)
		{
			// Retrieve the original object reference from the GCHandle which is used to pass the value safely to and from C++ (unsafe/unmanaged) code.
			sml.Kernel kernel = (sml.Kernel)((GCHandle)kernelPtr).Target ;

			// Retrieve arbitrary data from callback data object (note data here can be null, but the wrapper object callbackData won't be so this call is safe)
			// This field's usage is up to the user and passed in during registation call and passed back here.  Can be used to provide context.
			Object userData = (Object)((GCHandle)callbackData).Target ;

			System.Console.Out.WriteLine(eventID + " kernel " + kernel + " with user data " + userData) ;
		}

		static void MyPrintEventCallback(sml.smlPrintEventId eventID, IntPtr callbackData, IntPtr agentPtr, String message)
		{
			// Retrieve the original object reference from the GCHandle which is used to pass the value safely to and from C++ (unsafe/unmanaged) code.
			sml.Agent agent = (sml.Agent)((GCHandle)agentPtr).Target ;

			// Retrieve arbitrary data from callback data object (note data here can be null, but the wrapper object callbackData won't be so this call is safe)
			// This field's usage is up to the user and passed in during registation call and passed back here.  Can be used to provide context.
			Object userData = (Object)((GCHandle)callbackData).Target ;

			System.Console.Out.WriteLine(eventID + " agent " + agent.GetAgentName() + " message " + message + " with user data " + userData) ;
		}

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			Class1 test = new Class1() ;

			bool result = false ;
			try
			{
				result = test.Test() ;
			}
			catch (Exception ex)
			{
				System.Console.Out.WriteLine(ex) ;
			}

			System.Console.Out.WriteLine("-----------------------------") ;
			if (result)
				System.Console.Out.WriteLine("Tests passed") ;
			else
				System.Console.Out.WriteLine("Tests failed") ;

			System.Console.Out.WriteLine("Press return to exit") ;
			System.Console.In.ReadLine() ;
		}

		bool Test()
		{
			//
			// TODO: Add code to start application here
			//
			sml.Kernel kernel = sml.Kernel.CreateKernelInNewThread("SoarKernelSML") ;

			// Make sure the kernel was ok
			if (kernel.HadError())
				throw new Exception("Error initializing kernel: " + kernel.GetLastErrorDescription()) ;

			sml.Agent agent = kernel.CreateAgent("First") ;

			// We test the kernel for an error after creating an agent as the agent
			// object may not be properly constructed if the create call failed so
			// we store errors in the kernel in this case.  Once this create is done we can work directly with the agent.
			if (kernel.HadError())
				throw new Exception("Error creating agent: " + kernel.GetLastErrorDescription()) ;

			bool ok = agent.LoadProductions("..\\Tests\\testcsharpsml.soar") ;
			if (!ok)
			{
				System.Console.Out.WriteLine("Load failed") ; 
				return false ;
			}

			Agent pAgent = agent ;	// So it's easier copying existing C++ code here
			Kernel pKernel = kernel ;

			Identifier pInputLink = agent.GetInputLink() ;

			if (pInputLink == null)
				throw new Exception("Error getting the input link") ;

			// Some random adds
			Identifier pID = pAgent.CreateIdWME(pInputLink, "plane") ;
			StringElement pWME1 = pAgent.CreateStringWME(pID, "type", "Boeing747") ;
			IntElement pWME2    = pAgent.CreateIntWME(pID, "speed", 200) ;
		
			// Then add some tic tac toe stuff which should trigger output
			Identifier pSquare = pAgent.CreateIdWME(pInputLink, "square") ;
			StringElement pEmpty = pAgent.CreateStringWME(pSquare, "content", "RANDOM") ;
			IntElement pRow = pAgent.CreateIntWME(pSquare, "row", 1) ;
			IntElement pCol = pAgent.CreateIntWME(pSquare, "col", 2) ;
		
			ok = pAgent.Commit() ;

			// Quick test of init-soar
			pAgent.InitSoar() ;

			// Update the square's value to be empty.  This ensures that the update
			// call is doing something.  Otherwise, when we run we won't get a match.
			pAgent.Update(pEmpty, "EMPTY") ;
			ok = pAgent.Commit() ;

			String myTestData = "my data" ;
			sml.Agent.RunEventCallback runCall = new sml.Agent.RunEventCallback(MyRunEventCallback) ;			
			sml.Agent.ProductionEventCallback prodCall = new sml.Agent.ProductionEventCallback(MyProductionEventCallback) ;			
			sml.Agent.PrintEventCallback printCall = new sml.Agent.PrintEventCallback(MyPrintEventCallback) ;			
			sml.Kernel.AgentEventCallback agentCall = new sml.Kernel.AgentEventCallback(MyAgentEventCallback) ;			
			sml.Kernel.SystemEventCallback systemCall = new sml.Kernel.SystemEventCallback(MySystemEventCallback) ;			

			int runCallbackID	= agent.RegisterForRunEvent(sml.smlRunEventId.smlEVENT_AFTER_DECISION_CYCLE, runCall, myTestData) ;
			int prodCallbackID	= agent.RegisterForProductionEvent(sml.smlProductionEventId.smlEVENT_AFTER_PRODUCTION_FIRED, prodCall, myTestData) ;
			int printCallbackID	= agent.RegisterForPrintEvent(sml.smlPrintEventId.smlEVENT_PRINT, printCall, myTestData) ;
			int agentCallbackID	= kernel.RegisterForAgentEvent(sml.smlAgentEventId.smlEVENT_BEFORE_AGENT_REINITIALIZED, agentCall, myTestData) ;
			int systemCallbackID= kernel.RegisterForSystemEvent(sml.smlSystemEventId.smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED, systemCall, myTestData) ;

			// Trigger some run and production events
			agent.RunSelf(3) ;

			// Trigger an agent event
			agent.InitSoar() ;

			ok = agent.UnregisterForRunEvent(runCallbackID) ;
			ok = ok && agent.UnregisterForProductionEvent(prodCallbackID) ;
			ok = ok && agent.UnregisterForPrintEvent(printCallbackID) ;
			ok = ok && kernel.UnregisterForAgentEvent(agentCallbackID) ;
			ok = ok && kernel.UnregisterForSystemEvent(systemCallbackID) ;

			if (!ok)
			{
				System.Console.Out.WriteLine("Failed to unregister an event") ;
				return false ;
			}

			// Send notifications that kernel is about to be deleted
			kernel.Shutdown() ;

			// C# delete the kernel explicitly
			kernel.Dispose() ;

			return ok ;
		}
	}
}

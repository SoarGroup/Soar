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

		static void MyXMLEventCallback(sml.smlXMLEventId eventID, IntPtr callbackData, IntPtr agentPtr, IntPtr pXML)
		{
			// Retrieve the original object reference from the GCHandle which is used to pass the value safely to and from C++ (unsafe/unmanaged) code.
			sml.Agent agent = (sml.Agent)((GCHandle)agentPtr).Target ;

			// Retrieve arbitrary data from callback data object (note data here can be null, but the wrapper object callbackData won't be so this call is safe)
			// This field's usage is up to the user and passed in during registation call and passed back here.  Can be used to provide context.
			Object userData = (Object)((GCHandle)callbackData).Target ;

			// Retrieve the xml object.  We don't own this object, so to keep it we'd need to copy it (or its contents).
			// This way when C# deallocated the object the underlying C++ object isn't deleted either (which is correct as the corresponding C++ code
			// doesn't pass ownership in the equivalent callback either).
			sml.ClientXML xml = (sml.ClientXML)((GCHandle)pXML).Target ;

			// Convert the XML to string form so we can look at it.
			String xmlString = xml.GenerateXMLString(true) ;

			String name = agent.GetAgentName() ;
			System.Console.Out.WriteLine(eventID + " agent " + name + " xml " + xmlString) ;
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

		static void MyUpdateEventCallback(sml.smlUpdateEventId eventID, IntPtr callbackData, IntPtr kernelPtr, smlRunFlags runFlags)
		{
			// Retrieve the original object reference from the GCHandle which is used to pass the value safely to and from C++ (unsafe/unmanaged) code.
			sml.Kernel kernel = (sml.Kernel)((GCHandle)kernelPtr).Target ;

			// Retrieve arbitrary data from callback data object (note data here can be null, but the wrapper object callbackData won't be so this call is safe)
			// This field's usage is up to the user and passed in during registation call and passed back here.  Can be used to provide context.
			Object userData = (Object)((GCHandle)callbackData).Target ;

			System.Console.Out.WriteLine(eventID + " kernel " + kernel + " run flags " + runFlags + " with user data " + userData) ;
		}

		static void MyStringEventCallback(sml.smlStringEventId eventID, IntPtr callbackData, IntPtr kernelPtr, String str)
		{
			// Retrieve the original object reference from the GCHandle which is used to pass the value safely to and from C++ (unsafe/unmanaged) code.
			sml.Kernel kernel = (sml.Kernel)((GCHandle)kernelPtr).Target ;

			// Retrieve arbitrary data from callback data object (note data here can be null, but the wrapper object callbackData won't be so this call is safe)
			// This field's usage is up to the user and passed in during registation call and passed back here.  Can be used to provide context.
			Object userData = (Object)((GCHandle)callbackData).Target ;

			System.Console.Out.WriteLine(eventID + " kernel " + kernel + " string " + str + " with user data " + userData) ;
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

		static void MyOutputEventCallback(IntPtr callbackData, IntPtr agentPtr, String commandName, IntPtr outputWME)
		{
			// Retrieve the original object reference from the GCHandle which is used to pass the value safely to and from C++ (unsafe/unmanaged) code.
			sml.Agent agent = (sml.Agent)((GCHandle)agentPtr).Target ;

			// Retrieve arbitrary data from callback data object (note data here can be null, but the wrapper object callbackData won't be so this call is safe)
			// This field's usage is up to the user and passed in during registation call and passed back here.  Can be used to provide context.
			Object userData = (Object)((GCHandle)callbackData).Target ;

			// Retrieve the wme.  We don't own this object, so to keep it we'd need to copy it (or its contents).
			// This way when C# deallocated the object the underlying C++ object isn't deleted either (which is correct as the corresponding C++ code
			// doesn't pass ownership in the equivalent callback either).
			sml.WMElement wme = (sml.WMElement)((GCHandle)outputWME).Target ;

			String id  = wme.GetIdentifier().GetIdentifierSymbol() ;
			String att = wme.GetAttribute() ;
			String val = wme.GetValueAsString() ;

			System.Console.Out.WriteLine("Output received under ^move attribute for agent " + agent.GetAgentName() + " output wme " + id + " ^" + att + " " + val) ;
		}

		static void MyOutputNotificationCallback(IntPtr callbackData, IntPtr agentPtr)
		{
			// Retrieve the original object reference from the GCHandle which is used to pass the value safely to and from C++ (unsafe/unmanaged) code.
			sml.Agent agent = (sml.Agent)((GCHandle)agentPtr).Target ;

			// Retrieve arbitrary data from callback data object (note data here can be null, but the wrapper object callbackData won't be so this call is safe)
			// This field's usage is up to the user and passed in during registation call and passed back here.  Can be used to provide context.
			Object userData = (Object)((GCHandle)callbackData).Target ;

			System.Console.Out.WriteLine("Agent " + agent.GetAgentName() + " has just received some output") ;

			for (int i = 0 ; i < agent.GetNumberOutputLinkChanges() ; i++)
			{
				sml.WMElement wme = agent.GetOutputLinkChange(i) ;
				System.Console.Out.WriteLine("Output wme was " + wme.GetIdentifier().GetIdentifierSymbol() + " ^" + wme.GetAttribute() + " " + wme.GetValueAsString()) ;
			}
		}

		static String MyTestRhsFunction(sml.smlRhsEventId eventID, IntPtr callbackData, IntPtr kernelPtr, String agentName, String functionName, String argument)
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

			System.Console.Out.WriteLine(eventID + " agent " + agentName + " function " + functionName + " arg " + argument) ;

			// This is the result of the RHS function and can be placed into working memory as the result of the call
			// (whether this happens or not depends on how the RHS function is used in the production rule).
			return "result" ;
		}

		static String MyTestClientMessageCallback(sml.smlRhsEventId eventID, IntPtr callbackData, IntPtr kernelPtr, String agentName, String clientName, String message)
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

			System.Console.Out.WriteLine(eventID + " agent " + agentName + " client " + clientName + " message " + message) ;

			return "result" ;
		}

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			//System.Console.Out.WriteLine("Press return to start") ;
			//System.Console.In.ReadLine() ;

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
			sml.Agent.OutputEventCallback outputCall = new sml.Agent.OutputEventCallback(MyOutputEventCallback) ;
			sml.Agent.XMLEventCallback xmlCall		 = new sml.Agent.XMLEventCallback(MyXMLEventCallback) ;
			sml.Agent.OutputNotificationCallback noteCall = new sml.Agent.OutputNotificationCallback(MyOutputNotificationCallback) ;
			sml.Kernel.AgentEventCallback agentCall = new sml.Kernel.AgentEventCallback(MyAgentEventCallback) ;
			sml.Kernel.SystemEventCallback systemCall = new sml.Kernel.SystemEventCallback(MySystemEventCallback) ;	
			sml.Kernel.UpdateEventCallback updateCall = new sml.Kernel.UpdateEventCallback(MyUpdateEventCallback) ;
			sml.Kernel.StringEventCallback strCall    = new sml.Kernel.StringEventCallback(MyStringEventCallback) ;
			sml.Kernel.RhsFunction rhsCall			  = new sml.Kernel.RhsFunction(MyTestRhsFunction) ;
			sml.Kernel.ClientMessageCallback clientCall	= new sml.Kernel.ClientMessageCallback(MyTestClientMessageCallback) ;

			int runCallbackID	= agent.RegisterForRunEvent(sml.smlRunEventId.smlEVENT_AFTER_DECISION_CYCLE, runCall, myTestData) ;
			int prodCallbackID	= agent.RegisterForProductionEvent(sml.smlProductionEventId.smlEVENT_AFTER_PRODUCTION_FIRED, prodCall, myTestData) ;
			int printCallbackID	= agent.RegisterForPrintEvent(sml.smlPrintEventId.smlEVENT_PRINT, printCall, myTestData) ;
			int outputCallbackID= agent.AddOutputHandler("move", outputCall, myTestData) ;
			int noteCallbackID  = agent.RegisterForOutputNotification(noteCall, myTestData) ;
			int xmlCallbackID   = agent.RegisterForXMLEvent(sml.smlXMLEventId.smlEVENT_XML_TRACE_OUTPUT, xmlCall, myTestData) ;
			int agentCallbackID	= kernel.RegisterForAgentEvent(sml.smlAgentEventId.smlEVENT_BEFORE_AGENT_REINITIALIZED, agentCall, myTestData) ;
			int systemCallbackID= kernel.RegisterForSystemEvent(sml.smlSystemEventId.smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED, systemCall, myTestData) ;
			int updateCallbackID= kernel.RegisterForUpdateEvent(sml.smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, updateCall, myTestData) ;
			int stringCallbackID= kernel.RegisterForStringEvent(sml.smlStringEventId.smlEVENT_EDIT_PRODUCTION, strCall, myTestData) ;
			int rhsCallbackID	= kernel.AddRhsFunction("test-rhs", rhsCall, myTestData) ;
			int clientCallbackID= kernel.RegisterForClientMessageEvent("test-client", clientCall, myTestData) ;

			// Running the agent will trigger most of the events we're listening for so
			// we can check that they're working correctly.
			agent.RunSelf(3) ;
			//kernel.RunAllAgents(3) ;

			// Trigger an agent event
			agent.InitSoar() ;

			ok = agent.UnregisterForRunEvent(runCallbackID) ;
			ok = ok && agent.UnregisterForProductionEvent(prodCallbackID) ;
			ok = ok && agent.UnregisterForPrintEvent(printCallbackID) ;
			ok = ok && agent.RemoveOutputHandler(outputCallbackID) ;
			ok = ok && agent.UnregisterForOutputNotification(noteCallbackID) ;
			ok = ok && agent.UnregisterForXMLEvent(xmlCallbackID) ;
			ok = ok && kernel.UnregisterForAgentEvent(agentCallbackID) ;
			ok = ok && kernel.UnregisterForSystemEvent(systemCallbackID) ;
			ok = ok && kernel.UnregisterForUpdateEvent(updateCallbackID) ;
			ok = ok && kernel.UnregisterForStringEvent(stringCallbackID) ;
			ok = ok && kernel.RemoveRhsFunction(rhsCallbackID) ;
			ok = ok && kernel.UnregisterForClientMessageEvent(clientCallbackID) ;

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

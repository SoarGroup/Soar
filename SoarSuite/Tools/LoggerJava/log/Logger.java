/********************************************************************************************
*
* Logger.java
* 
* Description:	
* 
* Created on 	Feb 8, 2006
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package log;

import sml.* ;

import java.io.* ;

public class Logger implements Agent.xmlEventInterface
{
	Kernel	m_Kernel ;
	FileWriter m_OutputFile ;
	boolean m_IsLogging = false ;
	
	public static final String   kLineSeparator = System.getProperty("line.separator");

	/////////////////////////////////////////////////////////////////
	//
	//	 This handler is called with parsed XML data from the execution trace.
	//	 There's a lot of interesting stuff in here that could be logged.
	//	 If you need to log things that don't appear in the trace, you can register
	//	 for other events (like production firings) and log extra information.
	//
	//	 If you don't want to process the trace but just capture it consider
	//	 listening for the smlPrintEventId.smlEVENT_PRINT event instead
	//	 which sends the trace just as strings.
	//
	////////////////////////////////////////////////////////////////	/
	public void xmlEventHandler(int eventID, Object data, Agent agent, ClientXML xml)
	{
		try
		{
			// Setting this flag to true will dump the entire XML trace into the log file
			// That can be helpful if you'd like to process it later or you'd like to see what
			// information is available.
			boolean kLogRawXML = false ;
	
			if (kLogRawXML)
			{
				String str = xml.GenerateXMLString(true) ;
				m_OutputFile.write(str + kLineSeparator) ;
			}
	
			// This will always succeed.  If this isn't really trace XML
			// the methods checking on tag names etc. will just fail.
			// This is a cast.
			ClientTraceXML pRootXML = xml.ConvertToTraceXML() ;
	
			// The root object is just a <trace> tag.  The substance is in the children
			// so we'll get the first child which should exist.
			for (int i = 0 ; i < pRootXML.GetNumberChildren() ; i++)
			{
				ClientTraceXML childXML = new ClientTraceXML() ;
				boolean ok = pRootXML.GetChild(childXML, i) ;
				ClientTraceXML pTraceXML = childXML ;
		
				if (!ok)
				{
					// In Java we must explicitly delete XML objects we create
					// Otherwise, the underlying C++ object won't be reclaimed until the gc runs which
					// may not happen before the app finishes, in which case it'll look like a memory leak even
					// though technically that's not the case.
					childXML.delete() ;
					return ;
				}
				
				// To figure out the format of the XML you can either look at the ClientTraceXML class
				// (which has methods grouped appropriately) or you can look at the XML output directly.
				// It's designed to be readable, so looking at a few packets you should quickly get the hang
				// of what's going on and what attributes are available to be read.
				// Here are a couple of examples.
		
				// Check if this is a new state
				if (pTraceXML.IsTagState())
				{
					String count = pTraceXML.GetDecisionCycleCount() ;
					String stateID = pTraceXML.GetStateID() ;
					String impasseObject = pTraceXML.GetImpasseObject() ;
					String impasseType = pTraceXML.GetImpasseType() ;
		
					m_OutputFile.write("New state at decision " + count + " was " + stateID + " (" + impasseObject + " " + impasseType + ")" + kLineSeparator) ;
				}
		
				// Check if this is a new operator
				if (pTraceXML.IsTagOperator())
				{
					String count = pTraceXML.GetDecisionCycleCount() ;
					String operatorID = pTraceXML.GetOperatorID() ;
					String operatorName = pTraceXML.GetOperatorName() ;
		
					m_OutputFile.write("Operator at decision " + count + " was " + operatorID + " name " + operatorName + kLineSeparator) ;
				}
	
				// In Java we must explicitly delete XML objects we create
				// Otherwise, the underlying C++ object won't be reclaimed until the gc runs which
				// may not happen before the app finishes, in which case it'll look like a memory leak even
				// though technically that's not the case.
				childXML.delete() ;
			}
			
			// Flushing the file means we can look at it while it's still being formed
			// at the cost of a small reduction in performance.
			m_OutputFile.flush() ;
		}
		catch (IOException ex)
		{
			System.out.println(ex) ;
		}
	}
	
	public String startLogging(String filename, boolean append)
	{
		try
		{
			m_OutputFile = new FileWriter(filename, append) ;
		}
		catch (IOException ex)
		{
			return "Failed to open log file " + filename + " with exception " + ex ;
		}

		// Connect to the kernel we wish to capture logging information from.
		m_Kernel = Kernel.CreateRemoteConnection(true, null) ;

		if (m_Kernel.HadError())
		{
			String error = m_Kernel.GetLastErrorDescription() ;

			m_Kernel.delete() ;
			m_Kernel = null ;
			
			return error ;
		}

		// This logger just logs the first agent's output.
		// It could easily be extended to log an agent whose name we pass in or to log
		// all agents if that's what you need.
		Agent pAgent = m_Kernel.GetAgentByIndex(0) ;

		if (pAgent == null)
		{
			String error = "Connected to kernel, but there are no agents running" ;
			
			m_Kernel.Shutdown() ;
			m_Kernel.delete() ;
			m_Kernel = null ;

			return error ;
		}

		// Start listening for XML trace events.
		pAgent.RegisterForXMLEvent(smlXMLEventId.smlEVENT_XML_TRACE_OUTPUT, this, null) ;

		// We'll also explicitly set the watch level to level 1 here.  This determines how much
		// output is sent out to the event handler.  This setting is for the agent, so if you change it in
		// the debugger (e.g. to watch 0) it will affect what gets logged.  This is a limitation in the kernel.
		pAgent.ExecuteCommandLine("watch 1") ;

		m_IsLogging = true ;
		return null ;
	}
	
	public void stopLogging()
	{
		try
		{
			if (m_OutputFile != null)
			{
				m_OutputFile.close() ;
				m_OutputFile = null ;
			}
		}
		catch (IOException ex)
		{
			System.out.println(ex) ;
		}
		
		if (m_Kernel != null)
		{
			m_Kernel.Shutdown() ;
			m_Kernel.delete() ;
			m_Kernel = null ;
		}
		
		m_IsLogging = false ;
	}
	
	public boolean isLogging()
	{
		return m_IsLogging ;
	}

}

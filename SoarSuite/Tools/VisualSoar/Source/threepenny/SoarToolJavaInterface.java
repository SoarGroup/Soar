package threepenny;

// 3P
// Class that wraps the STI C library
/*
public class SoarToolJavaInterface
{
	// Load the native DLL
	static
	{
		System.loadLibrary("SoarToolJavaInterface1");
	}

	////////////////////////////////////////		
	// Member data

	// This stores our STI handle which is returned by the jniSTI_InitInterfaceLibrary call.
	private int				m_hSTI;

	////////////////////////////////////////		
	// Access to member data
	private int				GetSTIHandle()		{ return m_hSTI; }
	
	////////////////////////////////////////	
	// Native methods
	private native int		jniSTI_InitInterfaceLibrary(String sName, boolean bIsRuntime);
	private native boolean	jniSTI_InitListenPort(int hSTI);
	
	private native boolean	jniSTI_EstablishConnections(int hSTI, String sRemoteIPAddress, boolean bStopOnFirstNotFound);
	private native int		jniSTI_GetNumberConnections(int hSTI);
	private native String	jniSTI_GetConnectionName(int hSTI, int nIndex);
	
	private native void		jniSTI_EnableConnectionByIndex(int hSTI, int nIndex, boolean bEnable);
	private native void		jniSTI_EnableConnectionByName(int hSTI, String sName, boolean bEnable);
	private native boolean	jniSTI_IsConnectionEnabledByIndex(int hSTI, int nIndex);
	private native boolean	jniSTI_IsConnectionEnabledByName(int hSTI, String sName);
	
	private native boolean	jniSTI_TerminateInterfaceLibrary(int hSTI);
	
	private native boolean	jniSTI_PumpMessages(int hSTI, boolean bProcessAllPendingMessages);
	private native boolean	jniSTI_IsIncomingCommandAvailable(int hSTI);
	private native boolean	jniSTI_GetIncomingCommand(int hSTI, SoarToolJavaCommand commandObject);
	private native void		jniSTI_PopIncomingCommand(int hSTI);
	
	private native boolean	jniSTI_SendCommand(int hSTI, boolean bSynch, SoarToolJavaCommand commandObject);
	private native boolean	jniSTI_SendProduction(int hSTI, String sProductionName, String sProductionBody);
	private native boolean	jniSTI_SendFile(int hSTI, String sFilename);
	private native boolean	jniSTI_SendProductionMatches(int hSTI, String sProductionName);
	private native boolean	jniSTI_SendExciseProduction(int hSTI, String sProductionName);
	private native boolean	jniSTI_SendRawCommand(int hSTI, String sCommandString);
	
	////////////////////////////////////////
	// Public access methods

	// Initialize the library including the listening port
	public boolean Init(String sName, boolean bIsRuntime)
	{
		// Initialize the interface library
		m_hSTI = jniSTI_InitInterfaceLibrary(sName, bIsRuntime);
		if (m_hSTI != 0)
		{
			// Initialize the listening port
			if (jniSTI_InitListenPort(GetSTIHandle()) == true)
			{
				// Establish a connection
				if (jniSTI_EstablishConnections(GetSTIHandle(), "", true) == true)
				{
					return true;
				}

			}
		}
		
		// An error occurred
		return false;
	}
	
	// Terminate the library
	public boolean Term()
	{
		// Pump any remaining messages
		PumpMessages(true);
		
		// Terminate the socket library
		return jniSTI_TerminateInterfaceLibrary(GetSTIHandle());
	}
	
	// Get an array of our connections
	// Returns null if there aren't any connections
	//
	// TODO: We may want to cache these values somehow to avoid
	// memory buildup of the string allocations.
	public String[] GetConnectionNames()
	{
		// Get the number of connections
		int nNumConnections=jniSTI_GetNumberConnections(GetSTIHandle());
	
		// Return null if we don't have any connections
		if (nNumConnections == 0)
		{
			return null;
		}
		
		// Allocate the string array
		String[] connectionNames=new String[nNumConnections];
		
		// Get each connection name
		int i;
		for (i=0; i < nNumConnections; i++)
		{
			connectionNames[i]=jniSTI_GetConnectionName(GetSTIHandle(), i);
		}
		
		// Return our array
		return connectionNames;
	}
	
	// Enable a connection by its index
	public void	EnableConnectionByIndex(int nIndex, boolean bEnable)
	{
		jniSTI_EnableConnectionByIndex(GetSTIHandle(), nIndex, bEnable);
	}
	
	// Enable a connection by its name
	public void	EnableConnectionByName(String sName, boolean bEnable)
	{
		jniSTI_EnableConnectionByName(GetSTIHandle(), sName, bEnable);
	}

	// Determines if a connection based on its index is enabled
	public boolean	IsConnectionEnabledByIndex(int nIndex)
	{
		return jniSTI_IsConnectionEnabledByIndex(GetSTIHandle(), nIndex);
	}

	// Determines if a connection based on its name is enabled
	public boolean	IsConnectionEnabledByName(String sName)
	{
		return jniSTI_IsConnectionEnabledByName(GetSTIHandle(), sName);
	}

	// Pump messages in the queue
	public boolean PumpMessages(boolean bProcessAllPendingMessages)
	{
		return jniSTI_PumpMessages(GetSTIHandle(), bProcessAllPendingMessages);
	}
	
	// Returns true/false indicating if there is an incommand command available
	// for processing.
	public boolean IsIncomingCommandAvailable()
	{
		return jniSTI_IsIncomingCommandAvailable(GetSTIHandle());
	}

	// Get our incoming command from the top of the queue and
	// fill its description into the commandObject object.
	public boolean GetIncomingCommand(SoarToolJavaCommand commandObject)
	{
		return jniSTI_GetIncomingCommand(GetSTIHandle(), commandObject);
	}
	
	// Remove the top incoming command from the queue 
	public void PopIncomingCommand()
	{
		jniSTI_PopIncomingCommand(GetSTIHandle());
	}

	// Send a command to the runtime either synchronous (wait) or asynchronous (no waiting)
	public boolean SendCommand(boolean bSynch, SoarToolJavaCommand commandObject)
	{
		return jniSTI_SendCommand(GetSTIHandle(), bSynch, commandObject);
	}
	
	// Send a production to the runtime
	public boolean SendProduction(String sProductionName, String sProductionBody)
	{
		return jniSTI_SendProduction(GetSTIHandle(), sProductionName, sProductionBody);
	}
	
	// Send a file to the runtime
	public boolean SendFile(String sFilename)
	{
		return jniSTI_SendFile(GetSTIHandle(), sFilename);
	}
	
	// Send the matches command to the runtime
	public boolean SendProductionMatches(String sProductionName)
	{
		return jniSTI_SendProductionMatches(GetSTIHandle(), sProductionName);
	}

	// Send the excise production command to the runtime
	public boolean SendExciseProduction(String sProductionName)
	{
		return jniSTI_SendExciseProduction(GetSTIHandle(), sProductionName);
	}
		
	// Send a raw command to the runtime
	public boolean SendRawCommand(String sCommandString)
	{
		return jniSTI_SendRawCommand(GetSTIHandle(), sCommandString);
	}
}
*/
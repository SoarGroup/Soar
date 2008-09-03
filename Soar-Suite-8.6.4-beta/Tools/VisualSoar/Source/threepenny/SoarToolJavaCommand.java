package threepenny;

// 3P
// Class that contains the description and data of a command from the Soar Tools Interface
// NOTE! Methods and or data in this class may be accessed from native code.  Be very
//		 careful if you decide to change a method or piece of member data.
/*
public class SoarToolJavaCommand
{
	// Constants
	// Make sure that these match the values in the
	// C library STI_Interface\CommandIDs.h
	public static final int STI_kEditProduction	  = 1001 ;

	/////////////////////////////////
	// Member Data
	private int		m_nCommandID;			// ID of the command
	private int		m_nCommandFlags;		// Flags for the command
	private int		m_nSystemMessage;		// TODO: Find out what this does
	private int[]	m_integerParamArray;	// Array of long parameters
	private String	m_stringParam;			// String parameter 
	private int		m_nDataSize;			// Size of the data buffer in this command structure
											// TODO: Is there a way to just ask Java what
											// 		 the m_dataBuffer array size is?
	private byte[]	m_dataBuffer;			// Data buffer for our command
	
	/////////////////////////////////										
	// Access to member data
	public void		SetCommandID(int nID)							{ m_nCommandID=nID; }
	public void		SetCommandFlags(int nFlags)						{ m_nCommandFlags=nFlags; }
	public void		SetSystemMessage(int nSystemMessage)			{ m_nSystemMessage=nSystemMessage; }
	public void		SetIntegerParam(int nParamIndex, int nParam)	{ m_integerParamArray[nParamIndex]=nParam; }
	public void		SetStringParam(String sParam)					{ m_stringParam=sParam; }
	public void		SetDataSize(int nDataSize)						{ m_nDataSize=nDataSize; }
	public void		SetDataBuffer(byte[] buffer)					{ m_dataBuffer=buffer; }
	
	public int		GetCommandID()									{ return m_nCommandID; }
	public int		GetCommandFlags()								{ return m_nCommandFlags; }
	public int		GetSystemMessage()								{ return m_nSystemMessage; }
	public int		GetIntegerParam(int nParamIndex)				{ return m_integerParamArray[nParamIndex]; }
	public String	GetStringParam()								{ return m_stringParam; }
	public int		GetDataSize()									{ return m_nDataSize; }
	public byte[]	GetDataBuffer()									{ return m_dataBuffer; }
	
	// Constructor
	public SoarToolJavaCommand()
	{
		m_integerParamArray=new int[6];
	}
}
*/
/********************************************************************************************
*
* SoarAgentEvent.java
* 
* Description:	
* 
* Created on 	Jan 31, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc.events;

import sml.* ;

/************************************************************************
 * 
 * Used to report a change to an agent (added, removed etc.)
 * 
 ************************************************************************/
public class SoarAgentEvent extends java.util.EventObject
{
	public static int kAgentAdded = 1 ;
	public static int kAgentRemoved = 2 ;
	public static int kListChanged = 3 ;	// More generic -- just says the list isn't the same now (use the more specific event if you can)
	
	private int m_Type ;
	private Agent m_Agent ;
	
	public SoarAgentEvent(Object source, int type, Agent agent)
	{
		super(source) ;
		
		m_Type = type ;
		m_Agent = agent ;
	}

	public boolean isAgentAdded() 		{ return m_Type == kAgentAdded ; }
	public boolean isAgentRemoved() 	{ return m_Type == kAgentRemoved ; }
	public boolean isAgentListChanged() { return m_Type == kAgentAdded || m_Type == kAgentRemoved || m_Type == kListChanged ; }
	public Agent   getAgent()			{ return m_Agent ; }
}

/********************************************************************************************
*
* AgentFocusEvent.java
* 
* Description:	
* 
* Created on 	Feb 2, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc.events;

import sml.* ;

/************************************************************************
 * 
 * Event fired when a frame switches to tracking a different agent.
 * 
 ************************************************************************/
public class AgentFocusEvent extends java.util.EventObject
{
	public static final int kGettingFocus = 1 ;
	public static final int kLosingFocus  = 2 ;
	public static final int kGone   	  = 3 ;	// Agent has been deleted (usually because kernel is gone)
	
	private int m_Event ;
	private Agent m_Agent ;
	
	public AgentFocusEvent(Object source, int event, Agent agent)
	{
		super(source) ;
		
		m_Event = event ;
		m_Agent = agent ;
	}
	
	public boolean isGettingFocus() { return m_Event == kGettingFocus ; }
	public boolean isLosingFocus()  { return m_Event == kLosingFocus ; }
	public boolean isAgentGone()	{ return m_Event == kGone ; }	// Agent will be null
	public Agent   getAgent()		{ return m_Agent ; }
}

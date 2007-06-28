/********************************************************************************************
*
* AgentFocusGenerator.java
* 
* Description:	
* 
* Created on 	Feb 2, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc.events;

import java.util.Iterator;
import sml.* ;

/************************************************************************
 * 
 * Event fired when a frame switches to tracking a different agent.
 * 
 ************************************************************************/
public class AgentFocusGenerator
{
	protected java.util.ArrayList m_Listeners = new java.util.ArrayList();

	public synchronized void addAgentFocusListener(AgentFocusListener listener)
	{
		if (m_Listeners.contains(listener))
			return ;
		
		m_Listeners.add(listener);
	}

	public synchronized void removeAgentFocusListener(AgentFocusListener listener)
	{
		m_Listeners.remove(listener);
	}

	public synchronized void fireAgentGettingFocus(Object source, Agent agent)
	{
		AgentFocusEvent event = new AgentFocusEvent(source, AgentFocusEvent.kGettingFocus, agent) ;
		
		for (Iterator iter = m_Listeners.iterator() ; iter.hasNext() ;)
		{
			AgentFocusListener listener = (AgentFocusListener) iter.next();
			listener.agentGettingFocus(event) ;
		}
	}

	public synchronized void fireAgentLosingFocus(Object source, Agent agent)
	{
		AgentFocusEvent event = new AgentFocusEvent(source, AgentFocusEvent.kLosingFocus, agent) ;
		
		for (Iterator iter = m_Listeners.iterator() ; iter.hasNext() ;)
		{
			AgentFocusListener listener = (AgentFocusListener) iter.next();
			listener.agentLosingFocus(event) ;
		}
	}

	public synchronized void fireAgentGone(Object source)
	{
		AgentFocusEvent event = new AgentFocusEvent(source, AgentFocusEvent.kGone, null) ;
		
		for (Iterator iter = m_Listeners.iterator() ; iter.hasNext() ;)
		{
			AgentFocusListener listener = (AgentFocusListener) iter.next();
			listener.agentGone(event) ;
		}
	}

}

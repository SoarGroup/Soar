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
package edu.umich.soar.debugger.doc.events;

import java.util.Iterator;

import sml.Agent;

/************************************************************************
 * 
 * Event fired when a frame switches to tracking a different agent.
 * 
 ************************************************************************/
public class AgentFocusGenerator
{
    protected java.util.ArrayList<AgentFocusListener> m_Listeners = new java.util.ArrayList<AgentFocusListener>();

    public synchronized void addAgentFocusListener(AgentFocusListener listener)
    {
        if (m_Listeners.contains(listener))
            return;

        m_Listeners.add(listener);
    }

    public synchronized void removeAgentFocusListener(
            AgentFocusListener listener)
    {
        m_Listeners.remove(listener);
    }

    public synchronized void fireAgentGettingFocus(Object source, Agent agent)
    {
        AgentFocusEvent event = new AgentFocusEvent(source,
                AgentFocusEvent.kGettingFocus, agent);

        for (Iterator<AgentFocusListener> iter = m_Listeners.iterator(); iter
                .hasNext();)
        {
            AgentFocusListener listener = iter.next();
            listener.agentGettingFocus(event);
        }
    }

    public synchronized void fireAgentLosingFocus(Object source, Agent agent)
    {
        AgentFocusEvent event = new AgentFocusEvent(source,
                AgentFocusEvent.kLosingFocus, agent);

        for (Iterator<AgentFocusListener> iter = m_Listeners.iterator(); iter
                .hasNext();)
        {
            AgentFocusListener listener = iter.next();
            listener.agentLosingFocus(event);
        }
    }

    public synchronized void fireAgentGone(Object source)
    {
        AgentFocusEvent event = new AgentFocusEvent(source,
                AgentFocusEvent.kGone, null);

        for (Iterator<AgentFocusListener> iter = m_Listeners.iterator(); iter
                .hasNext();)
        {
            AgentFocusListener listener = iter.next();
            listener.agentGone(event);
        }
    }

}

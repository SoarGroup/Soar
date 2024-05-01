/********************************************************************************************
 *
 * SoarChangeGenerator.java
 *
 * Created on 	Nov 10, 2003
 *
 * @author 		Doug
 * @version
 *
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger.doc.events;

import sml.Agent;

/********************************************************************************************
 *
 * Event fired when a change occurs in the soar process (e.g. kernel
 * connected/disconnected, agent created/destroyed)
 *
 *
 ********************************************************************************************/
public class SoarChangeGenerator
{
    protected final java.util.ArrayList<SoarChangeListener> m_Listeners = new java.util.ArrayList<>();

    public synchronized void addSoarChangeListener(SoarChangeListener listener)
    {
        m_Listeners.add(listener);
    }

    public synchronized void removeSoarChangeListener(
            SoarChangeListener listener)
    {
        m_Listeners.remove(listener);
    }

    public synchronized void fireSoarConnectionChanged(Object source,
            boolean connected, boolean remote)
    {
        SoarConnectionEvent event = new SoarConnectionEvent(source, connected,
                remote);
        for (SoarChangeListener listener : m_Listeners) {
            listener.soarConnectionChanged(event);
        }
    }

    public synchronized void fireAgentListChanged(Object source, SoarAgentEvent.EventType type,
                                                  Agent agent) {
        SoarAgentEvent event = new SoarAgentEvent(source, type, agent);

        for (SoarChangeListener current : m_Listeners) {
            current.soarAgentListChanged(event);
        }
    }

}

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

import java.util.Iterator;

import sml.Agent;

/********************************************************************************************
* 
* Event fired when a change occurs in the soar process (e.g. kernel connected/disconnected, agent created/destroyed)
* 
* 
********************************************************************************************/
public class SoarChangeGenerator
{
	protected java.util.ArrayList<SoarChangeListener> m_Listeners = new java.util.ArrayList<SoarChangeListener>();

	public synchronized void addSoarChangeListener(SoarChangeListener listener)
	{
		m_Listeners.add(listener);
	}

	public synchronized void removeSoarChangeListener(SoarChangeListener listener)
	{
		m_Listeners.remove(listener);
	}

	public synchronized void fireSoarConnectionChanged(Object source, boolean connected, boolean remote)
	{
		SoarConnectionEvent event = new SoarConnectionEvent(source, connected, remote) ;
		java.util.Iterator<SoarChangeListener> listeners = m_Listeners.iterator();
		while( listeners.hasNext() )
		{
			SoarChangeListener listener = ( listeners.next() ) ;			
			listener.soarConnectionChanged( event );
		}
	}

	public synchronized void fireAgentListChanged(Object source, int type, Agent agent)
	{
		SoarAgentEvent event = new SoarAgentEvent(source, type, agent) ;
		
		for (Iterator<SoarChangeListener> listener = m_Listeners.iterator() ; listener.hasNext() ;)
		{
			SoarChangeListener current = listener.next() ;
			current.soarAgentListChanged(event) ;
		}
	}

}

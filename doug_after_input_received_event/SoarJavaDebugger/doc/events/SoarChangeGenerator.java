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
package doc.events;

import sml.Agent;

/********************************************************************************************
* 
* Event fired when a change occurs in the soar process (e.g. kernel connected/disconnected, agent created/destroyed)
* 
* 
********************************************************************************************/
public class SoarChangeGenerator
{
	protected java.util.ArrayList m_Listeners = new java.util.ArrayList();

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
		java.util.Iterator listeners = m_Listeners.iterator();
		while( listeners.hasNext() )
		{
			SoarChangeListener listener = ( (SoarChangeListener) listeners.next() ) ;			
			listener.soarConnectionChanged( event );
		}
	}

	public synchronized void fireAgentListChanged(Object source, int type, Agent agent)
	{
		SoarAgentEvent event = new SoarAgentEvent(source, type, agent) ;
		java.util.Iterator listeners = m_Listeners.iterator();
		while( listeners.hasNext() )
		{
			SoarChangeListener listener = ( (SoarChangeListener) listeners.next() ) ;			
			listener.soarAgentListChanged( event );
		}
	}

}

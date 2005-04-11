/********************************************************************************************
*
* AgentFocusListener.java
* 
* Description:	
* 
* Created on 	Feb 2, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc.events;

/************************************************************************
 * 
 * Event fired when a frame switches to tracking a different agent.
 * 
 ************************************************************************/
public interface AgentFocusListener
{
	/** Fired when a frame is switching to a new agent.  Listeners may wish to register for events with the new agent */
	public void agentGettingFocus(AgentFocusEvent e) ;

	/** Fired when a frame is switching away from an existing agent.  Listeners may wish to unregister for events from this agent */
	public void agentLosingFocus(AgentFocusEvent e) ;

	/** Fired when a frame is switching away from an existing agent and the agent has been deleted already.  Listeners may wish to clear events they had registered with the agent */
	public void agentGone(AgentFocusEvent e) ;
}

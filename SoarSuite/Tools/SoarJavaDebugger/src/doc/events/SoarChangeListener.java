/********************************************************************************************
*
* SoarMajorChangeListener.java
* 
* Description:	
* 
* Created on 	Jan 29, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc.events;

/************************************************************************
 * 
 * Used to report substantial events within Soar
 * 
 * The notion is that these events are relatively rare while other
 * events (e.g. trace output) may be very frequent.
 * 
 ************************************************************************/
public interface SoarChangeListener
{
	// Fired when kernel created/destroyed, remote connections made/disconnected etc.
	public void soarConnectionChanged(SoarConnectionEvent e) ;
	
	// Fired when an agent is created/destroyed or the list of agents has changed suddenly (e.g. when we connect to a remote kernel)
	public void soarAgentListChanged(SoarAgentEvent e) ;

}

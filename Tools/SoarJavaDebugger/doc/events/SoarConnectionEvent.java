/********************************************************************************************
*
* SoarConnectionEvent.java
* 
* Description:	
* 
* Created on 	Jan 29, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc.events;

import java.util.*;

/************************************************************************
 * 
 * Fired when a kernel is created/destroyed or we connect/disconnect etc.
 * 
 ************************************************************************/
public class SoarConnectionEvent extends EventObject
{
	/** True if connecting, false if disconnecting */
	private boolean m_Connect ;
	
	/** True if remote, false if local */
	private boolean m_Remote ;
	
	public SoarConnectionEvent(Object source, boolean connect, boolean remote)
	{
		super(source) ;
		
		m_Connect = connect ;
		m_Remote  = remote ;
	}
	
	public boolean isConnect()
	{
		return m_Connect;
	}
	public boolean isRemote()
	{
		return m_Remote;
	}
}

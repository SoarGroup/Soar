/********************************************************************************************
*
* SoarChangeEvent.java
* 
* Created on 	Nov 10, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc.events;

import java.util.EventObject;

/********************************************************************************************
* 
* Event fired when a change occurs in the soar process (e.g. output is generated from a command)
* 
********************************************************************************************/
public class SoarChangeEvent extends EventObject
{
	public final static int kOutputEvent = 1 ;
	public final static int kSoarStopped = 2 ;
	
	private String	m_Message ;
	private int		m_Code ;
	
	public SoarChangeEvent(Object source, int code, String message)
	{
		super(source) ;
		
		m_Message = message ;
		m_Code	  = code ;
	}
	
	public String getMessage() 	{ return m_Message ; }
	
	public boolean isOutputEvent() 		{ return m_Code == kOutputEvent ; }
	public boolean isSoarStoppedEvent()	{ return m_Code == kSoarStopped ; }
}

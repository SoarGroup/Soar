/********************************************************************************************
*
* DebuggerCommands.java
* 
* Description:	
* 
* Created on 	Apr 6, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc;

import modules.AbstractView;
import debugger.MainFrame;

/************************************************************************
 * 
 * A set of commands that extend the Soar command set.
 * 
 * I'm not sure if these should include the script commands or not so
 * for now I'm handling these separately.
 * 
 ************************************************************************/
public class DebuggerCommands
{
	protected MainFrame m_Frame ;
	protected Document  m_Document ;
	
	public final static String kClear = "clear" ;
	public final static String kQuit  = "quit" ;
	
	protected String[] kCommands = new String[] { kClear, kQuit } ;
	
	public DebuggerCommands(MainFrame frame, Document doc)
	{
		m_Frame = frame ;
		m_Document = doc ;
	}

	public String getExpandedCommand(String command)
	{
		return m_Document.getExpandedCommandLine(command) ;
	}
	
	public boolean isCommand(String command)
	{
		for (int i = 0 ; i < kCommands.length ; i++)
		{
			if (kCommands[i].equalsIgnoreCase(command))
				return true ;
		}
		
		return false ;
	}
	
	public Object executeCommand(AbstractView view, String command, boolean echoCommand)
	{
		String expanded = m_Document.getExpandedCommandLine(command) ;

		if (kClear.equalsIgnoreCase(command))
		{
			view.clearDisplay() ;
			return null ;
		}
		
		if (kQuit.equalsIgnoreCase(command))
		{
			m_Frame.close() ;
		}
		
		return null ;
	}
}

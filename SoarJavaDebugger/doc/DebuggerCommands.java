/********************************************************************************************
*
* DebuggerCommands.java
* 
* Description:	
* 
* Created on 	Mar 19, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc;

import debugger.MainFrame;

/************************************************************************
 * 
 * A set of commands that produce actions within the debugger
 * (e.g. closing a window or operating a menu item).
 * 
 * This allows us to script the debugger from within modules and
 * do pretty arbitrary code actions using a simple sort of scripting.
 * 
 ************************************************************************/
public class DebuggerCommands
{
	protected MainFrame m_Frame ;
	protected Document	m_Document ;
	
	public DebuggerCommands(MainFrame frame, Document doc)
	{
		m_Frame = frame ;
		m_Document = doc ;
	}
	
	public Object executeCommand(String command, boolean echoCommand)
	{
		if (command == null)
			return null ;
		
		String[] tokens = command.split(" ") ;
		
		if (tokens.length == 0)
			return null ;
		
		String first = tokens[0] ;

		// Load a demo
		if (first.equals("demo"))
		{
			// Token[1] = demo path (starting from the root folder for all demos)
			// Token[2] = the top level soar file to load
			m_Frame.loadDemo(new java.io.File(tokens[1], tokens[2]), echoCommand) ;
		}
		
		return null ;
	}
}

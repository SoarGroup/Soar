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

import general.ElementXML;
import manager.Pane;
import modules.AbstractView;
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

		// Load a demo: "demo <folder-relative-to-m_DemoPath> <demo.soar>"
		if (first.equals("demo"))
		{
			m_Frame.loadDemo(new java.io.File(tokens[1], tokens[2]), echoCommand) ;
			return null ;
		}

		// Add a new view: addview <framename> <viewname> sash right|left|top|bottom
		if (first.equals("addview"))
		{
			String frameName = tokens[1] ;
			String viewName  = tokens[2] ;
			String winType   = tokens[3] ;
			String direction = tokens[4] ;
			
			// To the user we are adding a view, but internally we're adding a pane
			// if we're adding a sash and then that pane will contain the view
			MainFrame frame = m_Document.getFrameByName(frameName) ;
			AbstractView view = frame.getNameRegister().getView(viewName) ;
			Pane pane = view.getPane() ;
			
			// The plan is to convert the current set of windows to XML
			// then modify that tree structure and then load the XML back in.
			// To make this work I think when we convertToXML we should attach the
			// XML to the windows as data (at least to the views).  Then we can
			// go back from the view to the correct node in the XML that represents it
			// and the manipulation at that level shouldn't be too hard.
			ElementXML xml = frame.getMainWindow().convertToXML() ;
			
			return null ;
		}
		
		return null ;
	}
}

/********************************************************************************************
*
* ComboCommandLineWindow.java
* 
* Description:	Mimic a simple command line window -- type commands directly to execute them.
* 				For now we still just use a combo box for the input -- later we'll do a version
* 				which uses a normal command line prompt.
* 
* Created on 	Jan 29, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import general.ElementXML;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.events.*;

import debugger.MainFrame;
import doc.Document;

/************************************************************************
 * 
 * This version clears the last command from the combo box (so you're ready to type another),
 * keeps all of the output and doesn't do anything when Soar stops running.
 * 
 * Getting closer to a real command line window.
 * 
 ************************************************************************/
public class TraceView extends BaseCommandView
{
	public TraceView()
	{
		m_ClearEachCommand = false ;
		m_UpdateOnStop = false ;
		m_ClearComboEachCommand = true ;
		m_ComboAtTop = false ;
		m_ShowTraceOutput = true ;
		m_PromptForCommands = "<Type commands here>" ;
	}
	
	public Color getBackgroundColor()
	{
		return getMainFrame().m_White ;
	}
	
	public boolean canBePrimeWindow() { return true ; }
	
	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public String getModuleBaseName() { return "trace" ; }
	
}

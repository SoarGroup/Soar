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

import java.awt.Font;
import java.awt.event.MouseListener;

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
public class TraceView extends ComboCommandView
{
	public TraceView()
	{
		m_ClearEachCommand = false ;
		m_UpdateOnStop = false ;
		m_ClearComboEachCommand = true ;
		m_ComboAtTop = false ;
		m_ShowTraceOutput = true ;
	}
}

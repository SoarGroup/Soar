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
import dialogs.PropertiesDialog;
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
	
	public void showProperties()
	{
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[3] ;
		
		properties[0] = new PropertiesDialog.BooleanProperty("Update automatically on stop", m_UpdateOnStop) ;
		properties[1] = new PropertiesDialog.BooleanProperty("Clear display before each command", m_ClearEachCommand) ;
		properties[2] = new PropertiesDialog.BooleanProperty("Combo at top", m_ComboAtTop) ;
		
		PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;
		
		m_UpdateOnStop = ((PropertiesDialog.BooleanProperty)properties[0]).getValue() ;
		m_ClearEachCommand = ((PropertiesDialog.BooleanProperty)properties[1]).getValue() ;
		boolean comboAtTop = ((PropertiesDialog.BooleanProperty)properties[2]).getValue() ;

		// We need to know if this value has changed as we'll need to rebuild the layout
		boolean comboMoving = (comboAtTop != m_ComboAtTop) ;
		m_ComboAtTop = comboAtTop ;
		
		if (comboMoving)
		{
			// Convert everything to XML (using the new setting we just changed)
			ElementXML xml = m_Frame.getMainWindow().convertToXML() ;

			// Rebuild the entire layout from the new XML structure.
			m_Frame.getMainWindow().loadFromXMLNoThrow(xml) ;
		}
	}

	
}

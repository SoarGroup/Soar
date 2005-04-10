/********************************************************************************************
*
* ComboCommandWindowClear.java
* 
* Created on 	Nov 22, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import java.util.ArrayList;

import debugger.MainFrame;
import dialogs.PropertiesDialog;
import doc.* ;

/********************************************************************************************
* 
* This version of the BaseCommandView does not erase the output window after each command,
* but instead keeps a full trace.
* 
********************************************************************************************/
public class KeepCommandView extends BaseCommandView
{
	public KeepCommandView()
	{
		m_ClearComboEachCommand = false ;
		m_ClearEachCommand = false ;
		m_UpdateOnStop = false ;
	}
	
	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public String getModuleBaseName() { return "keep" ; }

	public void showProperties()
	{
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[3] ;

		boolean decisionUpdating = (m_UpdateEveryNthDecision > 0) ;
		
		properties[0] = new PropertiesDialog.BooleanProperty("Update automatically on stop", m_UpdateOnStop) ;
		properties[1] = new PropertiesDialog.BooleanProperty("Clear display before each command", m_ClearEachCommand) ;
		properties[2] = new PropertiesDialog.IntProperty("Update automatically every n'th decision (0 => none)", m_UpdateEveryNthDecision) ;

		boolean ok = PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;

		if (ok)
		{
			m_UpdateOnStop = ((PropertiesDialog.BooleanProperty)properties[0]).getValue() ;
			m_ClearEachCommand = ((PropertiesDialog.BooleanProperty)properties[1]).getValue() ;		
			m_UpdateEveryNthDecision = ((PropertiesDialog.IntProperty)properties[2]).getValue() ;
	
			if (this.getAgentFocus() != null)
			{
				// Make sure we're getting the events to match the new settings
				this.registerForAgentEvents(this.getAgentFocus()) ;
			}
		}
	}
}

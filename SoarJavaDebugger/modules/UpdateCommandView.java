/********************************************************************************************
*
* UpdateCommandView.java
* 
* Description:	
* 
* Created on 	Mar 18, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import dialogs.PropertiesDialog;

/************************************************************************
 * 
* This version of the BaseCommandView updates at the end of each run
* and clears each time a command is issued.
 * 
 ************************************************************************/
public class UpdateCommandView extends BaseCommandView
{
	public UpdateCommandView()
	{
		m_ClearComboEachCommand = false ;
		m_ClearEachCommand = true ;
		m_UpdateOnStop = true ;
	}
	
	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public String getModuleBaseName() { return "update" ; }
	
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
				this.registerForAgentEvents(this.getAgentFocus()) ;
			}
		}
	}
}

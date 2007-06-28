/********************************************************************************************
*
* AbstractSingleCommandView.java
* 
* Description:	
* 
* Created on 	Apr 10, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.* ;

import dialogs.PropertiesDialog;

/************************************************************************
 * 
 * A window that shows the output from a single command, rather than
 * from a complete run.
 * 
 ************************************************************************/
public abstract class AbstractSingleTextView extends AbstractTextView
{
	public Color getBackgroundColor()
	{
		if (!m_UpdateOnStop)
			return getMainFrame().m_White ;

		// Default to standard background color
		return getMainFrame().getDisplay().getSystemColor(SWT.COLOR_WIDGET_BACKGROUND) ;
	}

	public void showProperties()
	{
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[3] ;
		
		properties[0] = new PropertiesDialog.BooleanProperty("Update automatically on stop", m_UpdateOnStop) ;
		properties[1] = new PropertiesDialog.BooleanProperty("Clear display before each command", m_ClearEachCommand) ;
		properties[2] = new PropertiesDialog.IntProperty("Update automatically every n'th decision (0 => none)", m_UpdateEveryNthDecision) ;

		boolean ok = PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;

		if (ok)
		{
			m_UpdateOnStop = ((PropertiesDialog.BooleanProperty)properties[0]).getValue() ;
			m_ClearComboEachCommand = !m_UpdateOnStop ;
			m_ClearEachCommand = ((PropertiesDialog.BooleanProperty)properties[1]).getValue() ;		
			m_UpdateEveryNthDecision = ((PropertiesDialog.IntProperty)properties[2]).getValue() ;
	
			if (this.getAgentFocus() != null)
			{
				// Make sure we're getting the events to match the new settings
				this.unregisterForAgentEvents(this.getAgentFocus()) ;
				this.registerForAgentEvents(this.getAgentFocus()) ;
			}
			
			// Some changes should be reflected in our background color
			getDisplayControl().setBackground(getBackgroundColor()) ;
		}
	}

}

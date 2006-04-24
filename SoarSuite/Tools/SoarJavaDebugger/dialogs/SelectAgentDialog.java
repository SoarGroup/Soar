/********************************************************************************************
*
* SelectAgentDialog.java
* 
* Description:	
* 
* Created on 	Feb 16, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package dialogs;

import helpers.FormDataHelper;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.events.*;

/*****************************************************************************************
 * 
 * Create a simple dialog for getting input (to match Swing's JOptionPane.showInputDialog)
 * 
 *****************************************************************************************/
public class SelectAgentDialog extends BaseDialog
{
	private String m_Result ;
	private List   m_Agents ;
	
	/********************************************************************************************
	* 
	* Create a simple dialog asking the user for input (a single string).
	* 
	* @param parent			The parent for this dialog (we'll center the dialog within this window)
	* @param title			The title for the dialog
	* @param prompt			The message used to prompt for input
	* @param initialValue	The value to start with in the text field (pass "" for nothing)
	* @return The value the user entered or null if they cancelled the dialog 
	********************************************************************************************/
	public static String showDialog(Composite parent, String title, String[] agentNames)
	{
		// Create the dialog window
		final SelectAgentDialog dialog = new SelectAgentDialog(parent, title, agentNames) ;
				
		dialog.getDialog().setSize(400, 200) ;
		dialog.centerDialog(parent) ;
		dialog.open() ;

		// If the user makes a choice we'll close the dialog (pretty aggressive)
		// Moved to after the creation of the dialog, to try to fix a problem on GTK where a selection event is fired immediately when
		// the list is created (causing the dialog to close).
		dialog.m_Agents.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { dialog.endDialog(true) ; } } ) ;

		dialog.pumpMessages() ;
		
		return dialog.m_Result ;
	}

	/********************************************************************************************
	* 
	* Create the dialog -- the constructor is private because we use a static method to build this.
	* 
	********************************************************************************************/
	private SelectAgentDialog(Composite parent, String title, String[] agentNames)
	{
		// Create a basic dialog with OK and Cancel buttons
		super(parent, title, true) ;
				
		int margin = 10 ;
		
		getOpenArea().setLayout(new FormLayout()) ;

		// Create the list of agents and anchor it to the top
		m_Agents = new List(getOpenArea(), SWT.SINGLE | SWT.V_SCROLL | SWT.BORDER) ;
		m_Agents.setItems(agentNames) ;
		m_Agents.setLayoutData(FormDataHelper.anchorTop(margin)) ;
	}
	
	/********************************************************************************************
	* 
	* Close the dialog -- either successfully or cancelled.
	* 
	********************************************************************************************/
	protected void endDialog(boolean ok)
	{
		// If the user cancelled or no agents are selected we're done
		if (!ok || m_Agents.getSelectionCount() == 0)
		{
			m_Result = null ;
		}
		else
		{
			m_Result = m_Agents.getSelection()[0] ;
		}
		
		// Close the dialog
		super.endDialog(ok) ;
	}
}

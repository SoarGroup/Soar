/********************************************************************************************
*
* NewWindowDialog.java
* 
* Created on 	Nov 22, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package dialogs;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;

import debugger.MainFrame;
import doc.* ;

/********************************************************************************************
* 
* This dialog offers a list of available modules (window classes) and allows the user to select one.
* 
* This dialog (like most I write) is designed as a panel so it can be used in a dialog or
* placed on another window if desired.
* 
********************************************************************************************/
public class NewWindowDialog extends BaseDialog
{
	private int    m_Result ; // Index in module list
	private List   m_Modules ;
	private Text   m_Description ;
	private ModuleList m_ModuleList ;
	private MainFrame m_Frame ;
	
	/********************************************************************************************
	* 
	* Create a simple dialog asking the user to pick a module (window type)
	* 
	* @param parent			The parent for this dialog (we'll center the dialog within this window)
	* @param title			The title for the dialog
	* @param modules		The list of modules the user should select from
	* @return The value the user entered or null if they cancelled the dialog 
	********************************************************************************************/
	public static Module showDialog(MainFrame frame, String title, ModuleList modules)
	{
		// Create the dialog window
		NewWindowDialog dialog = new NewWindowDialog(frame, title, modules) ;
				
		dialog.getDialog().pack() ;
		dialog.getDialog().setSize(500, 300) ;
		dialog.centerDialog(frame.getWindow()) ;
		dialog.open() ;
		
		dialog.pumpMessages() ;
		
		if (dialog.m_Result == -1)
			return null ;
		
		return modules.get(dialog.m_Result) ;
	}

	/********************************************************************************************
	* 
	* Create the dialog -- the constructor is private because we use a static method to build this.
	* 
	********************************************************************************************/
	private NewWindowDialog(MainFrame frame, String title, ModuleList modules)
	{
		// Create a basic dialog with OK and Cancel buttons
		super(frame.getWindow(), title, true) ;
		
		m_Frame = frame ;
		m_ModuleList = modules ;
		
		// Create an array of names
		String[] moduleNames = new String[modules.size()] ;
		for (int i = 0 ; i < modules.size() ; i++)
		{
			moduleNames[i] = ((Module)modules.get(i)).getName() ;
		}

		//getOpenArea().setLayout(new FormLayout()) ;

		// Create the text description area
		m_Description = new Text(getOpenArea(), SWT.MULTI | SWT.WRAP | SWT.V_SCROLL | SWT.BORDER) ;
				
		// Create the list of agents and anchor it to the top
		m_Modules = new List(getOpenArea(), SWT.SINGLE | SWT.V_SCROLL | SWT.BORDER) ;
		m_Modules.setItems(moduleNames) ;
		
		FormData data = new FormData() ;
		data.left = new FormAttachment(0) ;
		data.top  = new FormAttachment(0) ;
//		data.right = new FormAttachment(m_Description) ;
		data.bottom = new FormAttachment(100) ;
		m_Modules.setLayoutData(data) ;
		
		data = new FormData() ;
		data.left = new FormAttachment(m_Modules) ;
		data.top  = new FormAttachment(0) ;
		data.right = new FormAttachment(100) ;
		data.bottom = new FormAttachment(100) ;
		m_Description.setLayoutData(data) ;
		
		// If the user makes a choice we'll close the dialog (pretty aggressive)
		m_Modules.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { selectionChanged() ; } } ) ;
		
		// Look up the initial selection
		int selection = m_Frame.getAppIntegerProperty("NewWindow.Module") ;
		if (selection == Integer.MAX_VALUE || selection == -1)
			selection = 0 ;
		
		m_Modules.select(selection) ;
		selectionChanged() ;
	}
	
	/********************************************************************************************
	* 
	* Close the dialog -- either successfully or cancelled.
	* 
	********************************************************************************************/
	protected void endDialog(boolean ok)
	{
		// If the user cancelled or no agents are selected we're done
		if (!ok || m_Modules.getSelectionIndex() == -1)
		{
			m_Result = -1 ;
		}
		else
		{
			m_Result = m_Modules.getSelectionIndex() ;
			m_Frame.setAppProperty("NewWindow.Module", m_Result) ;
		}
		
		// Close the dialog
		super.endDialog(ok) ;
	}
		
	/********************************************************************************************
	* 
	* The user selected a new module.
	* 
	********************************************************************************************/
	private void selectionChanged()
	{
		int selection = m_Modules.getSelectionIndex() ;
		
		if (selection == -1)
			return ;

		// Retrieve the selected module			
		Module module = m_ModuleList.get(selection) ;
		
		// Change the description
		m_Description.setText(module.getDescription()) ;	
	}
}


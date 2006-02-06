/********************************************************************************************
*
* KernelLocationDialog.java
* 
* Description:	
* 
* Created on 	Apr 12, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package dialogs;

import general.SaveLoad;
import helpers.FormDataHelper;

import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.events.*;

import debugger.MainFrame;

/************************************************************************
 * 
 * Allows the user to enter the path to the Soar kernel DLL to load.
 * 
 ************************************************************************/
public class KernelLocationDialog extends BaseDialog
{
	private Button		m_Browse ;
	private Text		m_KernelFilename ;
	private String		m_Result ;
	private MainFrame	m_Frame ;
	
	/********************************************************************************************
	* 
	* Create a simple dialog asking the user for input (a single string).
	* 
	* @param parent			The parent for this dialog (we'll center the dialog within this window)
	* @param title			The title for the dialog
	* @return The value the user entered or null if they cancelled the dialog 
	********************************************************************************************/
	public static String showDialog(MainFrame frame, String title)
	{
		// Create the dialog window
		Composite parent = frame.getWindow() ;
		KernelLocationDialog dialog = new KernelLocationDialog(frame, title) ;
				
		dialog.getDialog().setSize(600, 200) ;
		dialog.centerDialog(parent) ;
		dialog.open() ;
		
		dialog.pumpMessages() ;
		
		return dialog.m_Result ;
	}

	/********************************************************************************************
	* 
	* Create the dialog -- the constructor is private because we use a static method to build this.
	* 
	********************************************************************************************/
	private KernelLocationDialog(MainFrame frame, String title)
	{
		// Create a basic dialog with OK and Cancel buttons
		super(frame.getWindow(), title, true) ;

		m_Frame = frame ;

		getOpenArea().setLayout(new FormLayout()) ;

		// Create a container for the text entry portion
		Composite group = new Group(getOpenArea(), 0) ;

		GridLayout layout = new GridLayout() ;
		layout.numColumns = 3 ;
//		layout.makeColumnsEqualWidth = true ;
		group.setLayout(layout) ;
		
		// Create the text entry fields
		Label pathLabel = new Label(group, SWT.RIGHT) ;
		pathLabel.setText("Path to soar library:") ;
		m_KernelFilename = new Text(group, 0) ;
		
		// Set the initial value to match user's last choice
		String path = m_Frame.getAppStringProperty("Kernel.Location") ;
		if (path != null)
			m_KernelFilename.setText(path) ;

		m_Browse = new Button(group, SWT.PUSH) ;
		m_Browse.setText("Browse...") ;
		m_Browse.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) {
			browsePressed() ;
		} } ) ;
		
		// Lay them out
		GridData data = new GridData(GridData.FILL_HORIZONTAL) ;
		m_KernelFilename.setLayoutData(data) ;

		// Add some help information
		// Using read-only text because label doesn't want to wrap.
		Text help = new Text(getOpenArea(), SWT.MULTI | SWT.WRAP | SWT.READ_ONLY) ;
		help.setText("\nEnter the path to the SoarKernelSML library (including the name of the DLL).\nYou can enter just the name of the library if the folder that contains this library appears in the Windows PATH.") ;
		
		FormData form = FormDataHelper.anchorTop(0) ;
		group.setLayoutData(form) ;
		
		form = new FormData() ;
		form.top = new FormAttachment(group) ;
		form.left = new FormAttachment(0) ;
		form.right = new FormAttachment(100) ;
		help.setLayoutData(form) ;
		
		getDialog().layout() ;
	}
	
	protected void browsePressed()
	{
		String filename = SaveLoad.LoadFileDialog(getDialog(), new String[] { "*.dll" }, new String[] { "Soar Kernel SML (*.dll)" } , m_Frame.getAppProperties(), "KernelSave", "KernelLoad") ;
		
		if (filename != null)
		{
			m_KernelFilename.setText(filename) ;
		}
	}
	
	/********************************************************************************************
	* 
	* Close the dialog -- either successfully or cancelled.
	* 
	********************************************************************************************/
	protected void endDialog(boolean ok)
	{
		// Store the user's choices (even if they cancel)
		// If the user cancelled or no agents are selected we're done
		if (!ok)
		{
			m_Result = null ;
		}
		else
		{
			m_Result = m_KernelFilename.getText() ;
			m_Frame.setAppProperty("Kernel.Location", m_Result) ;
		}
		
		// Close the dialog
		super.endDialog(ok) ;
	}	
}

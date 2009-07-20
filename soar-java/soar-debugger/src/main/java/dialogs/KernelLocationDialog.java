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

import java.io.File;

import general.SaveLoad;
import helpers.FormDataHelper;

import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.*;
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
	private Button		m_Browse2 ;
	private Text		m_LibraryFilename ;
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
		
		dialog.m_Dialog.pack() ;
		
		// Make it look a little tidier by shifting from the default values
		dialog.getDialog().setSize(dialog.getDialog().getSize().x * 85/100, dialog.getDialog().getSize().y * 120/100) ;

		dialog.centerDialog(parent) ;
		dialog.open() ;
		
		dialog.pumpMessages() ;
		
		return dialog.m_Result ;
	}

	private String getParentFolder(String path)
	{
		File file = new File(path) ;
		String parentFolder = file.getParent() ;
		
		if (parentFolder != null)
		{
			File parent = new File(parentFolder) ;
			return parent.getParent() ;
		}
		
		return null ;
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

		Composite pathGroup = new Group(getOpenArea(), 0) ;
		pathGroup.setLayoutData(FormDataHelper.anchorFull(0)) ;

		// Create a container for the text entry portion
		Composite group1 = new Group(pathGroup, 0) ;

		GridLayout layout1 = new GridLayout() ;
		layout1.numColumns = 3 ;
		group1.setLayout(layout1) ;
		
		// Create the text entry fields
		Label pathLabel = new Label(group1, SWT.RIGHT) ;
		pathLabel.setText("Path to soar library:") ;
		m_KernelFilename = new Text(group1, 0) ;
		
		// Set the initial value to match user's last choice
		String path = m_Frame.getAppStringProperty("Kernel.Location") ;
		if (path != null && path.length() > 0)
			m_KernelFilename.setText(path) ;

		m_KernelFilename.addModifyListener(new ModifyListener() { public void modifyText(ModifyEvent e) { kernelPathModified() ; } } ) ;
		
		m_Browse = new Button(group1, SWT.PUSH) ;
		m_Browse.setText("Browse...") ;
		m_Browse.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) {
			browsePressed() ;
		} } ) ;

		// Create a container for the text entry portion
		Composite group2 = new Group(pathGroup, 0) ;

		GridLayout layout2 = new GridLayout() ;
		layout2.numColumns = 3 ;
		group2.setLayout(layout2) ;
		
		// Create the text entry fields
		Label pathLabel2 = new Label(group2, SWT.RIGHT) ;
		pathLabel2.setText("Path to parent library folder:") ;
		m_LibraryFilename = new Text(group2, 0) ;

		// Set the initial default value, but allow this to be overridden from an earlier saved setting
		kernelPathModified() ;

		// Set the initial value to match user's last choice
		String path2 = m_Frame.getAppStringProperty("Kernel.Library.Location") ;
		if (path2 != null && path2.length() > 0)
			m_LibraryFilename.setText(path2) ;
		
		m_Browse2 = new Button(group2, SWT.PUSH) ;
		m_Browse2.setText("Browse...") ;
		m_Browse2.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) {
			browse2Pressed() ;
		} } ) ;

		// Add some help information
		// Using read-only text because label doesn't want to wrap.
		Text help = new Text(pathGroup, SWT.MULTI | SWT.WRAP | SWT.READ_ONLY) ;
		help.setText("\nFor 'soar library' enter the path to the SoarKernelSML library (including the name of the DLL).\nYou can enter just the name of the library if the folder that contains this library appears in the Windows PATH.\n\nFor 'parent library folder' this defaults to the folder above where the soar library is loaded from.\nIt is used to locate the demos and help files so if they are in unusual places you can override the default here.\n\nIf you change the path to SoarKernelSML.dll you will want to delete the current kernel and create a new local instance.\n") ;

		// Lay them out
		GridData data = new GridData(GridData.FILL_HORIZONTAL) ;
		m_LibraryFilename.setLayoutData(data) ;

		data = new GridData(GridData.FILL_HORIZONTAL) ;
		m_KernelFilename.setLayoutData(data) ;

		pathGroup.setLayout(new FormLayout()) ;
		group1.setLayoutData(FormDataHelper.anchorTop(0)) ;
		group2.setLayoutData(FormDataHelper.anchorTop(group1)) ;
		help.setLayoutData(FormDataHelper.anchorTop(group2)) ;
		
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

	protected void browse2Pressed()
	{
		String folder = SaveLoad.SelectFolderDialog(getDialog(), m_LibraryFilename.getText(), "Select Soar library location", "Select the folder that contains the Soar library") ;
		
		if (folder != null)
		{
			m_LibraryFilename.setText(folder) ;
		}
	}
	
	protected void kernelPathModified()
	{
		String parent = getParentFolder(m_KernelFilename.getText()) ;
		if (parent != null)
			m_LibraryFilename.setText(parent) ;
		else
			m_LibraryFilename.setText("") ;
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
			
			String libraryLocation = m_LibraryFilename.getText() ;
			m_Frame.setAppProperty("Kernel.Library.Location", libraryLocation) ;
			
			if (libraryLocation != null && libraryLocation.length() > 0)
				m_Frame.executeCommandPrimeView(m_Frame.getDocument().getSoarCommands().setLibraryLocationCommand(libraryLocation), true) ;
		}
		
		// Close the dialog
		super.endDialog(ok) ;
	}	
}

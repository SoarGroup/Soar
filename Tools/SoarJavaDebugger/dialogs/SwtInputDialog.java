/********************************************************************************************
*
* SwtInputDialog.java
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
public class SwtInputDialog extends BaseDialog
{
	private String m_Result ;
	private Text   m_EntryField ;
	private boolean m_MultiLine ;
	
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
	public static String showDialog(Composite parent, String title, String prompt, String initialValue)
	{
		// Create the dialog window
		SwtInputDialog dialog = new SwtInputDialog(parent, title, prompt, initialValue, false) ;
		
		dialog.getDialog().setSize(400, 110) ;
		dialog.centerDialog(parent) ;
		dialog.open() ;
		
		dialog.pumpMessages() ;
		
		return dialog.m_Result ;
	}

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
	public static String showMultiLineDialog(Composite parent, String title, String prompt, String initialValue)
	{
		// Create the dialog window
		SwtInputDialog dialog = new SwtInputDialog(parent, title, prompt, initialValue, true) ;
		
		dialog.getDialog().setSize(400, 200) ;
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
	private SwtInputDialog(Composite parent, String title, String prompt, String initialValue, boolean multiLine)
	{
		// Create a basic dialog with OK and Cancel buttons
		super(parent, title, true) ;
				
		int margin = 10 ;
		m_MultiLine = multiLine ;
		
		getOpenArea().setLayout(new FormLayout()) ;
		
		// Place the prompt text
		Label promptLabel = new Label(getOpenArea(), SWT.NULL) ;
		promptLabel.setText(prompt) ;
		promptLabel.setLayoutData(FormDataHelper.anchorTopLeft(margin)) ;
		
		// The field where the user types their response
		Text entryField = new Text(getOpenArea(), m_MultiLine ? SWT.MULTI : SWT.NULL) ;
		entryField.setText(initialValue) ;
		entryField.setFocus() ;
		
		// Pressing return during text entry closes the dialog like pressing "ok".
		if (!multiLine)
			entryField.addKeyListener(new KeyAdapter() { public void keyPressed(KeyEvent e) { if (e.character == '\r') endDialog(true) ; } } ) ;
		
		m_EntryField = entryField ;

		// Place the entry next to the prompt
		FormData data = new FormData() ;
		data.left  = new FormAttachment(promptLabel, 10) ;
		data.right = new FormAttachment(100, -margin) ;
		data.top   = new FormAttachment(0, margin) ;
		data.bottom = new FormAttachment(100, -margin) ;
		
		entryField.setLayoutData(data) ;
	}
	
	/********************************************************************************************
	* 
	* Close the dialog -- either successfully or cancelled.
	* 
	********************************************************************************************/
	protected void endDialog(boolean ok)
	{
		if (!ok)
		{
			m_Result = null ;
		}
		else
		{
			m_Result = m_EntryField.getText() ;
		}
		
		// Close the dialog
		super.endDialog(ok) ;
	}
}

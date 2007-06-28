/********************************************************************************************
*
* BaseDialog.java
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
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.events.*;

/*****************************************************************************************
 * 
 * Create a simple dialog with OK and cancel buttons.
 * 
 * This dialog is intended to be the base class for other dialogs to save some legwork.
 * SwtInputDialog shows an example of its use.
 * 
 *****************************************************************************************/
public class BaseDialog
{
	// The dialog box
	protected Shell 	m_Dialog ;
	protected Button	m_OK ;
	protected Button	m_Cancel ;
	protected Composite m_OpenArea ;
	protected boolean	m_Cancelled ;
	
	protected Composite getOpenArea() 	{ return m_OpenArea ; }
	protected Shell		getDialog() 	{ return m_Dialog ; }
	
	/********************************************************************************************
	* 
	* It looks like I need to pump my message loop here to make this a blocking modal dialog
	* as we want it to be.  There may be a more SWT appropriate solution to this.
	* 
	********************************************************************************************/
	protected void pumpMessages()
	{
		while (!m_Dialog.isDisposed())
		{
			if (!m_Dialog.getDisplay().readAndDispatch())
			{
				m_Dialog.getDisplay().sleep() ;
			}
		}		
	}
	
	protected void open()
	{
		m_Dialog.open() ;
	}
	
	/********************************************************************************************
	* 
	* Center the dialog on the given control.
	* 
	********************************************************************************************/
	protected void centerDialog(Control control)
	{
		Point pt = control.getLocation() ;
		Point size = control.getSize() ;
		Point dialogSize = m_Dialog.getSize() ;

		// 'pt' is relative to the control parent's location unless
		// the control is a shell in which case it's in screen coords.
		// Therefore we need to adjust the position relative to the screen coordinates
		// of the shell (unless we've been passed a shell already).
		Point screen = control.getShell().getLocation() ;

		if (control == control.getShell())
			screen = new Point(0,0) ;

		Point center = new Point(pt.x + size.x/2 - dialogSize.x/2 + screen.x, pt.y + size.y/2 - dialogSize.y/2 + screen.y) ;
		m_Dialog.setLocation(center) ;
	}
	
	/********************************************************************************************
	* 
	* Create a basic dialog with ok and cancel buttons and set the title
	* 
	********************************************************************************************/
	protected BaseDialog(Composite parent, String title, boolean modal)
	{
		// Create the dialog
		m_Dialog = new Shell(parent.getShell(), modal ? SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL | SWT.RESIZE : SWT.DIALOG_TRIM | SWT.RESIZE) ;
		m_Dialog.setText(title) ;
		m_Dialog.setLayout(new FormLayout()) ;
		
		int margin = 10 ;
		
		// The open area is filled in by dialogs derived from this one.
		Composite openArea = new Composite(m_Dialog, 0) ;
		openArea.setLayout(new FormLayout()) ;
		
		// Add ok and cancel buttons at the bottom right
		Button ok = new Button(m_Dialog, SWT.PUSH) ;
		ok.setText("OK") ;
		ok.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { endDialog(true) ; } }) ;
		
		Button cancel = new Button(m_Dialog, SWT.PUSH) ;
		cancel.setText("Cancel") ;
		cancel.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { endDialog(false) ; } }) ;
		cancel.setLayoutData(FormDataHelper.anchorBottomRight(margin)) ;

		FormData okData = new FormData() ;
		okData.bottom = new FormAttachment(100, -margin) ;
		okData.right  = new FormAttachment(cancel, -10) ;
		ok.setLayoutData(okData);

		FormData openData = FormDataHelper.anchorTop(0) ;
		openData.bottom = new FormAttachment(ok) ;
		openArea.setLayoutData(openData) ;
		
		// Make these members so derived classes can work with them
		m_OK = ok ;
		m_Cancel = cancel ;
		m_OpenArea = openArea ;
		
		m_Dialog.setDefaultButton(ok) ;
	}
		
	/********************************************************************************************
	* 
	* Close the dialog -- either successfully or cancelled.
	* 
	********************************************************************************************/
	protected void endDialog(boolean ok)
	{
		m_Cancelled = !ok ;
		m_Dialog.close() ;
		m_Dialog.dispose() ;
	}
}

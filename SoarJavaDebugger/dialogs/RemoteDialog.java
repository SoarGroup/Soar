/********************************************************************************************
*
* RemoteDialog.java
* 
* Description:	
* 
* Created on 	Mar 16, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package dialogs;

import helpers.FormDataHelper;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;

import debugger.MainFrame;

import sml.Kernel;

/************************************************************************
 * 
 * Asks the user for an IP address and port to use to connect to a remote
 * Soar instance.
 * 
 ************************************************************************/
public class RemoteDialog extends BaseDialog
{
	public static class RemoteInfo
	{
		protected String 	m_IP ;
		protected int 		m_Port ;
		
		public String getIP()
		{
			return m_IP;
		}
		public int getPort()
		{
			return m_Port;
		}
	}
	
	private Text		m_IP ;
	private Text		m_Port ;
	private RemoteInfo	m_Result ;
	
	/********************************************************************************************
	* 
	* Create a simple dialog asking the user for input (a single string).
	* 
	* @param parent			The parent for this dialog (we'll center the dialog within this window)
	* @param title			The title for the dialog
	* @return The value the user entered or null if they cancelled the dialog 
	********************************************************************************************/
	public static RemoteInfo showDialog(Composite parent, String title)
	{
		// Create the dialog window
		RemoteDialog dialog = new RemoteDialog(parent, title) ;
				
		dialog.getDialog().setSize(400, 200) ;
		dialog.centerDialog(parent) ;
		dialog.getDialog().open() ;
		
		dialog.pumpMessages() ;
		
		return dialog.m_Result ;
	}

	/********************************************************************************************
	* 
	* Create the dialog -- the constructor is private because we use a static method to build this.
	* 
	********************************************************************************************/
	private RemoteDialog(Composite parent, String title)
	{
		// Create a basic dialog with OK and Cancel buttons
		super(parent, title, true) ;

		int margin = 10 ;

		// Create a container for the text entry portion
		Composite group = new Group(getDialog(), 0) ;

		GridLayout layout = new GridLayout() ;
		layout.numColumns = 2 ;
		layout.makeColumnsEqualWidth = true ;
		group.setLayout(layout) ;
		
		// Create the text entry fields
		Label ip = new Label(group, SWT.RIGHT) ;
		ip.setText("IP address:") ;
		m_IP = new Text(group, 0) ;
		
		Label port = new Label(group, SWT.RIGHT) ;
		port.setText("Port number:") ;
		m_Port = new Text(group, 0) ;
		m_Port.setText(Integer.toString(Kernel.GetDefaultPort())) ;

		// Lay them out
		GridData data = new GridData(GridData.FILL_HORIZONTAL) ;
		ip.setLayoutData(data) ;
		data = new GridData(GridData.FILL_HORIZONTAL) ;
		m_IP.setLayoutData(data) ;
		data = new GridData(GridData.FILL_HORIZONTAL) ;
		port.setLayoutData(data) ;
		data = new GridData(GridData.FILL_HORIZONTAL) ;
		m_Port.setLayoutData(data) ;

		// Add some help information
		// Using read-only text because label doesn't want to wrap.
		Text help = new Text(getDialog(), SWT.MULTI | SWT.WRAP | SWT.READ_ONLY) ;
		help.setText("You can leave the IP address blank if you are connecting to a process on the same machine.  The default port number for Soar is " + Kernel.GetDefaultPort()) ;
		
		FormData form = FormDataHelper.anchorTop(0) ;
		group.setLayoutData(form) ;
		
		form = new FormData() ;
		form.top = new FormAttachment(group) ;
		form.left = new FormAttachment(0) ;
		form.right = new FormAttachment(100) ;
		form.bottom = new FormAttachment(m_OK) ;
		help.setLayoutData(form) ;
		
		getDialog().layout() ;
	}
	
	/********************************************************************************************
	* 
	* Close the dialog -- either successfully or cancelled.
	* 
	********************************************************************************************/
	protected void endDialog(boolean ok)
	{
		// If the user cancelled or no agents are selected we're done
		if (!ok)
		{
			m_Result = null ;
		}
		else
		{
			m_Result = new RemoteInfo() ;

			String ip = m_IP.getText() ;
			m_Result.m_IP = (ip == null || ip.length() == 0) ? null : ip ;
			
			try
			{
				m_Result.m_Port = Integer.parseInt(m_Port.getText()) ;
			}
			catch (NumberFormatException e)
			{
				MainFrame.ShowMessageBox(m_Dialog.getShell(), "Invalid port value", "The port number must be a positive integer.") ;
				return ;
			}
		}
		
		// Close the dialog
		super.endDialog(ok) ;
	}	
}

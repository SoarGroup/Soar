/********************************************************************************************
*
* LoggingDialog.java
* 
* Description:	
* 
* Created on 	Feb 9, 2006
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package dialogs;

import general.SaveLoad;
import helpers.Logger;
import modules.AbstractView;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import debugger.MainFrame;
import dialogs.RemoteDialog.RemoteInfo;

public class LoggingDialog extends BaseDialog
{
	private AbstractView m_View ;
	private MainFrame 	m_Frame ;
	private Text 		m_FilenameText ;
	private Button		m_Browse ;
	private Button 		m_Append ;
	private Button 		m_FlushAfterWrite ;
//	private Button 		m_Filtered ;
	
	public static LoggingDialog showDialog(MainFrame frame, String title, AbstractView view)
	{
		// Create the dialog window
		Composite parent = frame.getWindow() ;
		LoggingDialog dialog = new LoggingDialog(frame, title, view) ;
				
		//dialog.getDialog().setSize(550, 180) ;
		dialog.getDialog().pack() ;
		dialog.centerDialog(parent) ;
		dialog.open() ;
		
		dialog.pumpMessages() ;

		return dialog ;
	}

	private LoggingDialog(MainFrame frame, String title, AbstractView view)
	{
		super(frame.getWindow().getShell(), title, false) ;
		m_View = view ;
		m_Frame = frame ;
		
		createContents(getOpenArea(), view.getLogger()) ;
	}

	protected void createContents(final Composite window, Logger logger)
	{
		window.setLayout(new GridLayout(2, false));
		
		m_OK.setVisible(true) ;
		m_Cancel.setVisible(true) ;
			    
		// Add the text input fields
		Composite text = new Composite(window, SWT.NONE);
		text.setLayoutData(new GridData(GridData.FILL_HORIZONTAL)) ;
		text.setLayout(new GridLayout(1, true)) ;
		
		new Label(text, SWT.RIGHT).setText("&Filename:");
		m_FilenameText = new Text(text, SWT.BORDER);
		GridData data = new GridData(GridData.FILL_HORIZONTAL);
		data.horizontalSpan = 2;
		m_FilenameText.setLayoutData(data);
		m_FilenameText.setText(logger.getFilename()) ;
		m_FilenameText.selectAll() ;
		
		// Browse for a filename
		m_Browse = new Button(text, SWT.PUSH) ;
		m_Browse.setText("&Browse...") ;
		data = new GridData(GridData.FILL_HORIZONTAL);
		data.horizontalSpan = 1;
		m_Browse.setLayoutData(data);

		Composite options = new Composite(text, SWT.NULL) ;
		options.setLayout(new GridLayout(1, true)) ;
		options.setLayoutData(new GridData(GridData.FILL_HORIZONTAL)) ;
//		options.setLayout(new RowLayout()) ;
		
		// Add the match case checkbox
		m_Append = new Button(options, SWT.CHECK);
		m_Append.setText("&Append to existing file");
		m_Append.setSelection(logger.getAppend()) ;
		
		m_FlushAfterWrite = new Button(options, SWT.CHECK) ;
		m_FlushAfterWrite.setText("&Flush after each write (bit slower but log always up to date)") ;
		m_FlushAfterWrite.setSelection(logger.getFlush()) ;
								
		// Do a find
		m_Browse.addSelectionListener(new SelectionAdapter() {
		  public void widgetSelected(SelectionEvent event) {
				String filename = SaveLoad.SaveFileDialog(m_Frame.getWindow(), new String[] { "*.txt" }, new String[] { "Log file (*.txt)" } , m_Frame.getAppProperties(), "LogSave", "LogLoad") ;
				m_FilenameText.setText(filename) ;
		  }
		});
		
		// Set defaults
		m_OK.setSelection(true);
		m_FilenameText.setFocus();
		getDialog().setDefaultButton(m_OK);
	  }
	
	/********************************************************************************************
	* 
	* Close the dialog -- either successfully or cancelled.
	* 
	********************************************************************************************/
	protected void endDialog(boolean ok)
	{
		// Only process values if the user selected ok
		if (ok)
		{
			String filename = this.m_FilenameText.getText() ;
			boolean append = this.m_Append.getSelection() ;
			boolean flush = this.m_FlushAfterWrite.getSelection() ;
			
			m_View.getLogger().setFilename(filename) ;
			m_View.getLogger().setAppend(append) ;
			m_View.getLogger().setFlush(flush) ;
			
			String result = m_View.getLogger().startLogging(append) ;
			
			if (result != null)
			{
				m_Frame.ShowMessageBox("Error starting log", result) ;
				return ;
			}
		}
		
		// Close the dialog
		super.endDialog(ok) ;
	}	

}

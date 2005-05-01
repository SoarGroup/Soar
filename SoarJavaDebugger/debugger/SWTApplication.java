/********************************************************************************************
*
* SWTApplication.java
* 
* Description:	
* 
* Created on 	Mar 2, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package debugger;

import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.*;
import org.eclipse.swt.custom.*;
import org.eclipse.swt.dnd.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.program.*;
import org.eclipse.swt.graphics.*;

import java.io.* ;

import sml.Agent;
import sml.Kernel;
import doc.Document;

/************************************************************************
 * 
 * SWT based application (we used to have a Swing version too)
 * 
 ************************************************************************/
public class SWTApplication
{
	private Document m_Document = null ;

	static Display display;
	static Shell shell;
	static CoolBar coolBar;
	static Menu chevronMenu = null;
	
	// This is just a little place for me to easily test SWT concepts
	public void startTest(String[] args) throws Exception
	{
		Display display = new Display ();
		Shell shell = new Shell (display);
		shell.setLayout (new FillLayout ());
		final Table table = new Table(shell, SWT.BORDER | SWT.MULTI);
		table.setLinesVisible (true);
		for (int i=0; i<3; i++) {
			TableColumn column = new TableColumn (table, SWT.NONE);
			column.setWidth(100);
		}
		for (int i=0; i<3; i++) {
			TableItem item = new TableItem (table, SWT.NONE);
			item.setText(new String [] {"" + i, "" + i , "" + i});
		}
		final TableEditor editor = new TableEditor (table);
		editor.horizontalAlignment = SWT.LEFT;
		editor.grabHorizontal = true;
		table.addListener (SWT.MouseDown, new Listener () {
			public void handleEvent (Event event) {
				Rectangle clientArea = table.getClientArea ();
				Point pt = new Point (event.x, event.y);
				int index = table.getTopIndex ();
				while (index < table.getItemCount ()) {
					boolean visible = false;
					final TableItem item = table.getItem (index);
					for (int i=0; i<table.getColumnCount (); i++) {
						Rectangle rect = item.getBounds (i);
						if (rect.contains (pt)) {
							final int column = i;
							final Text text = new Text (table, SWT.NONE);
							Listener textListener = new Listener () {
								public void handleEvent (final Event e) {
									switch (e.type) {
										case SWT.FocusOut:
											item.setText (column, text.getText ());
											text.dispose ();
											break;
										case SWT.Traverse:
											switch (e.detail) {
												case SWT.TRAVERSE_RETURN:
													item.setText (column, text.getText ());
													//FALL THROUGH
												case SWT.TRAVERSE_ESCAPE:
													text.dispose ();
													e.doit = false;
											}
											break;
									}
								}
							};
							text.addListener (SWT.FocusOut, textListener);
							text.addListener (SWT.Traverse, textListener);
							editor.setEditor (text, item, i);
							text.setText (item.getText (i));
							text.selectAll ();
							text.setFocus ();
							return;
						}
						if (!visible && rect.intersects (clientArea)) {
							visible = true;
						}
					}
					if (!visible) return;
					index++;
				}
			}
		});
		shell.pack ();
		shell.open ();
		while (!shell.isDisposed ()) {
			if (!display.readAndDispatch ()) display.sleep ();
		}
		display.dispose ();
	}
	
	private void clearClipboard()
	{
	 	Clipboard clipboard = new Clipboard(display);
		String textData = "";	// Copying an empty string to the clipboard to erase it
		TextTransfer textTransfer = TextTransfer.getInstance();
		clipboard.setContents(new Object[]{textData}, new Transfer[]{textTransfer});
		clipboard.dispose();		
	}	
	
	// Returns true if a given option appears in the list
	// (Use this for simple flags like -remote)
	private boolean hasOption(String[] args, String option)
	{
		for (int i = 0 ; i < args.length ; i++)
		{
			if (args[i].equalsIgnoreCase(option))
				return true ;
		}
		
		return false ;
	}

	// Returns the next argument after the matching option.
	// (Use this for parameters like -port ppp)
	private String getOptionValue(String[] args, String option)
	{
		for (int i = 0 ; i < args.length-1 ; i++)
		{
			if (args[i].equalsIgnoreCase(option))
				return args[i+1] ;
		}
		
		return null ;
	}

	// -remote => use a remote connection (with default ip and port values)
	// -ip xxx => use this IP value (implies remote connection)
	// -port ppp => use this port (implies remote connection)
	// Without any remote options we start a local kernel
	public void startApp(String[] args) throws Exception
	{
		// Check for command line options
		boolean remote = hasOption(args, "-remote") ;
		String ip 	= getOptionValue(args, "-ip") ;
		String port = getOptionValue(args, "-port") ;
		
		if (ip != null || port != null)
			remote = true ;
		
		String errorMsg = null ;

		int portNumber = Kernel.GetDefaultPort() ;
		if (port != null)
		{
			try
			{
				portNumber = Integer.parseInt(port) ;
			}
			catch (NumberFormatException e)
			{
				errorMsg = "Passed invalid port value " + port ;
				System.out.println(errorMsg) ;
			}
		}
		
		Display display = new Display() ;
		Shell shell = new Shell(display) ;
		
		// We default to showing the contents of the clipboard in the search dialog
		// so clear it when the app launches, so we don't get random junk in there.
		// Once the app is going, whatever the user is copying around is a reasonable
		// thing to start from, but things from before are presumably unrelated.
		clearClipboard() ;
		
		// The document manages the Soar process
		m_Document = new Document() ;
		
		MainFrame frame = new MainFrame(shell, m_Document) ;
		frame.initComponents();
				
		// We wait until we have a frame up before starting the kernel
		// so it's just as if the user chose to do this manually
		// (saves some special case logic in the frame)
		if (!remote)
		{
			Agent agent = m_Document.startLocalKernel(Kernel.GetDefaultPort()) ;
			frame.setAgentFocus(agent) ;	
		}
		else
		{
			// Start a remote connection
			try
			{
				m_Document.remoteConnect(ip, portNumber) ;
			} catch (Exception e)
			{
				errorMsg = e.getMessage() ;
			}
		}
		
		shell.open() ;

		// We delay any error message until after the shell has been opened
		if (errorMsg != null)
			MainFrame.ShowMessageBox(shell, "Error connecting to remote kernel", errorMsg, SWT.OK) ;

		m_Document.pumpMessagesTillClosed(display) ;

		display.dispose() ;
	}

}

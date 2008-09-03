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

import helpers.FormDataHelper;

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
	
	int m_OffsetY ;
	int m_MinY ;
	int m_MaxY ;
	
	// This is just a little place for me to easily test SWT concepts
	public void startTest(String[] args) throws Exception
	{
		final Display display = new Display ();
		Shell shell = new Shell (display);
		shell.setLayout (new FillLayout ());
		
		Composite main = new Composite(shell, 0) ;
		main.setLayout(new FormLayout()) ;
		
		final Text text = new Text(main, SWT.MULTI|SWT.V_SCROLL) ;
		final Canvas canvas = new Canvas(main, 0) ;
		
		m_OffsetY = 0 ;
		m_MinY = -1 ;
		m_MaxY = -1 ;

		canvas.addPaintListener(new PaintListener() { public void paintControl(PaintEvent e)
		{
			GC gc = e.gc;
			gc.setBackground(display.getSystemColor(SWT.COLOR_GREEN)) ;
			
			Rectangle client = canvas.getClientArea ();
			int height = text.getLineCount() * text.getLineHeight() ;
			if (height < client.height)
				height = client.height ;
			
			if (m_MinY == -1)
			{
				m_MinY = 0 ;
				m_MaxY = client.height ;
			}
			
			double prop = 1.0 - (m_OffsetY / (double)(m_MaxY - m_MinY)) ;
			System.out.println(m_OffsetY) ;
			gc.fillRectangle (0, 0, client.width, (int)(height * prop));
			gc.setBackground(display.getSystemColor(SWT.COLOR_YELLOW)) ;
			gc.fillRectangle (0, (int)(height * prop), client.width, client.height);
		}
		} ) ;
		
		text.getVerticalBar().addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e)
		{
			ScrollBar vBar = (ScrollBar)e.getSource() ;
			int vSelection = vBar.getSelection ();
			m_OffsetY = vSelection ;
			m_MinY = vBar.getMinimum() ;
			m_MaxY = vBar.getMaximum() ;
//			int destY = -vSelection - m_OffsetY;
			/*
			Rectangle rect = image.getBounds ();
			shell.scroll (0, destY, 0, 0, rect.width, rect.height, false);
			*/
//			Rectangle rect = canvas.getBounds() ;
//			canvas.scroll(0, destY, 0, 0, rect.width, rect.height, false) ;
			canvas.redraw() ;
//			m_OffsetY = -vSelection;
		}
		}) ;
		
		text.setLayoutData(FormDataHelper.anchorRight(0)) ;
		canvas.setLayoutData(FormDataHelper.anchorLeft(0)) ;
		
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
	// -agent <name> => select this agent as initial agent (for use with remote connection)
	public void startApp(String[] args) throws Exception
	{
		// Check for command line options
		boolean remote = hasOption(args, "-remote") ;
		String ip 	= getOptionValue(args, "-ip") ;
		String port = getOptionValue(args, "-port") ;
		String agentName = getOptionValue(args, "-agent") ;
		
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
				m_Document.remoteConnect(ip, portNumber, agentName) ;
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

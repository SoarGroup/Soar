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
		
		// The container lets us control the layout of the controls
		// within this window
		Composite parent = shell ;
		Composite container	   = new Composite(parent, SWT.NULL) ;

		RowLayout layout = new RowLayout(SWT.HORIZONTAL) ;
//		layout.wrap = true ;
		layout.fill = true ;
		container.setLayout(layout) ;				

		for (int i = 0 ; i < 10 ; i ++)
		{
			Button button = new Button(container, SWT.PUSH) ;
			button.setText("Button " + Integer.toString(i)) ;
		}
		
		shell.pack ();
		shell.open ();
		while (!shell.isDisposed ()) {
			if (!display.readAndDispatch ()) display.sleep ();
		}
		display.dispose ();
	}
	
	/*
	private void clearClipboard()
	{
	 	Clipboard clipboard = new Clipboard(display);
		String textData = "";	// Copying an empty string to the clipboard to erase it
		TextTransfer textTransfer = TextTransfer.getInstance();
		clipboard.setContents(new Object[]{textData}, new Transfer[]{textTransfer});
		clipboard.dispose();		
	}	
	*/
	
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

	// Command line options:
	// -remote => use a remote connection (with default ip and port values)
	// -ip xxx => use this IP value (implies remote connection)
	// -port ppp => use this port (implies remote connection)
	// -agent <name> => select this agent as initial agent (for use with remote connection)
	// Without any remote options we start a local kernel
	//
	// -maximize => start with maximized window
	// -width <width> -height <height> => start with this window size
	// -x <x> -y <y> => start with this window position
	// (Providing width/height/x/y => not a maximized window)
	public void startApp(String[] args) throws Exception
	{
		// The document manages the Soar process
		m_Document = new Document() ;
		
		// Check for command line options
		boolean remote = hasOption(args, "-remote") ;
		String ip 	= getOptionValue(args, "-ip") ;
		String port = getOptionValue(args, "-port") ;
		String agentName = getOptionValue(args, "-agent") ;
		
		boolean maximize = hasOption(args, "-maximize") ;

		// Remote args
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

		// Window size args
		// We'll override the existing app property values
		if (maximize)
			m_Document.getAppProperties().setAppProperty("Window.Max", maximize) ;

		String[] options = new String[] { "-width", "-height", "-x", "-y" } ;
		String[] props = new String[] { "Window.width", "Window.height", "Window.x", "Window.y" } ;
		
		for (int i = 0 ; i < options.length ; i++)
		{
			String value = getOptionValue(args, options[i]) ;
			if (value != null)
			{
				try
				{
					int val = Integer.parseInt(value) ;
					m_Document.getAppProperties().setAppProperty(props[i], val) ;
					
					// If provide any window information we'll start not-maximized
					m_Document.getAppProperties().setAppProperty("Window.Max", false) ;
				}
				catch (NumberFormatException e)
				{
					System.out.println("Passed invalid value " + value + " for option " + options[i]) ;
				}
			}
		}
		
		Display display = new Display() ;
		Shell shell = new Shell(display) ;
		
		// We default to showing the contents of the clipboard in the search dialog
		// so clear it when the app launches, so we don't get random junk in there.
		// Once the app is going, whatever the user is copying around is a reasonable
		// thing to start from, but things from before are presumably unrelated.
		// This currently fails on Linux version of SWT
		//clearClipboard() ;

		final MainFrame frame = new MainFrame(shell, m_Document) ;
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

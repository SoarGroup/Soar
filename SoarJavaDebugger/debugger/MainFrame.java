/** 
 * MainFrame.java
 *
 * Title:			Soar Debugger
 * Description:	
 * @author			Doug
 * @version			
 */

package debugger;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.events.*;

import java.io.* ;
import java.util.*;

import manager.*;
import menu.*;
import general.* ;
import dialogs.SwtInputDialog;
import doc.* ;
import doc.events.*;
import modules.* ;
import helpers.* ;

import sml.* ;

/************************************************************************
* 
* The frame manages a top level window in the debugger and includes a menu bar.
* 
* There may be multiple MainFrames within a single debugger, sharing an instance
* of Soar (a kernel).  Soar can be running locally (within the debugger) or
* remotely (inside another process--usually an environment).
* 
* If a user wishes to use multiple kernels at once they will need to start
* multiple debugger processes (which they are free to do).
* 
* The most likely use of multiple MainFrames will be when there are multiple
* agents with each agent having a separate window.
* 
* Each MainFrame is connected to a single "Document" which is shared by all
* frames and which manages the Soar process itself.
* 
* The current design associates a default agent with a MainFrame window
* so it's possible for children of that frame to inherit their choice of agent from
* the MainFrame and have all windows working with a single agent.
* However, I would that to just be a default and to support multiple windows
* within a single MainFrame working with different agents if that proved useful
* (by having module windows override that default choice).
* 
************************************************************************/
public class MainFrame
{
	private static final String kNoAgent = "<no agent>" ;

	private static String m_WindowLayoutFile = "SoarDebuggerWindows.xml" ;

	private Composite m_Parent = null ;
	
	/* The main window that contains everything else */	
	private MainWindow m_MainWindow = null ;

	/** The menu bar */	
	private Menu m_MenuBar = null ;

	/** The menus in the menu bar */
	private FileMenu 	m_FileMenu = null ;
	private EditMenu 	m_EditMenu = null ;
	private KernelMenu 	m_KernelMenu = null ;
	private AgentMenu	m_AgentMenu = null ;
	private DemoMenu	m_DemoMenu  = null ;
	
	/** The main document object -- represents the Soar process.  There is only one of these ever in the debugger. */
	private Document	m_Document = null ;

	/** We associate a default agent with a MainFrame, so that windows within that frame can choose to work with that agent if they're not doing something fancy */
//	private String		m_DefaultAgentName = null ;
	
	public Document 	getDocument()	{ return m_Document ; }
	public Menu			getMenuBar()	{ return m_MenuBar ; }
	public MainWindow	getMainWindow()	{ return m_MainWindow ; }
	public Composite	getWindow()		{ return m_MainWindow.getWindow() ; }
	
	/** The font to use for text output (e.g. output from Soar) */
	private Font	m_TextFont = null ;

	/** Windows can register with the frame to learn when it switches focus to a different agent */
	private AgentFocusGenerator m_AgentFocusGenerator = new AgentFocusGenerator() ;
	
	/** The agent this window is currently focused on -- can be null */
	private Agent	m_AgentFocus ;
	
	private SoarChangeListener m_SoarChangeListener = null ;
	
	private boolean m_Shown = false ;
	private boolean m_bClosing = false ;
	
	private java.awt.print.PageFormat m_PageFormat = new java.awt.print.PageFormat() ;
	
	/** We'll keep a list of colors here that we wish to use elsewhere.  When the frame is disposed we should dispose them */
	public Color	m_White ;
	
	// List of all color objects we own and should dispose of when frame closes
	private ArrayList m_Colors = new ArrayList() ;
	
	public MainFrame(Composite parent, Document doc)
	{
		m_Parent = parent ;

		m_Document = doc ;
		// Add ourselves to the list of frames in use
		doc.addFrame(this) ;
			
		m_White = new Color(getDisplay(), 255, 255, 255) ;
		m_Colors.add(m_White) ;	// So we dispose of it when MainFrame is killed

		m_MainWindow = new MainWindow(this, doc, parent) ;
		m_MenuBar  = new Menu(getShell(), SWT.BAR);
		
		// Fill the space with main panel
		m_Parent.setLayout(new FillLayout(SWT.HORIZONTAL)) ;
		
		// Listen for changes to the state of Soar and update our menus accordingly
		m_SoarChangeListener = new SoarChangeListener() {
			public void soarConnectionChanged(SoarConnectionEvent e)
			{
				// If the connection has changed reset the focus to null
				setAgentFocus(null) ;
				
				updateMenus() ; 
			} ;
			
			public void soarAgentListChanged(SoarAgentEvent e)
			{
				// If we're removing the current focus agent then
				// set the focus to null for this window.
				if (e.isAgentRemoved() && Document.isSameAgent(e.getAgent(), m_AgentFocus))
					setAgentFocus(null) ;
				
				updateMenus() ; 
			} ;
		} ;
		
		getDocument().addSoarChangeListener(m_SoarChangeListener) ;	
	}
	
	public Shell getShell()
	{
		return m_Parent.getShell() ;
	}
	
	public Display getDisplay()
	{
		return m_Parent.getDisplay() ;
	}
	
	public static void ShowMessageBox(Shell shell, String title, String text)
	{
		// Only show messages once the shell itself is going
		if (!shell.isVisible())
			return ;
		
		if (title == null)
			title = "Error" ;
		if (text == null)
			text = "<No message>" ;
		
		// Display an SWT message box
		MessageBox msg = new MessageBox(shell, 0) ;
		msg.setText(title) ;
		msg.setMessage(text) ;
		msg.open() ;
	}

	public void ShowMessageBox(String title, String text)
	{		
		// Display an SWT message box
		ShowMessageBox(getShell(), title, text) ;
	}

	public void ShowMessageBox(String text)
	{		
		// Display an SWT message box
		ShowMessageBox(getShell(), "Error", text) ;
	}
	
	public void setTextFont(FontData fontData)
	{
		Font oldFont = m_TextFont ;
		
		// BUGBUG: Should record this in app settings for next run
		
		m_TextFont = new Font(getDisplay(), fontData) ;

		getMainWindow().setTextFont(m_TextFont) ;
		
		// Release the font we were using, once we've stopped using it.
		if (oldFont != null)
			oldFont.dispose() ;
	}
	
	public FontData ShowFontDialog()
	{
		FontDialog dialog = new FontDialog(getShell()) ;
		FontData data = dialog.open() ;
		
		return data ;
	}
	
	/************************************************************************
	*
	* Close the window when the close box is clicked.
	* 
	* @param e				Window closing event
	* 
	*************************************************************************/
	private void thisWindowClosing()
	{	
		// BUGBUG: Should ask if we wish to destroy the agent (if there is a focus agent)

		// Keep track of the fact that we're in the act of closing this window
		m_bClosing = true ;
		
		// Need to explicitly release the focus which in turn will cause any listeners to unregister
		// from this agent (is its still alive).  Otherwise our listeners will still be registered and
		// will try to display output in windows that are disposed.
		this.setAgentFocus(null) ;
		
		// Record the current window positions as properties,
		// which we can then save.
		RecordWindowPositions() ;

		// Look up the name of the default window layout
		File layoutFile = AppProperties.GetSettingsFilePath(m_WindowLayoutFile) ;
		
		// Save the current window positions and other information to the layout file
		this.SaveLayoutFile(layoutFile.toString()) ;

		// Save the user's preferences to the properties file.
		try
		{
			this.getAppProperties().Save() ;
		}
		catch (Throwable t)
		{
			general.Debug.println("Error saving properties file: " + t.getMessage()) ;
		}
		
		// Remove us from the list of active frames
		this.getDocument().removeFrame(this) ;
		
		// Dispose of all of the colors we created
		for (int i = 0 ; i < m_Colors.size() ; i++)
		{
			Color color = (Color)m_Colors.get(i) ;
			color.dispose() ;
		}
		
		if (m_TextFont != null)
			m_TextFont.dispose() ;

		// Exit the app if we're the last frame
		if (this.getDocument().getNumberFrames() == 0)
		{
			getDocument().close() ;
			System.exit(0);
		}
	}
	
	public void close()
	{
		thisWindowClosing() ;
		this.getShell().dispose();
	}
	
	public String ShowInputDialog(String title, String prompt, String initialValue)
	{
		String name = SwtInputDialog.showDialog(this.getShell(), title, prompt, initialValue) ;
		return name ;
	}
	
	/** Switch to focusing on a new agent. */
	public void setAgentFocus(Agent agent)
	{
		// If we're already focused on this agent nothing to do
		if (m_AgentFocus == agent)
			return ;
		
		/** First let everyone know that focus is going away from one agent */
		if (m_AgentFocus != null && m_Document.isAgentValid(m_AgentFocus))
			m_AgentFocusGenerator.fireAgentLosingFocus(this, m_AgentFocus) ;
		
		/** Now let everyone know that focus has gone to the new agent */
		m_AgentFocus = agent ;
		
		if (m_AgentFocus != null)
			m_AgentFocusGenerator.fireAgentGettingFocus(this, m_AgentFocus) ;
		
		// If we're shutting down nothing to update
		if (this.getMainWindow().getWindow().isDisposed())
			return ;
		
		// Update the title to show the new agent name
		updateTitle() ;
		
		// Update the state of the menus
		updateMenus() ;
	}

	/** Let windows listener for when the frame changes focus to a different agent */
	public synchronized void addAgentFocusListener(AgentFocusListener listener) 	{ m_AgentFocusGenerator.addAgentFocusListener(listener); }
	public synchronized void removeAgentFocusListener(AgentFocusListener listener) 	{ m_AgentFocusGenerator.removeAgentFocusListener(listener); }
	
	public Agent getAgentFocus()
	{
		return m_AgentFocus ;
	}
	
	private void updateTitle()
	{
		// Don't want to register an asynch update to the window's title if
		// we're in the act of closing down.
		if (this.m_bClosing)
			return ;
		
		final String agentName = (m_AgentFocus == null ? kNoAgent : m_AgentFocus.GetAgentName()) ;

		// Need to make sure we make this change in the SWT thread as the event may come to us
		// in a different thread
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
        		getShell().setText("Soar Java Debugger - " + agentName);
            }
         }) ;
	}
	
	/** Enable/disable menu items to reflect the current state of the Soar/the debugger. */
	private void updateMenus()
	{
		m_KernelMenu.updateMenu() ;
		m_AgentMenu.updateMenu() ;
	}
	
	public boolean LoadLayoutFile(String filename)
	{
		return false ;
//		return m_PanelTree.LoadFromLayoutFile(getDocument(), filename) ;
	}

	public boolean SaveLayoutFile(String filename)
	{
		return getMainWindow().saveLayoutToFile(filename) ;
	}
	
	public void useDefaultLayout()
	{
		getMainWindow().useDefaultLayout() ;
		
		/*
		ComboCommandWindow comboWindow1 = new ComboCommandLineWindow() ;
		comboWindow1.setMainFrame(this) ;
		comboWindow1.setDocument(getDocument()) ;
//		comboWindow1.setChannel(0) ;
		DebuggerHostWindow trace = new DebuggerHostWindow(this, getDocument(), "Trace", comboWindow1) ;
		*/
/*		
		ComboCommandWindow comboWindow2 = new ComboCommandWindowKeep() ;
		comboWindow2.setMainFrame(this) ;
		comboWindow2.setDocument(getDocument()) ;
//		comboWindow2.setChannel(1) ;
		DebuggerHostWindow general = new DebuggerHostWindow(this, getDocument(), "General", comboWindow2) ;

		ComboCommandWindow comboWindow3 = new ComboCommandWindow() ;
		comboWindow3.setMainFrame(this) ;
		comboWindow3.setDocument(getDocument()) ;
//		comboWindow3.setChannel(2) ;
		comboWindow3.setInitialCommand("print -stack") ;
		DebuggerHostWindow stack = new DebuggerHostWindow(this, getDocument(), "Stack", comboWindow3) ;

		ComboCommandWindow comboWindow4 = new ComboCommandWindow() ;
		comboWindow4.setMainFrame(this) ;
		comboWindow4.setDocument(getDocument()) ;
//		comboWindow4.setChannel(3) ;
		comboWindow4.setInitialCommand("print <o>") ;
		DebuggerHostWindow op = new DebuggerHostWindow(this, getDocument(), "Operator", comboWindow4) ;

		ComboCommandWindow comboWindow5 = new ComboCommandWindow() ;
		comboWindow5.setMainFrame(this) ;
		comboWindow5.setDocument(getDocument()) ;
//		comboWindow5.setChannel(4) ;
		comboWindow5.setInitialCommand("pref <s> operator -names") ;
		DebuggerHostWindow opPrefs = new DebuggerHostWindow(this, getDocument(), "Op Pref", comboWindow5) ;

		ComboCommandWindow comboWindow6 = new ComboCommandWindow() ;
		comboWindow6.setMainFrame(this) ;
		comboWindow6.setDocument(getDocument()) ;
//		comboWindow6.setChannel(5) ;
		comboWindow6.setInitialCommand("print <s>") ;
		DebuggerHostWindow state = new DebuggerHostWindow(this, getDocument(), "State", comboWindow6) ;
		
		ButtonPanel buttons = new ButtonPanel() ;
		buttons.setMainFrame(this) ;
		buttons.setDocument(getDocument()) ;
//		buttons.setChannel(6) ;
		buttons.addButton("Init-soar", "init-soar", 0) ;
		buttons.addButton("Excise chunks", "excise -chunks", 0) ;
		buttons.addButton("Run 5", "run 5", 0) ;	// Channel 0 so output goes to trace window
		buttons.addButton("Run", "run", 0) ;
		buttons.addButton("Stop", "stop-soar", 0) ;
		DebuggerHostWindow buttonWindow = new DebuggerHostWindow(this, getDocument(), "Buttons", buttons) ;
		
		// Make sure the existing tree is removed (if there is one)
		m_PanelTree.clear() ;
		
		// Arrange the initial window layout
		DebuggerPanelNode root = m_PanelTree.getRoot() ;
		root.addBranchBottom(general, 0.8) ;
		root.getTop().addBranchLeft(trace, 0.45) ;
		root.getTop().getLeft().addBranchBottom(buttonWindow, 0.92) ;
		root.getTop().getRight().addBranchTop(stack, 0.3) ;
		root.getTop().getRight().getBottom().addBranchTop(state, 0.3) ;
		root.getTop().getRight().getBottom().getBottom().addPanel(op) ;
		root.getTop().getRight().getBottom().getBottom().addPanel(opPrefs) ;

  		m_PanelTree.createPanels() ;
  		
  		// Reset the location of all panel dividers
		m_PanelTree.setPanelDividers() ;

  		// BUGBUG: Due to some problem in this function we currently have to call it twice.
  		// Need to figure out why this is...
		m_PanelTree.setPanelDividers() ;
		*/
		
//		DebuggerPanelNode root = m_PanelTree.getRoot() ;
//		root.addPanel(trace) ;
		
//		m_PanelTree.createPanels() ;
//		root.addBranchBottom(trace, 0.8) ;

	 }
	
	/************************************************************************
	*
	* Initializes the frame and all of its children.
	* 
	* Called by Application after the frame is constructed.
	* 
	*************************************************************************/
	public void initComponents()
	{
		// the following code sets the frame's initial state

		m_MenuBar.setVisible(true);

		// Add the menus
		m_FileMenu = FileMenu.createMenu(this, getDocument(), "File", 'F') ;
		m_EditMenu = menu.EditMenu.createMenu(this, getDocument(), "Edit", 'E') ;
		m_DemoMenu = DemoMenu.createMenu(this, getDocument(), "Demos", 'D') ;
		m_AgentMenu = AgentMenu.createMenu(this, getDocument(), "Agent", 'A') ;
		m_KernelMenu = KernelMenu.createMenu(this, getDocument(), "Kernel", 'k') ;
				
		// Look up the name of the default window layout
		File layoutFile = AppProperties.GetSettingsFilePath(m_WindowLayoutFile) ;

		boolean loaded = false ;

		// If we have an existing window layout stored, try to load it.		
		if (layoutFile.exists())
		{
			loaded = LoadLayoutFile(layoutFile.toString()) ;
		}
		
		// If we didn't load a layout, use a default layout
		/*
		if (!loaded)
		{
			useDefaultLayout() ;
		}
		*/
		
		getShell().setSize(new Point(704, 616));
		getShell().setMenuBar(m_MenuBar);
		//getContentPane().setLayout(null);
		getShell().setText("Soar Debugger");
//		getContentPane().add(jMainPanel);
//		getContentPane().add(jBottomPanel);
//		getContentPane().add(jLeftPanel);

		getShell().addShellListener(new ShellAdapter() {

			public void shellClosed(ShellEvent e) {
				thisWindowClosing();
			}
		});
		
		// Maximize the window
		// For JDK 1.4: this.setExtendedState(getExtendedState() | JFrame.MAXIMIZED_BOTH);
		// JDK 1.3 approximation.
		//Dimension size = Toolkit.getDefaultToolkit().getScreenSize();
		//size.height -= 60 ;	// Leave a space for Windows task bar...and hope it's at the bottom and single height.
		//this.setSize(size) ;
		getShell().setMaximized(true) ;
		
		// Try to load the user's font preference
		String fontName = this.getAppStringProperty("TextFont.Name") ;
		int    fontSize = this.getAppIntegerProperty("TextFont.Size") ;
		int	   fontStyle = this.getAppIntegerProperty("TextFont.Style") ;
		
		if (fontName != null && fontName.length() > 0 &&
			fontSize != Integer.MAX_VALUE && fontSize > 0 &&
			fontStyle != Integer.MAX_VALUE && fontStyle >= 0)
		{
			setTextFont(new FontData(fontName, fontSize, fontStyle)) ;
		}
		
		// Make sure our menus are enabled correctly
		updateMenus() ;
		
		// BUGBUG: Until we get save/load for layouts working go with the default on start up
		useDefaultLayout() ;
	}
		
	public Font	getTextFont()
	{
		return m_TextFont ;
	}
		
  	public AppProperties getAppProperties()
  	{
  		return this.getDocument().getAppProperties() ;
  	}
	  
	public void setAppProperty(String property, String value)
	{
		this.getAppProperties().setProperty(property, value) ;
	}
	
	public void setAppProperty(String property, double value)
	{
		this.getAppProperties().setProperty(property, Double.toString(value)) ;
	}
	
	public void setAppProperty(String property, int value)
	{
		this.getAppProperties().setProperty(property, Integer.toString(value)) ;
	}
	
	public void setAppProperty(String property, boolean value)
	{
		this.getAppProperties().setProperty(property, String.valueOf(value)) ;
	}
	
	public String getAppStringProperty(String property)
	{
		return this.getAppProperties().getProperty(property) ;
	}
	
	public boolean getAppBooleanProperty(String property, boolean defaultValue)
	{
		String value = this.getAppProperties().getProperty(property) ;
		
		if (value != null)
		{
			return (value.equalsIgnoreCase("true")) ;
		}
		
		return defaultValue ;
	}
	
	public double getAppDoubleProperty(String property)
	{
		try
		{
			String value = this.getAppProperties().getProperty(property) ;
			if (value != null)
			{
				double d = Double.parseDouble(value); 
				return d ;
			}
		}
		catch (NumberFormatException e)
		{
		}
		
		return Double.NaN ;
	}
	
	public int getAppIntegerProperty(String property)
	{
		try
		{
			String value = this.getAppProperties().getProperty(property) ;
			if (value != null)
			{
				int i = Integer.parseInt(value) ;
				return i ;
			}
		}
		catch (NumberFormatException e)
		{
		}
		
		return Integer.MAX_VALUE ;
	}
  	
  	private void initialize()
  	{
  	}
  	
  	public int getWidth()
  	{
  		return m_MainWindow.getWidth() ;
  	}
  	
  	public int getHeight()
  	{
  		return m_MainWindow.getHeight() ;
  	}
  	
  	public Point getLocationOnScreen()
  	{
  		return m_MainWindow.getLocationOnScreen() ;
  	}
  	
  	/** Load the productions for a specific demo.  Filename should be folder/demo.soar -- the path to the demos folder is stored in m_DemoMenu itself */
  	public void loadDemo(File file)
  	{
  		this.m_DemoMenu.loadDemo(file) ;
  	}
  	
  	/** The prime view is generally the trace window.  More specifically it's the first view that reports it can be a prime view */
  	public AbstractView getPrimeView()
  	{
  		return m_MainWindow.getPrimeView() ;
  	}
  	
  	/** Executes a command in the prime view (if there is one).  If there is none, just executes it directly and eats the output */
  	public void executeCommandPrimeView(String commandLine, boolean echoCommand)
  	{
  		AbstractView prime = getPrimeView() ;
  		
  		if (prime != null)
  		{
  			// Send the command to the view so that the output (if any) has a place to be displayed
  			prime.executeAgentCommand(commandLine, echoCommand) ;
  		}
  		else
  		{
  			// Just execute the command directly.  It may return a result but there's no where to display it.
  			if (getAgentFocus() != null)
  				m_Document.sendAgentCommand(getAgentFocus(), commandLine) ;
  		}
  	}
  	
	/************************************************************************
	* 
	* Display the given text in this view (if possible).
	* 
	* This method is used to programmatically insert text that Soar doesn't generate
	* into the output window.
	* 
	*************************************************************************/
  	public void displayTextInPrimeView(String text)
  	{
  		AbstractView prime = getPrimeView() ;
  		
  		if (prime == null)
  			return ;
  		
  		prime.displayText(text) ;
  	}
		
	protected void RecordWindowPositions()
	{
	}	
	/*
	public void jMenuTestTakeSnapshotActionPerformed(java.awt.event.ActionEvent e)
	{
		// Find a filename that doesn't already exist.
		String baseFile = "C:\\snap" ;
		String filename = null ;
		for (int i = 0 ; i < 50 ; i++)
		{
			filename = baseFile + Integer.toString(i) + ".jpg" ;
			File file = new File(filename) ;
			
			if (!file.exists())
				break ;
		}
		
		String imageFile = filename ;

		int maxX = 500 ;
		int maxY = 500 ;
		int width = 100 ;
		int height = 100 ;
		
		JComponent component = null ;
		
		width = maxX ;
		height = maxY ;
		
		try
		{
			FileOutputStream out = new FileOutputStream(imageFile);
			java.awt.image.BufferedImage bi = null ;
			bi = (java.awt.image.BufferedImage)createImage(width, height);
			Graphics g = bi.getGraphics();
			component.paintAll(g);
			com.sun.image.codec.jpeg.JPEGImageEncoder encoder = com.sun.image.codec.jpeg.JPEGCodec.createJPEGEncoder(out);
			encoder.encode(bi);
			out.flush();
			out.close();
		}
		catch (IOException ioe) {}
		catch(java.awt.image.RasterFormatException except) {}
	}
		
	public void jMenuFilePrintActionPerformed(java.awt.event.ActionEvent e)
	{
        java.awt.print.PrinterJob printerJob = java.awt.print.PrinterJob.getPrinterJob() ;

		boolean doPrint = printerJob.printDialog();

		if (doPrint)
		{
			// Indicate which view we want to print
			java.awt.print.Book book = new java.awt.print.Book();
			//book.append(jWorkspacePanel, m_PageFormat);
			printerJob.setPageable(book);
			
			// Start printing
			try
			{
				printerJob.print();

			} catch (java.awt.print.PrinterException exception)
			{
				JOptionPane.showMessageDialog(this, "Printing error: " + exception);
			}
		}
	}
	
	public void jMenuFilePageSetupActionPerformed(java.awt.event.ActionEvent e)
	{
		java.awt.print.PrinterJob printerJob = java.awt.print.PrinterJob.getPrinterJob() ;
		
		m_PageFormat = printerJob.pageDialog(m_PageFormat) ;
		
		// Store the new setting for landscape/portrait.
		// BADBAD: Should store more really (e.g. paper choice).
		boolean landscape = (m_PageFormat.getOrientation() == java.awt.print.PageFormat.LANDSCAPE) ;
		this.setAppProperty("Printing.Landscape", landscape) ;
	}
	*/
}

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
	private MainWindow m_MainPanel = null ;

	/** The menu bar */	
	private Menu m_MenuBar = null ;

	/** The menus in the menu bar */
	private FileMenu 	m_FileMenu = null ;
	private EditMenu 	m_EditMenu = null ;
	private RemoteMenu 	m_RemoteMenu = null ;
	private AgentMenu	m_AgentMenu = null ;
	
	/** The main document object -- represents the Soar process.  There is only one of these ever in the debugger. */
	private Document	m_Document = null ;

	/** We associate a default agent with a MainFrame, so that windows within that frame can choose to work with that agent if they're not doing something fancy */
//	private String		m_DefaultAgentName = null ;
	
	public Document 	getDocument()	{ return m_Document ; }
	public Menu			getMenuBar()	{ return m_MenuBar ; }
	public MainWindow	getMainPanel()	{ return m_MainPanel ; }
	public Composite	getWindow()		{ return m_MainPanel.getWindow() ; }
	
	/** The font to use for text output (e.g. output from Soar) */
	private Font	m_TextFont = null ; //Application.kFixedWidthFont ;

	/** Windows can register with the frame to learn when it switches focus to a different agent */
	private AgentFocusGenerator m_AgentFocusGenerator = new AgentFocusGenerator() ;
	
	/** The agent this window is currently focused on -- can be null */
	private Agent	m_AgentFocus ;
	
	private SoarChangeListener m_SoarChangeListener = null ;
	
	private boolean m_Shown = false ;
	private java.awt.print.PageFormat m_PageFormat = new java.awt.print.PageFormat() ;
	
	public MainFrame(Composite parent, Document doc)
	{
		m_Parent = parent ;
		m_MainPanel = new MainWindow(this, doc, parent) ;
		m_MenuBar  = new Menu(getShell(), SWT.BAR);
		
		// Fill the space with main panel
		m_Parent.setLayout(new FillLayout(SWT.HORIZONTAL)) ;
		
		m_Document = doc ;
				
		// Add ourselves to the list of frames in use
		doc.addFrame(this) ;
		
		// Listen for changes to the state of Soar and update our menus accordingly
		m_SoarChangeListener = new SoarChangeListener() {
			public void soarConnectionChanged(SoarConnectionEvent e)
			{
				// If the connection has changed reset the focus to null
				// This also invalidates any existing agent reference we had so don't try
				// to unregister event handlers etc.
				setAgentFocus(null, true) ;
				
				updateMenus() ; 
			} ;
			
			public void soarAgentListChanged(SoarAgentEvent e)
			{
				// If we're removing the current focus agent then
				// set the focus to null for this window.
				if (e.isAgentRemoved() && Document.isSameAgent(e.getAgent(), m_AgentFocus))
					setAgentFocus(null, false) ;
				
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
	
	public void setVisible(boolean state)
	{
		// BUGBUG - SWT: Not sure how this maps
	}
		
	public void ShowMessageBox(String title, String text)
	{
		// Only show messages once the shell itself is going
		if (!getShell().isVisible())
			return ;
		
		if (title == null)
			title = "Error" ;
		if (text == null)
			text = "<No message>" ;
		
		// Display an SWT message box
		MessageBox msg = new MessageBox(getShell(), 0) ;
		msg.setText(title) ;
		msg.setMessage(text) ;
		msg.open() ;
	}
	
	public void close()
	{
		thisWindowClosing() ;
		this.getShell().dispose();
	}
	
	public void ShowMessageBox(String text)
	{		
		// Display an SWT message box
		ShowMessageBox("Error", text) ;
	}

	public String ShowInputDialog(String title, String prompt, String initialValue)
	{
		String name = SwtInputDialog.showDialog(this.getShell(), title, prompt, initialValue) ;
		return name ;
	}
	
	/** Use this after creating a new window to select the specific agent to focus on */
	public void setAgentFocus(Agent agent)
	{
		setAgentFocus(agent, false) ;
	}
	
	/** Switch to focusing on a new agent.  If invalidate is true then we can't clear events from the previous agent (it's gone) */
	private void setAgentFocus(Agent agent, boolean invalidateExistingAgents)
	{
		// If we're already focused on this agent nothing to do
		if (m_AgentFocus == agent)
			return ;
		
		/** First let everyone know that focus is going away from one agent */
		if (m_AgentFocus != null)
			m_AgentFocusGenerator.fireAgentLosingFocus(this, invalidateExistingAgents ? null : m_AgentFocus) ;
		
		/** Now let everyone know that focus has gone to the new agent */
		m_AgentFocus = agent ;
		
		if (m_AgentFocus != null)
			m_AgentFocusGenerator.fireAgentGettingFocus(this, m_AgentFocus) ;
		
		// If we're shutting down nothing to update
		if (this.getMainPanel().getWindow().isDisposed())
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
		m_RemoteMenu.updateMenu() ;
		m_AgentMenu.updateMenu() ;
	}
	
	public boolean LoadLayoutFile(String filename)
	{
		return false ;
//		return m_PanelTree.LoadFromLayoutFile(getDocument(), filename) ;
	}

	public boolean SaveLayoutFile(String filename)
	{
		return false ;
//		return m_PanelTree.SaveLayoutToFile(filename) ;
	}
	
	public void UseDefaultLayout()
	{
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

		// Add the file menu
		m_FileMenu = FileMenu.createMenu(this, getDocument(), "File", 'F') ;
		//jMenuBar1.add(m_FileMenu.getJMenu()) ;
		
		// Add the edit menu
		m_EditMenu = menu.EditMenu.createMenu(this, getDocument(), "Edit", 'E') ;
		//jMenuBar1.add(m_EditMenu.getJMenu()) ;
		
		// Add the agent menu
		m_AgentMenu = AgentMenu.createMenu(this, getDocument(), "Agent", 'A') ;
		//jMenuBar1.add(m_AgentMenu.getJMenu()) ;

		// Add the remote connection menu
		m_RemoteMenu = RemoteMenu.createMenu(this, getDocument(), "Remote", 'R') ;
		//jMenuBar1.add(m_RemoteMenu.getJMenu()) ;
				
		// Look up the name of the default window layout
		File layoutFile = AppProperties.GetSettingsFilePath(m_WindowLayoutFile) ;

		boolean loaded = false ;

		// If we have an existing window layout stored, try to load it.		
		if (layoutFile.exists())
		{
			loaded = LoadLayoutFile(layoutFile.toString()) ;
		}
		
		// If we didn't load a layout, use a default layout
		if (!loaded)
		{
			UseDefaultLayout() ;
		}

		getShell().setSize(new Point(704, 616));
		getShell().setMenuBar(m_MenuBar);
		//getContentPane().setLayout(null);
		getShell().setText("Soar Debugger");
//		getContentPane().add(jMainPanel);
//		getContentPane().add(jBottomPanel);
//		getContentPane().add(jLeftPanel);

		m_Parent.addControlListener(new ControlAdapter() {
			public void controlResized(ControlEvent e) {
				thisComponentResized(m_Parent.getClientArea());
			}
		});
		
		getShell().addShellListener(new ShellAdapter() {
			// BUGBUG: Appears to be no SWT equivalent for opened
			/*
			public void windowOpened(java.awt.event.WindowEvent e) {
				thisWindowOpened(e);
			}
			*/
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
			setTextFont(new Font(getDisplay(), fontName, fontSize, fontStyle)) ;
		}
		
		// Make sure our menus are enabled correctly
		updateMenus() ;
	}
		
	public Font	getTextFont()
	{
		return m_TextFont ;
	}
	
	public void setTextFont(Font font)
	{
		m_TextFont = font ;
/*	BUGBUG: Need to update this	for SWT
		// Change all future text areas.
		UIManager.put("TextArea.font",font) ;
			
		// Change all of the existing windows
		if (this.getDebuggerTree() != null)
			this.getDebuggerTree().setTextFont(font) ;
			
		// Store the new font values
		setAppProperty("TextFont.Name", font.getFontName()) ;
		setAppProperty("TextFont.Size", font.getSize()) ;
		setAppProperty("TextFont.Style", font.getStyle()) ;
*/
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
  		return m_MainPanel.getWidth() ;
  	}
  	
  	public int getHeight()
  	{
  		return m_MainPanel.getHeight() ;
  	}
  	
  	public Point getLocationOnScreen()
  	{
  		return m_MainPanel.getLocationOnScreen() ;
  	}
  	
	/************************************************************************
	*
	* Called when the component is added to a parent container: we do
	* initialization here that has to wait until the component has been created.
	* 
	*************************************************************************/
	public void addNotify() {
		/* BUGBUG: Need to find SWT equiv
		super.addNotify();
		
		if (m_Shown)
			return;
			
		// resize frame to account for menubar
		JMenuBar jMenuBar = getJMenuBar();
		if (jMenuBar != null) {
			int jMenuBarHeight = jMenuBar.getPreferredSize().height;
			Dimension dimension = getSize();
			dimension.height += jMenuBarHeight;
			setSize(dimension);
		}

		initialize() ;

		m_Shown = true;
		*/
	}

	/************************************************************************
	*
	* Close the window when the close box is clicked.
	* 
	* @param e				Window closing event
	* 
	*************************************************************************/
	void thisWindowClosing()
	{	
		// BUGBUG: Should ask if we wish to destroy the agent (if there is a focus agent)

		// In any case we MUST unregister for events from the current agent or we'll
		// get callbacks to the views which are closing...they'll try to display them in controls
		// that have been disposed.  It's bad.  But for now I can't figure out how to tell when
		// the current agent has been destroyed (and so we can't unregister) and when it hasn't.
		// this.setAgentFocus(null, false) ;
		
		setVisible(false);
		
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

		// Delete the main frame window
		//m_MainPanel.dispose();
		
		// Remove us from the list of active frames
		this.getDocument().removeFrame(this) ;
		
		// Exit the app if we're the last frame
		if (this.getDocument().getNumberFrames() == 0)
		{
			getDocument().close() ;
			System.exit(0);
		}
	}
	
	/************************************************************************
	*
	* Returns the divider position as a percentage of the total
	* height (or width) of the pane.
	* 
	* @param pane			The split pane
	* 
	* @return Divider location as a percentage of total pane height (or width)
	* 
	*************************************************************************/
	protected double getDividerProportionalLocation(SwtSplitPane pane)
	{
		return pane.getDividerLocation() ;
	}
/*
	// Gets the amount beyond the divider, rather than the amount before the divider
	// so we can keep that part constant.
	protected int getDividerLocationRemainder(JSplitPane pane)
	{		
		// The remainder depends on the way the pane is split.
		int size = 0 ;
		if (pane.getOrientation() == JSplitPane.VERTICAL_SPLIT)
		{
			size = pane.getHeight() ;
			
			if (pane.getBottomComponent() == null ||
				pane.getTopComponent() == null)
				return size ;
		}
		else
		{
			size = pane.getWidth() ;

			if (pane.getLeftComponent() == null ||
				pane.getRightComponent() == null)
				return size ;
		}
			
		int pos = pane.getDividerLocation() ;

		if (pos == -1)
			return pos ;

		int remainder = size - pos ;
		
		return remainder ;
	}
	
	// Set the location based on a fixed value beyond the divider
	// rather than before the divider.
	protected void setDividerLocationRemainder(JSplitPane pane, int remainder)
	{
		// The remainder depends on the way the pane is split.
		int size = 0 ;
		if (pane.getOrientation() == JSplitPane.VERTICAL_SPLIT)
			size = pane.getHeight() ;
		else
			size = pane.getWidth() ;
			
		int pos = size - remainder ;
		
		pane.setDividerLocation(pos) ;
	}
*/
	/************************************************************************
	*
	* Lays out the position of the child windows.  We handle our own layout
	* for this frame, because it's complex and using a layout manager would
	* be more complicated than just doing it ourselves.
	*
	*************************************************************************/
	/*
	public void layoutChildWindows() {
		// Get the size of this frame.
		Point thisSize = m_MainPanel.getSize() ;
		
		m_MainPanel.setSize(thisSize) ;
		m_PanelTree.setPanelDividers() ;
	}
	*/
	/************************************************************************
	*
	* Layout the windows in their initial position when the window is opened.
	* 
	* @param e				Window opening event.
	* 
	*************************************************************************/
	/*
	public void thisWindowOpened(java.awt.event.WindowEvent e)
	{
		// Make sure all of the child windows are laid out correctly.
		this.layoutChildWindows() ;
	}
	*/
	/************************************************************************
	*
	* Layout the child windows in their correct positions whenever the
	* frame is resized.
	* 
	* @param e				Window resize event.
	* 
	*************************************************************************/
	public void thisComponentResized(Rectangle newSize)
	{
	}
	
	public void jMenuFileExitActionPerformed(java.awt.event.ActionEvent e)
	{
		thisWindowClosing() ;
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

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
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.events.*;

import java.io.*;
import java.util.*;

import manager.*;
import menu.*;
import general.*;
import dialogs.SwtInputDialog;
import doc.*;
import doc.events.*;
import modules.*;

import sml.*;

/*******************************************************************************
 * 
 * The frame manages a top level window in the debugger and includes a menu bar.
 * 
 * There may be multiple MainFrames within a single debugger, sharing an
 * instance of Soar (a kernel). Soar can be running locally (within the
 * debugger) or remotely (inside another process--usually an environment).
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
 * The current design associates a default agent with a MainFrame window so it's
 * possible for children of that frame to inherit their choice of agent from the
 * MainFrame and have all windows working with a single agent. However, I would
 * that to just be a default and to support multiple windows within a single
 * MainFrame working with different agents if that proved useful (by having
 * module windows override that default choice).
 *  
 ******************************************************************************/
public class MainFrame
{
	public static final FontData kDefaultFontData = new FontData("Courier New", 8, SWT.NORMAL);

	private static final String kNoAgent = "<no agent>";
	private static String m_WindowLayoutFile ;

	private Composite m_Parent = null;

	/* The main window that contains everything else */
	private MainWindow m_MainWindow = null;

	/** The menu bar */
	private Menu m_MenuBar = null;

	/** The menus in the menu bar */
	private FileMenu 		m_FileMenu = null;
	private EditMenu 		m_EditMenu = null;
	private KernelMenu 		m_KernelMenu = null;
	private AgentMenu 		m_AgentMenu = null;
	private DemoMenu 		m_DemoMenu = null;
	private CommandsMenu 	m_CommandsMenu = null;
	private DebugLevelMenu 	m_DebugLevelMenu = null;
	private PrintMenu 		m_PrintMenu = null;
	private LayoutMenu 		m_LayoutMenu = null;
	private HelpMenu 		m_HelpMenu = null ;

	/**
	 * The main document object -- represents the Soar process. There is only
	 * one of these ever in the debugger.
	 */
	private Document m_Document = null;

	/** Used to script the debugger itself */
	private ScriptCommands m_ScriptCommands = null;

	/**
	 * Extended commands set that the user could type at the command line (might
	 * fold this into scripts or vice versa one day--not sure)
	 */
	private DebuggerCommands m_DebuggerCommands = null;

	/** Map of module names that are currently in use in this frame */
	private NameRegister m_NameMap = new NameRegister();

	/**
	 * Each frame has a unique name within the debugger (for the life of one
	 * running of the app)
	 */
	private String m_Name;

	/**
	 * We associate a default agent with a MainFrame, so that windows within
	 * that frame can choose to work with that agent if they're not doing
	 * something fancy
	 */
	//	private String m_DefaultAgentName = null ;
	public Document getDocument()
	{
		return m_Document;
	}
	public Menu getMenuBar()
	{
		return m_MenuBar;
	}
	public MainWindow getMainWindow()
	{
		return m_MainWindow;
	}
	public Composite getWindow()
	{
		return m_MainWindow.getWindow();
	}

	/** The font to use for text output (e.g. output from Soar) */
	private Font m_TextFont = null;

	/**
	 * Windows can register with the frame to learn when it switches focus to a
	 * different agent
	 */
	private AgentFocusGenerator m_AgentFocusGenerator = new AgentFocusGenerator();

	/** The agent this window is currently focused on -- can be null */
	private Agent m_AgentFocus;

	private SoarChangeListener m_SoarChangeListener = null;

	private boolean m_bClosing = false;

	//private java.awt.print.PageFormat m_PageFormat = new java.awt.print.PageFormat();

	/**
	 * We'll keep a list of colors here that we wish to use elsewhere. When the
	 * frame is disposed we should dispose them
	 */
	public Color m_White;

	// List of all color objects we own and should dispose of when frame closes
	private ArrayList m_Colors = new ArrayList();

	public MainFrame(Composite parent, Document doc)
	{
		m_Parent = parent;

		m_Document = doc;

		// Add ourselves to the list of frames in use and
		// get back a unique name to use
		String name = doc.addFrame(this);

		m_Name = name;

		m_ScriptCommands = new ScriptCommands(this, doc);
		m_DebuggerCommands = new DebuggerCommands(this, doc);

		m_White = new Color(getDisplay(), 255, 255, 255);
		m_Colors.add(m_White); // So we dispose of it when MainFrame is killed

		m_MainWindow = new MainWindow(this, doc, parent);
		m_MenuBar = new Menu(getShell(), SWT.BAR);

		// Fill the space with main panel
		m_Parent.setLayout(new FillLayout(SWT.HORIZONTAL));

		// Listen for changes to the state of Soar and update our menus
		// accordingly
		m_SoarChangeListener = new SoarChangeListener() {
			public void soarConnectionChanged(SoarConnectionEvent e)
			{
				// If the connection has changed reset the focus to null
				clearAgentFocus(false);

				updateMenus();
			};

			public void soarAgentListChanged(SoarAgentEvent e)
			{
				// If we're removing the current focus agent then
				// set the focus to null for this window.
				if (e.isAgentRemoved() && Document.isSameAgent(e.getAgent(), m_AgentFocus))
				{
					// If this agent is being closed down then decide if we
					// should
					// destroy the window or not.
					boolean destroyOnClose = m_Document.isCloseWindowWhenDestroyAgent();

					if (destroyOnClose)
					{
						// We need to switch out of this thread because we're in
						// a handler for
						// the before_agent_destroyed() event and calling
						// close() should shutdown the
						// kernel if this is the last window. So we thread
						// switch.
						getDisplay().asyncExec(new Runnable() {
							public void run()
							{
								close();
							}
						});
						return;
					}

					// If we don't destroy the window we need to set the current
					// agent to being nothing
					clearAgentFocus(true);
				}

				updateMenus();
			};
		};

		getDocument().addSoarChangeListener(m_SoarChangeListener);
	}

	public static MainFrame createNewFrame(Display display, Document doc)
	{
		// Create a new window for this agent
		Shell shell = new Shell(display);

		MainFrame frame = new MainFrame(shell, doc);
		frame.initComponents();

		shell.open();

		return frame;
	}

	public String getName()
	{
		return m_Name;
	}

	public Shell getShell()
	{
		return m_Parent.getShell();
	}

	public Display getDisplay()
	{
		return m_Parent.getDisplay();
	}

	public static int ShowMessageBox(Shell shell, String title, String text, int style)
	{
		// Only show messages once the shell itself is going
		if (!shell.isVisible())
			return SWT.CANCEL;

		if (title == null)
			title = "Error";
		if (text == null)
			text = "<No message>";

		// Display an SWT message box
		MessageBox msg = new MessageBox(shell, style);
		msg.setText(title);
		msg.setMessage(text);
		int result = msg.open();

		return result;
	}

	public void ShowMessageBox(String title, String text)
	{
		// Display an SWT message box
		ShowMessageBox(getShell(), title, text, 0);
	}

	public void ShowMessageBox(String text)
	{
		// Display an SWT message box
		ShowMessageBox(getShell(), "Error", text, 0);
	}

	/***************************************************************************
	 * 
	 * Show a message box with a particular icon or set of buttons
	 * 
	 * @param title
	 *            The title
	 * @param text
	 *            The content of the message box
	 * @param style
	 *            e.g. SWT.ICON_INFORMATION or SWT.OK | SWT.CANCEL
	 * @return SWT.OK or SWT.CANCEL
	 **************************************************************************/
	public int ShowMessageBox(String title, String text, int style)
	{
		return ShowMessageBox(getShell(), title, text, style);
	}

	public void setTextFont(FontData fontData)
	{
		Font oldFont = m_TextFont;

		// Record our font choice for the next run
		this.setAppProperty("TextFont.Name", fontData.getName());
		this.setAppProperty("TextFont.Size", fontData.getHeight());
		this.setAppProperty("TextFont.Style", fontData.getStyle());

		// Build the new font
		m_TextFont = new Font(getDisplay(), fontData);

		getMainWindow().setTextFont(m_TextFont);

		// Release the font we were using, once we've stopped using it
		// (in the windows managed by "main window")
		if (oldFont != null)
			oldFont.dispose();
	}

	public FontData ShowFontDialog()
	{
		FontDialog dialog = new FontDialog(getShell());

		// Select our current font as the initial font
		if (m_TextFont != null)
			dialog.setFontList(m_TextFont.getFontData());

		FontData data = dialog.open();

		return data;
	}

	/***************************************************************************
	 * 
	 * Close the window when the close box is clicked.
	 * 
	 * @param e
	 *            Window closing event
	 *  
	 **************************************************************************/
	private void thisWindowClosing()
	{
		// Keep track of the fact that we're in the act of closing this window
		m_bClosing = true;

		// Need to explicitly release the focus which in turn will cause any
		// listeners to unregister from this agent (is its still alive).
		// Otherwise our listeners will
		// still be registered and will try to display output in windows that
		// are disposed.
		// This has the potential to deadlock (waiting to issue unregister calls
		// while we're running) so we put
		// it in a separate thread to avoid that.
		Thread clearFocus = new Thread() {
			public void run()
			{
				clearAgentFocus(false);
			}
		};
		clearFocus.start();

		// DJP: Experiment
		//		if (this.getDocument().getNumberFrames() > 1)
		//			this.setAgentFocus(null);

		// Record the current window positions as properties,
		// which we can then save.
		RecordWindowPositions();

		// Save current layout file
		saveCurrentLayoutFile();

		// Save the user's preferences to the properties file.
		try
		{
			this.getAppProperties().Save();
		} catch (Throwable t)
		{
			general.Debug.println("Error saving properties file: " + t.getMessage());
		}

		// Remove us from the list of active frames
		this.getDocument().removeFrame(this);

		// Dispose of all of the colors we created
		for (int i = 0; i < m_Colors.size(); i++)
		{
			Color color = (Color) m_Colors.get(i);
			color.dispose();
		}

		if (m_TextFont != null)
			m_TextFont.dispose();

		// Exit the app if we're the last frame
		if (this.getDocument().getNumberFrames() == 0)
		{
			getDocument().close(true);
		}
	}

	public void close()
	{
		thisWindowClosing();
		this.getShell().dispose();
	}

	public String ShowInputDialog(String title, String prompt, String initialValue)
	{
		if (initialValue == null)
			initialValue = "";

		String name = SwtInputDialog.showDialog(this.getShell(), title, prompt, initialValue);
		return name;
	}

	public void clearAgentFocus(boolean canUnregisterEvents)
	{
		setAgentFocusInternal(null, canUnregisterEvents) ;
	}
	
	public void setAgentFocus(Agent agent)
	{
		setAgentFocusInternal(agent, true) ;
	}
	
	/** Switch to focusing on a new agent. */
	private void setAgentFocusInternal(Agent agent, boolean canUnregisterEvents)
	{
		// If we're already focused on this agent nothing to do
		if (m_AgentFocus == agent)
			return;

		/** First let everyone know that focus is going away from one agent */
		if (m_AgentFocus != null)
		{
			if (m_Document.isAgentValid(m_AgentFocus) && canUnregisterEvents)
				m_AgentFocusGenerator.fireAgentLosingFocus(this, m_AgentFocus);
			else
				m_AgentFocusGenerator.fireAgentGone(this);
		}

		/** Now let everyone know that focus has gone to the new agent */
		m_AgentFocus = agent;

		if (m_AgentFocus != null)
			m_AgentFocusGenerator.fireAgentGettingFocus(this, m_AgentFocus);

		// If we're shutting down nothing to update
		if (this.getMainWindow().getWindow().isDisposed())
			return;

		// Update the title to show the new agent name
		updateTitle();

		// Update the state of the menus
		updateMenus();
	}

	/**
	 * Let windows listener for when the frame changes focus to a different
	 * agent
	 */
	public synchronized void addAgentFocusListener(AgentFocusListener listener)
	{
		m_AgentFocusGenerator.addAgentFocusListener(listener);
	}
	public synchronized void removeAgentFocusListener(AgentFocusListener listener)
	{
		m_AgentFocusGenerator.removeAgentFocusListener(listener);
	}

	public Agent getAgentFocus()
	{
		return m_AgentFocus;
	}

	private void updateTitle()
	{
		// Don't want to register an asynch update to the window's title if
		// we're in the act of closing down.
		if (this.m_bClosing)
			return;

		final String agentName = (m_AgentFocus == null ? kNoAgent : m_AgentFocus.GetAgentName());
		boolean remote = m_Document.isRemote();
		final String remoteString = remote ? "remote " : "";

		// Need to make sure we make this change in the SWT thread as the event
		// may come to us
		// in a different thread
		Display.getDefault().asyncExec(new Runnable() {
			public void run()
			{
				getShell().setText("Soar Debugger in Java - " + remoteString + agentName);
			}
		});
	}

	/**
	 * Enable/disable menu items to reflect the current state of the Soar/the
	 * debugger.
	 */
	private void updateMenus()
	{
		m_KernelMenu.updateMenu();
		m_AgentMenu.updateMenu();
	}

	/***************************************************************************
	 * 
	 * Save the current layout so when we next launch the app next we'll go back
	 * to this layout.
	 * 
	 * @return True if save file successfully
	 **************************************************************************/
	public boolean saveCurrentLayoutFile()
	{
		// Look up the name of the default window layout
		File layoutFile = AppProperties.GetSettingsFilePath(m_WindowLayoutFile);

		// Save the current window positions and other information to the layout
		// file
		return this.saveLayoutFile(layoutFile.toString());
	}

	/***************************************************************************
	 * 
	 * Load a layout (window positions, types of windows etc.) from a file.
	 * 
	 * @param filename
	 *            The file to load
	 * @param showErrors
	 *            If true display message box on error during load
	 * @return True if load file successfully
	 **************************************************************************/
	public boolean loadLayoutFile(String filename, boolean showErrors)
	{
		return getMainWindow().loadLayoutFromFile(filename, showErrors);
	}

	public boolean saveLayoutFile(String filename)
	{
		return getMainWindow().saveLayoutToFile(filename);
	}

	public void useDefaultLayout()
	{
		getMainWindow().useDefaultLayout();
	}

	/***************************************************************************
	 * 
	 * Generates a unique name from a base name (e.g. from "trace" might create
	 * "trace3"). The name is unique within the frame so we can use it to
	 * cross-reference windows within a layout. It may not be unique within the
	 * entire debugger.
	 * 
	 * @param baseName
	 *            Cannot contain digits
	 * @return The generated name (which has been registered as in use)
	 **************************************************************************/
	public String generateName(String baseName, AbstractView view)
	{
		// Try 200 times -- should be plenty.
		for (int i = 1; i < 200; i++)
		{
			String candidate = baseName + i;

			if (!m_NameMap.isNameInUse(candidate))
			{
				m_NameMap.registerName(candidate, view);
				return candidate;
			}
		}

		throw new IllegalStateException("Could not generate a unique name for basename " + baseName);
	}

	/** Look up the view based on its name * */
	public AbstractView getView(String viewName)
	{
		if (viewName == null)
			return null;

		AbstractView view = m_NameMap.getView(viewName);
		return view;
	}

	public void registerViewName(String name, AbstractView view)
	{
		m_NameMap.registerName(name, view);
	}

	public void unregisterViewName(String name)
	{
		m_NameMap.unregisterName(name);
	}

	public boolean isViewNameInUse(String name)
	{
		return m_NameMap.isNameInUse(name);
	}

	public void clearNameRegistry()
	{
		m_NameMap.clear();
	}

	public void loadSource()
	{
		m_FileMenu.loadSource();
	}

	public FileMenu getFileMenu()
	{
		return m_FileMenu;
	}
	public KernelMenu getKernelMenu()
	{
		return m_KernelMenu;
	}

	private static String getUserLayoutFilename(String version)
	{
		return "SoarDebuggerWindows" + version + ".dlf" ;
	}
	
	public boolean loadUserLayoutFile()
	{
		// Look up the name of the default window layout
		m_WindowLayoutFile = getUserLayoutFilename(Document.kVersion) ;
		File layoutFile = AppProperties.GetSettingsFilePath(m_WindowLayoutFile);

		boolean loaded = false;

		// If this version doesn't exist, go back to an earlier version (if the user happens to have that one)
		// and read it instead.
		for (int i = 0 ; !layoutFile.exists() && i < Document.kPrevVersions.length ; i++)
		{
			layoutFile = AppProperties.GetSettingsFilePath(getUserLayoutFilename(Document.kPrevVersions[i]));
		}
		
		// If we have an existing window layout stored, try to load it.
		if (layoutFile.exists())
		{
			loaded = loadLayoutFile(layoutFile.toString(), true);
		}
		
		return loaded ;
	}
	
	/***************************************************************************
	 * 
	 * Initializes the frame and all of its children.
	 * 
	 * Called by Application after the frame is constructed.
	 *  
	 **************************************************************************/
	public void initComponents()
	{
		// Add the menus
		m_FileMenu = FileMenu.createMenu(this, getDocument(), "&File");
		m_EditMenu = menu.EditMenu.createMenu(this, getDocument(), "&Edit");
		m_PrintMenu = PrintMenu.createMenu(this, getDocument(), "&Print");
		m_CommandsMenu = CommandsMenu.createMenu(this, getDocument(), "&Commands");
		m_DebugLevelMenu = DebugLevelMenu.createMenu(this, getDocument(), "&Debug Level");
		m_DemoMenu = DemoMenu.createMenu(this, getDocument(), "De&mos");
		m_LayoutMenu = LayoutMenu.createMenu(this, getDocument(), "&Layout");
		m_AgentMenu = AgentMenu.createMenu(this, getDocument(), "&Agents");
		m_KernelMenu = KernelMenu.createMenu(this, getDocument(), "&Kernel");
		m_HelpMenu   = HelpMenu.createMenu(this, getDocument(), "&Help") ;
		
		getShell().setMenuBar(m_MenuBar);

		boolean loaded = loadUserLayoutFile() ;

		// If we didn't load a layout, use a default layout
		if (!loaded)
		{
			System.out.println("Failed to load the stored layout, so using default instead");
			useDefaultLayout();
		}

		getShell().addShellListener(new ShellAdapter() {

			public void shellClosed(ShellEvent e)
			{
				thisWindowClosing();
			}
		});

		// Set the initial window size
		boolean max = this.getAppBooleanProperty("Window.Max", true);

		if (max)
		{
			// Maximize the window
			getShell().setMaximized(true);
		} else
		{
			int width = this.getAppIntegerProperty("Window.width");
			int height = this.getAppIntegerProperty("Window.height");
			int xPos = this.getAppIntegerProperty("Window.x");
			int yPos = this.getAppIntegerProperty("Window.y");

			if (width > 0 && width < Integer.MAX_VALUE && height > 0 && height < Integer.MAX_VALUE)
				getShell().setSize(width, height);

			if (xPos >= 0 && xPos < Integer.MAX_VALUE && yPos > 0 && yPos != Integer.MAX_VALUE)
				getShell().setLocation(xPos, yPos);
		}

		// Try to load the user's font preference
		String fontName = this.getAppStringProperty("TextFont.Name");
		int fontSize = this.getAppIntegerProperty("TextFont.Size");
		int fontStyle = this.getAppIntegerProperty("TextFont.Style");

		if (fontName != null && fontName.length() > 0 && fontSize != Integer.MAX_VALUE && fontSize > 0 && fontStyle != Integer.MAX_VALUE
				&& fontStyle >= 0)
		{
			setTextFont(new FontData(fontName, fontSize, fontStyle));
		} else
		{
			setTextFont(kDefaultFontData);
		}

		// Make sure our menus are enabled correctly
		updateMenus();
		updateTitle();
	}

	public Font getTextFont()
	{
		return m_TextFont;
	}

	public AppProperties getAppProperties()
	{
		return this.getDocument().getAppProperties();
	}

	public void setAppProperty(String property, String value)
	{
		this.getAppProperties().setAppProperty(property, value);
	}

	public void setAppProperty(String property, double value)
	{
		this.getAppProperties().setAppProperty(property, value);
	}

	public void setAppProperty(String property, int value)
	{
		this.getAppProperties().setAppProperty(property, value);
	}

	public void setAppProperty(String property, boolean value)
	{
		this.getAppProperties().setAppProperty(property, value);
	}

	public String getAppStringProperty(String property)
	{
		return this.getAppProperties().getAppStringProperty(property);
	}

	public boolean getAppBooleanProperty(String property, boolean defaultValue)
	{
		return this.getAppProperties().getAppBooleanProperty(property, defaultValue);
	}

	public double getAppDoubleProperty(String property)
	{
		return this.getAppProperties().getAppDoubleProperty(property);
	}

	public int getAppIntegerProperty(String property)
	{
		return this.getAppProperties().getAppIntegerProperty(property);
	}

	public int getWidth()
	{
		return m_MainWindow.getWidth();
	}

	public int getHeight()
	{
		return m_MainWindow.getHeight();
	}

	/**
	 * Load the productions for a specific demo. Filename should be
	 * folder/demo.soar -- the path to the demos folder is stored in m_DemoMenu
	 * itself
	 */
	public void loadDemo(File file, boolean echoCommand)
	{
		this.m_DemoMenu.loadDemo(file, echoCommand);
	}
	
	public String getLibraryLocation()
	{
		return this.m_DemoMenu.getLibraryLocation() ;
	}

	/**
	 * The prime view is generally the trace window. More specifically it's the
	 * first view that reports it can be a prime view
	 */
	public AbstractView getPrimeView()
	{
		return m_MainWindow.getPrimeView();
	}

	public AbstractView[] getAllViews()
	{
		return m_MainWindow.getAllViews(false);
	}

	public AbstractView[] getAllOutputViews()
	{
		return m_MainWindow.getAllViews(true);
	}

	/**
	 * Executes a command in the prime view (if there is one). If there is none,
	 * just executes it directly and eats the output
	 */
	public String executeCommandPrimeView(String commandLine, boolean echoCommand)
	{
		AbstractView prime = getPrimeView();

		String result = null;

		if (prime != null)
		{
			// Send the command to the view so that the output (if any) has a
			// place to be displayed
			result = prime.executeAgentCommand(commandLine, echoCommand);
		} else
		{
			// Just execute the command directly. It may return a result but
			// there's no where to display it.
			if (getAgentFocus() != null)
				result = m_Document.sendAgentCommand(getAgentFocus(), commandLine);
		}

		return result;
	}

	/***************************************************************************
	 * 
	 * Execute a command and return the XML form of the output.
	 * 
	 * This is used to evaluate a command which we wish to parse within the
	 * debugger e.g. "set-library-location" (with no args) returns the path to
	 * the library. We ask for the XML form and parse that, making us robust
	 * against changes to the string form shown to the user.
	 * 
	 * NOTE: You should do explicit clean up of the 'response' object we can
	 * check for memory leaks when the debugger exits. If we don't do this
	 * explicitly, the memory will eventually get cleaned up, but only once the
	 * gc runs...could be a while and may not happen before we exit (not
	 * required by Java to do so) so it looks like a leak.
	 * 
	 * So the caller should follow this pattern: AnalyzeXML response = new
	 * AnalyzeXML() ; bool ok = executeCommandXML(commandLine, response) ; ...
	 * process response ... response.delete() ;
	 * 
	 * @param commandLine
	 *            The command to execute (e.g. "watch")
	 * @param response
	 *            An XML object which is filled in by Soar
	 * @return
	 **************************************************************************/
	public boolean executeCommandXML(String commandLine, ClientAnalyzedXML response)
	{
		if (response == null)
			throw new IllegalArgumentException("Must allocate the response and pass it in.  The contents will be filled in during the call.");

		boolean result = false;

		if (getAgentFocus() != null)
			result = m_Document.sendAgentCommandXML(getAgentFocus(), commandLine, response);

		return result;
	}

	public String getCommandResult(String command, String resultParameter)
	{
		ClientAnalyzedXML response = new ClientAnalyzedXML() ;
		boolean ok = executeCommandXML(command, response) ;

		// Check if the command failed
		if (!ok)
		{
			response.delete() ;
			return "" ;
		}
		
		// Debug code to look at the result of the command in XML
		//String check = response.GenerateXMLString(true) ;
		String result = response.GetArgString(resultParameter) ;
		
		// We do explicit clean up so we can check for memory leaks when debugger exits.
		// (See executeCommandXMLcomment for more).
		response.delete() ;
		
		return result ;
	}

	/**
	 * Executes a command that affects the debugger itself, using some form of
	 * scripting language we define
	 */
	public void executeScriptCommand(AbstractView view, String commandLine, boolean echoCommand)
	{
		// We can use "thisframe" and "thisview" as reference values. This
		// function binds them to specific names.
		commandLine = m_ScriptCommands.replaceVariables(this, view, commandLine);

		// Now execute the command
		m_ScriptCommands.executeCommand(commandLine, echoCommand);
	}

	/***************************************************************************
	 * 
	 * Debugger commands are like scripting commands, but they could potentially
	 * be typed by the user at the command line.
	 * 
	 * @param commandLine
	 * @return True if is a command we recognize
	 **************************************************************************/
	public boolean isDebuggerCommand(String commandLine)
	{
		return m_DebuggerCommands.isCommand(commandLine);
	}

	public String getExpandedCommand(String commandLine)
	{
		return m_DebuggerCommands.getExpandedCommand(commandLine);
	}

	public Object executeDebuggerCommand(AbstractView view, String commandLine, boolean echoCommand)
	{
		return m_DebuggerCommands.executeCommand(view, commandLine, echoCommand);
	}

	/***************************************************************************
	 * 
	 * Display the given text in this view (if possible).
	 * 
	 * This method is used to programmatically insert text that Soar doesn't
	 * generate into the output window.
	 *  
	 **************************************************************************/
	public void displayTextInPrimeView(String text)
	{
		AbstractView prime = getPrimeView();

		if (prime == null)
			return;

		prime.displayText(text);
	}

	protected void RecordWindowPositions()
	{
		this.setAppProperty("Window.Max", getShell().getMaximized());

		int width = getShell().getSize().x;
		int height = getShell().getSize().y;

		this.setAppProperty("Window.width", width);
		this.setAppProperty("Window.height", height);

		int xPos = getShell().getLocation().x;
		int yPos = getShell().getLocation().y;

		this.setAppProperty("Window.x", xPos);
		this.setAppProperty("Window.y", yPos);
	}
	/*
	 * public void
	 * jMenuTestTakeSnapshotActionPerformed(java.awt.event.ActionEvent e) { //
	 * Find a filename that doesn't already exist. String baseFile = "C:\\snap" ;
	 * String filename = null ; for (int i = 0 ; i < 50 ; i++) { filename =
	 * baseFile + Integer.toString(i) + ".jpg" ; File file = new File(filename) ;
	 * 
	 * if (!file.exists()) break ; }
	 * 
	 * String imageFile = filename ;
	 * 
	 * int maxX = 500 ; int maxY = 500 ; int width = 100 ; int height = 100 ;
	 * 
	 * JComponent component = null ;
	 * 
	 * width = maxX ; height = maxY ;
	 * 
	 * try { FileOutputStream out = new FileOutputStream(imageFile);
	 * java.awt.image.BufferedImage bi = null ; bi =
	 * (java.awt.image.BufferedImage)createImage(width, height); Graphics g =
	 * bi.getGraphics(); component.paintAll(g);
	 * com.sun.image.codec.jpeg.JPEGImageEncoder encoder =
	 * com.sun.image.codec.jpeg.JPEGCodec.createJPEGEncoder(out);
	 * encoder.encode(bi); out.flush(); out.close(); } catch (IOException ioe) {}
	 * catch(java.awt.image.RasterFormatException except) {} }
	 * 
	 * public void jMenuFilePrintActionPerformed(java.awt.event.ActionEvent e) {
	 * java.awt.print.PrinterJob printerJob =
	 * java.awt.print.PrinterJob.getPrinterJob() ;
	 * 
	 * boolean doPrint = printerJob.printDialog();
	 * 
	 * if (doPrint) { // Indicate which view we want to print
	 * java.awt.print.Book book = new java.awt.print.Book();
	 * //book.append(jWorkspacePanel, m_PageFormat);
	 * printerJob.setPageable(book); // Start printing try { printerJob.print(); }
	 * catch (java.awt.print.PrinterException exception) {
	 * JOptionPane.showMessageDialog(this, "Printing error: " + exception); } } }
	 * 
	 * public void jMenuFilePageSetupActionPerformed(java.awt.event.ActionEvent
	 * e) { java.awt.print.PrinterJob printerJob =
	 * java.awt.print.PrinterJob.getPrinterJob() ;
	 * 
	 * m_PageFormat = printerJob.pageDialog(m_PageFormat) ; // Store the new
	 * setting for landscape/portrait. // BADBAD: Should store more really (e.g.
	 * paper choice). boolean landscape = (m_PageFormat.getOrientation() ==
	 * java.awt.print.PageFormat.LANDSCAPE) ;
	 * this.setAppProperty("Printing.Landscape", landscape) ; }
	 */
}
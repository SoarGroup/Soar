/********************************************************************************************
*
* DebuggerWindow.java
* 
* Created on 	Nov 12, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;


import general.JavaElementXML;
import helpers.Logger;
import manager.MainWindow;
import manager.Pane;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.dnd.Clipboard;
import org.eclipse.swt.dnd.TextTransfer;
import org.eclipse.swt.events.*;

import doc.* ;
import doc.events.*;
import debugger.* ;

import sml.Agent;

/********************************************************************************************
* 
* The functionality that a debugger window needs to support.
* 
********************************************************************************************/
public abstract class AbstractView implements AgentFocusListener
{	
	/** The document is used to represent the Soar process.  There is one shared document
	 *  for the entire debugger. **/
	protected Document 		m_Document ;
	
	/** The main frame that owns this window.  **/
	protected MainFrame		m_Frame ;
	
	/** The window that owns this view */
	protected Pane			m_Pane ;

	/** The name of this view -- this name is unique within the frame (not the entire debugger) so we can cross-reference based on this name */
	protected String		m_Name ;
	
	/** The window which will contain all others within this view */
	protected Composite		m_Container ;
	
	/** A class to help with logging, should this window wish to offer that **/
	protected Logger		m_Logger = new Logger() ;
	
	/** The line separator Soar uses and that we therefore use. */
	public static final String kLineSeparator = "\n" ;
	
	/** The line separator that this platform uses (e.g. Windows has \r\n).  Unless you're sure, you usually want the other one (kLineSeparator) when working with Soar commands/output. **/
	public static final String kSystemLineSeparator = System.getProperty("line.separator") ;
	
	public static final String kTagView = "view" ;
		
	/********************************************************
	 * All AbstractView's need to have a default constructor
	 * as that's how we build them (using reflection) when
	 * loading/saving or picking up module plug-ins.
	 *********************************************************/
	public AbstractView()
	{
	}
		
	/** The main frame that owns this window.  **/
	public MainFrame getMainFrame() { return m_Frame; }

	/** The document is used to represent the Soar process.  There is one shared document for the entire debugger. **/
	public Document getDocument() 	{ return m_Document ; }

	/** The name of this view -- this name is unique within the frame (not the entire debugger) so we can cross-reference based on this name */
	public String	getName() 		{ return m_Name ; }

	/** The window that owns this view */
	public Pane getPane() 			{ return m_Pane ; }
	
	/** The SWT window that contains everything within this module */
	public Composite getWindow()	{ return m_Container ; }

	/** The logger helper class */
	public Logger getLogger() { return m_Logger ; }
	
	/********************************************************************************************
	 * 
	 * Returns the agent that is associated with the main frame.
	 * A given window can choose to override this and work with a different agent.
	 * 
	 * This can return null if no agent is currently selected in the main frame.
	 * 
	********************************************************************************************/
	public Agent getAgentFocus()
	{
		return m_Frame.getAgentFocus() ;
	}
	
	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public abstract String getModuleBaseName() ;

	/********************************************************************************************
	* 
	* Return true if this view shouldn't be user resizable.  E.g. A text window would return false
	* but a bar for buttons would return true.
	* 
	********************************************************************************************/
	public abstract boolean isFixedSizeView() ;

	/********************************************************************************************
	* 
	* Change the font we use to display text items in this window.
	* 
	********************************************************************************************/
	public abstract void setTextFont(Font f) ;
		
	/********************************************************************************************
	* 
	* Initialize this window and its children.
	* Should call initContainer() at the start to complete initialization of the abstract view.
	* 
	********************************************************************************************/
	public abstract void init(MainFrame frame, Document doc, Pane parentPane) ;
	
	/********************************************************************************************
	* 
	* Copy current selection to the clipboard.
	* 
	********************************************************************************************/
	public abstract void copy() ;

	/********************************************************************************************
	* 
	* Execute whatever is on the clipboard as a command
	* 
	********************************************************************************************/
	public void paste()
	{
		// Retrieve the current contents of the clipboard
	    Clipboard clipboard = new Clipboard(this.m_Container.getDisplay());
	    TextTransfer textTransfer = TextTransfer.getInstance();
	    String textData = (String)clipboard.getContents(textTransfer);
	    clipboard.dispose();
	    
	    // If there's something text based on the clipboard, execute it.
	    if (textData != null)
	    	this.executeAgentCommand(textData, true) ;
	}

    // Creates a convenient listener for Control-V.
	// Adding a key listener for this event allows us to call our paste method so commands are pasted into the command
	// buffer rather than into the window.
	protected Listener m_ControlV = new Listener() { public void handleEvent(Event e) {
    	if (e.type == SWT.KeyDown)
    	{
	    	int key = e.keyCode ;
	    	int mask = e.stateMask ;
	    	
	    	if (key == 'v' && (mask & SWT.CTRL) > 0)
	    		paste() ;
    	}
    } } ;

	/************************************************************************
	* 
	* Converts this object into an XML representation.
	* 
	* @param tagName		The tag name to use for the top XML element created by this view
	* @param storeContent	If true, record the content from the display (e.g. the text from a trace window)
	* 
	*************************************************************************/
	public abstract general.JavaElementXML convertToXML(String tagName, boolean storeContent) ;

	/************************************************************************
	* 
	* Rebuild the object from an XML representation.
	* 
	* @param frame			The top level window that owns this window
	* @param doc			The document we're rebuilding
	* @param parent			The pane window that owns this view
	* @param element		The XML representation of this command
	* 
	*************************************************************************/
	public abstract void loadFromXML(MainFrame frame, doc.Document doc, Pane parent, general.JavaElementXML element) throws Exception ;

	/************************************************************************
	* 
	* Execute a command (send it to Soar) and display the output in a manner
	* appropriate to this view.
	* 
	* @param Command		The command line to execute
	* @param echoCommand	If true, display the command in the output window as well.
	* 
	* The result (if any) is also returned to the caller.
	* 
	*************************************************************************/
	public abstract String executeAgentCommand(String command, boolean echoCommand) ;
	
	/************************************************************************
	* 
	* Returns true if this window can display output from commands executed through
	* the "executeAgentCommand" method.
	* 
	*************************************************************************/
	public abstract boolean canDisplayOutput() ;

	/************************************************************************
	* 
	* Display the given text in this view (if possible).
	* 
	* This method is used to programmatically insert text that Soar doesn't generate
	* into the output window.
	* 
	*************************************************************************/
	public abstract void displayText(String text) ;

	/************************************************************************
	* 
	* Return true from a subclass if the window is a trace window.  We'll send
	* menu commands (like "source file") to this window and display the results of
	* the command here.  Multiple windows can return true in which case the first is
	* selected (currently).
	* 
	*************************************************************************/
	public boolean canBePrimeWindow() { return false ; }
	
	/************************************************************************
	* 
	* Set the focus to this window so the user can type commands easily.
	* Return true if this window wants the focus (some don't have a sensible
	* place to focus on).
	* 
	*************************************************************************/
	public abstract boolean setFocus() ;
	public abstract boolean hasFocus() ;
	
	/************************************************************************
	* 
	* Given a context menu and a control, fill in the items you want to 
	* see in the menu.  The simplest is to just call "fillWindowMenu".
	* 
	* This call is made after the user has clicked to bring up the menu
	* so we can create a dymanic menu based on the current context.
	* 
	* You also have to call createContextMenu() to request a context menu
	* be attached to a specific control.
	* 
	*************************************************************************/
	protected abstract void fillInContextMenu(Menu contextMenu, Control control, int mouseX, int mouseY) ;
	
	/************************************************************************
	* 
	* Search for the next occurance of 'text' in this view and place the selection
	* at that point.
	* 
	* @param text			The string to search for
	* @param searchDown		If true search from top to bottom
	* @param matchCase		If true treat the text as case-sensitive
	* @param wrap			If true after reaching the bottom, continue search from the top
	* @param searchHidden	If true and this view has hidden text (e.g. unexpanded tree nodes) search that text
	* 
	*************************************************************************/
	public abstract boolean find(String text, boolean searchDown, boolean matchCase, boolean wrap, boolean searchHiddenText) ;
    
	/************************************************************************
	* 
	* Register and unregister for Soar events for this agent.
	* (E.g. a trace window might register for the print event)
	* 
	* ClearAgentEvents is called when the agent has already been deleted
	* (so we can't unregister but should just clear our references)
	* 
	*************************************************************************/
	protected abstract void registerForAgentEvents(Agent agent) ;	
	protected abstract void unregisterForAgentEvents(Agent agent) ;
	protected abstract void clearAgentEvents() ;
	
	/** Close down this window, doing any necessary clean up */
	public void close(boolean dispose)
	{
		unregisterName() ;
		unregisterForAgentEvents(getAgentFocus()) ;

		if (dispose)
			m_Container.dispose() ;
	}
	
	/** This method is called when we initialize the module. */
	protected void setValues(MainFrame frame, Document doc, Pane parentPane)
	{
		m_Frame 	= frame ;
		m_Document  = doc ;
		m_Pane 	    = parentPane ;
		
		// We want to know when the frame focuses on particular agents
		m_Frame.addAgentFocusListener(this) ;
		
		// Initialize the logger (in case we wish to use it)
		m_Logger.setView(this) ;
	}
	
	/** Generates a unique name for this window */
	public void generateName(MainFrame frame)
	{
		if (m_Name != null)
			throw new IllegalStateException("Should only call this once.  If we really want to allow this then if m_Name != null unregister this name before generating a new name.") ;
		
		// Use the frame that's passed in, in case our frame pointer is yet to be initialized
		m_Name = frame.generateName(getModuleBaseName(), this) ;
	}
	
	public void changeName(String newName)
	{
		unregisterName() ;
		m_Name = newName ;
		m_Frame.registerViewName(newName, this) ;
	}
	
	protected void unregisterName()
	{
		m_Frame.unregisterViewName(getName()) ;		
	}
	
	public void agentGettingFocus(AgentFocusEvent e)
	{
		if (e.getAgent() != null)
			registerForAgentEvents(e.getAgent()) ;
	}
	
	public void agentLosingFocus(AgentFocusEvent e)
	{
		// We may be passed "null" for the agent losing focus if the
		// agent is no longer valid to access (e.g. we shutdown the kernel).
		// We still get a notification in case we need to do any other clean-up.
		unregisterForAgentEvents(e.getAgent()) ;
	}
	
	public void agentGone(AgentFocusEvent e)
	{
		clearAgentEvents() ;
	}
	
	protected void addItem(Menu menu, String text, final String command)
	{
		MenuItem item = new MenuItem (menu, SWT.PUSH);
		item.setText (text) ;
		
		final AbstractView view = this ;
		
		item.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) { m_Frame.executeScriptCommand(view, command, false) ; } } ) ;
	}	
	
	/********************************************************************************************
	 * 
	 * Display a dialog that allows the user to adjust properties for this window
	 * e.g. choosing whether to clear the window everytime a new command executes or not.
	 * 
	********************************************************************************************/
	public abstract void showProperties() ;
	
	/************************************************************************
	* 
	* Clear the display (the text part if any)
	* 
	*************************************************************************/
	public abstract void clearDisplay() ;
	
	/************************************************************************
	* 
	* Override and return false if it doesn't make sense to clear this
	* type of view and so we shouldn't offer it to the user in the context menu.
	* 
	*************************************************************************/	
	public boolean offerClearDisplay() { return true ; }

	/************************************************************************
	* 
	* Override and return false if it doesn't make sense to log the contexts of this
	* type of view and so we shouldn't offer it to the user in the context menu.
	* 
	*************************************************************************/	
	public boolean offerLogging() { return true ; }

	public void fillWindowMenu(Menu menu, boolean asSubMenu, boolean includeCopyPaste)
	{
		if (asSubMenu)
		{
			MenuItem header = new MenuItem(menu, SWT.CASCADE) ;
			header.setText("Window") ;
			
			Menu windowMenu = new Menu(menu.getShell(), SWT.DROP_DOWN) ;
			header.setMenu(windowMenu) ;

			menu = windowMenu ;
		}

		// Copy and paste where paste really means to paste into the command stream, not into the window.
		if (includeCopyPaste)
		{
			addItem(menu, "Copy", "copy " + m_Frame.getName() + " " + this.getName()) ;
			addItem(menu, "Paste", "paste " + m_Frame.getName() + " " + this.getName()) ;
			new MenuItem(menu, SWT.SEPARATOR) ;
		}
		
		addItem(menu, "Properties ...", "properties " + m_Frame.getName() + " " + this.getName()) ;

		if (offerLogging())
		{
			if (!m_Logger.isLogging())
				addItem(menu, "Log this window...", "log dialog " + m_Frame.getName() + " " + this.getName()) ;
			else
				addItem(menu, "Stop logging to " + m_Logger.getFilePath(), "log stop " + m_Frame.getName() + " " + this.getName()) ;
		}
		
		if (offerClearDisplay())
			addItem(menu, "Clear window", "clear " + m_Frame.getName() + " " + this.getName()) ;

		new MenuItem(menu, SWT.SEPARATOR) ;
		addItem(menu, "Add tab ...", "addtab " + m_Frame.getName() + " " + this.getName()) ;		
		addItem(menu, "Add window to right ...", "addview " + m_Frame.getName() + " " + this.getName() + " " + MainWindow.kAttachRightValue) ;
		addItem(menu, "Add window to left ...", "addview " + m_Frame.getName() + " " + this.getName() + " " + MainWindow.kAttachLeftValue) ;
		addItem(menu, "Add window to top ...", "addview " + m_Frame.getName() + " " + this.getName() + " " + MainWindow.kAttachTopValue) ;
		addItem(menu, "Add window to bottom ...", "addview " + m_Frame.getName() + " " + this.getName() + " " + MainWindow.kAttachBottomValue) ;
		
		// Some commands are only if we're showing tabs (so names are visible)
		if (!getPane().isSingleView())
		{
			if (getPane().isTabAtTop())
				addItem(menu, "Move tabs to bottom", "movetabs " + m_Frame.getName() + " " + this.getName() + " bottom") ;
			else
				addItem(menu, "Move tabs to top", "movetabs " + m_Frame.getName() + " " + this.getName() + " top");
		}
		new MenuItem(menu, SWT.SEPARATOR) ;
		addItem(menu, "Replace window ...", "replaceview " + m_Frame.getName() + " " + this.getName()) ;
		addItem(menu, "Rename window ...", "renameview " + m_Frame.getName() + " " + this.getName()) ;
		new MenuItem(menu, SWT.SEPARATOR) ;
		addItem(menu, "Remove window", "removeview " + m_Frame.getName() + " " + this.getName()) ;
	}
	
	protected Menu createContextMenu(final Control control)
	{
		// Create a custom context menu for the text area
		final Menu menu = new Menu (control.getShell(), SWT.POP_UP);
		menu.addMenuListener(new MenuListener() {
			public void menuShown(MenuEvent e)
			{
				// Clear any existing items from the menu and then create new items
				while (menu.getItemCount() > 0)
				{
					MenuItem child = menu.getItem(0) ;
					child.dispose() ;
				}

				// We'll build the menu dynamically based on the text the user selects etc.
				Point mouse = control.getDisplay().getCursorLocation() ;
				fillInContextMenu(menu, control, mouse.x, mouse.y) ;
			}
			public void menuHidden(MenuEvent e)
			{
			}
		
		}) ;
		control.setMenu (menu);
		return menu ;
	}
	
	/** Look up the node in the XML tree that matches this pane.  Need to have called "convertToXML" for everything before calling this */
	public JavaElementXML getElementXML()
	{
		return (JavaElementXML)getWindow().getData(Pane.kXMLKey) ;
	}
	
}



/********************************************************************************************
*
* ComboCommandWindow.java
* 
* Created on 	Nov 12, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import manager.Pane;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.internal.win32.OS;
import org.eclipse.swt.internal.win32.TCHAR;
import org.eclipse.swt.events.*;

import com.sun.corba.se.internal.corba.EncapsInputStream;

import sml.Agent;
import sml.smlPrintEventId;
import sml.smlRunEventId;

import debugger.* ;
import doc.* ;
import doc.events.*;
import general.* ;
import helpers.*;

/********************************************************************************************
* 
* Simple window with a combo box at the top where the user enters commands and a history
* of those commands is stored.
* 
********************************************************************************************/
public class ComboCommandView extends AbstractView
{
	/** Everything lives inside this container so we can control the layout */
	private Composite m_Container ;

	private Text m_Text ;
	
	private Combo m_CommandCombo ;

	protected Menu m_ContextMenu ;
	
	private boolean		m_Inited = false ;
		
	/** If true, clear the text window each time we execute a command. */
	protected boolean	m_ClearEachCommand = true ;
	
	/** If true, we clear the command entry (combo box) line each time we execute a command */
	protected boolean 	m_ClearComboEachCommand = false ;
	
	/** If true, run the current command again whenever Soar stops running */
	protected boolean	m_UpdateOnStop = true ;
	
	/** If true, will display output from "run" commands */
	protected boolean	m_ShowTraceOutput = false ;
	
	/** Prompt the user to type commands in the combo box with this text */
	protected String 	m_PromptForCommands ;
	
	/** If true, the combo box is at the top of the window (otherwise at the bottom) */
	protected boolean	m_ComboAtTop = true ;
		
	protected SoarChangeListener m_ChangeListener ;
	
	/** The history of commands for this window */
	protected CommandHistory m_CommandHistory = new CommandHistory() ;
	
	protected int m_StopCallback ;
	protected int m_PrintCallback ;
		
	// The constructor must take no arguments so it can be called
	// from the loading code and the new window dialog
	public ComboCommandView()
	{
	}

	
	protected ParseSelectedText.SelectedObject getCurrentSelection()
	{
		if (m_Text.getCaretPosition() == -1)
			return null ;
		
		ParseSelectedText selection = new ParseSelectedText(m_Text.getText(), m_Text.getCaretPosition(), 0) ;
		
		return selection.getParsedObject() ;
	}
		
	// This method can be overridden by derived classes
	protected void fillInContextMenu(Menu contextMenu)
	{
		// Get the current selected text
		ParseSelectedText.SelectedObject selectionObject = getCurrentSelection() ;
		String selection = "<none>" ;
		
		if (selectionObject != null)
			selection = selectionObject.toString() ;

		// Clear any existing items from the menu and then create new items
		while (contextMenu.getItemCount() > 0)
		{
			MenuItem child = contextMenu.getItem(0) ;
			child.dispose() ;
		}
		
		boolean simple = true ;
		
		// For now let's dump the output into the main trace window
		// That lets us do interesting things by copying output into one of the
		// scratch windows and working with it there.
		AbstractView outputView = m_MainFrame.getPrimeView() ;
		if (outputView == null) outputView = this ;
		
		selectionObject.fillMenu(getDocument(), outputView, contextMenu, simple) ;
	}

	public void setInitialCommand(String command)
	{
		m_CommandHistory.UpdateHistoryList(command, true) ;
		makeComboBoxMatchHistory() ;
	}
	
	private void layoutControls()
	{
		// I'll use forms everywhere for consistency and so it's easier
		// to extend them later if we wish to add something.
		m_Container.setLayout(new FormLayout()) ;

		if (!this.m_ComboAtTop)
		{
			FormData attachBottom = FormDataHelper.anchorFull(0) ;
			attachBottom.bottom = new FormAttachment(m_CommandCombo) ;
			
			m_Text.setLayoutData(attachBottom) ;
			m_CommandCombo.setLayoutData(FormDataHelper.anchorBottom(0)) ;
		}
		else
		{
			FormData attachTop = FormDataHelper.anchorFull(0) ;
			attachTop.top = new FormAttachment(m_CommandCombo) ;
			
			m_Text.setLayoutData(attachTop) ;
			m_CommandCombo.setLayoutData(FormDataHelper.anchorTop(0)) ;			
		}
		
		m_Container.layout() ;
	}
		
	public void Init(MainFrame frame, Document doc, Pane parentPane)
	{
		if (m_Inited)
			return ;

		m_MainFrame = frame ;
		m_Document  = doc ;
		setPane(parentPane) ;
		
		Composite parent = parentPane.getWindow() ;
		
		// The container lets us control the layout of the controls
		// within this window
		m_Container	   = new Composite(parent, SWT.NULL) ;
		
		// Make the text output wrap and have scrollbars
		m_Text 		   = new Text(m_Container, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL | SWT.WRAP | SWT.READ_ONLY) ;
		m_CommandCombo = new Combo(m_Container, 0) ;
		
		if (m_PromptForCommands != null)
			m_CommandCombo.setText(m_PromptForCommands) ;
		
		layoutControls() ;
		
		m_Inited = true ;

		m_Text.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e)
			{ if (e.button == 2 || e.button == 3) rightButtonPressed(e) ; } ;
		}) ;
		
		// Listen for key presses on the combo box so we know when the user presses return
		m_CommandCombo.addKeyListener(new KeyAdapter() { public void keyPressed(KeyEvent e) { comboKeyPressed(e) ; } }) ;
		
		// Decide how many rows to show in the combo list
		m_CommandCombo.setVisibleItemCount(this.m_CommandHistory.kMaxHistorySize > 10 ? 10 : this.m_CommandHistory.kMaxHistorySize) ;

		if (getBackgroundColor() != null)
			m_Text.setBackground(getBackgroundColor()) ;
		
		// Listen for when this window is disposed and unregister for anything we registered for
		m_Container.addDisposeListener(new DisposeListener() { public void widgetDisposed(DisposeEvent e) { removeListeners() ; } } ) ;
		
		// We want to know when the frame focuses on particular agents
		m_MainFrame.addAgentFocusListener(this) ;
		
		// Create a custom context menu for the text area
		final Menu menu = new Menu (m_Text.getShell(), SWT.POP_UP);
		menu.addMenuListener(new MenuListener() {
			public void menuShown(MenuEvent e)
			{
				// We'll build the menu dynamically based on the text the user selects etc.
				fillInContextMenu(menu) ;
			}
			public void menuHidden(MenuEvent e)
			{
			}
		
		}) ;
		m_ContextMenu = menu ;
		m_Text.setMenu (menu);
	}
	
	 /*******************************************************************************************
	 * 
	 * When the user clicks the right mouse button, sets the selection to that location (just like a left click).
	 * This makes right clicking on a piece of text much easier as it's just one click rather than
	 * having to left click to place the selection and then right click to bring up the menu.
	 * 
	*******************************************************************************************
	 */
	protected void rightButtonPressed(MouseEvent e)
	{
		// Unfortunately, SWT doesn't support getting a character location from a position
		// so I'm adding support for it here.  However, this support is pure Windows code.
		// We'll need to figure out how to have code like this and still compile the debugger
		// on Linux (even if this option won't work on Linux).
		
		// Send an EM_CHARFROMPOS message to the underlying edit control
		int handle = m_Text.handle ;
		int lParam = e.y << 16 | e.x ;	// Coords are packed as high-word, low-word
		int result = OS.SendMessage (handle, OS.EM_CHARFROMPOS, 0, lParam);

		// Break out the character and line position from the result
		int charPos = result & (0xFFFF) ;
		int linePos = (result >>> 16) ;
		
		// Set the selection to the character position (which is measured from the first character
		// in the control).
		m_Text.clearSelection() ;
		m_Text.setSelection(charPos) ;
		
		//System.out.println("Char " + charPos + " Line " + linePos) ;
	}
	
	public Color getBackgroundColor()
	{
		return null ;
	}
	
	// We need to remove listeners that we registered for within the debugger here.
	// Agent listeners (from Soar) are handled separately.
	private void removeListeners()
	{
		m_MainFrame.removeAgentFocusListener(this) ;
	}
	
	/************************************************************************
	* 
	* Set the focus to this window so the user can type commands easily.
	* Return true if this window wants the focus.
	* 
	*************************************************************************/
	public boolean setFocus()
	{
		// For us, we focus on the combo box, where the user types commands.
		m_CommandCombo.setFocus() ;
		
		return true ;
	}
	
	public boolean hasFocus()
	{
		return m_CommandCombo.isFocusControl() ;
	}
	
	private void comboKeyPressed(KeyEvent e)
	{
		Combo combo = (Combo)e.getSource() ;
				
		// This character may be platform specific...I wonder how we look it up from SWT?
		if (e.character == '\r')
		{
			String command = combo.getText() ;
			commandEntered(command, true) ;	
		}
	}
	
	private void makeComboBoxMatchHistory()
	{
		String[] history = m_CommandHistory.getHistory() ;
		
		this.m_CommandCombo.setItems(history) ;		
	}
	
	private void commandEntered(String command, boolean updateHistory)
	{
		// Clear the text area if this window is configured that way.
		if (this.m_ClearEachCommand)
			m_Text.setText("") ;
		
		// Update the combo box history list
		if (updateHistory)
		{
			this.m_CommandHistory.UpdateHistoryList(command, true) ;

			// Update the combo box to match the history list
			makeComboBoxMatchHistory() ;
		}
		
		// Clear the text from the edit control in the combo box if we're
		// configured that way.
		if (m_ClearComboEachCommand)
			this.m_CommandCombo.setText("") ;

		// Send the command to Soar
		executeAgentCommand(command, false) ;
	}
	
	public void setTextFont(Font f)
	{
		m_Text.setFont(f) ;
	}
	
	/************************************************************************
	* 
	* Converts this object into an XML representation.
	* 
	*************************************************************************/
	public general.ElementXML ConvertToXML(String title)
	{
		ElementXML element = new ElementXML(title) ;
		
		// It's useful to record the class name to uniquely identify the type
		// of object being stored at this point in the XML tree.
		Class cl = this.getClass() ;
		element.addAttribute(ElementXML.kClassAttribute, cl.getName()) ;

		// Store this object's properties.
		//element.addAttribute("Channel", Integer.toString(m_Channel)) ;
		element.addAttribute("UpdateOnStop", Boolean.toString(m_UpdateOnStop)) ;
		element.addAttribute("ClearEachCommand", Boolean.toString(m_ClearEachCommand)) ;
		element.addAttribute("ClearComboEachCommand", Boolean.toString(this.m_ClearComboEachCommand)) ;
		element.addAttribute("ComboAtTop", Boolean.toString(this.m_ComboAtTop)) ;
		element.addAttribute("ShowTraceOutput", Boolean.toString(m_ShowTraceOutput)) ;
		
		element.addChildElement(this.m_CommandHistory.ConvertToXML("History")) ;
		
		return element ;
	}

	/************************************************************************
	* 
	* Create an instance of the class.  It does not have to be fully initialized
	* (it's the caller's responsibility to finish the initilization).
	* 
	*************************************************************************/
	public static ComboCommandView createInstance()
	{
		return new ComboCommandView() ;
	}
	
	/************************************************************************
	* 
	* Rebuild the object from an XML representation.
	* 
	* @param frame			The top level window that owns this window
	* @param doc			The document we're rebuilding
	* @param element		The XML representation of this command
	* 
	*************************************************************************/
	public void LoadFromXML(MainFrame frame, doc.Document doc, general.ElementXML element) throws Exception
	{
		m_MainFrame		   = frame ;
		m_Document		   = doc ;
		//m_Channel 		   = element.getAttributeIntThrows("Channel") ;
		m_UpdateOnStop	   = element.getAttributeBooleanThrows("UpdateOnStop") ;
		m_ClearEachCommand = element.getAttributeBooleanThrows("ClearEachCommand") ;
		m_ClearComboEachCommand = element.getAttributeBooleanThrows("ClearComboEachCommand") ;
		m_ComboAtTop	   = element.getAttributeBooleanThrows("ComboAtTop") ;
		m_ShowTraceOutput  = element.getAttributeBooleanThrows("ShowTraceOutput") ;

		ElementXML history = element.findChildByName("History") ;
		if (history != null)
			this.m_CommandHistory.LoadFromXML(doc, history) ;
		
		// Reset the combo box to match the history list we just loaded	
		makeComboBoxMatchHistory() ;
	}
	
	public void runEventHandler(int eventID, Object data, Agent agent, int phase)
	{
		// TEMPTEMP: Removed for now - buggy (getting multiple events from a simple "run 1")
		if (false && this.m_UpdateOnStop && eventID == smlRunEventId.smlEVENT_AFTER_RUNNING.swigValue())
		{
			System.out.println("Received run event") ;
			
			// Retrieve the current command in the combo box
			
			// BUGBUG: Not sure how to make this thread safe yet.
			final String command = getCommandText() ;

			// We don't want to execute "run" commands when Soar stops--or we'll get into an
			// infinite loop.
			if (!getDocument().getSoarCommands().isRunCommand(command))
			{
				// If Soar is running in the UI thread we can make
				// the update directly.
				if (!Document.kDocInOwnThread)
					commandEntered(command, false) ;
				else
				{
					// Have to make update in the UI thread.
					// Callback comes in the document thread.
			        Display.getDefault().asyncExec(new Runnable() {
			            public void run() {
			            	commandEntered(command, false) ;
			            }
			         }) ;
				}
			}
		}
	}

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
	public String executeAgentCommand(String command, boolean echoCommand)
	{
		if (echoCommand)
			appendText("\n" + command) ;
		
		String result = getDocument().sendAgentCommand(getAgentFocus(), command) ;

		// Output from Soar doesn't include newlines and assumes that we insert
		// a newline before the result of the command is displayed.
		if (result != null && result.length() > 0)
			appendText("\n" + result) ;

		return result ;
	}
	
	/************************************************************************
	* 
	* Display the given text in this view (if possible).
	* 
	* This method is used to programmatically insert text that Soar doesn't generate
	* into the output window.
	* 
	*************************************************************************/
	public void displayText(String text)
	{
		appendText(text) ;
	}

	private void appendText(final String text)
	{
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			m_Text.append(text) ;
			return ;
		}

		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
            	m_Text.append(text) ;
            }
         }) ;
	}
/*
	private void setText(final String text)
	{
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			m_Text.setText(text) ;
			return ;
		}

		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
            	m_Text.setText(text) ;
            }
         }) ;
	}
*/
	private String getCommandText()
	{
		if (!Document.kDocInOwnThread)
			return m_CommandCombo.getText() ;

		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
        	String result ;
            public void run() {
            	result = m_CommandCombo.getText() ;
            }
         }) ;

        return "p <s>" ;
	}

	public void printEventHandler(int eventID, Object data, Agent agent, String message)
	{
		if (m_Text.isDisposed())
		{
			if (eventID == smlPrintEventId.smlEVENT_PRINT.swigValue())
				System.out.println("Naughty -- we're still registered for the print event although our window has been closed: agent " + agent.GetAgentName()) ;
			else
				System.out.println("Naughty -- got another print event although our window has been closed: agent " + agent.GetAgentName()) ;
			return ;
		}
		
		if (eventID == smlPrintEventId.smlEVENT_PRINT.swigValue())
			appendText(message) ;
	}

	protected void registerForAgentEvents(Agent agent)
	{
		if (agent == null)
			return ;
		
		m_StopCallback  = agent.RegisterForRunEvent(smlRunEventId.smlEVENT_AFTER_RUNNING, this, "runEventHandler", this) ;
		
		if (m_ShowTraceOutput)
			m_PrintCallback = agent.RegisterForPrintEvent(smlPrintEventId.smlEVENT_PRINT, this, "printEventHandler", this) ;		
	}
	
	protected void unregisterForAgentEvents(Agent agent)
	{
		if (agent == null)
			return ;
	
		System.out.println("Unregister events for " + agent.GetAgentName()) ;
		
		boolean ok = agent.UnregisterForRunEvent(m_StopCallback) ;

		if (m_ShowTraceOutput)
		{
			System.out.println("Unregister print event for " + agent.GetAgentName()) ;
			ok = agent.UnregisterForPrintEvent(m_PrintCallback) && ok ;
		}
		
		if (!ok)
			throw new IllegalStateException("Problem unregistering for events") ;
	}
}

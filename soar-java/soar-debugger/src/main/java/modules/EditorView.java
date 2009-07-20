/********************************************************************************************
*
* EditorView.java
* 
* Description:	
* 
* Created on 	Apr 30, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import general.JavaElementXML;
import helpers.CommandHistory;
import helpers.FormDataHelper;
import manager.Pane;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.widgets.*;

import sml.Agent;
import debugger.MainFrame;
import doc.Document;

/************************************************************************
 * 
 * Provides a window where a production can be edited and then
 * loaded into Soar.
 * 
 ************************************************************************/
public class EditorView extends AbstractView
{
	protected Composite	m_ComboContainer ;
	protected Combo 	m_CommandCombo ;
	protected Text		m_Text ;
	protected Button	m_LoadButton ;
	
	/** If true, the combo box is at the top of the window (otherwise at the bottom) */
	protected boolean	m_ComboAtTop = true ;
			
	/** The history of commands for this window */
	protected CommandHistory m_CommandHistory = new CommandHistory() ;
				
	// The constructor must take no arguments so it can be called
	// from the loading code and the new window dialog
	public EditorView()
	{
	}
	
	public String getModuleBaseName()
	{
		return "editor" ;
	}

	public boolean isFixedSizeView()
	{
		return false;
	}

	/************************************************************************
	* 
	* Returns true if this window can display output from commands executed through
	* the "executeAgentCommand" method.
	* 
	*************************************************************************/
	public boolean canDisplayOutput()
	{
		return false ;
	}
	
	/********************************************************************************************
	* 
	* Copy current selection to the clipboard.
	* 
	********************************************************************************************/
	public void copy()
	{
		m_Text.copy() ;
	}
	
	/********************************************************************************************
	* 
	* Execute whatever is on the clipboard as a command.
	* Overriding the default behavior to produce a simple paste into the window.
	* (Usually we intercept this and execute what's on the command line)
	* 
	********************************************************************************************/
	public void paste()
	{
		m_Text.paste() ;
	}

	public void setTextFont(Font f)
	{
		m_Text.setFont(f) ;
	}

	/** The control we're using to display the output in this case **/
	protected Control getDisplayControl()
	{
		return m_Text ; 
	}
	
	/** Separate out the laying out of the combo box as we might want to put controls next to it */
	protected void layoutComboBar(boolean top)
	{
		m_ComboContainer.setLayout(new FormLayout()) ;

		FormData containerData = top ? FormDataHelper.anchorTop(0) : FormDataHelper.anchorBottom(0) ;
		m_ComboContainer.setLayoutData(containerData) ;

		FormData comboData = FormDataHelper.anchorTopLeft(0) ;
		comboData.right = new FormAttachment(m_LoadButton) ;
		m_CommandCombo.setLayoutData(comboData) ;

		FormData buttonData = FormDataHelper.anchorTop(0) ;
		buttonData.left = null ;
		m_LoadButton.setLayoutData(buttonData) ;
	}
	
	/** Layout the combo box and the main display window */
	protected void layoutControls()
	{
		// I'll use forms everywhere for consistency and so it's easier
		// to extend them later if we wish to add something.
		m_Container.setLayout(new FormLayout()) ;
		
		if (!this.m_ComboAtTop)
		{
			FormData attachBottom = FormDataHelper.anchorFull(0) ;
			attachBottom.bottom = new FormAttachment(m_ComboContainer) ;
			
			getDisplayControl().setLayoutData(attachBottom) ;
			layoutComboBar(m_ComboAtTop) ;
		}
		else
		{
			FormData attachTop = FormDataHelper.anchorFull(0) ;
			attachTop.top = new FormAttachment(m_ComboContainer) ;
			
			getDisplayControl().setLayoutData(attachTop) ;
			layoutComboBar(m_ComboAtTop) ;
		}
		
		// Lots of attempts to make sure the size is computed correctly
		m_Container.pack(true) ;
		m_Container.layout() ;
		m_ComboContainer.pack(true) ;
		m_ComboContainer.layout() ;
		m_Container.getParent().pack(true) ;
		m_Container.getParent().layout() ;
		
		// Create a context menu for m_Text.
		// It will be filled in via a call to fillInContextMenu when the menu is popped up
		// (this allows for dynamic content)
		createContextMenu(getDisplayControl()) ;		
	}

	public Color getBackgroundColor() { return m_Frame.m_White ; }

	/********************************************************************************************
	* 
	* Create the window that will display the production being edited
	* 
	********************************************************************************************/
	protected void createDisplayControl(Composite parent)
	{
		m_Text = new Text(parent, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL | SWT.WRAP) ;
		m_Text.pack() ;
		
		// Listen for Ctrl-Return to load the production immediately
		m_Text.addKeyListener(new KeyAdapter() { public void keyPressed(KeyEvent e) { textKeyPressed(e) ; } } ) ;
		
		createContextMenu(m_Text) ;
		
		m_LoadButton = new Button(m_ComboContainer, SWT.PUSH) ;
		m_LoadButton.setText("Load production [Ctrl-Return]") ;
		m_LoadButton.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { loadProduction() ; } } ) ;
	}
	
	protected String getProduction()
	{
		// The edit control returns lines with \r\n (DOS style).
		// We want just the simple newline version or we get extra junk
		// in the output window when we load the production.
		String production = m_Text.getText() ;
		
		production = production.replaceAll("\r","") ;
		
		return production ;
	}
	
	protected void loadProduction()
	{
		String production = getProduction() ;
		
		if (production != null && production != "")
		{
			m_Frame.getPrimeView().executeAgentCommand(production, true) ;
		}
	}
	
	/********************************************************************************************
	 * @param frame
	 * @param doc
	 * @param parentPane
	 * 
	 * @see modules.AbstractView#init(debugger.MainFrame, doc.Document, manager.Pane)
	 ********************************************************************************************/
	public void init(MainFrame frame, Document doc, Pane parentPane)
	{
		setValues(frame, doc, parentPane) ;
		Composite parent = parentPane.getWindow() ;
		
		// The container lets us control the layout of the controls
		// within this window
		m_Container	   = new Composite(parent, SWT.NULL) ;
		
		m_ComboContainer = new Composite(m_Container, 0) ;
		m_CommandCombo = new Combo(m_ComboContainer, 0) ;
				
		// Listen for key presses on the combo box so we know when the user presses return
		m_CommandCombo.addKeyListener(new KeyAdapter() { public void keyPressed(KeyEvent e) { comboKeyPressed(e) ; } }) ;
		
		// Decide how many rows to show in the combo list
		m_CommandCombo.setVisibleItemCount(CommandHistory.kMaxHistorySize > 10 ? 10 : CommandHistory.kMaxHistorySize) ;

		// Create the control that will display output from the commands
		createDisplayControl(m_Container) ;
		
		getDisplayControl().setBackground(getBackgroundColor()) ;
		
		layoutControls() ;
	}

	private void comboKeyPressed(KeyEvent e)
	{
		Combo combo = (Combo)e.getSource() ;
				
		// This character may be platform specific...I wonder how we look it up from SWT?
		if (e.character == '\r')
		{
			if ((e.stateMask & SWT.CONTROL) > 0)
			{
				// Control-return => load production into Soar
				loadProduction() ;
			}
			else
			{
				// Display the production based on the name entered
				String production = combo.getText() ;
				productionEntered(production, true) ;	
			}
		}
	}

	protected void textKeyPressed(KeyEvent e)
	{
		//Text text = (Text)e.getSource() ;
		
		if (e.character == '\r' && (e.stateMask & SWT.CONTROL) > 0)
		{
			loadProduction() ;
			e.doit = false ;
		}
	}
	
	private void makeComboBoxMatchHistory(boolean placeTopItemInCombo)
	{
		// Changing the list of items clears the text entry field
		// which we may not wish to do, so we'll keep it and manually
		// reset it.
		String text = m_CommandCombo.getText() ;

		String[] history = m_CommandHistory.getHistory() ;
		this.m_CommandCombo.setItems(history) ;	
		
		if (placeTopItemInCombo && history[0] != null)
			m_CommandCombo.setText(history[0]) ;
		else
			m_CommandCombo.setText(text) ;
	}
		
	private void productionEntered(String production, boolean updateHistory)
	{
		// Update the combo box history list
		if (updateHistory)
		{
			this.m_CommandHistory.UpdateHistoryList(production, true) ;

			// Update the combo box to match the history list
			makeComboBoxMatchHistory(false) ;
		}
		
		String commandLine = m_Document.getSoarCommands().getPrintCommand(production) ;
		String result = m_Document.sendAgentCommand(getAgentFocus(), commandLine) ;
		
		if (result != null && result != "")
			m_Text.setText(result) ;
	}

	public JavaElementXML convertToXML(String tagName, boolean storeContent)
	{
		JavaElementXML element = new JavaElementXML(tagName) ;
		
		// It's useful to record the class name to uniquely identify the type
		// of object being stored at this point in the XML tree.
		Class cl = this.getClass() ;
		element.addAttribute(JavaElementXML.kClassAttribute, cl.getName()) ;

		if (m_Name == null)
			throw new IllegalStateException("We've created a view with no name -- very bad") ;
		
		// Store this object's properties.
		element.addAttribute("Name", m_Name) ;
		element.addAttribute("ComboAtTop", Boolean.toString(this.m_ComboAtTop)) ;
		
		if (this.m_CommandCombo != null)
		{
			String command = m_CommandCombo.getText() ;
			if (command != null && command.length() != 0)
				this.m_CommandHistory.UpdateHistoryList(command, true) ;
		}

		element.addChildElement(this.m_CommandHistory.ConvertToXML("History")) ;
		
		return element ;
	}

	public void loadFromXML(MainFrame frame, Document doc, Pane parent, JavaElementXML element) throws Exception
	{
		setValues(frame, doc, parent) ;

		m_Name			   = element.getAttribute("Name") ;
		m_ComboAtTop	   = element.getAttributeBooleanThrows("ComboAtTop") ;
		
		JavaElementXML history = element.findChildByName("History") ;
		if (history != null)
			this.m_CommandHistory.LoadFromXML(doc, history) ;
		
		// Register that this module's name is in use
		frame.registerViewName(m_Name, this) ;
		
		// Actually create the window
		init(frame, doc, parent) ;

		// Reset the combo box to match the history list we just loaded	
		makeComboBoxMatchHistory(true) ;
	}

	public String executeAgentCommand(String command, boolean echoCommand)
	{
		return null;
	}

	public void displayText(String text)
	{
	}

	public boolean setFocus()
	{
		return false;
	}

	public boolean hasFocus()
	{
		return false;
	}

	protected void fillInContextMenu(Menu contextMenu, Control control, int mouseX, int mouseY)
	{
		fillWindowMenu(contextMenu, false, true) ;
	}

	public boolean find(String text, boolean searchDown, boolean matchCase, boolean wrap, boolean searchHiddenText)
	{
		return false;
	}

	protected void registerForAgentEvents(Agent agent)
	{
	}

	protected void unregisterForAgentEvents(Agent agent)
	{
	}

	protected void clearAgentEvents()
	{
	}

	public void showProperties()
	{
		m_Frame.ShowMessageBox("Properties", "There are currently no properties for this view") ;
	}

	public void clearDisplay()
	{
		m_Text.setText("") ;
	}
}

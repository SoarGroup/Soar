/********************************************************************************************
*
* ButtonPanel.java
* 
* Created on 	Nov 23, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import general.ElementXML;

import manager.Pane;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.events.*;

import sml.Agent;

import java.util.*;
import debugger.MainFrame;
import doc.Document;

/********************************************************************************************
* 
* Shows a series of buttons for issuing commands.
* 
********************************************************************************************/
public class ButtonView extends AbstractView
{
	protected Composite	m_Container ;
	
	protected static class ButtonInfo
	{
		protected String			m_Name ;
		protected String			m_Command ;
		protected SelectionListener m_Listener ;
	}
	
	/** A list of ButtonInfo objects */
	protected ArrayList m_ButtonList = new ArrayList() ;
	
	public ButtonView()
	{
	}
	
	public void setTextFont(Font f)
	{
		// We ignore this as our window doesn't display text.
	}

	/**
	 * @see modules.AbstractView#registerContextMenuMouseListener(MouseListener)
	 */
	public void registerContextMenuMouseListener(MouseListener listener)
	{
		// We only add it to the parent window, not to each button.
		// This (a) seems more logical from the user perspective since the menu is about the panel (not the buttons)
		// and  (b) means we don't have to store and re-add this listener as the list of buttons changes.
		// SWT: this.getControl().addMouseListener(listener) ;
	}

	/********************************************************************************************
	* 
	* Remove ourselves from any events that we are listening for (in preparation for deleting this window)
	* 
	********************************************************************************************/
	public void removeListeners()
	{
	}
	
	/********************************************************************************************
	* 
	* Create a new button with visible text of "name" that issues command "command" when the
	* user presses the button.  (A null command is valid and does nothing).
	* 
	* The caller can also (optionally) register a listener which will be called when the button
	* is pressed, to perform additional actions.
	* 
	********************************************************************************************/
	public void addButton(String name, String command, SelectionListener listener)
	{
		ButtonInfo button = new ButtonInfo() ;
		button.m_Name    = name ;
		button.m_Command = command ;
		button.m_Listener = listener ;
		
		m_ButtonList.add(button) ;
	}

	/********************************************************************************************
	* 
	* Create a new button with visible text of "name" that issues command "command" when the
	* user presses the button.  (A null command is valid and does nothing).
	* 
	********************************************************************************************/
	public void addButton(String name, String command)
	{
		addButton(name, command, null) ;
	}
	
	public void Init(MainFrame frame, Document doc, Pane parentPane)
	{
		m_MainFrame = frame ;
		m_Document  = doc ;
		
		setPane(parentPane) ;		
		createButtonPanel(m_Pane.getWindow()) ;
	}

	protected void createButtonPanel(final Composite parent)
	{
		// The container lets us control the layout of the controls
		// within this window
		m_Container	   = new Composite(parent, SWT.NULL) ;

		// BUGBUG: Need to figure out how to make the button pane resize itself
		// to be multiple rows when needed.
		// These are some efforts that have not succeeded.
		RowLayout layout = new RowLayout() ;
		layout.wrap = true ;
		m_Container.setLayout(layout) ;		
//		m_Container.addControlListener(new ControlAdapter() {
//			public void controlResized(ControlEvent e) { parent.getParent().layout(true) ; } }) ;
		
		// Create and add buttons for each button info structure
		for (int i = 0 ; i < m_ButtonList.size() ; i++)
		{
			final ButtonInfo info = (ButtonInfo)m_ButtonList.get(i) ;
			
			final int pos = i ;
			Button button = new Button(m_Container, SWT.PUSH) ;
			button.setText(info.m_Name) ;
			
			// When the user presses the button we call our default handler and
			// optionally a handler registered with the button (to do a custom action)
			button.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) { buttonPressed(e, pos) ; if (info.m_Listener != null) info.m_Listener.widgetSelected(e); } ;
			}) ;
		}
	}

	protected void buttonPressed(SelectionEvent e, int pos)
	{
		ButtonInfo button = (ButtonInfo)m_ButtonList.get(pos) ;
		String command = button.m_Command ;

		// Execute the command in the prime view for the debugger
		if (command != null)
			m_MainFrame.executeCommandPrimeView(command, true) ;
	}

	public String executeAgentCommand(String command, boolean echoCommand)
	{		
		// Send the command to Soar but there's no where to display the output
		// so we just eat it.
		String result = getDocument().sendAgentCommand(getAgentFocus(), command) ;
		
		return result ;
	}
	
	public void displayText(String text)
	{
		// Nowhere to display it so eat it.
	}
	
	/************************************************************************
	* 
	* Set the focus to this window so the user can type commands easily.
	* Return true if this window wants the focus.
	* 
	*************************************************************************/
	public boolean setFocus()
	{
		return false ;
	}

	public boolean hasFocus()
	{
		return false ;
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

		element.addAttribute("ButtonCount", Integer.toString(m_ButtonList.size())) ;
		
		// Save information for each button in the panel
		for (int i = 0 ; i < m_ButtonList.size() ; i++)
		{
			ButtonInfo button = (ButtonInfo)m_ButtonList.get(i) ;
			
			ElementXML child = new ElementXML("Button") ;
			child.addAttribute("Name", button.m_Name) ;
			child.addAttribute("Command", button.m_Command) ;
			
			element.addChildElement(child) ;
		}
			
		return element ;
	}

	/************************************************************************
	* 
	* Create an instance of the class.  It does not have to be fully initialized
	* (it's the caller's responsibility to finish the initilization).
	* 
	*************************************************************************/
	public static ButtonView createInstance()
	{
		return new ButtonView() ;
	}

	/************************************************************************
	* 
	* Rebuild the object from an XML representation.
	* 
	* @param doc			The document we're rebuilding
	* @param element		The XML representation of this command
	* 
	*************************************************************************/
	public void LoadFromXML(MainFrame frame, doc.Document doc, general.ElementXML element) throws Exception
	{
		m_MainFrame		   = frame ;		
		m_Document		   = doc ;
		
		m_ButtonList.clear() ;
		
		int size = element.getAttributeIntThrows("ButtonCount") ;
		
		for (int i = 0 ; i < size ; i++)
		{
			ElementXML child = element.getChild(i) ;
			
			ButtonInfo button = new ButtonInfo() ;
			button.m_Name    = child.getAttributeThrows("Name") ;
			button.m_Command = child.getAttributeThrows("Command") ;
			
			m_ButtonList.add(button) ;
		}
		
		// Reset the list of buttons to match the list we just loaded	
		//createButtonPanel() ;
	}

	/** So far the button panel doesn't care about events from the agent */
	protected void registerForAgentEvents(Agent agent)
	{
	}
	
	protected void unregisterForAgentEvents(Agent agent)
	{
	}

}

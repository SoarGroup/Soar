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
		protected String	m_Name ;
		protected String	m_Command ;
		protected String	m_InternalCommand ;
	}
	
	/** A list of ButtonInfo objects */
	protected ArrayList m_ButtonList = new ArrayList() ;
	protected String 	m_LinkedViewName ;
	
	public ButtonView()
	{
	}
	
	public void setTextFont(Font f)
	{
		// We ignore this as our window doesn't display text.
	}

	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public String getModuleBaseName() { return "buttons" ; }

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
	* The caller can also (optionally) register an internal command.  That is a command that
	* does something inside the debugger based on a set of scripted commands that the debugger
	* itself supports.
	* 
	********************************************************************************************/
	public void addButton(String name, String command, String internalCommand)
	{
		ButtonInfo button = new ButtonInfo() ;
		button.m_Name    = name ;
		button.m_Command = command ;
		button.m_InternalCommand = internalCommand ;
		
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
	
	 /*******************************************************************************************
	 * 
	 * The button pane can be linked to a specific view -- in which case commands are executed there.
	 * If it is not linked it defaults to using the prime view for output.
	 * We use the name of the view instead of the view itself so that we can save and load the
	 * connection and also when loading we may load this button pane before the view it is linked
	 * to and just storing the name avoids a problem of when to resolve the name into the view.
	 * 
	 ********************************************************************************************/
	public void setLinkedView(String viewName)
	{
		m_LinkedViewName = viewName ;
	}
	
	public void init(MainFrame frame, Document doc, Pane parentPane)
	{
		setValues(frame, doc, parentPane) ;
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
				public void widgetSelected(SelectionEvent e) { buttonPressed(e, pos) ; } ;
			}) ;
		}
	}
	
	protected AbstractView getLinkedView()
	{
		if (m_LinkedViewName == null)
			return null ;

		// Even when we have a name it's possible that the view it referred to
		// is no longer around, so this can still return null.
		return m_MainFrame.getNameRegister().getView(m_LinkedViewName) ;
	}
	
	protected void buttonPressed(SelectionEvent e, int pos)
	{
		ButtonInfo button = (ButtonInfo)m_ButtonList.get(pos) ;
		String command = button.m_Command ;

		// Execute the command in the prime view for the debugger
		if (command != null)
		{
			AbstractView linkedView = getLinkedView() ;
			if (linkedView != null)
				linkedView.executeAgentCommand(command, true) ;
			else
				m_MainFrame.executeCommandPrimeView(command, true) ;
		}
		
		if (button.m_InternalCommand != null)
		{
			m_MainFrame.executeDebuggerCommand(button.m_InternalCommand, true) ;
		}
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
	public general.ElementXML convertToXML(String title)
	{
		ElementXML element = new ElementXML(title) ;
		
		// It's useful to record the class name to uniquely identify the type
		// of object being stored at this point in the XML tree.
		Class cl = this.getClass() ;
		element.addAttribute(ElementXML.kClassAttribute, cl.getName()) ;

		// Store this object's properties.
		element.addAttribute("Name", m_Name) ;
		element.addAttribute("ButtonCount", Integer.toString(m_ButtonList.size())) ;

		if (m_LinkedViewName != null)
			element.addAttribute("LinkedView", m_LinkedViewName) ;
		
		// Save information for each button in the panel
		for (int i = 0 ; i < m_ButtonList.size() ; i++)
		{
			ButtonInfo button = (ButtonInfo)m_ButtonList.get(i) ;
			
			ElementXML child = new ElementXML("Button") ;
			child.addAttribute("Name", button.m_Name) ;
			
			if (button.m_Command != null)
				child.addAttribute("Command", button.m_Command) ;
			
			if (button.m_InternalCommand != null)
				child.addAttribute("InternalCommand", button.m_InternalCommand) ;
			
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
	* @param frame			The top level window that owns this window
	* @param doc			The document we're rebuilding
	* @param parent			The pane window that owns this view
	* @param element		The XML representation of this command
	* 
	*************************************************************************/
	public void loadFromXML(MainFrame frame, doc.Document doc, Pane parent, general.ElementXML element) throws Exception
	{
		setValues(frame, doc, parent) ;
		
		m_ButtonList.clear() ;
		
		m_Name   = element.getAttributeThrows("Name") ;
		int size = element.getAttributeIntThrows("ButtonCount") ;
		m_LinkedViewName = element.getAttribute("LinkedView") ;	// This one is optional
		
		for (int i = 0 ; i < size ; i++)
		{
			ElementXML child = element.getChild(i) ;
			
			ButtonInfo button = new ButtonInfo() ;
			button.m_Name    = child.getAttributeThrows("Name") ;
			button.m_Command = child.getAttribute("Command") ;
			button.m_InternalCommand = child.getAttribute("InternalCommand") ;
			
			m_ButtonList.add(button) ;
		}
		
		// Register that this module's name is in use
		frame.getNameRegister().registerName(m_Name, this) ;

		// Actually create the window
		init(frame, doc, parent) ;
	}

	/** So far the button panel doesn't care about events from the agent */
	protected void registerForAgentEvents(Agent agent)
	{
	}
	
	protected void unregisterForAgentEvents(Agent agent)
	{
	}

}

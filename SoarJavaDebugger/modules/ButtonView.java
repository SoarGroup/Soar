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
	protected static class ButtonInfo
	{
		protected String	m_Name ;
		protected String	m_Command ;
		protected int		m_Channel ;
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
		this.getControl().addMouseListener(listener) ;
	}

	/********************************************************************************************
	* 
	* Remove ourselves from any events that we are listening for (in preparation for deleting this window)
	* 
	********************************************************************************************/
	public void removeListeners()
	{
	}
	
	public void addButton(String name, String command, int channel)
	{
		ButtonInfo button = new ButtonInfo() ;
		button.m_Name    = name ;
		button.m_Command = command ;
		button.m_Channel = channel ;
		
		m_ButtonList.add(button) ;
	}

	public void Init(MainFrame frame, Document doc, Composite parent)
	{
		m_MainFrame = frame ;
		m_Document  = doc ;
		createButtonPanel() ;
	}

	protected void createButtonPanel()
	{
		// Remove any existing buttons
		/* BUGBUG SWT
		this.removeAll() ;
		
		// Create and add buttons for each button info structure
		for (int i = 0 ; i < m_ButtonList.size() ; i++)
		{
			ButtonInfo info = (ButtonInfo)m_ButtonList.get(i) ;
			
			final int pos = i ;
			JButton button = new JButton(info.m_Name) ;
			button.addActionListener(new ActionListener() { public void actionPerformed(ActionEvent e) { buttonPressed(e, pos) ; } } ) ;
			
			this.add(button) ;
		}
		*/
	}

	/* BUGBUG SWT
	protected void buttonPressed(ActionEvent e, int pos)
	{
		ButtonInfo button = (ButtonInfo)m_ButtonList.get(pos) ;
		
		getDocument().sendAgentCommand(getAgentFocus(), button.m_Command) ;
	}
	*/

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
			child.addAttribute("Channel", Integer.toString(button.m_Channel)) ;
			
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
		//m_Channel 		   = element.getAttributeIntThrows("Channel") ;
		
		m_ButtonList.clear() ;
		
		int size = element.getAttributeIntThrows("ButtonCount") ;
		
		for (int i = 0 ; i < size ; i++)
		{
			ElementXML child = element.getChild(i) ;
			
			ButtonInfo button = new ButtonInfo() ;
			button.m_Name    = child.getAttributeThrows("Name") ;
			button.m_Command = child.getAttributeThrows("Command") ;
			button.m_Channel = child.getAttributeIntThrows("Channel") ;
			
			m_ButtonList.add(button) ;
		}
		
		// Reset the list of buttons to match the list we just loaded	
		createButtonPanel() ;
	}

	/** So far the button panel doesn't care about events from the agent */
	protected void registerForAgentEvents(Agent agent)
	{
	}
	
	protected void unregisterForAgentEvents(Agent agent)
	{
	}

}

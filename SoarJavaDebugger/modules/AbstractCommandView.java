/********************************************************************************************
*
* BaseTextCommandView.java
* 
* Description:	
* 
* Created on 	Mar 29, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import general.ElementXML;

import manager.Pane;
import menu.ParseSelectedText;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.custom.*;

import sml.Agent;

import debugger.MainFrame;
import doc.Document;

/************************************************************************
 * 
* Supports a range of views that take the form of a combo box for entering commands
* and a text window to display the output.
 * 
 ************************************************************************/
public abstract class AbstractCommandView extends AbstractComboView
{
	private Text m_Text ;

	// The constructor must take no arguments so it can be called
	// from the loading code and the new window dialog
	public AbstractCommandView()
	{
		m_StopCallback = -1 ;
		m_PrintCallback = -1 ;
		m_DecisionCallback = -1 ;
	}

	/** The control we're using to display the output in this case **/
	protected Control getDisplayControl()
	{
		return m_Text ;
	}

	/************************************************************************
	* 
	* Clear the display control.
	* 
	*************************************************************************/
	public void clearDisplay()
	{
		m_Text.setText("") ;
	}

	protected ParseSelectedText.SelectedObject getCurrentSelection(int mouseX, int mouseY)
	{
		if (m_Text.getCaretPosition() == -1)
			return null ;
		
		ParseSelectedText selection = new ParseSelectedText(m_Text.getText(), m_Text.getCaretPosition()) ;
		
		return selection.getParsedObject() ;
	}

	/********************************************************************************************
	* 
	* Create the window that will display the output
	* 
	********************************************************************************************/
	protected void createDisplayControl(Composite parent)
	{
		m_Text = new Text(parent, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL | SWT.WRAP | SWT.READ_ONLY) ;
		
		// We want a right click to set the selection instantly, so you can right click on an ID
		// rather than left-clicking on the ID and then right click to bring up menu.
		m_Text.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e)
			{ if (e.button == 2 || e.button == 3) rightButtonPressed(e) ; } ;
		}) ;
		
		createContextMenu(m_Text) ;
	}
	
	/*******************************************************************************************
	 * 
	 * When the user clicks the right mouse button, sets the selection to that location (just like a left click).
	 * This makes right clicking on a piece of text much easier as it's just one click rather than
	 * having to left click to place the selection and then right click to bring up the menu.
	 * 
	********************************************************************************************/
	protected void rightButtonPressed(MouseEvent e)
	{	
		// Found a solution to replace this:
		// Need to switch to StyledText and the getOffsetAtLocation() method will do this for me.
		// Hmmm...tried it but the performance seems to be just horrible so for now I'm not
		// making the switch, however if we keep the current code I need to find a way to
		// only build it into the Windows version which will no doubt be ugly to do.
		// Update: It may be that the performance just looks horrible but isn't, because the
		// display doesn't automatically scroll in the way that the basic control does.
		// I don't know.  Need to investigate this whole area further.
		
		// Unfortunately, SWT doesn't support getting a character location from a position
		// so I'm adding support for it here.  However, this support is pure Windows code.
		// We'll need to figure out how to have code like this and still compile the debugger
		// on Linux (even if this option won't work on Linux).
		if (true)	// Comment out this section on Linux or set this to false (if that allows it to compile)
		{	
			// Send an EM_CHARFROMPOS message to the underlying edit control
			int handle = m_Text.handle ;
			int lParam = e.y << 16 | e.x ;	// Coords are packed as high-word, low-word
			int result = org.eclipse.swt.internal.win32.OS.SendMessage (handle, org.eclipse.swt.internal.win32.OS.EM_CHARFROMPOS, 0, lParam);
	
			// Break out the character and line position from the result
			int charPos = result & (0xFFFF) ;
			int linePos = (result >>> 16) ;
			
			// Set the selection to the character position (which is measured from the first character
			// in the control).
			m_Text.clearSelection() ;
			m_Text.setSelection(charPos) ;
			//System.out.println("Char " + charPos + " Line " + linePos) 
		}
	}

	/********************************************************************************************
	 * @param element
	 * 
	 * @see modules.AbstractComboView#storeContent(general.ElementXML)
	 ********************************************************************************************/
	protected void storeContent(ElementXML element)
	{
		if (m_Text.getText() != null)
			element.addContents(m_Text.getText()) ;
	}

	/********************************************************************************************
	 * @param element
	 * 
	 * @see modules.AbstractComboView#restoreContent(general.ElementXML)
	 ********************************************************************************************/
	protected void restoreContent(ElementXML element)
	{
		String text = element.getContents() ;
		
		// Fill in any text we stored (we may not have done so)
		// and move the cursor to the bottom
		if (text != null)
		{
			m_Text.setText(text) ;
			m_Text.setSelection(text.length()) ;
		}
	}

	/********************************************************************************************
	 * 
	 * Register for events of particular interest to this view
	 * 
	 ********************************************************************************************/
	protected void registerForViewAgentEvents(Agent agent)
	{
	}

	protected void clearViewAgentEvents()
	{
	}

	protected boolean unregisterForViewAgentEvents(Agent agent)
	{
		return true ;
	}

	/********************************************************************************************
	 * @param text
	 * 
	 * @see modules.AbstractComboView#appendText(java.lang.String)
	 ********************************************************************************************/
	protected void appendText(final String text)
	{
		m_Text.append(text) ;
	}
}

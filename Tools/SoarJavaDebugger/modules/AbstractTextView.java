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

import java.io.PrintWriter;

import general.JavaElementXML;

import menu.ParseSelectedText;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.custom.*;

import sml.Agent;

/************************************************************************
 * 
* Supports a range of views that take the form of a combo box for entering commands
* and a text window to display the output.
 * 
 ************************************************************************/
public abstract class AbstractTextView extends AbstractComboView
{
	// We have to use a StyledText control so that we can have right clicks set the selection before bringing up the menu
	// This isn't supported in a platform independent way by the vanilla Text control.  But be aware the StyledText control
	// is about 10 times slower to update than a simple Text view, so we're not using it for trace output.
	private StyledText m_Text ;
	//private boolean m_Logging;
	private PrintWriter m_LogWriter;

	// The constructor must take no arguments so it can be called
	// from the loading code and the new window dialog
	public AbstractTextView()
	{
		m_StopCallback = -1 ;
		m_PrintCallback = -1 ;
		m_DecisionCallback = -1 ;
		//m_Logging = false;
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

	/********************************************************************************************
	 * 
	 * 	Scroll the display control to the bottom
	 * 
	 ********************************************************************************************/
	public void scrollBottom()
	{
		// Move the selection to the end and make sure it's visible
		int length = m_Text.getCharCount() ;
		m_Text.setSelection(length) ;
		m_Text.showSelection() ;
	}

	public void copy()
	{
		m_Text.copy() ;
	}
	
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
	public boolean find(String text, boolean searchDown, boolean matchCase, boolean wrap, boolean searchHiddenText)
	{
		String windowText = m_Text.getText() ;
		
		// If we're case insensitive shift all to lower case
		if (!matchCase)
		{
			windowText = windowText.toLowerCase() ;
			text = text.toLowerCase() ;
		}
		
		// Find out where we're starting from
		Point selectionPoint = m_Text.getSelection() ;
		int selectionStart = selectionPoint.x ;
		
		int start = -1 ;
		boolean done ;
		do
		{
			// Assume we're done after this pass unless told otherwise
			done = true ;
			
			if (searchDown)
			{
				start = windowText.indexOf(text, selectionStart + 1) ;
			}
			else
			{
				start = windowText.lastIndexOf(text, selectionStart - 1) ;
			}
			
			if (start == -1)
			{
				if (wrap)
				{
					// If fail to find text with the basic search repeat it here
					// which produces a wrap effect.
					done = false ;
					wrap = false ;	// Only do it once
					selectionStart = searchDown ? -1 : windowText.length() ;
				}
				else
				{
					// If we're not wrapping (or already did the wrap) return false
					// to signal we failed to find anything.
					return false ;
				}
			}
		} while (!done) ;
		
		int end = start + text.length() ;
		
		// Set the newly found text to be selected
		m_Text.setSelection(start, end) ;
		
		return true ;
	}
	
	protected ParseSelectedText.SelectedObject getCurrentSelection(int mouseX, int mouseY)
	{
		// Find out if mouseX, mouseY is over the text at all
		try
		{
			Point mouse = m_Text.toControl(mouseX, mouseY) ;
			m_Text.getOffsetAtLocation(mouse) ;
		}
		catch (IllegalArgumentException ex)
		{
			// If the click wasn't over any letters we come here and return null to indicate
			// no text was selected (we'll just show the default context menu).
			return null ;
		}
		
		// Look up the currently selected text
		int pos = m_Text.getCaretOffset() ;
		if (pos == -1)
			return null ;
		
		ParseSelectedText selection = new ParseSelectedText(m_Text.getText(), pos) ;
		
		return selection.getParsedObject(this.m_Document, this.getAgentFocus()) ;
	}

	/********************************************************************************************
	* 
	* Create the window that will display the output
	* 
	********************************************************************************************/
	protected void createDisplayControl(Composite parent)
	{
		m_Text = new StyledText(parent, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL | SWT.READ_ONLY) ;
//		m_Text = new Text(parent, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL | SWT.WRAP | SWT.READ_ONLY) ;
		
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
		try
		{
			Point mouse = new Point(e.x, e.y) ;
			int offset = m_Text.getOffsetAtLocation(mouse) ;
			m_Text.setSelection(offset) ;
		}
		catch (IllegalArgumentException ex)
		{
			// If the click is out of range of the text ignore it.
		}

		// This is the Text control solution -- Windows only
		// Dropped now that we're using the StyledText.
		// Unfortunately, SWT doesn't support getting a character location from a position
		// so I'm adding support for it here.  However, this support is pure Windows code.
		// We'll need to figure out how to have code like this and still compile the debugger
		// on Linux (even if this option won't work on Linux).
		/*
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
		*/
	}

	/********************************************************************************************
	 * @param element
	 * 
	 * @see modules.AbstractComboView#storeContent(general.JavaElementXML)
	 ********************************************************************************************/
	protected void storeContent(JavaElementXML element)
	{
		if (m_Text.getText() != null)
			element.addContents(m_Text.getText()) ;
	}

	/********************************************************************************************
	 * @param element
	 * 
	 * @see modules.AbstractComboView#restoreContent(general.JavaElementXML)
	 ********************************************************************************************/
	protected void restoreContent(JavaElementXML element)
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

	protected void scrollToBottom()
	{
		m_Text.setSelection(m_Text.getCharCount()) ;
	}
	
	/********************************************************************************************
	 * @param text
	 * 
	 * @see modules.AbstractComboView#appendText(java.lang.String)
	 ********************************************************************************************/
	protected void appendText(final String text)
	{
		if (m_Text.isDisposed())
			return ;
		
		m_Text.append(text) ;
		if (m_LogWriter != null) m_LogWriter.print(text);

		// If we're clearing the window after each command we should
		// leave it at the top.  Otherwise, scroll it.
		if (!m_ClearEachCommand)
			scrollToBottom() ;
	}
}

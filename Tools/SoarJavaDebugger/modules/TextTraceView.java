/********************************************************************************************
*
* ComboCommandLineWindow.java
* 
* Description:	Mimic a simple command line window -- type commands directly to execute them.
* 				For now we still just use a combo box for the input -- later we'll do a version
* 				which uses a normal command line prompt.
* 
* Created on 	Jan 29, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import java.io.PrintWriter;

import general.JavaElementXML;

import menu.ParseSelectedText;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.events.*;

import sml.Agent;

import dialogs.PropertiesDialog;

/************************************************************************
 * 
 * This version clears the last command from the combo box (so you're ready to type another),
 * keeps all of the output and doesn't do anything when Soar stops running.
 * 
 * Getting closer to a real command line window.
 * 
 ************************************************************************/
public class TextTraceView extends AbstractComboView
{
	private Text m_Text ;
	private boolean m_Logging;
	private PrintWriter m_LogWriter;

	public TextTraceView()
	{
		m_StopCallback = -1 ;
		m_PrintCallback = -1 ;
		m_DecisionCallback = -1 ;
		m_Logging = false;
		
		m_ClearEachCommand = false ;
		m_UpdateOnStop = false ;
		m_ClearComboEachCommand = true ;
		m_ComboAtTop = false ;
		m_ShowTraceOutput = true ;
		m_ShowEchoOutput = true ;
		m_PromptForCommands = "<Type commands here>" ;
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
	* Copy current selection to the clipboard.
	* 
	********************************************************************************************/
	public void copy()
	{
		m_Text.copy() ;
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
		int pos = m_Text.getCaretPosition() ;
//		int pos = m_Text.getCaretOffset() ;
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
//		m_Text = new StyledText(parent, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL | SWT.WRAP | SWT.READ_ONLY) ;
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
		// We could use:
		// a) A StyledText window which supports this (via getOffsetAtLocation) but it's 10-20 times slower
		//    to update so a bad idea for a trace window.
		// b) Windows specific code to send a message to the Text widget.  This is fast but makes the debugger
		//    platform specific.
		// So the compromise is to drop support for auto-select on right click in the text trace window
		// and use a Styled Text control in text non-trace windows.
		// This makes sense because we plan to drop this text trace window and replace it with folding text soon.
		
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

	/********************************************************************************************
	 * @param text
	 * 
	 * @see modules.AbstractComboView#appendText(java.lang.String)
	 ********************************************************************************************/
	protected void appendText(final String text)
	{
		m_Text.append(text) ;
		if (m_LogWriter != null) m_LogWriter.print(text);
	}
	
	public Color getBackgroundColor()
	{
		return getMainFrame().m_White ;
	}
	
	public boolean canBePrimeWindow() { return true ; }
	
	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public String getModuleBaseName() { return "texttrace" ; }
	
	public void showProperties()
	{
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[3] ;
		
		properties[0] = new PropertiesDialog.BooleanProperty("Update automatically on stop", m_UpdateOnStop) ;
		properties[1] = new PropertiesDialog.BooleanProperty("Clear display before each command", m_ClearEachCommand) ;
		properties[2] = new PropertiesDialog.BooleanProperty("Combo at top", m_ComboAtTop) ;
		
		PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;
		
		m_UpdateOnStop = ((PropertiesDialog.BooleanProperty)properties[0]).getValue() ;
		m_ClearEachCommand = ((PropertiesDialog.BooleanProperty)properties[1]).getValue() ;
		boolean comboAtTop = ((PropertiesDialog.BooleanProperty)properties[2]).getValue() ;

		// We need to know if this value has changed as we'll need to rebuild the layout
		boolean comboMoving = (comboAtTop != m_ComboAtTop) ;
		m_ComboAtTop = comboAtTop ;
		
		if (comboMoving)
		{
			// Convert everything to XML (using the new setting we just changed)
			boolean storeContent = true ;
			JavaElementXML xml = m_Frame.getMainWindow().convertToXML(storeContent) ;

			// Rebuild the entire layout from the new XML structure.
			m_Frame.getMainWindow().loadFromXMLNoThrow(xml) ;
		}
	}

	
}

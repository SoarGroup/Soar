/********************************************************************************************
*
* FoldingTextView.java
* 
* Description:	
* 
* Created on 	May 6, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import general.JavaElementXML;
import helpers.FormDataHelper;

import java.util.ArrayList;
import java.util.Iterator;

import manager.Pane;
import menu.ParseSelectedText;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import sml.Agent;
import sml.ClientTraceXML;
import sml.ClientXML;
import sml.smlXMLEventId;
import debugger.MainFrame;
import dialogs.PropertiesDialog;
import doc.Document;
import helpers.* ;

/************************************************************************
 * 
 * A text view for showing trace information from Soar.
 * 
 * This text view allows for "folding" which is expanding and collapsing
 * part of the text, as if it were a tree.  Very similar to an editing
 * window in Eclipse.
 *  
 ************************************************************************/
public class FoldingTextView extends AbstractComboView implements Agent.xmlEventInterface
{
	protected FoldingText m_FoldingText ;
	
	protected int m_xmlCallback = -1 ;
	
	/** How many spaces we indent for a subgoal */
	protected int m_IndentSize = 3 ;
	
	/** When true, expand the tree automatically as it's created */
	protected boolean m_ExpandTracePersistent = false ;
	
	/** When true, we expand the tree as it's created -- but this one is not persistent between debugger sessions */
	protected boolean m_ExpandTrace = false ;
	
	/** When true, keep debugger and Soar locked together so the debugger's UI doesn't get behind Soar.  Tying the processes together usually makes Soar run a lot slower, but sometimes that will be useful */
	protected boolean m_LockToSoar = false ;
	
	/** The last root (top level item) added to the tree.  We add new sub items under this */
	protected TreeItem m_LastRoot ;
	
	protected Composite	m_Buttons ;

	protected Button m_ExpandButton ;
	protected Button m_ExpandPageArrow ;
	protected Menu   m_ExpandPageMenu ;
	
	protected Label  m_FilterLabel ;
	protected Button m_FilterArrow ;
	protected Menu	 m_FilterMenu ;
	
	/** Controls whether we cache strings that are due to be subtree nodes and only add the nodes when the user clicks--or not */
	protected final static boolean kCacheSubText = true ;
	
	/** We use this structure if we're caching sub nodes in the tree for expansion only when the user clicks */
	protected static class TreeData
	{
		protected ArrayList m_Lines = new ArrayList() ;
		
		public void addLine(String text) 	{ m_Lines.add(text) ; }
		public Iterator getLinesIterator() 	{ return m_Lines.iterator() ; }
	}
	
	public FoldingTextView()
	{
		m_ClearEachCommand = false ;
		m_UpdateOnStop = false ;
		m_ClearComboEachCommand = true ;
		m_ComboAtTop = false ;
		m_ShowTraceOutput = false ;
		m_ShowEchoOutput = true ;
		m_PromptForCommands = "<Type commands here>" ;	
	}

	/** This window can be the main display window */
	public boolean canBePrimeWindow() { return true ; }

	public Color getBackgroundColor() { return m_Frame.m_White ; }
	
	protected void updateButtonState()
	{
		m_ExpandButton.setText(m_ExpandTrace ? "Collapse" : " Expand ") ;
		m_ExpandButton.setData("expand", m_ExpandTrace ? Boolean.TRUE : Boolean.FALSE) ;
		
		// Set the checkboxes to match current filter state
		for (int i = 0 ; i < m_FilterMenu.getItemCount() ; i++)
		{
			MenuItem item = m_FilterMenu.getItem(i) ;
			
			Long typeObj = (Long)item.getData("type") ;
			if (typeObj == null)
				continue ;

			// Update the checkbox to match whether this type is visible or not
			long type = typeObj.longValue() ;
			item.setSelection((m_FoldingText.isTypeVisible(type))) ;
			
			// Enable/disable the item to match whether filtering is enabled at all
			item.setEnabled(m_FoldingText.isFilteringEnabled()) ;
		}

		// Change the color of the label if any filtering is enabled, so it's clear that this is happening.
		// Don't want to be doing this by accident and not realize it.
		if (m_FoldingText.isFilteringEnabled() && m_FoldingText.getExclusionFilter() != 0)
		{
			m_FilterLabel.setForeground(getMainFrame().getDisplay().getSystemColor(SWT.COLOR_BLUE)) ;			
		}
		else
		{
			m_FilterLabel.setForeground(getMainFrame().getDisplay().getSystemColor(SWT.COLOR_GRAY)) ;
		}
	}
	
	/********************************************************************************************
	* 
	* Create the window that will display the output
	* 
	********************************************************************************************/
	protected void createDisplayControl(Composite parent)
	{
		m_FoldingText = new FoldingText(parent) ;
		m_LastRoot = null ;
		
		createContextMenu(m_FoldingText.getTextWindow()) ;
		
		m_Buttons = new Composite(m_ComboContainer, 0) ;
		m_Buttons.setLayout(new RowLayout()) ;
		final Composite owner = m_Buttons ;
		
		m_FoldingText.getTextWindow().addKeyListener(new KeyAdapter() {
			// If the user tries to type into the main text window, move the focus down
			// to the combo box where they can usefully type.
			public void keyPressed(KeyEvent e)  {
				// Only for keys that generate characters (so arrows keys, shift etc. can be safely pressed)
				if ( (e.character >= 'A' && e.character <= 'Z') ||
					 (e.character >= 'a' && e.character <= 'z') ||
					 (e.character >= '0' && e.character <= '9') ||
					 e.character == '!')
				{
					m_CommandCombo.setFocus() ;
					m_CommandCombo.setText(Character.toString(e.character)) ;
					m_CommandCombo.setSelection(new Point(m_CommandCombo.getText().length(), m_CommandCombo.getText().length())) ;
				}
			}
		}) ;

		// Add a button that offers an expand/collapse option instantly (for just one page)
		m_ExpandTrace = m_ExpandTracePersistent ;		
		m_ExpandButton = new Button(owner, SWT.PUSH);
		
		m_ExpandButton.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e)
		{
			m_ExpandTrace = (e.widget.getData("expand") == Boolean.FALSE) ;
			updateButtonState() ;
			
			// We expand the current page if the user asks for "expand" but we're not making this symmetric
			// because "collapse" probably means "I'm done with detailed debugging" but not necessarily "I don't want to see what I was just working on".
			// If you don't agree with that logic just comment out the "if".
			if (m_ExpandTrace)
			{
				expandPage(m_ExpandTrace) ;
			}
		} } ) ;
		
		m_ExpandPageArrow = new Button(owner, SWT.ARROW | SWT.DOWN) ;
		m_ExpandPageMenu  = new Menu(owner) ;

		m_ExpandPageArrow.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent event)
		{ 	Point pt = m_ExpandPageArrow.toDisplay(new Point(event.x, event.y)) ;
			m_ExpandPageMenu.setLocation(pt.x, pt.y) ;
			m_ExpandPageMenu.setVisible(true) ;
		} }) ;

		MenuItem menuItem = new MenuItem (m_ExpandPageMenu, SWT.PUSH);
		menuItem.setText ("Expand page");
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { expandPage(true) ; } } ) ;

		// Note: Doing expand page then collapse page doesn't get you back to where you started--the page will be showing far fewer
		// blocks when you hit "collapse page" so it does less work.  Collapse page may have little value.
		menuItem = new MenuItem (m_ExpandPageMenu, SWT.PUSH);
		menuItem.setText ("Collapse page");		
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { expandPage(false) ; } } ) ;

		menuItem = new MenuItem (m_ExpandPageMenu, SWT.PUSH);
		menuItem.setText ("Expand all");		
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { expandAll(true) ; } } ) ;

		menuItem = new MenuItem (m_ExpandPageMenu, SWT.PUSH);
		menuItem.setText ("Collapse all");		
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { expandAll(false) ; } } ) ;
		
		// Add a button that controls whether we are filtering or not
		Composite labelHolder = new Composite(owner, SWT.NULL) ;
		labelHolder.setLayout(new GridLayout(1, true)) ;
		
		m_FilterLabel = new Label(labelHolder, 0);
		m_FilterLabel.setText("Filters") ;

		// Place the label in the center of a tiny grid layout so we can
		// align the text to match the expand button along side
		GridData data = new GridData() ;
		data.horizontalAlignment = SWT.CENTER ;
		data.verticalAlignment = SWT.CENTER ;
		m_FilterLabel.setLayoutData(data) ;
		
		m_FilterArrow = new Button(owner, SWT.ARROW | SWT.DOWN) ;
		m_FilterMenu  = new Menu(owner) ;

		m_FilterArrow.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent event)
		{ 	Point pt = m_FilterArrow.toDisplay(new Point(event.x, event.y)) ;
			m_FilterMenu.setLocation(pt.x, pt.y) ;
			m_FilterMenu.setVisible(true) ;
		} }) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Phases") ;
		menuItem.setData("type", new Long(TraceType.kPhase)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kPhase) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Preferences") ;
		menuItem.setData("type", new Long(TraceType.kPreference)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kPreference) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Wme Changes") ;
		menuItem.setData("type", new Long(TraceType.kWmeChange)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kWmeChange) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Production Firings") ;
		menuItem.setData("type", new Long(TraceType.kFiring)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kFiring) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Production Retractions") ;
		menuItem.setData("type", new Long(TraceType.kRetraction)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kRetraction) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Stack Trace") ;
		menuItem.setData("type", new Long(TraceType.kStack)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kStack) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Rhs Writes and Messages") ;
		menuItem.setData("type", new Long(TraceType.kRhsWrite)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kRhsWrite) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Learning") ;
		menuItem.setData("type", new Long(TraceType.kLearning)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kLearning) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Full Learning") ;
		menuItem.setData("type", new Long(TraceType.kFullLearning)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kFullLearning) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Verbose") ;
		menuItem.setData("type", new Long(TraceType.kVerbose)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kVerbose) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Warnings") ;
		menuItem.setData("type", new Long(TraceType.kWarning)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kWarning) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.PUSH);
		menuItem.setText ("Show all") ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { m_FoldingText.setExclusionFilter(0, true) ; updateButtonState() ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.PUSH);
		menuItem.setText ("Hide all") ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { m_FoldingText.setExclusionFilter(TraceType.kAllExceptTopLevel, true) ; updateButtonState() ; } } ) ;

		updateButtonState() ;

		// Attempted patches for Linux
		int delayMillis = 5000 ;
		parent.getDisplay().timerExec(delayMillis, new Runnable() { public void run() { System.out.println("Relaying out windows") ; m_FoldingText.getTextWindow().pack() ; m_FoldingText.getWindow().layout(true, true) ; } } ) ;

		parent.getDisplay().timerExec(delayMillis*2, new Runnable() { public void run() { System.out.println("Relaying out windows 2") ; m_FoldingText.getTextWindow().pack() ; m_FoldingText.getWindow().getParent().getParent().layout(true, true) ; } } ) ;
}
	
	protected void changeFilter(Widget widget, long type)
	{
		MenuItem item = (MenuItem)widget ;
		boolean selected = item.getSelection() ;
		
		m_FoldingText.changeExclusionFilter(type, !selected, true) ;
		
		// A change to one button can affect others
		updateButtonState() ;
	}
	
	/********************************************************************************************
	* 
	* Copy current selection to the clipboard.
	* 
	********************************************************************************************/
	public void copy()
	{
		m_FoldingText.getTextWindow().copy() ;
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
		boolean found = false ;
		
		// Show the wait cursor as the hidden search could take a while
		Cursor wait = new Cursor(getWindow().getDisplay(), SWT.CURSOR_WAIT) ;
		getWindow().getShell().setCursor(wait) ;
				
		String windowText = m_FoldingText.getAllText(searchHiddenText) ;
		
		// If we're case insensitive shift all to lower case
		if (!matchCase)
		{
			windowText = windowText.toLowerCase() ;
			text = text.toLowerCase() ;
		}
		
		// Find out where we're starting from
		Point selectionPoint = m_FoldingText.getTextWindow().getSelection() ;
		int selectionStart = selectionPoint.x ;
		
		// If we're searching the entire set of text need to switch to the position within the entire set of text
		if (searchHiddenText)
			selectionStart = m_FoldingText.convertVisibleToAllCharPos(selectionStart) ;
		
		int origStart = selectionStart ;
		
		int start = -1 ;
		boolean wrapped = false ;
		boolean done ;
		do
		{
			if (searchDown)
			{
				start = windowText.indexOf(text, selectionStart + 1) ;
			}
			else
			{
				start = windowText.lastIndexOf(text, selectionStart - 1) ;
			}
			
			if (start != -1)
			{
				// We found some text, so set the selection and we're done.
				found = true ;
				done = true ;				

				// Unless we've done a wrapped search and passed our start point
				// in which case we're actually done
				if (wrapped && ((searchDown && (start >= origStart)) || (!searchDown && (start <= origStart))))
					found = false ;
			}
			else
			{
				if (wrap && !wrapped)
				{
					// If fail to find text with the basic search repeat it here
					// which produces a wrap effect.
					done = false ;
					wrapped = true ;	// Only do it once
					selectionStart = searchDown ? -1 : windowText.length() ;
				}
				else
				{
					// If we're not wrapping (or already did the wrap) return false
					// to signal we failed to find anything.
					found = false ;
					done = true ;
				}
			}
		} while (!done) ;
		
		if (found)
		{
			int end = start + text.length() ;

			// If we're searching in hidden text need to convert back to the visible space and
			// possibly expand the block
			if (searchHiddenText)
			{
				// Force block to expand if needed
				m_FoldingText.makeCharPosVisible(start) ;
				
				start = m_FoldingText.convertAllToVisibleCharPos(start) ;
				end   = m_FoldingText.convertAllToVisibleCharPos(end) ;
			}
			
			// Set the newly found text to be selected
			m_FoldingText.setSelection(start, end) ;
		}
		
		getWindow().getShell().setCursor(null) ;
		wait.dispose() ;
		return found ;
	}
	
	/************************************************************************
	* 
	* Go from current selection (where right click occured) to the object
	* selected by the user (e.g. a production name).
	* 
	*************************************************************************/
	protected ParseSelectedText.SelectedObject getCurrentSelection(int mouseX, int mouseY)
	{
		// Switchfrom screen coords to coords based on the text window
		Point pt = m_FoldingText.getTextWindow().toControl(mouseX, mouseY) ;
		mouseX = pt.x ;
		mouseY = pt.y ;

		int line = m_FoldingText.getLine(mouseY) ;
		if (line == -1)
			return null ;
		
		String text = m_FoldingText.getTextForLine(line) ;		
		if (text == null)
			return null ;

		int pos = m_FoldingText.getCharacterPosition(text, mouseX) ;
		if (pos == -1)
			return null ;

		// Sometimes we need to search back up the trace or down the trace to determine the context
		// so we'll add these lines above and below the to current line, adjusting the position as we do.
		// This is just a heuristic but it should cover 99% of cases.
		int bufferLinesAbove = Math.min(20, line) ;
		int bufferLinesBelow = 1 ;
		String combinedText = text ;
		int combinedPos = pos ;
		for (int i = 0 ; i < bufferLinesAbove ; i++)
		{
			String lineText = m_FoldingText.getTextForLine(line - i - 1) ;
			
			if (lineText != null)
			{
				combinedText = lineText + combinedText ;
				combinedPos += lineText.length() ;
			}
		}
		for (int i = 0 ; i < bufferLinesBelow ; i++)
		{
			String lineText = m_FoldingText.getTextForLine(line + i + 1) ;
			if (lineText != null)
				combinedText = combinedText + lineText ;
		}

		// Go from the text to a Soar selection object (e.g. an id or an attribute -- that sort of thing)
		ParseSelectedText selection = new ParseSelectedText(combinedText, combinedPos) ;
		
		return selection.getParsedObject(this.m_Document, this.getAgentFocus()) ;
		
	}
	
	protected void expandPage(boolean state)
	{
		m_FoldingText.expandPage(state) ;
	}
	
	protected void expandAll(boolean state)
	{
		m_FoldingText.expandAll(state) ;
	}
	
	protected void layoutComboBar(boolean top)
	{
		m_ComboContainer.setLayout(new FormLayout()) ;

		FormData containerData = top ? FormDataHelper.anchorTop(0) : FormDataHelper.anchorBottom(0) ;
		m_ComboContainer.setLayoutData(containerData) ;

		FormData comboData = FormDataHelper.anchorTopLeft(0) ;
		comboData.right = new FormAttachment(m_Buttons) ;
		m_CommandCombo.setLayoutData(comboData) ;

		FormData buttonData = FormDataHelper.anchorTop(0) ;
		buttonData.left = null ;
		m_Buttons.setLayoutData(buttonData) ;
	}

	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public String getModuleBaseName()
	{
		return "trace" ;
	}

	/** The control we're using to display the output in this case **/
	protected Control getDisplayControl()
	{
		return m_FoldingText.getTextWindow() ;
	}

	/** 
	 * Returns the entire window, within which the display control lies.
	 * 
	 * Usually the display control is all there is, but this method allows us to define
	 * a container that surrounds the display control and includes other supporting controls.
	 * In which case this method should be overriden.
	 */
	protected Control getDisplayWindow()
	{
		return m_FoldingText.getWindow() ;
	}

	public void setTextFont(Font f)
	{
		super.setTextFont(f) ;
		
		// Changing the font means we need to redraw the icon bar, just as if we scrolled
		m_FoldingText.scrolled() ;
	}

	/************************************************************************
	* 
	* Add the text to the view in a thread safe way (switches to UI thread)
	* 
	*************************************************************************/
	protected void appendSubTextSafely(final String text, final boolean redrawTree, final long type)
	{
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			appendSubText(text, type) ;
			return ;
		}

		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
            	appendSubText(text, type) ;
            }
         }) ;
	}
		
	/************************************************************************
	* 
	* Add the text to the view (this method assumes always called from UI thread)
	* 
	*************************************************************************/
	protected void appendSubText(String text, long type)
	{
		String[] lines = text.split(kLineSeparator) ;

		for (int i = 0 ; i < lines.length ; i++)
		{	
			if (lines[i].length() == 0 && i == 0)
				continue ;

			m_FoldingText.appendSubText(lines[i] + kLineSeparator, m_ExpandTracePersistent || m_ExpandTrace, type) ;
			m_Logger.log(lines[i], false, true) ;
		}
	}
	
	/************************************************************************
	* 
	* Add the text to the view (this method assumes always called from UI thread)
	* 
	* Soar text output tends to assume a starting line separator model
	* (e.g. write (crlf) | Hello World |).  This is because of the way the old trace output
	* was built up a little bit at a time.
	* 
	* For the folding text view we want to switch that to a more normal terminating separator model
	* (e.g. write | Hello World | (crlf)).  This is helpful because the folding view tracks text on
	* a line by line basis and "keeping a line open to accept more input in a bit" adds complexity.
	* 
	* Switching between the two generally makes no difference -- except in the case
	* where the user is explicitly issuing RHS writes.  In that case we'll let them continue
	* to use the leading separator model (write (crlf) (crlf) |Hello|) but switch it over to
	* end terminated here.  This has the effect that a single (write | Hello |) will now not
	* appear on the end of the previous line of trace output (as it would have in earlier Soars).
	* (This was generally just a slip anyway but now it won't be possible because the previous line
	* of output in the trace has already added a trailing line separator).
	* But we do still want to allow a series of writes to build up some text
	* (e.g. writing out a board position).  To make that work we collect up all RHS writes that
	* occur sequentially and only then process them.  This avoids us inserting unwanted line separators
	* between writes that really are meant to concatenate together.  Fortunately, the code for this
	* is a lot smaller than this comment and occurs in the method calling here.
	* 
	*************************************************************************/
	protected void appendText(String text, long type)
	{
		String[] lines = text.split(kLineSeparator) ;

		for (int i = 0 ; i < lines.length ; i++)
		{	
			if (lines[i].length() == 0 && i == 0)
				continue ;

			m_FoldingText.appendText(lines[i] + kLineSeparator, type) ;
			m_Logger.log(lines[i], false, true) ;
		}
	}
	
	protected void appendText(String text)
	{
		appendText(text, TraceType.kTopLevel) ;
	}
	
	/************************************************************************
	* 
	* Clear the display control.
	* 
	*************************************************************************/
	public void clearDisplay()
	{
		m_FoldingText.clear() ;
	}
	
	/********************************************************************************************
	 * 
	 * 	Scroll the display control to the bottom
	 * 
	 ********************************************************************************************/
	public void scrollBottom()
	{
		m_FoldingText.scrollBottom() ;
	}

	protected boolean isNextChildRhsWrite(ClientXML xmlParent, int childIndex, int nChildren)
	{
		// We're interested in the next child
		childIndex++ ;
		
		if (childIndex >= nChildren)
			return false ;

		// Get the next child
		ClientTraceXML xmlTemp = new ClientTraceXML() ;
		xmlParent.GetChild(xmlTemp, childIndex) ;
		
		// See if its a RHS write
		boolean result = xmlTemp.IsTagRhsWrite() ;
		
		// Clean up
		xmlTemp.delete() ;
		
		return result ;
	}
	
	/********************************************************************************************
	 * 
	 * This handler should only be called from the UI thread as it does a lot of UI work.
	 * 
	 * @param agent
	 * @param xmlParent
	********************************************************************************************/
	protected void displayXmlTraceEvent(Agent agent, ClientXML xmlParent)
	{
		// For debugging
		//String message = xmlParent.GenerateXMLString(true) ;
		//System.out.println(message) ;
		//System.out.flush() ;
		
		int nChildren = xmlParent.GetNumberChildren() ;
		
		// We collect up all rhs writes that occur together and then process them at once
		StringBuffer rhsWrites = new StringBuffer() ;
		
		for (int childIndex = 0 ; childIndex < nChildren ; childIndex++)
		{
			// Analyze the children as ClientTraceXML objects
			ClientTraceXML xmlTrace = new ClientTraceXML() ;

			// Get each child in turn
			xmlParent.GetChild(xmlTrace, childIndex) ;
			
			// This is a state change (new decision)
			if (xmlTrace.IsTagState())
			{
				String text = XmlOutput.getStateText(agent, xmlTrace, m_IndentSize) ;

				if (text.length() != 0)
					this.appendText(text.toString(), TraceType.kStack) ;
				
			} else if (xmlTrace.IsTagOperator())
			{
				String text = XmlOutput.getOperatorText(agent, xmlTrace, m_IndentSize) ;
				
				if (text.length() != 0)
					this.appendText(text.toString(), TraceType.kStack) ;
				
			} else if (xmlTrace.IsTagRhsWrite())
			{
				String output = xmlTrace.GetString() ;
				
				// Collect all rhs write output together and process as a unit
				// (This lets us support writes that don't start with a leading (crlf) correctly)
				if (output.length() != 0)
				{
					rhsWrites.append(output) ;
	
					// When we reach the end of a sequence of RHS writes process them all
					if (!isNextChildRhsWrite(xmlParent, childIndex, nChildren))
					{
						this.appendText(rhsWrites.toString(), TraceType.kRhsWrite) ;
						rhsWrites = new StringBuffer() ;
					}
				}
			} else if (xmlTrace.IsTagPhase())
			{
				String status = xmlTrace.GetPhaseStatus() ;
				
				String output = XmlOutput.getPhaseText(agent, xmlTrace, status) ;
								
				// Don't show end of phase messages
				// BUGBUG? Perhaps we should include them now and just use the filtering to remove them
				boolean endOfPhase = (status != null && status.equalsIgnoreCase("end")) ;
				
				if (output.length() != 0 && !endOfPhase)
					this.appendSubText(output, TraceType.kPhase) ;
			} else if (xmlTrace.IsTagSubphase())
			{
				String output = XmlOutput.getSubphaseText(agent, xmlTrace) ;

				if (output.length() != 0)
					this.appendSubText(output, TraceType.kPhase) ;				
			}
			else if (xmlTrace.IsTagAddWme() || xmlTrace.IsTagRemoveWme())
			{
				boolean adding = xmlTrace.IsTagAddWme() ;
				String output = XmlOutput.getWmeChanges(agent, xmlTrace, adding) ;
				
				if (output.length() != 0)
					this.appendSubText(output, TraceType.kWmeChange) ;	
			} else if (xmlTrace.IsTagActionSideMarker())
			{
				String output = "--> " ;
				this.appendSubText(output, TraceType.kPreference) ;
			} else if (xmlTrace.IsTagPreference())
			{
				String output = XmlOutput.getPreferenceProductionText(agent, xmlTrace) ;
				
				if (output.length() != 0)
					this.appendSubText(output.toString(), TraceType.kPreference) ;
				
			} 
			else if (xmlTrace.IsTagFiringProduction() || xmlTrace.IsTagRetractingProduction())
			{
				boolean firing = xmlTrace.IsTagFiringProduction() ;

				String output = XmlOutput.getProductionFiring(agent, xmlTrace, firing) ;
				
				long type = (firing ? TraceType.kFiring : TraceType.kRetraction) ;

				if (output.length() != 0)
					this.appendSubText(output, type) ;			
	
			}
			else if (xmlTrace.IsTagLearning())
			{
				// Building chunk*name
				// Optionally followed by the production
				for (int i = 0 ; i < xmlTrace.GetNumberChildren() ; i++)
				{
					ClientTraceXML prod = new ClientTraceXML() ;
					xmlTrace.GetChild(prod, i) ;
					if (prod.IsTagProduction())
					{
						if (prod.GetNumberChildren() == 0)
						{
							// If this production has no children it's just a "we're building x" message.
							StringBuffer text = new StringBuffer() ;
							
							if (i > 0)
								text.append(kLineSeparator) ;
							
							text.append("Building ") ;
							text.append(prod.GetProductionName()) ;							
							appendText(text.toString(), TraceType.kLearning) ;			
						}
						else
						{	
							// If has children we're looking at a full production print
							// (Note -- we get the "we're building" from above as well as this so we don't
							// add the "build x" message when this comes in.
							String prodText = XmlOutput.getProductionText(agent, prod) ;
							
							if (prodText != null && prodText.length() != 0)
								this.appendSubText(prodText, TraceType.kFullLearning) ;			
						}
					}
					prod.delete() ;
				}
			}
			else if (xmlTrace.IsTagCandidate())
			{
				// Numeric indifferent preferences
				String output = XmlOutput.getNumericIndiffernceText(agent, xmlTrace) ;
				
				if (output.length() != 0)
					this.appendSubText(output, TraceType.kNumericIndifferent) ;
			}
			else if (xmlTrace.IsTagBacktraceResult() || xmlTrace.IsTagLocals() ||
					 xmlTrace.IsTagGroundedPotentials() || xmlTrace.IsTagUngroundedPotentials())
			{
				String output = XmlOutput.getBacktraceText(agent, xmlTrace, xmlTrace.GetTagName()) ;
				
				if (output.length() != 0)
					this.appendSubText(output, TraceType.kFullLearning) ;
			}
			else if (xmlTrace.IsTagMessage() || xmlTrace.IsTagWarning() || xmlTrace.IsTagError() || xmlTrace.IsTagVerbose())
			{
				StringBuffer text = new StringBuffer() ;
				
				// The body of the message
				text.append(xmlTrace.GetString()) ;
				
				// Some messages can include wmes as part of the report
				for (int i = 0 ; i < xmlTrace.GetNumberChildren() ; i++)
				{
					ClientTraceXML wme = new ClientTraceXML() ;
					xmlTrace.GetChild(wme, i) ;
					
					if (wme.IsTagWme())
					{
						if (i > 0)
							text.append(kLineSeparator) ;
						
						String output = XmlOutput.getWmeText(agent, wme) ;
						text.append(output) ;
					}
					
					wme.delete() ;
				}
				
				// Figure out the type of the message so we can filter it appropriately
				// We'll classify messages and rhs writes together for now
				long type = TraceType.kRhsWrite ;
				if (xmlTrace.IsTagWarning()) type = TraceType.kWarning ;
				else if (xmlTrace.IsTagVerbose()) type = TraceType.kVerbose ;
				else if (xmlTrace.IsTagError()) type = TraceType.kError ;
				
				if (text.length() != 0)
				{
					if (type != TraceType.kVerbose)
						this.appendText(text.toString(), type) ;
					else
						this.appendSubText(text.toString(), type) ;
				}
			}
			else
			{
				// These lines can be helpful if debugging this -- we
				// print out any XML we completely fail to understand.
				String xmlText = xmlParent.GenerateXMLString(true) ;
				System.out.println(xmlText) ;
			}

			// Manually clean up the child too
			xmlTrace.delete() ;	
		}
		
		// Technically this will happen when the object is garbage collected (and finalized)
		// but without it we'll get a million memory leak messages on shutdown because (a) gc may not have been run for a while
		// (b) even if it runs not all objects will be reclaimed and (c) finalize isn't guaranteed before we exit
		// so all in all, let's just call it ourselves here :)
		xmlParent.delete() ;
	}
	
	public static class RunWrapper implements Runnable
	{
		FoldingTextView m_View ;
		Agent 		  	m_Agent ;
		ClientXML 	  	m_XML ;
		
		public RunWrapper(FoldingTextView view, Agent agent, ClientXML xml)
		{
			m_Agent = agent ;
			m_XML   = xml ;
			m_View  = view ;
		}
		
		public void run()
		{
			m_View.displayXmlTraceEvent(m_Agent, m_XML) ;
		}
	}
	
	
	public void xmlEventHandler(int eventID, Object data, Agent agent, ClientXML xml)
	{
		if (eventID != smlXMLEventId.smlEVENT_XML_TRACE_OUTPUT.swigValue())
			return ;

		// The messages come collected into a parent <trace> tag so that one event
		// can send over many pieces of a trace in a single call.  Just more
		// efficient that way.
		
		ClientTraceXML xmlParent = xml.ConvertToTraceXML() ;
		if (!xmlParent.IsTagTrace() || xmlParent.GetNumberChildren() == 0)
		{
			xml.delete() ;
			return ;
		}
		// The conversion creates a new SWIG object which we have to delete.
		xmlParent.delete() ;
		
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			displayXmlTraceEvent(agent, xml) ;
			return ;
		}

		// Have to make update in the UI thread.
		// Callback comes in the document thread.
		// NOTE: I had to write a real class here to do this rather than just using my
		// "normal" trick of making agent and xml "final" and creating a class on the fly.
		// Doing that lead to an intermittement memory leak from the xml object--really hard to track down
		// and I'm still not fully clear on why that happened, but I suspect if this event was called again
		// before the wrapper had run, we had a problem.
		
		// We need to create a new copy of the XML we were passed because we're
		// going to use an asynch call, which won't execute until after this function has
		// completed and xml goes out of scope.  Why use the asynch method rather than syncExec()?
		// Using asynch here lets things run up to 20 times faster(!)
		// March 2006 update: We've now allowed this to become a synch call, specifically because
		// Linux needs these two to stay in step and some users might not want the UI to get ahead.
		ClientXML pKeep = new ClientXML(xml) ;
		
		if (m_LockToSoar)
	        Display.getDefault().syncExec(new RunWrapper(this, agent, pKeep)) ;
		else
			Display.getDefault().asyncExec(new RunWrapper(this, agent, pKeep)) ;
	}

	/********************************************************************************************
	 * 
	 * Register for events of particular interest to this view
	 * 
	 ********************************************************************************************/
	protected void registerForViewAgentEvents(Agent agent)
	{
		m_xmlCallback = agent.RegisterForXMLEvent(smlXMLEventId.smlEVENT_XML_TRACE_OUTPUT, this, null) ;
	}

	protected void clearViewAgentEvents()
	{
		m_xmlCallback = -1 ;
	}

	protected boolean unregisterForViewAgentEvents(Agent agent)
	{
		boolean ok = true ;
		
		if (m_xmlCallback != -1)
			ok = agent.UnregisterForXMLEvent(m_xmlCallback) ;

		m_xmlCallback = -1 ;
		
		return ok ;
	}

	
	protected void storeContent(JavaElementXML element)
	{
		
	}

	protected void restoreContent(JavaElementXML element)
	{
		
	}

	/********************************************************************************************
	 * 
	 * Display a dialog that allows the user to adjust properties for this window
	 * e.g. choosing whether to clear the window everytime a new command executes or not.
	 * 
	********************************************************************************************/
	public void showProperties()
	{
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[4] ;

		// Providing a range for indent so we can be sure we don't get back a negative value
		properties[0] = new PropertiesDialog.IntProperty("Indent per subgoal", m_IndentSize, 0, 10) ;
		properties[1] = new PropertiesDialog.BooleanProperty("Expand trace as it is created", m_ExpandTracePersistent) ;
		properties[2] = new PropertiesDialog.BooleanProperty("Capture data to support filtering", m_FoldingText.isFilteringEnabled()) ;
		properties[3] = new PropertiesDialog.BooleanProperty("Keep Soar locked to the speed of the debugger (makes Soar a lot slower)", m_LockToSoar) ;

		boolean ok = PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;

		if (ok)
		{
			m_IndentSize = ((PropertiesDialog.IntProperty)properties[0]).getValue() ;
			m_ExpandTracePersistent = ((PropertiesDialog.BooleanProperty)properties[1]).getValue() ;
			m_FoldingText.setFilteringEnabled(((PropertiesDialog.BooleanProperty)properties[2]).getValue()) ;
			m_LockToSoar = ((PropertiesDialog.BooleanProperty)properties[3]).getValue() ;
			
			// Make the button match the persistent property
			m_ExpandTrace = m_ExpandTracePersistent ;
			updateButtonState() ;
		}		
	}
	
	/************************************************************************
	* 
	* Converts this object into an XML representation.
	* 
	* @param tagName		The tag name to use for the top XML element created by this view
	* @param storeContent	If true, record the content from the display (e.g. the text from a trace window)
	* 
	*************************************************************************/
	public general.JavaElementXML convertToXML(String tagName, boolean storeContent)
	{
		JavaElementXML element = super.convertToXML(tagName, storeContent) ;
		element.addAttribute("indent", Integer.toString(m_IndentSize)) ;
		element.addAttribute("auto-expand", Boolean.toString(m_ExpandTracePersistent)) ;
		element.addAttribute("lock-to-soar", Boolean.toString(m_LockToSoar)) ;

		if (m_FoldingText != null)
		{
			element.addAttribute("filtering", Boolean.toString(m_FoldingText.isFilteringEnabled())) ;
			element.addAttribute("filter", Long.toString(m_FoldingText.getExclusionFilter())) ;
		}
		
		return element ;
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
	public void loadFromXML(MainFrame frame, doc.Document doc, Pane parent, general.JavaElementXML element) throws Exception
	{
		m_IndentSize = element.getAttributeIntThrows("indent") ;
		m_ExpandTracePersistent = element.getAttributeBooleanDefault("auto-expand", false) ;
		m_LockToSoar = element.getAttributeBooleanDefault("lock-to-soar", false) ;

		boolean filtering = element.getAttributeBooleanDefault("filtering", true) ;
		long filter = element.getAttributeLongDefault("filter", 0) ;
		
		super.loadFromXML(frame, doc, parent, element) ;
		
		// Have to wait until base class has been called and window has been created before setting these values
		if (m_FoldingText != null)
		{
			m_FoldingText.setFilteringEnabled(filtering) ;
			m_FoldingText.setExclusionFilter(filter, false) ;
			updateButtonState() ;
		}
	}
}

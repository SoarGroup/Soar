/********************************************************************************************
*
* TreeTraceView.java
* 
* Description:	Represents trace output using a tree, so we can capture trace information
* 				at one level of detail and display a lesser amount of detail.
* 
* Created on 	Mar 29, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;

import general.ElementXML;
import helpers.CommandHistory;
import helpers.FormDataHelper;
import manager.Pane;
import menu.ParseSelectedText;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.ControlEditor;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.custom.TableEditor;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;

import sml.*;
import debugger.MainFrame;
import dialogs.PropertiesDialog;
import doc.Document;

/************************************************************************
 * 
 * Represents trace output using a tree, so we can capture trace information
 * at one level of detail and display a lesser amount of detail.
 * 
 ************************************************************************/
public class TreeTraceView extends AbstractComboView
{
	protected Tree m_Tree ;
	
	protected int m_xmlCallback = -1 ;
	
	/** How many spaces we indent for a subgoal */
	protected int m_IndentSize = 3 ;
	
	/** When true, expand the tree automatically as it's created */
	protected boolean m_AutoExpand = false ;
	
	/** The last root (top level item) added to the tree.  We add new sub items under this */
	protected TreeItem m_LastRoot ;
	protected TreeItem m_DummyChild ;
	
	protected Composite	m_Buttons ;

	protected Button m_ExpandPageButton ;
	protected Button m_ExpandPageArrow ;
	protected Menu   m_ExpandPageMenu ;
	
	/** Controls whether we cache strings that are due to be subtree nodes and only add the nodes when the user clicks--or not */
	protected final static boolean kCacheSubText = true ;

	protected static final int kCachedSpaces = 100 ;
	protected static final String[] kPadSpaces = new String[kCachedSpaces] ;
	
	protected final static String kNextKey = "next" ;
	protected final static String kPrevKey = "prev" ;
	protected final static String kLazyKey = "lazy" ;
	
	static
	{
		// Fill in the kPadSpaces array
		StringBuffer buffer = new StringBuffer() ;
		
		for (int i = 0 ; i < kCachedSpaces ; i++)
		{
			kPadSpaces[i] = buffer.toString() ;
			buffer.append(" ") ;
		}
	}
	
	/** We use this structure if we're caching sub nodes in the tree for expansion only when the user clicks */
	protected static class TreeData
	{
		protected ArrayList m_Lines = new ArrayList() ;
		
		public void addLine(String text) 	{ m_Lines.add(text) ; }
		public Iterator getLinesIterator() 	{ return m_Lines.iterator() ; }
	}
	
	public TreeTraceView()
	{
		m_ClearEachCommand = false ;
		m_UpdateOnStop = false ;
		m_ClearComboEachCommand = true ;
		m_ComboAtTop = false ;
		m_ShowTraceOutput = false ;
		m_PromptForCommands = "<Type commands here>" ;		
	}

	/** This window can be the main display window */
	public boolean canBePrimeWindow() { return true ; }

	public Color getBackgroundColor() { return m_Frame.m_White ; }
	
	protected class ExpandListener implements Listener
	{
		public void handleEvent (final Event event) {
			TreeItem root = (TreeItem) event.item;
			
			expandRoot(root, true) ;
		}
	}
	
	/********************************************************************************************
	* 
	* Create the window that will display the output
	* 
	********************************************************************************************/
	protected void createDisplayControl(Composite parent)
	{
		m_Tree = new Tree(parent, SWT.SINGLE | SWT.BORDER) ;
		m_LastRoot = null ;
		m_DummyChild = null ;
		
		createContextMenu(m_Tree) ;

		// When the user expands a node in the tree we may unpack some cached data
		m_Tree.addListener (SWT.Expand, new ExpandListener()) ;

		m_Tree.addMouseListener(new MouseAdapter() { public void mouseDown(MouseEvent e) { if (e.button == 1) leftMouseDown(e.x, e.y) ; } } ) ;
		
		m_Buttons = new Composite(m_ComboContainer, 0) ;
		m_Buttons.setLayout(new RowLayout()) ;
		Composite owner = m_Buttons ;
		
		// Add a button that offers an expand/collapse option instantly (for just one page)
		m_ExpandPageButton = new Button(owner, SWT.PUSH);
		m_ExpandPageButton.setText("Expand page") ;
		m_ExpandPageButton.setData("expanded", m_AutoExpand ? Boolean.TRUE : Boolean.FALSE) ;	// When this flag is false pressing the button expands

		m_ExpandPageButton.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e)
		{ boolean expand = (e.widget.getData("expanded") == Boolean.FALSE) ;
		  expandPage(expand) ; } } ) ;
		
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

//		menuItem = new MenuItem (m_ExpandPageMenu, SWT.PUSH);
//		menuItem.setText ("Collapse page");		
//		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { expandPage(false) ; } } ) ;

		menuItem = new MenuItem (m_ExpandPageMenu, SWT.PUSH);
		menuItem.setText ("Expand all");		
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { expandAll(true) ; } } ) ;

		menuItem = new MenuItem (m_ExpandPageMenu, SWT.PUSH);
		menuItem.setText ("Collapse all");		
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { expandAll(false) ; } } ) ;
	}
	
	/********************************************************************************************
	 * 
	 * Returns the next item in the tree after the given item.
	 * 
	 * The 'next' algorithm traverses parents before their children.
	 * 
	 * If "includeCollapsed" is true we search inside collapsed nodes (which may also
	 * require lazy expansion as we cache data within the tree without expanding it).
	 * 
	 * @param current
	 * @param includeCollapsed
	 * @return
	********************************************************************************************/
	protected TreeItem getNextItemInternal(TreeItem current, boolean includeCollapsed)
	{
		// If the current item has children, return the first child
		if (current.getItemCount() > 0 && (current.getExpanded() || includeCollapsed))
		{
			return current.getItems()[0] ;
		}
		
		while (current != null)
		{
			// Look up the next sibling item in the tree
			// To make this efficient we link the nodes together when they
			// are created with next and prev key items.
			TreeItem next = (TreeItem)current.getData(kNextKey) ;
			
			if (next != null)
				return next ;
			
			// If we're at the end of the list of siblings, move to the parent and
			// the try to move to the next parent (by looping through here again).
			TreeItem parent = current.getParentItem() ;
			current = parent ;
		}
		
		return null ;
	}

	/********************************************************************************************
	 * 
	 * Returns the next item in the tree after the given item.
	 * 
	 * The 'next' algorithm traverses parents before their children.
	 * 
	 * If "includeCollapsed" is true we search inside collapsed nodes (which may also
	 * require lazy expansion as we cache data within the tree without expanding it).
	 * 
	 * @param current
	 * @param includeCollapsed
	 * @return
	********************************************************************************************/
	protected TreeItem getNextItem(TreeItem current, boolean includeCollapsed)
	{
		TreeItem next = getNextItemInternal(current, includeCollapsed) ;
		
		// If we've been asked to search hidden text we
		// need to expand the contents of nodes we land on that have lazy (cached) data
		if (includeCollapsed && next != null && next.getExpanded() == false && next.getParentItem() == null)
			expandRoot(next, false) ;
		
		return next ;
	}
	
	/********************************************************************************************
	 * 
	 * Returns the previous item in the tree after the given item.
	 * 
	 * The 'prev' algorithm traverses children before their parents.
	 * 
	 * If "includeCollapsed" is true we search inside collapsed nodes (which may also
	 * require lazy expansion as we cache data within the tree without expanding it).
	 * 
	 * @param current
	 * @param includeCollapsed
	 * @return
	********************************************************************************************/
	protected TreeItem getPrevItem(TreeItem current, boolean includeCollapsed)
	{
		TreeItem prev = (TreeItem)current.getData(kPrevKey) ;
		
		if (prev == null)
			return current.getParentItem() ;
		
		// If we've moved back to a node that has children
		// we want to the last descendant in that tree.
		current = prev ;
		while (current.getItemCount() > 0 && (current.getExpanded() || includeCollapsed))
		{
			// If we're searching inside collapsed nodes we may need to expand text that
			// has been cached within a tree node and is yet to be expanded.
			if (includeCollapsed && current.getParentItem() == null && current.getItemCount() == 1)
				expandRoot(current, false) ;
			
			current = current.getItems()[current.getItemCount()-1] ;
		}
			
		return current ;
	}

	/********************************************************************************************
	 * 
	 * Returns the first item in the tree (the first root)
	 * 
	********************************************************************************************/
	protected TreeItem getFirstTreeItem()
	{
		return m_Tree.getItems()[0] ;
	}
	
	/********************************************************************************************
	 * 
	 * Returns the last item in the tree (the last descendant of the last root in the tree).
	 * 
	 * @param includeCollapsed - Includes nodes that are currently collapsed in the calculation
	 * 
	********************************************************************************************/
	protected TreeItem getLastTreeItem(boolean includeCollapsed)
	{
		TreeItem last = m_Tree.getItems()[m_Tree.getItemCount() - 1] ;
		
		while (last.getItemCount() > 0 && (includeCollapsed || last.getExpanded()))
		{
			if (includeCollapsed && last.getParentItem() == null && last.getItemCount() == 1)
				expandRoot(last, false) ;
			
			last = last.getItems()[last.getItemCount()-1] ;
		}
		
		return last ;
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
		if (m_Tree.getItemCount() == 0)
			return false ;
		
		// Show the wait cursor as the hidden search could take a while
		Cursor wait = new Cursor(getWindow().getDisplay(), SWT.CURSOR_WAIT) ;
		getWindow().getShell().setCursor(wait) ;
		
		// Turn off redrawing while we're working with the tree
		// (searching hidden text could involve creating nodes)
		m_Tree.setRedraw(false) ;

		// If this is a search where we don't care about case we shift everything
		// to lower case and match that way.
		if (!matchCase)
			text = text.toLowerCase() ;
		
		// Get the current selection
		TreeItem[] selections = m_Tree.getSelection() ;
		
		// Default to the top of the entire tree if nothing is selected
		TreeItem selection = searchDown ? getFirstTreeItem() : getLastTreeItem(searchHiddenText) ;

		// Record where we started from.  If we get back to here we're done
		TreeItem originalSelection = selection ;
		
		// If we have an existing selection, start searching from the next line after this one
		if (selections.length > 0)
		{
			TreeItem start = searchDown ? getNextItem(selections[0], searchHiddenText) : getPrevItem(selections[0], searchHiddenText) ;
			if (start != null)
			{
				// Set the original selection to be the line before 'start' so if we get back to this line we're done.
				// (So if you find one match and seach for another with wrap you get a 'not found' rather than hitting this again)
				originalSelection = selections[0] ;
				selection = start ;
			}
		}
		
		boolean done = false ;
		boolean found = false ;
		
		while (!done)
		{
			// Get the text for the current item and check for a match
			String windowText = (matchCase) ? selection.getText() : selection.getText().toLowerCase() ;
			
			int match = windowText.indexOf(text) ;
			if (match != -1)
			{
				m_Tree.setSelection(new TreeItem[] { selection }) ;
				done = true ;
				found = true ;
				break ;
			}
			
			// Move on to the next item in the tree
			TreeItem next = searchDown ? getNextItem(selection, searchHiddenText) : getPrevItem(selection, searchHiddenText) ;

//			if (next != null)
//				System.out.println("Next after " + selection.getText() + " is " + next.getText()) ;

			selection = next ;
			
			if (selection == null)
			{
				if (wrap)
				{
					// Only wrap once
					wrap = false ;
					
					// Move the selection to the top/bottom as necessary to do the wrap
					selection = searchDown ? getFirstTreeItem() : getLastTreeItem(searchHiddenText) ;
				}
				else
				{
					done = true ;
				}
			}
			
			// If we get back to the start of the search we're done
			if (originalSelection == selection)
				done = true ;
		}
		
		m_Tree.setRedraw(true) ;
		
		getWindow().getShell().setCursor(null) ;
		wait.dispose() ;
		return found ;
	}
	
	/*********************************************************************************************
	 * 
	 * We want to allow people to copy text out of the tree view.
	 * That's not easy.
	 * 
	 * In order to do it when they click we create a text control on the line
	 * where they clicked and allow them to copy text from it.
	 * This is the same method that editable tables use to let you edit them.
	 * 
	 * @param e
	*******************************************************************************************
	 */
	protected void leftMouseDown(int mouseX, int mouseY)
	{
		// Figure out which item the user clicked on
		Point pt = new Point(mouseX, mouseY) ;
		TreeItem item = m_Tree.getItem(pt) ;
		
		if (item == null)
		{
			//System.out.println("No item clicked") ;
			return ;
		}

		ControlEditor editor = new ControlEditor (m_Tree);		
		editor.grabHorizontal = true;

		final Text text = new Text (m_Tree, SWT.READ_ONLY);
		Listener textListener = new Listener () {
			public void handleEvent (final Event e) {
				switch (e.type) {
					case SWT.FocusOut:
						//System.out.println("Focus lost") ;
						text.dispose ();
						break;
					case SWT.Traverse:
						switch (e.detail) {
							case SWT.TRAVERSE_RETURN:
								//FALL THROUGH
							case SWT.TRAVERSE_ESCAPE:
								//System.out.println("Traverse event") ;
								text.dispose ();
								e.doit = false;
						}
						break;
				}
			}
		};
		text.addListener (SWT.FocusOut, textListener);
		text.addListener (SWT.Traverse, textListener);
		text.setText (item.getText());
		text.setFont(m_Tree.getFont()) ;
		
		editor.setEditor(text) ;

		text.setBounds(item.getBounds()) ;

		// Replace the standard right click menu with ours
		createContextMenu(text) ;

		text.setFocus ();
		text.setVisible(true) ;

		// This attempt to set the selection isn't working...don't know why
		int x = item.getBounds().x ;
		int y = item.getBounds().y ;
		int offsetX = mouseX - x ;
		int selectionPoint = getSelectionPoint(offsetX, item) ;
		
		text.setSelection(offsetX, offsetX + 1) ;
//		text.selectAll ();
		
		//System.out.println("Set selection " + selectionPoint) ;
	}
	
	protected int getSelectionPoint(int x, TreeItem item)
	{
		// Now we have a location inside the item, but what we need to know
		// is which character they clicked on.  The only way to compute this I can think of
		// is to generate each substring in turn and check its length against the point.
		// When we reach the correct length of string we've found the character.
		// This is slow and a bit clumsy, but since it's just on a right-click I think it's ok.
		GC gc = new GC(m_Tree) ;
		
		int selectionPoint = -1 ;
		String text = item.getText() ;
		for (int i = 0 ; i < text.length() ; i++)
		{
			Point extent = gc.textExtent(text.substring(0, i)) ;
			if (extent.x > x)
			{
				selectionPoint = (i == 0) ? 0 : i-1 ;
				break ;
			}
		}

		gc.dispose() ;
		
		return selectionPoint ;
	}
	
	/************************************************************************
	* 
	* Go from current selection (where right click occured) to the object
	* selected by the user (e.g. a production name).
	* 
	*************************************************************************/
	protected ParseSelectedText.SelectedObject getCurrentSelection(int mouseX, int mouseY)
	{
		// Figure out which item the user clicked on
		Point pt = m_Tree.toControl(mouseX, mouseY) ;
		TreeItem item = m_Tree.getItem(pt) ;
		
		if (item == null)
		{
			return null ;
		}
		
		// We want to know how far into the item the mouse click was
		Rectangle bounds = item.getBounds() ;
		int offsetX = pt.x - bounds.x ;

		// Now we have a location inside the item, but what we need to know
		// is which character they clicked on.  The only way to compute this I can think of
		// is to generate each substring in turn and check its length against the point.
		// When we reach the correct length of string we've found the character.
		// This is slow and a bit clumsy, but since it's just on a right-click I think it's ok.
		int selectionPoint = getSelectionPoint(offsetX, item) ;
		
		if (selectionPoint == -1)
			return null ;

		String text = item.getText() ;

		// One final issue is that we sometimes need to search back up the trace
		// to find values (e.g. looking for the identifier from a print) which could
		// be several lines up.  So we need to collect the text from the previous <n> lines,
		// join it together (with spaces between the lines) and adjust the selection point
		// to account for all this.
		// For simplicity we'll just search up for the last <n> items from this one and collect
		// the text together.
		// BUGBUG: Still need to implement this.
		
		// Now take the text and the position and run our regular parse to figure out
		// what type of thing the user clicked on (e.g. a production/wme etc.)
		ParseSelectedText selection = new ParseSelectedText(text, selectionPoint) ;
		
		return selection.getParsedObject() ;
	}
	
	protected void expandPage(boolean state)
	{
		if (m_Tree.getItemCount() == 0)
			return ;
		
		// Stop redrawing while we expand/collapse everything then turn it back on
		m_Tree.setRedraw(false) ;
		
		TreeItem[] roots = m_Tree.getItems() ;
		
		// Find the root item on this page
		TreeItem top = m_Tree.getTopItem() ;
		while (top.getParentItem() != null)
			top = top.getParentItem() ;

		int start = -1 ;
		for (int i = 0 ; i < roots.length ; i++)
		{
			if (top == roots[i])
			{
				start = i ;
				break ;
			}
		}
		
		if (start == -1)
			throw new IllegalStateException("Error finding top of the tree") ;
		
		int pageSize = 50 ;
		// For now we'll approximate a page as <n> nodes in the tree.
		// The key is that it should be enough to fill a page but not so much
		// as to take forever to do.  I don't see a method to determine which item
		// is at the bottom of the page which would make this precise.
		int end = start + pageSize ;
		if (end > roots.length) end = roots.length ;
		
		for (int i = start ; i < end ; i++)
		{	
			// In SWT the programmatic call to expand an item does not
			// generate an expand event, so we have to explicitly handle the expansion
			// (i.e. unpack our cached text)
			if (state)
				expandRoot(roots[i], false) ;

			roots[i].setExpanded(state) ;
		}
		
		// Scroll to have the top (now expanded) visible
		m_Tree.showItem(top) ;
		
		m_Tree.setRedraw(true) ;		
	}
	
	protected void expandAll(boolean state)
	{
		// Stop redrawing while we expand/collapse everything then turn it back on
		m_Tree.setRedraw(false) ;
		
		TreeItem[] roots = m_Tree.getItems() ;
		for (int i = 0 ; i < roots.length ; i++)
		{	
			// In SWT the programmatic call to expand an item does not
			// generate an expand event, so we have to explicitly handle the expansion
			// (i.e. unpack our cached text)
			if (state)
				expandRoot(roots[i], false) ;

			roots[i].setExpanded(state) ;
		}
		
		m_Tree.setRedraw(true) ;
	}
	
	protected void expandRoot(TreeItem root, boolean redraw)
	{
		// We will have exactly one dummy child item
		// if there's cached data here.  This allows us to quickly
		// screen out most "already been expanded before" cases.
		if (root.getItemCount() != 1)
			return ;
		
		TreeItem[] items = root.getItems() ;
		
		TreeData treeData = (TreeData)items[0].getData(kLazyKey) ;
		
		// If there's no cached data here then once again we are done
		// (either we're not using a cache or its already been expanded)
		if (treeData == null)
			return ;
		
		// Stop updating while we add
		// (We may want a smarter way to do this if we're expanding the entire tree
		//  so the redraw is turned off for the entire expansion).
		if (redraw)
			root.getParent().setRedraw(false) ;
		
		// Get rid of the dummy item
		items[0].dispose() ;
		
		TreeItem prev = null ;
		
		for (Iterator iter = treeData.getLinesIterator() ; iter.hasNext() ;)
		{
			String text = (String)iter.next() ;

			String[] lines = text.split(getLineSeparator()) ;
			
			for (int i = 0 ; i < lines.length ; i++)
			{	
				if (lines[i].length() == 0)
					continue ;
				
				TreeItem node = new TreeItem (root, 0);
				node.setText (lines[i]);
				
				// Link the nodes together to make navigating from one node to the next
				// more efficient later
				if (prev != null)
					prev.setData(kNextKey, node) ;
				node.setData(kPrevKey, prev) ;
				prev = node ;
			}				
		}
					
		// Start updating again.
		if (redraw)
			root.getParent().setRedraw(true) ;
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
		return "treetrace" ;
	}

	/** The control we're using to display the output in this case **/
	protected Control getDisplayControl()
	{
		return m_Tree ;
	}

	/************************************************************************
	* 
	* Add the text to the view in a thread safe way (switches to UI thread)
	* 
	*************************************************************************/
	protected void appendSubTextSafely(final String text, final boolean redrawTree)
	{
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			appendSubText(text, redrawTree) ;
			return ;
		}

		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
            	appendSubText(text, redrawTree) ;
            }
         }) ;
	}
	
	/************************************************************************
	* 
	* Add the text to the view (this method assumes always called from UI thread)
	* 
	*************************************************************************/
	protected void appendSubText(String text, boolean redrawTree)
	{
		// Add the sub items under the last root in the tree
		if (m_LastRoot == null)
		{
			m_LastRoot = new TreeItem(m_Tree, 0) ;
		}

		// Creating nodes in the tree is expensive.  A more efficient
		// model is to create a dummy child node and store the text inside that node.
		// When the user clicks on the parent we quickly create the tree they expect to see.
		if (this.kCacheSubText && !m_AutoExpand)
		{
			if (m_LastRoot.getItemCount() == 0)
			{
				m_DummyChild = new TreeItem(m_LastRoot, 0) ;
				m_DummyChild.setData(kLazyKey, new TreeData()) ;
			}

			// The dummy child should always exist (unless somehow we already expanded the tree, which is possible)
			if (m_DummyChild != null)
			{
				TreeData currentData = (TreeData)m_DummyChild.getData(kLazyKey) ;
				currentData.addLine(text) ;
				return ;
			}
		}
		
		TreeItem lastItem = m_LastRoot ;
		
		// Stop updating the tree while we add to it
		if (redrawTree)
			m_Tree.setRedraw(false) ;

		String[] lines = text.split(getLineSeparator()) ;

		TreeItem prev = null ;

		// Get the previous item added to this root (if there is one)
		if (m_LastRoot.getItemCount() > 0)
			prev = m_LastRoot.getItems()[m_LastRoot.getItemCount()-1] ;
		
		for (int i = 0 ; i < lines.length ; i++)
		{	
			if (lines[i].length() == 0)
				continue ;
			
			TreeItem node = new TreeItem (lastItem, 0);
			node.setText (lines[i]);
			
			// Link the nodes together to make navigating from one node to the next
			// more efficient later
			if (prev != null)
				prev.setData(kNextKey, node) ;
			node.setData(kPrevKey, prev) ;
			prev = node ;

			if (m_AutoExpand && !lastItem.getExpanded())
			{
				lastItem.setExpanded(true) ;
			}
		}

		// Scroll to the bottom -- not doing this for sub items to save speed
		// Requires that we issue a final top level item at the end of a run.
		//if (lastItem != null)
		//	m_Tree.showItem(lastItem) ;

		// Redraw the updated tree
		if (redrawTree)
			m_Tree.setRedraw(redrawTree) ;		
	}
	
	/************************************************************************
	* 
	* Add the text to the view (this method assumes always called from UI thread)
	* 
	*************************************************************************/
	protected void appendText(String text)
	{
		String[] lines = text.split(getLineSeparator()) ;

		TreeItem lastItem = m_LastRoot ;
		
		// Stop updating the tree while we add to it
		m_Tree.setRedraw(false) ;

		for (int i = 0 ; i < lines.length ; i++)
		{	
			if (lines[i].length() == 0)
				continue ;
			
			TreeItem root = new TreeItem (m_Tree, 0);
			root.setText (lines[i]);
			
			// Link the nodes together with prev and next keys to make
			// searching more efficient.
			if (lastItem != null)
				lastItem.setData(kNextKey, root) ;
			root.setData(kPrevKey, lastItem) ;
			lastItem = root ;
		}

		// Scroll to the bottom -- horribly slow
		if (lastItem != m_LastRoot)
		{
			m_Tree.showItem(lastItem) ;
			m_LastRoot = lastItem ;
		}
		
		// Redraw the updated tree
		m_Tree.setRedraw(true) ;
	}

	/************************************************************************
	* 
	* Clear the display control.
	* 
	*************************************************************************/
	public void clearDisplay()
	{
		m_Tree.removeAll() ;
		m_LastRoot = null ;
		
		if (m_DummyChild != null && !m_DummyChild.isDisposed())
			m_DummyChild.dispose() ;
		m_DummyChild = null ;
	}
	
	/** Returns a string of spaces of the given length (>= 0).  This is an efficient calculation */
	protected String getSpaces(int length)
	{
		if (length <= 0)
			return "" ;
		
		// We use a lookup from a table if the length is reasonable (< 100 right now)
		if (length < kPadSpaces.length)
			return kPadSpaces[length] ;
		
		// Otherwise we have to generate it which is slow
		StringBuffer buffer = new StringBuffer() ;
		buffer.append(kPadSpaces[kPadSpaces.length - 1]) ;
		
		// If we use this a lot we could speed it up by using a binary addition process
		// but I hope to never use it (except in a run-away stack situation).
		for (int i = 0 ; i < length - kPadSpaces.length ; i++)
		{
			buffer.append(" ") ;
		}
		
		return buffer.toString() ;
	}
	
	/** Add spaces to the length until reaches minLength */
	protected String padLeft(String orig, int minLength)
	{
		if (orig.length() >= minLength)
			return orig ;
				
		// Add the appropriate number of spaces.
		return getSpaces(minLength - orig.length()) + orig ;
	}
	
	/** Returns a string to indent to a certain stack depth (depth stored as a string) */
	protected String indent(String depthStr, int modifier)
	{
		if (depthStr == null)
			return "" ;
		
		int depth = Integer.parseInt(depthStr) + modifier ;
		
		int indentSize = depth * m_IndentSize ;
		
		return getSpaces(indentSize) ;
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
		// Stop updating the tree control while we work on it
		m_Tree.setRedraw(false) ;
		
		// For debugging
		//String message = xmlParent.GenerateXMLString(true) ;
		//System.out.println(message) ;
		
		int nChildren = xmlParent.GetNumberChildren() ;
		
		for (int childIndex = 0 ; childIndex < nChildren ; childIndex++)
		{
			// Analyze the children as ClientTraceXML objects
			ClientTraceXML xmlTrace = new ClientTraceXML() ;

			// Get each child in turn
			xmlParent.GetChild(xmlTrace, childIndex) ;
			
			StringBuffer text = new StringBuffer() ;
			final int decisionDigits = 3 ;
			
			// This is a state change (new decision)
			if (xmlTrace.IsTagState())
			{
				// 3:    ==>S: S2 (operator no-change)
				text.append(padLeft(xmlTrace.GetDecisionCycleCount(), decisionDigits)) ;
				text.append(": ") ;
				text.append(indent(xmlTrace.GetStackLevel(), -1)) ;

				// Add an appropriate subgoal marker to match the indent size
				if (m_IndentSize == 3)
					text.append("==>") ;
				else if (m_IndentSize == 2)
					text.append("=>") ;
				else if (m_IndentSize == 1)
					text.append(">") ;
				else if (m_IndentSize > 3)
				{
					text.append(getSpaces(m_IndentSize - 3)) ;
					text.append("==>") ;
				}
				
				text.append("S: ") ;
				text.append(xmlTrace.GetStateID()) ;
				
				if (xmlTrace.GetImpasseObject() != null)
				{
					text.append(" (") ;
					text.append(xmlTrace.GetImpasseObject()) ;
					text.append(" ") ;
					text.append(xmlTrace.GetImpasseType()) ;
					text.append(")") ;
				}
				
				if (text.length() != 0)
					this.appendText(text.toString()) ;
			} else if (xmlTrace.IsTagOperator())
			{
				 //2:    O: O8 (move-block)
				text.append(padLeft(xmlTrace.GetDecisionCycleCount(), decisionDigits)) ;
				text.append(": ") ;
				text.append(indent(xmlTrace.GetStackLevel(), 0)) ;
				text.append("O: ") ;
				text.append(xmlTrace.GetOperatorID()) ;
				
				if (xmlTrace.GetOperatorName() != null)
				{
					text.append(" (") ;
					text.append(xmlTrace.GetOperatorName()) ;
					text.append(")") ;
				}
	
				if (text.length() != 0)
					this.appendText(text.toString()) ;
			} else if (xmlTrace.IsTagRhsWrite())
			{
				text.append(xmlTrace.GetString()) ;
				
				if (text.length() != 0)
					this.appendText(text.toString()) ;				
				
			} else if (xmlTrace.IsTagPhase())
			{
				String status = xmlTrace.GetPhaseStatus() ;
				
				text.append("--- ") ;
				text.append(xmlTrace.GetPhaseName()) ;
				text.append(" ") ;
				text.append("phase ") ;
				if (status != null)
					text.append(status) ;
				text.append("---") ;
				
				// Don't show end of phase messages
				boolean endOfPhase = (status != null && status.equalsIgnoreCase("end")) ;
				
				if (text.length() != 0 && !endOfPhase)
					this.appendSubText(text.toString(), false) ;
			}
			else if (xmlTrace.IsTagAddWme() || xmlTrace.IsTagRemoveWme())
			{
				boolean adding = xmlTrace.IsTagAddWme() ;
				for (int i = 0 ; i < xmlTrace.GetNumberChildren() ; i++)
				{
					ClientTraceXML child = new ClientTraceXML() ;
					xmlTrace.GetChild(child, i) ;
					
					if (child.IsTagWme())
					{
						text.append(adding ? "=>WM: (" : "<=WM: (") ;
						text.append(child.GetWmeTimeTag()) ;
						text.append(": ") ;
						text.append(child.GetWmeID()) ;
						text.append(" ^") ;
						text.append(child.GetWmeAttribute()) ;
						text.append(" ") ;
						text.append(child.GetWmeValue()) ;
						text.append(")") ;
					}
					
					child.delete() ;
				}
				
				if (text.length() != 0)
					this.appendSubText(text.toString(), false) ;	

			} else if (xmlTrace.IsTagPreference())
			{
				text.append("--> (") ;
				text.append(xmlTrace.GetPreferenceID()) ;
				text.append(" ^") ;
				text.append(xmlTrace.GetPreferenceAttribute()) ;
				text.append(" ") ;
				text.append(xmlTrace.GetPreferenceValue()) ;
				text.append(" + )") ;	// BUGBUG: + hard-coded for now -- PreferenceType present but empty

				if (text.length() != 0)
					this.appendSubText(text.toString(), false) ;
				
			} 
			else if (xmlTrace.IsTagFiringProduction() || xmlTrace.IsTagRetractingProduction())
			{
				boolean firing = xmlTrace.IsTagFiringProduction() ;

				for (int i = 0 ; i < xmlTrace.GetNumberChildren() ; i++)
				{
					ClientTraceXML child = new ClientTraceXML() ;
					xmlTrace.GetChild(child, i) ;
					if (child.IsTagProduction())
					{
						if (i > 0)
							text.append(getLineSeparator()) ;
						
						text.append(firing ? "Firing " : "Retracting ") ;
						text.append(child.GetProductionName()) ;
					}
					child.delete() ;
				}

				if (text.length() != 0)
					this.appendSubText(text.toString(), false) ;			
	
			}
			else
			{
				// These lines can be helpful if debugging this
				String xmlText = xmlParent.GenerateXMLString(true) ;
				System.out.println(xmlText) ;
			}

			// Manually clean up the child too
			xmlTrace.delete() ;	
		}
		
		// Technically this will happen when the object is garbage collected (and finalized)
		// but without it we'll get a million memory leak messages because (a) gc may not have been run for a while
		// (b) even if it runs not all objects will be reclaimed and (c) finalize isn't guaranteed before we exit
		// so all in all, let's just call it ourselves here :)
		xmlParent.delete() ;
		
		// Redraw the tree to show our changes
		m_Tree.setRedraw(true) ;
	}
	
	public static class RunWrapper implements Runnable
	{
		TreeTraceView m_View ;
		Agent 		  m_Agent ;
		ClientXML 	  m_XML ;
		
		public RunWrapper(TreeTraceView view, Agent agent, ClientXML xml)
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
		// completed and xml goes out of scope.
		ClientXML pKeep = new ClientXML(xml) ;
        Display.getDefault().syncExec(new RunWrapper(this, agent, pKeep)) ;
	}

	/********************************************************************************************
	 * 
	 * Register for events of particular interest to this view
	 * 
	 ********************************************************************************************/
	protected void registerForViewAgentEvents(Agent agent)
	{
		m_xmlCallback = agent.RegisterForXMLEvent(smlXMLEventId.smlEVENT_XML_TRACE_OUTPUT, this, "xmlEventHandler", null) ;
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

	
	protected void storeContent(ElementXML element)
	{
		
	}

	protected void restoreContent(ElementXML element)
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
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[2] ;

		// Providing a range for indent so we can be sure we don't get back a negative value
		properties[0] = new PropertiesDialog.IntProperty("Indent per subgoal", m_IndentSize, 0, 10) ;
		properties[1] = new PropertiesDialog.BooleanProperty("Expand trace as it is created", m_AutoExpand) ;

		boolean ok = PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;

		if (ok)
		{
			m_IndentSize = ((PropertiesDialog.IntProperty)properties[0]).getValue() ;
			m_AutoExpand = ((PropertiesDialog.BooleanProperty)properties[1]).getValue() ;
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
	public general.ElementXML convertToXML(String tagName, boolean storeContent)
	{
		ElementXML element = super.convertToXML(tagName, storeContent) ;
		element.addAttribute("indent", Integer.toString(m_IndentSize)) ;
		element.addAttribute("auto-expand", Boolean.toString(m_AutoExpand)) ;
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
	public void loadFromXML(MainFrame frame, doc.Document doc, Pane parent, general.ElementXML element) throws Exception
	{
		m_IndentSize = element.getAttributeIntThrows("indent") ;
		m_AutoExpand = element.getAttributeBooleanDefault("auto-expand", false) ;
		super.loadFromXML(frame, doc, parent, element) ;		
	}

}

/********************************************************************************************
*
* Pane.java
* 
* Description:	
* 
* Created on 	Feb 16, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package manager;

import modules.* ;
import general.JavaElementXML;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;

import debugger.MainFrame;
import doc.Document;

/************************************************************************
 * 
 * A pane represents a sub-window in the MainWindow.
 * 
 * A module is attached to a pane which gives it is content (e.g. a text window
 * for typing commands and receiving output or a tree control for viewing state).
 * 
 * Thus a pane is simply a place holder.  It can manage a single module
 * or a list of modules (through a tabbed window).
 * 
 ************************************************************************/
public class Pane
{
	// List of views attached to this pane
	// Each view represents a distinct module and only one is visible at a time (> 1 => we use some form of tab)
	private AbstractViewList	m_Views = new AbstractViewList() ;
	private Composite			m_Pane ;	/** This window becomes the parent of the modules beneath it */
	
	public static final String	kPaneKey = "Pane" ;
	public static final String	kXMLKey  = "XML" ;
	public static final String  kTagName = "pane" ;
	
	public static final String kAttributeSingleView = "singleView" ;
	public static final String kAttributeTabAtTop	= "tabAtTop" ;	
	public static final String kAttributeView		= "view" ;	
	
	/** Some windows can be vertically or horizontally oriented.  Others can just ignore this property */
	protected boolean		m_HorizontalOrientation = true ;
	protected boolean		m_SingleView = true ;
	protected boolean		m_TabAtTop = false ;
	
	/********************************************************************************************
	 * 
	 * A pane is a rectangular area on the screen.  A view is a particular window (e.g. a trace window)
	 * of which several could occupy the same area on the screen and we'd tab between them.
	 * 
	 * When created we need to know if there will be only one view in this pane or multiple views
	 * so we know whether to create a tabbed window or not.
	 * 
	 * (Why do we need to know at creation? Because SWT controls need to know their parents
	 *  when they are created, so the children of this pane need to know if their parent should
	 *  be a simple group or a tab folder.)
	 * 
	 * @param singleView	If true, there must be only one view in this pane
	 * @param tabAtTop		If true, put the tab at the top (if not a single window)
	********************************************************************************************/
	public Pane(Composite parent, boolean singleView, boolean tabAtTop)
	{
		// Default to horizontal (for many views orientation is ignored anyway)
		m_HorizontalOrientation = true ;

		m_SingleView = singleView ;
		m_TabAtTop = tabAtTop ;
		
		Composite top = null ;
		
		if (singleView)
		{
			// We provide a border around the pane so we can find the edges to drag
			m_Pane = new Group(parent, SWT.SHADOW_ETCHED_IN) ;
			FillLayout layout = new FillLayout(SWT.VERTICAL) ;
			m_Pane.setLayout(layout) ;

			// Identify which SWT window is at the top of this pane
			top = m_Pane ;
		}
		else
		{
			TabFolder tabFolder = new TabFolder (parent, tabAtTop ? SWT.BORDER | SWT.TOP : SWT.BORDER | SWT.BOTTOM);
			m_Pane = tabFolder ;
			m_Pane.setLayout(new FillLayout(SWT.VERTICAL)) ;
			
			// Identify which SWT window is at the top of this pane
			top = m_Pane ;
		}
		
		// Mark this composite as a pane window so we know we're
		// at a leaf in the layout and moving over to the actual module's content
		top.setData(kPaneKey, this) ;
		top.setData(MainWindow.kWindowType, MainWindow.kTypePane) ;
	}
	
	// Provide a way to create a Pane without a parent window
	// This is just so we can have a temporary pane object.
	// I'm including a String "noParent" parameter here to make sure folks don't use this by accident.
	public Pane(String noParent)
	{
	}
	
	public Composite getWindow()
	{
		return m_Pane ;
	}
	
	/** Some windows can be vertically or horizontally oriented.  Others can just ignore this property */
	public boolean isHorizontalOrientation()			{ return m_HorizontalOrientation ; }
	public void setHorizontalOrientation(boolean state)	{ m_HorizontalOrientation = state ; }

	public boolean isSingleView()						{ return m_SingleView ; }
	public void setSingleView(boolean state)			{ m_SingleView = state ; }

	public boolean isTabAtTop()							{ return m_TabAtTop ; }
	public void setTabAtTop(boolean state)				{ m_TabAtTop = state ; }

	public void addView(AbstractView view)
	{
		if (!m_SingleView)
		{
			TabItem item = new TabItem ((TabFolder)m_Pane, SWT.NULL);
			item.setText (view.getName());
			item.setControl (view.getWindow());
		}
		else
		{
			if (m_Views.size() != 0)
				throw new IllegalStateException("Adding multiple views to a pane that was created to only hold one") ;
		}
		
		m_Views.add(view) ;
	}
	
	/** Update the names on the tabs (if there are any) to reflect the current view names */
	public void updateTabs()
	{
		if (m_SingleView)
			return ;
		
		TabFolder tabFolder = (TabFolder)m_Pane ;
		for (int i = 0 ; i < m_Views.size() ; i++)
		{
			// Set the tab to match the view's name
			tabFolder.getItem(i).setText(m_Views.get(i).getName()) ;
		}
	}
	
	public boolean removeView(AbstractView view)
	{
		return m_Views.remove(view) ;
	}

	public AbstractView getVisibleView()
	{
		if (m_Views.size() == 0)
			return null ;
		
		if (m_SingleView)
			return m_Views.get(0) ;
		
		int selectionIndex = ((TabFolder)m_Pane).getSelectionIndex() ;

		if (selectionIndex == -1)
			return null ;
		
		return m_Views.get(selectionIndex) ;
	}
	
	public boolean setFocus()
	{
		AbstractView view = getVisibleView() ;
		return view.setFocus() ;
	}
	
	public int getNumberViews()
	{
		return m_Views.size() ;
	}
	
	public AbstractView getView(int index)
	{
		return m_Views.get(index) ;
	}
	
	public JavaElementXML convertToXML(String tagName, boolean storeContent)
	{
		JavaElementXML element = new JavaElementXML(tagName) ;

		// We'll record a reference from the window the XML
		// This allows us to quickly index into the XML tree at a specific
		// point given a pane or a view.
		if (getWindow() != null)
			getWindow().setData(kXMLKey, element) ;
		
		element.addAttribute("horiz", Boolean.toString(m_HorizontalOrientation)) ;
		element.addAttribute(kAttributeSingleView, Boolean.toString(m_SingleView)) ;
		element.addAttribute(kAttributeTabAtTop, Boolean.toString(m_TabAtTop)) ;

		AbstractView visibleView = getVisibleView() ;
		if (visibleView != null)
			element.addAttribute(kAttributeView, visibleView.getName()) ;
		
		int n = m_Views.size() ;
		for (int i = 0 ; i < n ; i++)
		{
			AbstractView view = getView(i) ;
			JavaElementXML child = view.convertToXML(AbstractView.kTagView, storeContent) ;

			// Provide an easy way to go from the view to the XML
			if (view.getWindow() != null)
				view.getWindow().setData(kXMLKey, child) ;
			
			element.addChildElement(child) ;
		}
		
		return element ;
	}

	/** Look up the node in the XML tree that matches this pane.  Need to have called "convertToXML" for everything before calling this */
	public JavaElementXML getElementXML()
	{
		return (JavaElementXML)getWindow().getData(kXMLKey) ;
	}
	
	/** This load code creates the pane, because we need to know what type of pane to create -- single or multiple **/
	public static Pane loadFromXML(MainFrame frame, Document doc, Composite parent, JavaElementXML element) throws Exception
	{
		boolean singleView = element.getAttributeBooleanThrows(kAttributeSingleView) ;
		boolean tabAtTop = element.getAttributeBooleanThrows(kAttributeTabAtTop) ;
		
		Pane pane = new Pane(parent, singleView, tabAtTop) ;
		pane.loadFromXMLInternal(frame, doc, parent, element) ;
		
		return pane ;
	}
	
	private void loadFromXMLInternal(MainFrame frame, Document doc, Composite parent, JavaElementXML element) throws Exception
	{
		m_Views.clear() ;

		// FIXUP: We used to use the term orientation (during development).
		// Changed to horiz which is more informative.  Should remove this extra test for the old value
		// once we're into release.
		String orientation = element.getAttribute("orientation") ;
		
		if (orientation == null)
		{
			m_HorizontalOrientation = element.getAttributeBooleanThrows("horiz") ;
		}
		else
		{
			m_HorizontalOrientation = Boolean.valueOf(orientation).booleanValue() ;
		}
		
		for (int i = 0 ; i < element.getNumberChildren() ; i++)
		{
			JavaElementXML child = element.getChild(i) ;
			
			// Build an instance of the view (using its default constructor)
			// This will throw if there is no default constructor.
			AbstractView view = (AbstractView)child.CreateObjectFromXMLDefaultConstructor() ;
						
			view.loadFromXML(frame, doc, this, child) ;
			
			this.addView(view) ;
			
			// Trying to make Linux happy
			view.getWindow().pack(true) ;
			view.getWindow().layout() ;
		}

		// If we specified a particular window to be selected (visible) then
		// move to that one.
		String visibleView = element.getAttribute(kAttributeView) ;
		if (visibleView != null && !this.m_SingleView)
		{
			for (int i = 0 ; i < m_Views.size() ; i++)
			{
				AbstractView view = m_Views.get(i) ;
				if (visibleView.equals(view.getName()))
				{
					((TabFolder)m_Pane).setSelection(i) ;
					break ;
				}
			}
		}
	}

}

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
import general.ElementXML;

import java.util.*;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.events.*;

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
	private Composite			m_Pane ;
	public static final String	kPaneKey = "Pane" ;
	public static final String	kXMLKey  = "XML" ;
	public static final String  kTagName = "pane" ;
	
	/** Some windows can be vertically or horizontally oriented.  Others can just ignore this property */
	protected boolean		m_HorizontalOrientation ;
	
	public Pane(Composite parent)
	{
		// Default to horizontal (for many views orientation is ignored anyway)
		m_HorizontalOrientation = true ;
		
		// We provide a border around the pane so we can find the edges to drag
		m_Pane = new Group(parent, SWT.SHADOW_ETCHED_IN) ;
		m_Pane.setLayout(new FillLayout(SWT.VERTICAL)) ;

		// Mark this composite as a pane window so we know we're
		// at a leaf in the layout and moving over to the actual module's content
		m_Pane.setData(kPaneKey, this) ;
	}
	
	// Provide a way to create a Pane without a parent window
	// This is just so we can have a temporary pane object.
	// I'm including a parent here to make sure folks don't use this by accident.
	public Pane(boolean noParent)
	{
		// Default to horizontal (for many views orientation is ignored anyway)
		m_HorizontalOrientation = true ;
	}
	
	public Composite getWindow()
	{
		return m_Pane ;
	}
	
	/** Some windows can be vertically or horizontally oriented.  Others can just ignore this property */
	public boolean isHorizontalOrientation()			{ return m_HorizontalOrientation ; }
	public void setHorizontalOrientation(boolean state)	{ m_HorizontalOrientation = state ; }

	public void addView(AbstractView view)
	{
		m_Views.add(view) ;
	}
	
	public boolean removeView(AbstractView view)
	{
		return m_Views.remove(view) ;
	}
	
	public boolean setFocus()
	{
		// BUGBUG: When we get to tabbed list should set focus to the visible view
		// For now just take first
		return m_Views.get(0).setFocus() ;
	}
	
	public int getNumberViews()
	{
		return m_Views.size() ;
	}
	
	public AbstractView getView(int index)
	{
		return m_Views.get(index) ;
	}
	
	public ElementXML convertToXML(String tagName)
	{
		ElementXML element = new ElementXML(tagName) ;

		// We'll record a reference from the window the XML
		// This allows us to quickly index into the XML tree at a specific
		// point given a pane or a view.
		if (getWindow() != null)
			getWindow().setData(kXMLKey, element) ;
		
		element.addAttribute("orientation", Boolean.toString(m_HorizontalOrientation)) ;
		
		int n = m_Views.size() ;
		for (int i = 0 ; i < n ; i++)
		{
			AbstractView view = getView(i) ;
			ElementXML child = view.convertToXML("view") ;
			element.addChildElement(child) ;
		}
		
		return element ;
	}

	/** Look up the node in the XML tree that matches this pane.  Need to have called "convertToXML" for everything before calling this */
	public ElementXML getElementXML()
	{
		return (ElementXML)getWindow().getData(kXMLKey) ;
	}
	
	protected void loadFromXML(MainFrame frame, Document doc, Composite parent, ElementXML element) throws Exception
	{
		m_Views.clear() ;
		
		m_HorizontalOrientation = element.getAttributeBooleanThrows("orientation") ;
		
		for (int i = 0 ; i < element.getNumberChildren() ; i++)
		{
			ElementXML child = element.getChild(i) ;
			
			// Build an instance of the view (using its default constructor)
			// This will throw if there is no default constructor.
			AbstractView view = (AbstractView)child.CreateObjectFromXMLDefaultConstructor() ;
						
			view.loadFromXML(frame, doc, this, child) ;
			
			this.addView(view) ;
		}
	}

}

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
	
	public Pane(Composite parent)
	{
		// We provide a border around the pane so we can find the edges to drag
		m_Pane = new Group(parent, SWT.SHADOW_ETCHED_IN) ;
		m_Pane.setLayout(new FillLayout(SWT.VERTICAL)) ;
		
		// Mark this composite as a pane window so we know we're
		// at a leaf in the layout and moving over to the actual module's content
		m_Pane.setData(kPaneKey, this) ;
	}
	
	public Composite getWindow()
	{
		return m_Pane ;
	}
	
	public void addView(AbstractView view)
	{
		m_Views.add(view) ;
		
		if (m_Views.size() != 1)
			throw new IllegalStateException("For now only supporting one view per pane") ;
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
		
		int n = m_Views.size() ;
		for (int i = 0 ; i < n ; i++)
		{
			AbstractView view = getView(i) ;
			ElementXML child = view.convertToXML("view") ;
			element.addChildElement(child) ;
		}
		
		return element ;
	}
	
	protected void loadFromXML(MainFrame frame, Document doc, Composite parent, ElementXML element) throws Exception
	{
		m_Views.clear() ;
		
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

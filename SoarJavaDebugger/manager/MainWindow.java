/********************************************************************************************
*
* MainWindow.java
* 
* Description:	
* 
* Created on 	Feb 16, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package manager;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.events.*;
import java.util.*;

import modules.* ;
import debugger.* ;
import doc.* ;

/************************************************************************
 * 
 * Represents the main content window (one per agent usually).
 *
 * It does not have a menu bar (that's owned by the frame) and usually will
 * contain a series of Panes each of which is attached to a module that
 * handles the display in that pane.
 * 
 * A Pane in turn can be either a single window or a tabbed window depending.
 * 
 ************************************************************************/
public class MainWindow
{
	// We contain the SWT window rather than deriving so it's easier to switch
	// around between frameworks and decouple the code from the particular window system
	// we're using.
	private Composite 	m_Window ;
	private MainFrame	m_MainFrame ;
	private Document	m_Document ;
	
	// The order of this list determines tab order
	private ArrayList	m_PaneList = new ArrayList() ;
	
	public MainWindow(MainFrame frame, Document doc, Composite parent)
	{
		m_MainFrame = frame ;
		m_Document  = doc ;
		
		m_Window = new Composite(parent, 0) ;
		m_Window.setLayout(new FormLayout()) ;
	}

	public Composite getWindow()
	{
		return m_Window ;
	}
	
	public Composite getParent()
	{
		return m_Window.getParent() ;
	}
	
  	public int getWidth()
  	{
  		Point size = m_Window.getSize() ;
  		return size.x ;
  	}
  	
  	public int getHeight()
  	{
  		Point size = m_Window.getSize() ;
  		return size.y ;
  	}
  	
  	public Point getLocationOnScreen()
  	{
  		// BUGBUG - SWT: This is relative to parent not screen.
  		return m_Window.getLocation() ;
  	}

  	public void setTextFont(Font f)
  	{
  		for (int i = 0 ; i < m_PaneList.size() ; i++)
  		{
  			Pane pane = (Pane)m_PaneList.get(i) ;
  			
  			for (int j = 0 ; j < pane.getNumberViews() ; j++)
  			{
  				AbstractView view = pane.getView(j) ;
  				view.setTextFont(f) ;
  			}
  		}
  	}
  	
  	/** Returns the view that currently has focus */
  	public AbstractView getFocusView()
  	{
  		for (int i = 0 ; i < m_PaneList.size() ; i++)
  		{
  			Pane pane = (Pane)m_PaneList.get(i) ;
  			
  			for (int j = 0 ; j < pane.getNumberViews() ; j++)
  			{
  				AbstractView view = pane.getView(j) ;
  				
  				if (view.hasFocus())
  					return view ;
  			}
  		}
  		
  		return null ;
  	}
  	
  	public int getTabPane(Pane focusPane)
  	{
  		for (int i = 0 ; i < m_PaneList.size() ; i++)
  		{
  			if (m_PaneList.get(i) == focusPane)
  				return i ;
  		}
  		
  		return -1 ;
  	}
  	
  	// BUGBUG: Right now the SWT controls implement this already so we need to integrate with that instead
  	// of rolling our own like this.
  	public void tabFocus(boolean forward)
  	{
  		if (m_PaneList.size() == 0)
  			return ;
  		
  		// Get the current focus pane
  		AbstractView focusView = getFocusView() ;
  		
  		int index = -1 ;
  		
  		if (focusView != null)
  			index = getTabPane(focusView.getPane()) ;

  		int tries = 0 ;
  		boolean done = false ;

  		// Move through the panes one at a time until one accepts the focus
  		// or we loop back to the start.
  		while (!done && tries < m_PaneList.size())
  		{
  			tries++ ;
  			
  			if (forward) { index++ ; if (index >= m_PaneList.size()) index = 0 ; }
  			else { index-- ; if (index < 0) index = m_PaneList.size() - 1 ; }	

  			Pane pane = (Pane)m_PaneList.get(index) ;
  			done = pane.setFocus() ;
  		}  		
  	}
  	
  	/********************************************************************************************
  	 * 
  	 * Create the default children that we use in the standard window layout.
  	 * 
  	********************************************************************************************/
  	public void useDefaultLayout()
  	{
  		// Close any existing windows and start fresh
  		if (m_Window != null)
  		{
  			Composite parent = m_Window.getParent() ;
  			
  			// BUGBUG: We need to unregister some events and register them again for this to work.
  			// Right now the panes aren't cleaning up completely correctly.
  			m_Window.dispose() ;
  			m_PaneList.clear() ;
  			
			m_Window = new Composite(parent, 0) ;
			m_Window.setLayout(new FormLayout()) ;
			
			// Note: Must update the parent because we've changed its children here.
			// Without this the new windows don't appear on screen at all.
	       	m_Window.getParent().layout() ;
  		}
  		
  		Pane top  		  = new Pane(m_Window) ;
  		Pane buttonPane   = new Pane(m_Window) ;
  		Pane bottom       = new Pane(m_Window) ;
  		Pane rightTop     = new Pane(m_Window) ;
  		Pane rightBottom  = new Pane(m_Window) ;

  		Sash divider1 = new Sash(m_Window, SWT.HORIZONTAL) ;
  		Sash divider2 = new Sash(m_Window, SWT.VERTICAL) ;
  		Sash divider3 = new Sash(m_Window, SWT.HORIZONTAL) ;

  		m_PaneList.add(top) ;
  		m_PaneList.add(buttonPane) ;
  		m_PaneList.add(bottom) ;
  		m_PaneList.add(rightTop) ;
  		m_PaneList.add(rightBottom) ;
  		
  		// Layout the three windows with a sash between them
    	FormData topData    = new FormData();
    	FormData buttonData = new FormData() ;
    	FormData bottomData = new FormData();
    	FormData rightTopData = new FormData() ;
    	FormData rightBottomData = new FormData() ;
    	
      	topData.left      = new FormAttachment(0);
    	topData.right     = new FormAttachment(divider2);
    	topData.top       = new FormAttachment(0);
      	topData.bottom    = new FormAttachment(buttonPane.getWindow());
      	
      	buttonData.left   = new FormAttachment(0) ;
      	buttonData.right  = new FormAttachment(divider2) ;
      	// If we bind the button's top to the window it makes the top window very small and the buttons very large
      	// buttonData.top    = new FormAttachment(top.getWindow()) ;
      	buttonData.bottom = new FormAttachment(divider1) ;
      	
      	bottomData.left   = new FormAttachment(0);
    	bottomData.right  = new FormAttachment(100);
    	bottomData.top    = new FormAttachment(divider1);
    	bottomData.bottom = new FormAttachment(100);   
    	
    	rightTopData.top  = new FormAttachment(0) ;
    	rightTopData.bottom = new FormAttachment(divider3) ;
    	rightTopData.left = new FormAttachment(divider2) ;
    	rightTopData.right = new FormAttachment(100) ;
    	
    	rightBottomData.top  = new FormAttachment(divider3) ;
    	rightBottomData.bottom = new FormAttachment(100) ;
    	rightBottomData.left = new FormAttachment(divider2) ;
    	rightBottomData.right = new FormAttachment(100) ;
 	
    	top.getWindow().setLayoutData(topData) ;
    	buttonPane.getWindow().setLayoutData(buttonData) ;
    	bottom.getWindow().setLayoutData(bottomData) ;
    	rightTop.getWindow().setLayoutData(rightTopData) ;
    	rightBottom.getWindow().setLayoutData(rightBottomData) ;
    	
		SelectionListener sashListener = new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				onDragSash(e);
			}
		};
		
		divider1.addSelectionListener(sashListener) ;
		divider2.addSelectionListener(sashListener) ;
		divider3.addSelectionListener(sashListener) ;
		
		layoutControls(divider1, 0.8) ;
		layoutControls(divider2, 0.7) ;
		layoutControls(divider3, 0.5) ;
		
		// Now connect up a specific type of view with these panes
		AbstractView trace = new TraceView() ;
		trace.Init(m_MainFrame, m_Document, top) ;

		// Create the button view
		ButtonView buttons = new ButtonView() ;
		buttons.addButton("Init-soar", "init-soar") ;
		buttons.addButton("Excise chunks", "excise --chunks") ;
		buttons.addButton("Run 5", "run 5") ;
		buttons.addButton("Run", "run") ;
		buttons.addButton("Stop", "stop-soar") ;
		buttons.setLinkedView(trace) ;	// Use the trace window for output from the buttons
		buttons.Init(m_MainFrame, m_Document, buttonPane) ;		
		
		// Create another trace window at the bottom for now
		AbstractView keep = new ComboCommandViewKeep() ;
		keep.Init(m_MainFrame, m_Document, bottom) ;
		
		// Start with the focus on the top trace window
		trace.setFocus() ;
		
		AbstractView update1 = new ComboCommandView() ;
		update1.Init(m_MainFrame, m_Document, rightTop) ;

		AbstractView update2 = new ComboCommandView() ;
		update2.Init(m_MainFrame, m_Document, rightBottom) ;

  		// Reset the default text font (once all windows have been created)
		// as part of "the default layout".
  		m_MainFrame.setTextFont(new FontData("Courier New", 8, SWT.NORMAL)) ;
  	}
  	
	public void layoutControls(Sash divider, double position)
	{
		if ((divider.getStyle() & SWT.HORIZONTAL) != 0)
		{
			FormData sashData = new FormData() ;
	 					
			sashData.left 	  = new FormAttachment(0) ;
	       	sashData.right    = new FormAttachment(100) ;
	       	sashData.top	  = new FormAttachment( (int)(position * 100)) ;	
		
	       	divider.setLayoutData(sashData) ;
		}
		else
		{
			FormData sashData = new FormData() ;
	 					
			sashData.top 	  = new FormAttachment(0) ;
	       	sashData.bottom   = new FormAttachment(100) ;
	       	sashData.left	  = new FormAttachment( (int)(position * 100)) ;	
		
	       	divider.setLayoutData(sashData) ;
		}
		
       	// Make the control update.
       	m_Window.layout() ;
	}
	
	void onDragSash(SelectionEvent event)
	{
		Sash sash = (Sash)event.widget ;
		Rectangle area = m_Window.getClientArea() ;
		
		double position = 0.5 ;
		
		// A vertical sash moves horizontally
		if ((sash.getStyle() & SWT.VERTICAL) != 0)
		{
			position = ((double)event.x) / (double)(area.width) ;
		}
		else
		{
			position = ((double)event.y) / (double)(area.height) ;
		}

		// Make sure we're in range
		if (position < 0.0) position = 0.0 ;
		if (position > 1.0) position = 1.0 ;
		
		layoutControls(sash, position) ;
	}
  	
}

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
import org.eclipse.swt.custom.SashForm;
import org.eclipse.swt.events.*;

import sml.Agent;

import helpers.FormDataHelper;

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

  	/** Returns the first view that reports it can be a "prime" view (a trace window usually) **/
  	public AbstractView getPrimeView()
  	{
  		for (int i = 0 ; i < m_PaneList.size() ; i++)
  		{
  			Pane pane = (Pane)m_PaneList.get(i) ;
  			
  			for (int j = 0 ; j < pane.getNumberViews() ; j++)
  			{
  				AbstractView view = pane.getView(j) ;
  				
  				if (view.canBePrimeWindow())
  					return view ;
  			}
  		}
  		
  		return null ;
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
  		Agent currentAgentFocus = null ;
  		
  		// Close any existing windows and start fresh
  		if (m_Window != null)
  		{
  			Composite parent = m_Window.getParent() ;
  			
  			// We need to clear the focus before deleting all of the windows
  			// so they unregister any events for the current agent
  			// Then at the end we'll reset the focus, allowing them to re-register
  			// for events.
  			currentAgentFocus = m_MainFrame.getAgentFocus() ;
  			m_MainFrame.setAgentFocus(null) ;
  			
  			// Now delete everything
  			m_Window.dispose() ;
  			m_PaneList.clear() ;
  			
			m_Window = new Composite(parent, 0) ;
			m_Window.setLayout(new FormLayout()) ;
  		}
  		
  		// Horiz sash has 3 windows has vertSashLeft and vertSashRight as its children
  		SashForm horizSash = new SashForm(m_Window, SWT.HORIZONTAL) ;
  		horizSash.setLayoutData(FormDataHelper.anchorFull(0)) ;

  		// Column of panes on the left
  		SashForm vertSashLeft = new SashForm(horizSash, SWT.VERTICAL) ;
  		vertSashLeft.setLayoutData(FormDataHelper.anchorFull(0)) ;

  		// Column of panes on the right
  		SashForm vertSashRight = new SashForm(horizSash, SWT.VERTICAL) ;
  		vertSashRight.setLayoutData(FormDataHelper.anchorFull(0)) ;

  		// The button bar is a fixed size window so it is linked to the window above
  		// and moves as one with it.  To make this happen we'll create this pair.
  		Composite pair	  = new Composite(vertSashLeft, 0) ;
  		pair.setLayout(new FormLayout()) ;

  		// These panes contain a SWT Window and a module/view that provides specific debugging content
  		Pane top  		  = new Pane(pair) ;
  		Pane buttonPane   = new Pane(pair) ;
  		Pane bottom       = new Pane(vertSashLeft) ;
  		Pane rightTop     = new Pane(vertSashRight) ;
  		Pane rightBottom  = new Pane(vertSashRight) ;

  		// Layout logic for just the top and buttonPane windows
  		FormData topData  = new FormData() ;
  		FormData buttonData = new FormData() ;
  		
      	topData.left      = new FormAttachment(0);
    	topData.right     = new FormAttachment(100);
    	topData.top       = new FormAttachment(0);
      	topData.bottom    = new FormAttachment(buttonPane.getWindow());
      	
      	buttonData.left   = new FormAttachment(0) ;
      	buttonData.right  = new FormAttachment(100) ;
      	// If we bind the button's top to the window it makes the top window very small and the buttons very large
      	// buttonData.top    = new FormAttachment(top.getWindow()) ;
      	buttonData.bottom = new FormAttachment(100) ;
      	
      	top.getWindow().setLayoutData(topData) ;
      	buttonPane.getWindow().setLayoutData(buttonData) ;

  		// Have to set the weights after we have added the panes, so that the size of the weights array
  		// matches the current list of controls
  		vertSashLeft.setWeights(new int[] { 80, 20 }) ;
  		vertSashRight.setWeights(new int[] { 50, 50 }) ;
  		horizSash.setWeights(new int[] { 70, 30 } ) ;
  		
  		// Record the list of panes in use
  		m_PaneList.add(top) ;
  		m_PaneList.add(buttonPane) ;
  		m_PaneList.add(bottom) ;
  		m_PaneList.add(rightTop) ;
  		m_PaneList.add(rightBottom) ;
		
		// Now connect up a specific type of view with these panes
		AbstractView trace = new TraceView() ;
		trace.Init(m_MainFrame, m_Document, top) ;

		// Create the button view
		ButtonView buttons = new ButtonView() ;
		buttons.addButton("Init-soar", "init-soar") ;
		buttons.addButton("Excise all", "excise --all") ;
		buttons.addButton("Excise chunks", "excise --chunks") ;
		buttons.addButton("Run 5", "run 5") ;
		buttons.addButton("Run", "run") ;
		buttons.addButton("Stop", "stop-soar") ;
		buttons.addButton("Towers of Hanoi", null, new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { m_MainFrame.loadDemo(new java.io.File("towers-of-hanoi", "towers-of-hanoi.soar")) ; } }) ;
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
  		
		// Note: Must update the parent because we've changed its children here.
		// Without this the new windows don't appear on screen at all.
       	m_Window.getParent().layout(true) ;
       	
       	// We reset the agent focus (if it existed before).
       	// This allows the new windows to all register for events with this agent.
       	m_MainFrame.setAgentFocus(currentAgentFocus) ;
  	}
  	
	public boolean saveLayoutToFile(String filename)
	{
		return false ;
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

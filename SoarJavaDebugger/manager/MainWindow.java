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
	private Composite m_Window ;
	private MainFrame	m_MainFrame ;
	private Document	m_Document ;
	
	public MainWindow(MainFrame frame, Document doc, Composite parent)
	{
		m_MainFrame = frame ;
		m_Document  = doc ;
		
		m_Window = new Composite(parent, 0) ;
		m_Window.setLayout(new FormLayout()) ;
		
		useDefaultLayout() ;
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
	
  	/********************************************************************************************
  	 * 
  	 * Create the default children that we use in the standard window layout.
  	 * 
  	********************************************************************************************/
  	public void useDefaultLayout()
  	{
  		Pane top = new Pane(m_Window) ;
  		final Sash divider1 = new Sash(m_Window, SWT.HORIZONTAL) ;
  		Pane buttonPane = new Pane(m_Window) ;
  		Pane bottom = new Pane(m_Window) ;
  		
		// Now connect up a specific type of view with these panes
		AbstractView trace = new TraceView() ;
		trace.Init(m_MainFrame, m_Document, top.getWindow()) ;

		// Create the button view
		ButtonView buttons = new ButtonView() ;
		buttons.addButton("Init-soar", "init-soar") ;
		buttons.addButton("Excise chunks", "excise --chunks") ;
		buttons.addButton("Run 5", "run 5") ;
		buttons.addButton("Run", "run") ;
		buttons.addButton("Stop", "stop-soar") ;
		buttons.setLinkedView(trace) ;	// Use the trace window for output from the buttons
		buttons.Init(m_MainFrame, m_Document, buttonPane.getWindow()) ;		
		
		// Create another trace window at the bottom for now
		AbstractView trace2 = new ComboCommandViewKeep() ;
		trace2.Init(m_MainFrame, m_Document, bottom.getWindow()) ;
		
		// Start with the focus on the top trace window
		trace.setFocus() ;
		
  		// Layout the three windows with a sash between them
    	FormData topData    = new FormData();
    	FormData buttonData = new FormData() ;
    	FormData bottomData = new FormData();
    	
      	topData.left      = new FormAttachment(0);
    	topData.right     = new FormAttachment(100);
    	topData.top       = new FormAttachment(0);
      	topData.bottom    = new FormAttachment(buttonPane.getWindow());
      	
      	buttonData.left   = new FormAttachment(0) ;
      	buttonData.right  = new FormAttachment(100) ;
      	// If we bind the button's top to the window it makes the top window very small and the buttons very large
      	// buttonData.top    = new FormAttachment(top.getWindow()) ;
      	buttonData.bottom = new FormAttachment(divider1) ;
      	
      	bottomData.left   = new FormAttachment(0);
    	bottomData.right  = new FormAttachment(100);
    	bottomData.top    = new FormAttachment(divider1);
    	bottomData.bottom = new FormAttachment(100);   
    	
    	top.getWindow().setLayoutData(topData) ;
    	buttonPane.getWindow().setLayoutData(buttonData) ;
    	bottom.getWindow().setLayoutData(bottomData) ;
    	
		Listener sashListener = new Listener() {
			public void handleEvent(Event e) {
				onDragSash(divider1, e);
			}
		};
		
		divider1.addListener(SWT.Selection, sashListener) ;
		
		layoutControls(divider1, 0.5) ;
  	}
  	
	public void layoutControls(Sash divider, double position)
	{
	 	FormData sashData = new FormData();
	 	
		sashData.left 	  = new FormAttachment(0) ;
       	sashData.right    = new FormAttachment(100) ;
       	sashData.top	  = new FormAttachment( (int)(position * 100)) ;	
		
       	divider.setLayoutData(sashData) ;
       	
       	// Make the control update
       	m_Window.layout() ;
	}
	
	void onDragSash(Sash sash, Event event)
	{
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

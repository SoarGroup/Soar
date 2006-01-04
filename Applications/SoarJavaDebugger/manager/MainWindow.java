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

import sml.Agent;

import general.JavaElementXML;
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
	private MainFrame	m_Frame ;
	private Document	m_Document ;
	public final static String kAttachKey 		  = "Attach" ;
	public final static String kAttachBottomValue = "Bottom" ;
	public final static String kAttachTopValue    = "Top" ;
	public final static String kAttachRightValue  = "Right" ;
	public final static String kAttachLeftValue   = "Left" ;
	
	// We'll store information inside the SWT windows to tell us how they fit together
	// in our window hierarchy.  This is mostly to support debugging.
	public final static String kWindowType		  = "WindowType" ;
	public final static String kTypeModule		  = "TypeModule" ;	// The top of a module itself
	public final static String kTypePane		  = "TypePane" ;	// A window that contains modules
	public final static String kTypePair		  = "TypePair" ;	// Contains one fixed size window and a variable size one (creating a single unit)
	public final static String kTypeSash		  = "TypeSash" ;	// Contains two variable sized windows joined with a sash control
	
	// The order of this list determines tab order
	private ArrayList	m_PaneList = new ArrayList() ;
	
	public MainWindow(MainFrame frame, Document doc, Composite parent)
	{
		m_Frame = frame ;
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

  	public int getNumberViews(boolean outputViewsOnly)
  	{
  		int count = 0 ;
  		for (int i = 0 ; i < m_PaneList.size() ; i++)
  		{
  			Pane pane = (Pane)m_PaneList.get(i) ;
  			
  			for (int j = 0 ; j < pane.getNumberViews() ; j++)
  			{
  				AbstractView view = pane.getView(j) ;
  				if (view.canDisplayOutput() || !outputViewsOnly)
  					count++ ;
  			}
  		}
  		return count ;
  	}
  	
  	public AbstractView[] getAllViews(boolean outputViewsOnly)
  	{
  		int counter = 0 ;
  		int count = getNumberViews(outputViewsOnly) ;
  		AbstractView views[] = new AbstractView[count] ;
  		
  		for (int i = 0 ; i < m_PaneList.size() ; i++)
  		{
  			Pane pane = (Pane)m_PaneList.get(i) ;
  			
  			for (int j = 0 ; j < pane.getNumberViews() ; j++)
  			{
  				AbstractView view = pane.getView(j) ;
  				if (view.canDisplayOutput() || !outputViewsOnly)
  					views[counter++] = view ;
  			}
  		}
  		
  		return views ;
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
  	
  	public int getPaneIndex(Pane pane)
  	{
  		for (int i = 0 ; i < m_PaneList.size() ; i++)
  		{
  			if (m_PaneList.get(i) == pane)
  				return i ;
  		}
  		
  		return -1 ;
  	}
  	  	
  	/** Attaches the first and second pane together such that the second pane doesn't resize -- e.g. for button bars */
  	protected void attachTogether(Composite first, Composite second, String attachType)
  	{
  		if (first.getParent() != second.getParent())
  			throw new IllegalStateException("To attach these two together they need to have the same parent") ;
  		
  		// Set the layout to be form layout (top's parent == bottom's parent so could use either)
  		Composite parent = first.getParent() ;
  		parent.setLayout(new FormLayout()) ;
  		
  		// Record how we are attaching these together (later we may offer other methods)
  		parent.setData(kAttachKey, attachType) ;
  		
  		// Layout logic for just the top and buttonPane windows
  		FormData firstData  = new FormData() ;
  		FormData secondData = new FormData() ;
  		
  		if (attachType.equals(kAttachBottomValue) || attachType.equals(kAttachTopValue))
  		{
	      	firstData.left    = new FormAttachment(0);
	    	firstData.right   = new FormAttachment(100);
	      	secondData.left   = new FormAttachment(0) ;
	      	secondData.right  = new FormAttachment(100) ;

	      	if (attachType.equals(kAttachBottomValue))
	    	{
		    	firstData.top       = new FormAttachment(0);
		      	firstData.bottom    = new FormAttachment(second);

		      	// If we bind the button's top to the window it makes the top window very small and the buttons very large
		      	//secondData.top    = new FormAttachment(first) ;
		      	secondData.bottom = new FormAttachment(100) ;
	    	}
	    	else
	    	{
		    	firstData.top       = new FormAttachment(second);
		      	firstData.bottom    = new FormAttachment(100);
		      	secondData.top		= new FormAttachment(0) ;
	    	}
  		}
  		else
  		{
	      	firstData.top     = new FormAttachment(0);
	    	firstData.bottom  = new FormAttachment(100);
	      	secondData.top    = new FormAttachment(0) ;
	      	secondData.bottom = new FormAttachment(100) ;

	      	if (attachType.equals(kAttachRightValue))
	    	{
		    	firstData.left   = new FormAttachment(0);
		      	firstData.right  = new FormAttachment(second);
		      	secondData.right = new FormAttachment(100) ;
	    	}
	    	else
	    	{
		    	firstData.left   = new FormAttachment(second);
		      	firstData.right  = new FormAttachment(100);
		      	secondData.left	 = new FormAttachment(0) ;
	    	}
  		}
  		      	
      	first.setLayoutData(firstData) ;
      	second.setLayoutData(secondData) ;
   	}
  	
	/** Close any existing windows and start fresh **/
  	protected void clearLayout()
  	{
  		if (m_Window == null)
  			return ;
  		
		Composite parent = m_Window.getParent() ;
		
		// We need to clear the focus before deleting all of the windows
		// so they unregister any events for the current agent
		// Then at the end we'll reset the focus, allowing them to re-register
		// for events.
		m_Frame.clearAgentFocus(true) ;
		
		// Unregister all names that were in use with this frame
		m_Frame.clearNameRegistry() ;
		
		// Now delete everything
		m_Window.dispose() ;
		m_PaneList.clear() ;
		
		m_Window = new Composite(parent, 0) ;
		m_Window.setLayout(new FormLayout()) ;
  	}
  	
  	/** Create the windows for the default layout.
  	 *  We're keeping this as a fallback, but now load the default layout from a file usually. */
  	protected void createInternalDefaultLayout()
  	{
  		if (m_Window.getChildren().length != 0)
  			throw new IllegalStateException("Need to start with a blank base window") ;
  		
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
  		// These panes contain a SWT Window and a module/view that provides specific debugging content
  		Composite pair	  		= new Composite(vertSashLeft, 0) ;
  		Pane top  		  	 	= new Pane(pair, true, true) ;
  		Pane buttonPane   	 	= new Pane(pair, true, true) ;
  		Pane bottom       	 	= new Pane(vertSashLeft, true, true) ;
  		Pane rightTop  	 		= new Pane(vertSashRight, true, true) ;
  		Composite pair2 		= new Composite(vertSashRight, 0) ;
  		Pane rightBottom     	= new Pane(pair2, true, true) ;
  		Pane buttonsRightBottom = new Pane(pair2, true, true) ;

  		// Attach the button panes to a window as a single, resizable unit
  		attachTogether(top.getWindow(), buttonPane.getWindow(), kAttachLeftValue) ;
  		buttonPane.setHorizontalOrientation(false) ;
  		attachTogether(rightBottom.getWindow(), buttonsRightBottom.getWindow(), kAttachTopValue) ;
      	
  		// Have to set the weights after we have added the panes, so that the size of the weights array
  		// matches the current list of controls
  		vertSashLeft.setWeights(new int[] { 80, 20 }) ;
  		vertSashRight.setWeights(new int[] { 65, 35 }) ;
  		horizSash.setWeights(new int[] { 60, 40 } ) ;
  		
  		// Record the list of panes in use
  		m_PaneList.add(top) ;
  		m_PaneList.add(buttonPane) ;
  		m_PaneList.add(bottom) ;
  		m_PaneList.add(rightTop) ;
  		m_PaneList.add(rightBottom) ;
  		m_PaneList.add(buttonsRightBottom) ;
		
		// Now connect up a specific type of view with these panes
		AbstractView trace = new FoldingTextView() ;
		trace.init(m_Frame, m_Document, top) ;
		trace.generateName(m_Frame) ;
		top.addView(trace) ;

		// Create the button view
		ButtonView buttons = new ButtonView() ;
		buttons.addDefaultCommands() ;	// We have a default set of commands to show on a button bar
		buttons.init(m_Frame, m_Document, buttonPane) ;
		buttons.generateName(m_Frame) ;
		buttonPane.addView(buttons) ;

		// Create another trace window at the bottom for now
		AbstractView keep = new KeepCommandView() ;
		keep.init(m_Frame, m_Document, bottom) ;
		keep.generateName(m_Frame) ;
		bottom.addView(keep) ;
		
		// Start with the focus on the top trace window
		trace.setFocus() ;

		// Command view for top right
		UpdateCommandView update2 = new UpdateCommandView() ;
		update2.init(m_Frame, m_Document, rightTop) ;
		update2.setInitialCommand("print <s>") ;
		update2.generateName(m_Frame) ;
		rightTop.addView(update2) ;
		
		// Command view for bottom right
		AbstractView update1 = new KeepCommandView() ;
		update1.init(m_Frame, m_Document, rightBottom) ;
		update1.generateName(m_Frame) ;
		rightBottom.addView(update1) ;

		// Button bar for right bottom
		buttons = new ButtonView() ;
		buttons.addButton("Matches", "matches") ;
		buttons.addButton("Print state", "print <s>") ;
		buttons.addButton("Print op", "print <o>") ;
		buttons.addButton("Print stack", "print --stack") ;
		buttons.init(m_Frame, m_Document, buttonsRightBottom) ;
		buttons.setLinkedView(update1.getName()) ;
		buttons.generateName(m_Frame) ;
		buttonsRightBottom.addView(buttons) ;

  		// Reset the default text font (once all windows have been created)
		// as part of "the default layout".
  		m_Frame.setTextFont(MainFrame.kDefaultFontData) ;  		
  	}
  	
  	/********************************************************************************************
  	 * 
  	 * Create the default children that we use in the standard window layout.
  	 * 
  	********************************************************************************************/
  	public void useDefaultLayout()
  	{
  		// We load the default layout as a file now.
  		boolean ok = m_Frame.loadLayoutFile("default-layout.dlf", false) ;

  		// If for some reason this fails (really shouldn't happen)
  		// then we'll use an internal layout as a fall back.
  		if (!ok)
  		{
	  		Agent currentAgentFocus = m_Frame.getAgentFocus() ;
	  		
	  		// Dispose of all existing windows
	  		clearLayout() ;
	  		
	  		// Build the new windows
	  		createInternalDefaultLayout() ;
	  		
			// Note: Must update the parent because we've changed its children here.
			// Without this the new windows don't appear on screen at all.
	       	m_Window.getParent().layout(true) ;
	       	
	       	// We reset the agent focus (if it existed before).
	       	// This allows the new windows to all register for events with this agent.
	       	m_Frame.setAgentFocus(currentAgentFocus) ;
  		}
  	}

  	public void loadFromXMLNoThrow(JavaElementXML root)
  	{
  		try
		{
			loadFromXML(root) ;
		} catch (Exception e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
  	}
  	
  	public void loadFromXML(JavaElementXML root) throws Exception
	{
		// Find the version of the layout file.
		String version = root.getAttributeThrows(JavaElementXML.kVersionAttribute) ;

		if (!version.equals("1.0"))
			throw new Exception("Layout file is from an unsupported version " + version) ;
		
		// Pick up the current focus so we can restore it at the end
  		Agent currentAgentFocus = m_Frame.getAgentFocus() ;

		// Get rid of any existing windows and create a new blank window
		clearLayout() ;
		
		// Load all of the windows
		loadChildrenFromXML(m_Frame, m_Document, m_Window, root) ;
  		
		// Note: Must update the parent because we've changed its children here.
		// Without this the new windows don't appear on screen at all.
       	m_Window.getParent().layout(true) ;
       	
       	// We reset the agent focus (if it existed before).
       	// This allows the new windows to all register for events with this agent.
       	m_Frame.setAgentFocus(currentAgentFocus) ;  	
       	
		// Apply the current font to the newly loaded windows.
		// (The font is considered a user's preference currently not part of the layout)
		setTextFont(m_Frame.getTextFont()) ;
	}
  	
	/************************************************************************
	* 
	* Loads the window layout from an XML file.
	* 
	*************************************************************************/
  	public boolean loadLayoutFromFile(String filename, boolean showErrors)
  	{
		try
		{
			JavaElementXML root = JavaElementXML.ReadFromFile(filename);
			
			loadFromXML(root) ;
			
			return true ;
		}
		catch (Exception e)
		{
			String msg = e.toString() ;

			// Report any errors to the user.
			if (showErrors)
				m_Frame.ShowMessageBox("Error loading file: " + filename, msg) ;	
			else
				System.out.println("Error loading file: " + filename + " " + msg) ;

			return false ;
		}  		
  	}
  	
	public boolean saveLayoutToFile(String filename)
	{
		// Convert to XML but without storing all of the current text in the trace etc.
		JavaElementXML root = convertToXML(false) ;

		try
		{
			// Write the tree out as a file.
			root.WriteToFile(filename);
			
			return true ;
		}
		catch (java.io.IOException e)
		{
			String msg = e.getMessage();

			// Report any errors to the user.
			m_Frame.ShowMessageBox("Error saving file: " + filename, msg) ;			
			return false ;
		}
	}
  		
	/** Convert a sash form (without its children) to XML */
	public JavaElementXML buildXMLForSashForm(int sashOrientation, int[] weights)
	{
		String tagName = "sash" ;
		JavaElementXML element = new JavaElementXML(tagName) ;
		
		// Record the class and style of this composite so we can rebuild it later
		element.addAttribute(JavaElementXML.kClassAttribute, SashForm.class.toString()) ;
		
		// The sash form stores its orientation (the style we really need) separately
		// from the control's style, so retreive it from orientation.
		element.addAttribute("style", Integer.toString(sashOrientation)) ;

		// Record the weights between the windows
		element.addAttribute("weights", Integer.toString(weights.length)) ;
		for (int i = 0 ; i < weights.length ; i++)
		{
			element.addAttribute("weight" + i, Integer.toString(weights[i])) ;
		}
		
		return element ;
	}
	
	/** Convert a sash form (and its children) to XML */
	protected JavaElementXML convertSashFormToXML(SashForm sash, boolean storeContent)
	{
		int style = sash.getOrientation() ;
		int[] weights = sash.getWeights() ;

		JavaElementXML element = buildXMLForSashForm(style, weights) ;
		
		// Add the children
		addChildrenToXML(element, sash, storeContent) ;
		
		return element ;
	}

	/** Add all of the children of this composite as children of the XML element **/
	protected void addChildrenToXML(JavaElementXML element, Composite composite, boolean storeContent)
	{
		Control[] controls = composite.getChildren() ;
		
		for (int i = 0 ; i < controls.length ; i++)
		{
			// Look up the logical type of window this is (we store this when the window is created).
			// This helps with debugging.
			// String type = (String)controls[i].getData(kWindowType) ;
			// System.out.println("Saving " + type) ;
			
			// When we reach a pane we're at a leaf in the layout logic and need
			// to switch to storing the pane information.
			Pane pane = (Pane)controls[i].getData(Pane.kPaneKey) ;

			if (pane != null)
			{
				element.addChildElement(pane.convertToXML(Pane.kTagName, storeContent)) ;				
				continue ;
			}
			
			// Children must be composite windows (either simple composites
			// or sash forms).  Anything else we ignore (like an actual Sash control).
			if (controls[i] instanceof Composite)
			{
				Composite comp = (Composite)controls[i] ;

				if (comp instanceof SashForm)
				{
					element.addChildElement(convertSashFormToXML((SashForm)comp, storeContent)) ;
				}
				else
				{
					element.addChildElement(convertCompositeToXML(comp, storeContent)) ;					
				}				
			}
		}
	}
	
	/** Constructs the XML for the composite window -- but not for its children */
	public JavaElementXML buildXMLforComposite(String attachType)
	{
		String tagName = "composite" ;
		JavaElementXML element = new JavaElementXML(tagName) ;
		
		// Record the class and style of this composite so we can rebuild it later
		element.addAttribute(JavaElementXML.kClassAttribute, Composite.class.toString()) ;

		//int style = composite.getStyle() ;
		int style = 0 ;	// Override this for now until I figure out why I get back such a different value
		element.addAttribute("style", Integer.toString(style)) ;
		
		// All composites beneath the top currently have to have this key
		// so we'll know how they were attached together and can rebuild it later
		if (attachType == null)
			throw new IllegalStateException("Composite window missing its required attach key.  When the layout was built did we connect up the windows without calling one of our attachX methods?") ;
		element.addAttribute("attach", attachType) ;
		
		return element ;
	}
	
	/** Convert a simple composite to XML **/
	protected JavaElementXML convertCompositeToXML(Composite composite, boolean storeContent)
	{
		// See how this composite's children are meant to be put together.
		String attachType = (String)composite.getData(kAttachKey) ;

		JavaElementXML element = buildXMLforComposite(attachType) ;
		
		addChildrenToXML(element, composite, storeContent) ;
		
		return element ;
	}
	
	// Convert the layout to XML, optionally recording content (e.g. text in a trace window).
	// If in doubt, you usually do NOT want the content--it could add enormously to the size of the XML.
	public JavaElementXML convertToXML(boolean storeContent)
	{
		JavaElementXML root = new JavaElementXML("Debugger") ;

		// Add a version to the layout file so we can have later debuggers handle earlier layout files
		// in a special manner if we wish.
		root.addAttribute(JavaElementXML.kVersionAttribute, "1.0") ;

		// The display consists of a list of panes (the modules that provide functionality) together
		// with a collection of SashForms and Composite windows that are used to lay them out.
		// We navigate down through the windows until we reach the panes, recording information as we go.
		// Once we reach the pane we switch to logic that's module specific about how to save that pane.
		addChildrenToXML(root, m_Window, storeContent) ;
		
		return root ;
	}
	
	protected void loadChildrenFromXML(MainFrame frame, Document doc, Composite parent, JavaElementXML element) throws Exception
	{
		for (int i = 0 ; i < element.getNumberChildren() ; i++)
		{
			JavaElementXML child = element.getChild(i) ;

			String tagName = child.getTagName() ;
			
			if (tagName.equalsIgnoreCase("sash") || tagName.equalsIgnoreCase("composite"))
				loadSwtFromXML(frame, doc, parent, child) ;
			else if (tagName.equalsIgnoreCase(Pane.kTagName))
				loadPaneFromXML(frame, doc, parent, child) ;
			else
				// Throw during development, but really we should just ignore this
				// when reading XML (in case we add tags later and load this into an earlier version)
				throw new Exception("Unknown tag " + tagName) ;
		}		
	}
	
	protected void loadPaneFromXML(MainFrame frame, Document doc, Composite parent, JavaElementXML element) throws Exception
	{
  		Pane pane = Pane.loadFromXML(frame, doc, parent, element) ;	
  		m_PaneList.add(pane) ;
	}
	
	protected void loadSwtFromXML(MainFrame frame, Document doc, Composite parent, JavaElementXML element) throws Exception
	{
		String tag = element.getTagName() ;
		//String className = element.getAttributeThrows(JavaElementXML.kClassAttribute) ;
		int style = element.getAttributeIntThrows("style") ;

		// We could rebuild a generic SWT object from the class using reflection, but it's actually simpler (so far) to just
		// handle the few cases we support explicitly
		if (tag.equalsIgnoreCase("sash"))
		{
			// Create the new form
			SashForm sash = new SashForm(parent, style) ;
	  		sash.setLayoutData(FormDataHelper.anchorFull(0)) ;
			sash.setData(MainWindow.kWindowType, MainWindow.kTypeSash) ;
			
			// Have to create all of the children *before* we can set the weights
			// (this is an oddity of SashForm)
			loadChildrenFromXML(frame, doc, sash, element) ;
			
			int numberWeights = element.getAttributeIntThrows("weights") ;
			int[] weights = new int[numberWeights] ;
			
			for (int i = 0 ; i < numberWeights ; i++)
			{
				weights[i] = element.getAttributeIntThrows("weight" + i) ;
			}
			
			// If there's an error connecting the weights together,
			// fall back on even spacing as a default
			if (weights.length != element.getNumberChildren())
			{
				System.out.println("Error: Number of weights didn't match number of children, so resetting them all to equal weight.  (Acceptable if removing windows, not if loading layouts).") ;
				weights = new int[element.getNumberChildren()] ;
				
				for (int w = 0 ; w < weights.length ; w++)
				{
					weights[w] = 100 / (weights.length) ;
				}
			}
			sash.setWeights(weights) ;				
		}
		else if (tag.equalsIgnoreCase("composite"))
		{
			Composite composite = new Composite(parent, style) ;
			composite.setData(MainWindow.kWindowType, MainWindow.kTypePair) ;

			// See how this composite's children are meant to be put together.
			String attachType = element.getAttributeThrows("attach") ;
			
			// Create the children so we have something to attach together below
			loadChildrenFromXML(frame, doc, composite, element) ;
			
			Control[] children = composite.getChildren() ;
			
			if (children.length != 2)
				throw new Exception("Trying to attach two windows together but found " + children.length + " instead") ;
			
			// The children must be composites -- either they are panes or other composites forming a tree of these pairs
			Composite top    = (Composite)children[0] ;
			Composite bottom = (Composite)children[1] ;
			
			// Finally now make the attachment
			attachTogether(top, bottom, attachType) ;
		}		
	}
	
}

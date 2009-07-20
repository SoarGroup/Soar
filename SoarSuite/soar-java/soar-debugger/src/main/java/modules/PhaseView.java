/********************************************************************************************
*
* PhaseView.java
* 
* Description:	
* 
* Created on 	Sep 25, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import java.io.InputStream;

import general.JavaElementXML;
import manager.Pane;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import sml.Agent;
import sml.Kernel;
import sml.smlAgentEventId;
import sml.smlPhase;
import sml.smlSystemEventId;
import sml.sml_Names;
import debugger.MainFrame;
import dialogs.PropertiesDialog;
import doc.Document;

/************************************************************************
 * 
 * Displays the phases of Soar's kernel as it is executing.
 * Also allows the user to select where Soar will stop when running by decision.
 * 
 ************************************************************************/
public class PhaseView extends AbstractFixedView implements Kernel.AgentEventInterface, Kernel.SystemEventInterface
{
	protected Canvas	m_PhaseDiagram ;
	protected String[]	m_ImageNames = { "phase-none.png",
			"phase-input.png", "phase-proposal.png", "phase-decision.png", "phase-apply.png", "phase-output.png", "phase-world.png",
			"phase-stop-cursor.png", "phase-stop-cursor2.png", "phase-stop-cursor-shadow.png", "phase-stop-cursor-shadow2.png", "phase-cursor.png" } ;
	
	protected Image[]	m_PhaseImages ;
	protected Image		m_StopCursor ;
	protected Image		m_StopCursorShadow ;
	protected Image		m_PhaseCursor ;
	
	protected int		m_StopCallback = -1 ;
	protected int		m_StartCallback = -1 ;
	protected int		m_InitCallback = -1 ;
	protected int		m_PropertyCallback = -1 ;
	protected int		m_DecisionCycle = 0 ;
	
	protected boolean	m_DrawPhase = true ;
	protected boolean	m_DrawShadow = false ;
	
	protected boolean	m_ShowMarkerBetweenPhases = true ;
	protected boolean	m_HourglassCursors = true ;
	
	// The phase which will execute when we next run Soar
	protected smlPhase	m_NextExecutionPhase = smlPhase.sml_INPUT_PHASE ;
	
	// Where agent will stop if run by decision.
	protected smlPhase	m_StopBeforePhase = smlPhase.sml_INPUT_PHASE ;
	
	protected smlPhase	m_StopBeforeShadow = smlPhase.sml_INPUT_PHASE ;
		
	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public String getModuleBaseName()
	{
		return "phaseview" ;
	}

	/********************************************************************************************
	* 
	* Initialize this window and its children.
	* Should call setValues() at the start to complete initialization of the abstract view.
	* 
	********************************************************************************************/
	public void init(MainFrame frame, Document doc, Pane parentPane)
	{
		setValues(frame, doc, parentPane) ;
		
		m_PhaseImages = new Image[m_ImageNames.length] ;
		
		for (int i = 0 ; i < m_ImageNames.length ; i++)
		{
			try
			{
				// Load the images from the input subfolder (either from disk relative to the app or from the JAR)
				String jarpath = "/images/" + m_ImageNames[i] ;
				InputStream is = this.getClass().getResourceAsStream(jarpath) ;
				
				m_PhaseImages[i] = new Image(frame.getDisplay(), is) ;
				
				is.close() ;			
			}
			catch (Exception ex)
			{
				System.out.println("Error reading image " + m_ImageNames[i]) ;
				System.out.println(ex) ;
			}
		}
		
		selectCursorImages() ;
				
		createPanel(m_Pane.getWindow()) ;
	}

	protected void selectCursorImages()
	{
		if (m_HourglassCursors)
		{
			m_StopCursor 		= m_PhaseImages[m_ImageNames.length-5] ;
			m_StopCursorShadow 	= m_PhaseImages[m_ImageNames.length-3] ;
			m_PhaseCursor 		= m_PhaseImages[m_ImageNames.length-1] ;
		}
		else
		{
			m_StopCursor 		= m_PhaseImages[m_ImageNames.length-4] ;
			m_StopCursorShadow 	= m_PhaseImages[m_ImageNames.length-2] ;
			m_PhaseCursor 		= m_PhaseImages[m_ImageNames.length-1] ;			
		}		
	}
	
	protected void createPanel(final Composite parent)
	{
		// The container lets us control the layout of the controls
		// within this window
		m_Container	   = new Composite(parent, SWT.NULL) ;
		m_Container.setLayout(null) ;	// We'll do it manually
		
		Canvas canvas = new Canvas(m_Container, SWT.NO_BACKGROUND | SWT.DOUBLE_BUFFERED) ;
		int w = m_PhaseImages[0].getImageData().width ;
		int h = m_PhaseImages[0].getImageData().height ;
		canvas.setSize(w, h + 8) ;	// BADBAD: Presumably this +4 is a margin from somewhere
		m_PhaseDiagram = canvas ;
		
		canvas.addListener (SWT.Paint, new Listener () {
			public void handleEvent (Event e) {
				paint(e.gc) ;
			}
		});

		canvas.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) { mouseClicked(e) ; }
		} ) ;

		canvas.addMouseMoveListener(new MouseMoveListener() {
			public void mouseMove(MouseEvent e) { mouseMoved(e) ; }
		}) ;

		canvas.addMouseTrackListener(new MouseTrackListener() {
			public void mouseEnter(MouseEvent e) { m_DrawShadow = true ; m_PhaseDiagram.redraw() ; }
			public void mouseExit(MouseEvent e)  { m_DrawShadow = false ; m_PhaseDiagram.redraw() ; }
			public void mouseHover(MouseEvent e) { } 
		}) ;

		// Create a context menu for the container and the diagram
		// It will be filled in via a call to fillInContextMenu when the menu is popped up
		// (this allows for dynamic content)
		createContextMenu(m_Container) ;
		createContextMenu(m_PhaseDiagram) ;
		
		// Layout the parent again, because this set of windows has changed
		// This only matters if we're recreating the windows
		parent.layout(true) ;
		
		// Get the current stop before phase and repaint the control
		updateNow(true) ;
	}
	
	protected smlPhase getPhaseFromPosition(int x, int y)
	{
		// Anyway over the "world" maps to stop before input phase
		if (y > 36 || x <= 24)
			return smlPhase.sml_INPUT_PHASE ;

		int phase = ((x-24) / 36) + 1 ;
		
		if (phase > smlPhase.sml_OUTPUT_PHASE.swigValue())
			phase = smlPhase.sml_INPUT_PHASE.swigValue() ;
		
		return smlPhase.swigToEnum(phase) ;
	}
	
	protected void mouseClicked(MouseEvent e)
	{
		// Only if left clicking
		if (e.button != 1)
			return ;
		
		m_StopBeforePhase = getPhaseFromPosition(e.x, e.y) ;
		m_PhaseDiagram.redraw() ;

		// Set the new stop phase
		String command = m_Document.getSoarCommands().getStopBeforeCommand(m_StopBeforePhase) ;
		m_Frame.executeCommandPrimeView(command, true) ;
	}
	
	protected void mouseMoved(MouseEvent e)
	{
		m_StopBeforeShadow = getPhaseFromPosition(e.x, e.y) ;
		m_PhaseDiagram.redraw() ;
	}
	
	protected void paint(GC gc)
	{
		if (m_DrawPhase)
			paintPhaseDiagram(gc, m_NextExecutionPhase.swigValue() + 1) ;
		else
			paintPhaseDiagram(gc, 0) ;
	}

	protected Point getCursorPosition(smlPhase phase)
	{
		// Position for center of stop before marker
		int x = 29 ;
		int y = 23 ;
				
		if (phase == smlPhase.sml_INPUT_PHASE)
			{ x = 31 ; y = 50 ;  }
		if (phase == smlPhase.sml_PROPOSAL_PHASE)
			{ x += 36 * 0 ; }
		if (phase == smlPhase.sml_DECISION_PHASE)
			{ x += 36 * 1 ; }
		if (phase == smlPhase.sml_APPLY_PHASE)
			{ x += 36 * 2 ; }
		if (phase == smlPhase.sml_OUTPUT_PHASE)
			{ x += 36 * 3 ; }

		return new Point(x, y) ;
	}
	
	protected void paintPhaseDiagram(GC gc, int imageIndex)
	{
		Canvas canvas = m_PhaseDiagram ;
		
		if (m_ShowMarkerBetweenPhases)
			imageIndex = 0 ;
		
		gc.drawImage(m_PhaseImages[imageIndex], 0, 0) ;
		
		// The cursors extend a touch below the images so we need to clean up the overlapping area
		gc.setBackground(canvas.getDisplay().getSystemColor(SWT.COLOR_WHITE)) ;
		gc.fillRectangle(0, m_PhaseImages[imageIndex].getImageData().height, canvas.getClientArea().width, 5) ;
		
		Point stop   = getCursorPosition(this.m_StopBeforePhase) ;
		Point shadow = getCursorPosition(this.m_StopBeforeShadow) ;
		Point phase  = getCursorPosition(this.m_NextExecutionPhase) ;

		// If we've set the location of the stop cursor draw that in
		// Draw the cursor centered on x,y
		int x = stop.x - m_StopCursor.getImageData().width/2 ;
		int y = stop.y - m_StopCursor.getImageData().height/2 ;
		gc.drawImage(m_StopCursor, x, y) ;
		
		if (m_ShowMarkerBetweenPhases)
		{
			// Draw the phase cursor centered on x,y
			x = phase.x - m_PhaseCursor.getImageData().width/2 ;
			y = phase.y - m_PhaseCursor.getImageData().height/2 ;
			gc.drawImage(m_PhaseCursor, x, y) ;			
		}

		if (m_DrawShadow)
		{
			// Draw the shadow cursor centered on x,y
			x = shadow.x - m_StopCursorShadow.getImageData().width/2 ;
			y = shadow.y - m_StopCursorShadow.getImageData().height/2 ;
			gc.drawImage(m_StopCursorShadow, x, y) ;

			if (m_StopBeforePhase == m_StopBeforeShadow)
			{
				x = stop.x - m_StopCursor.getImageData().width/2 ;
				y = stop.y - m_StopCursor.getImageData().height/2 ;
				gc.drawImage(m_StopCursor, x, y) ;				
			}
		}

		// Draw the decision cycle counter in the corner
		String decisions = Integer.toString(m_DecisionCycle) ;
		gc.drawText(decisions, 125, 41) ;
	}
	
	public void agentEventHandler(int eventID, Object data, String agentName)
	{
		if (eventID == smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED.swigValue() &&
			this.getAgentFocus() != null && agentName.equals(this.getAgentFocus().GetAgentName()))
		{
			m_DrawPhase = true ;
			updateNow(true) ;
		}
	}
	
	public void systemEventHandler(int eventID, Object data, Kernel kernel)
	{
		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue())
		{
			m_DrawPhase = true ;
			updateNow(false) ;
		}
		
		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue())
		{
			// Don't show the phase while we're running
			m_DrawPhase = false ;
			updateNow(false) ;
		}
		
		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_PROPERTY_CHANGED.swigValue())
		{
			updateNow(true) ;
		}
	}	
	
	public smlPhase getStopBeforePhase()
	{
		String result = m_Frame.getCommandResult(m_Document.getSoarCommands().getGetStopBeforeCommand(), sml_Names.getKParamPhase()) ;
		
		if (result == null || result == "")
			return smlPhase.sml_INPUT_PHASE ;

		int value = Integer.parseInt(result) ;
		return smlPhase.swigToEnum(value) ;
	}
	
	public void agentGettingFocus(doc.events.AgentFocusEvent ev)
	{
		super.agentGettingFocus(ev) ;

		// When an agent is assigned to this window, update the control to show the
		// current phase, decision cycle count and stop point.
		updateNow(true) ;
	}
	
	public void updateNow(final boolean getCurrentStopPhase)
	{
		Agent agent = this.getAgentFocus() ;
		if (agent == null)
			return ;
		
		if (this.m_Container.isDisposed())
			return ;

		m_NextExecutionPhase = agent.GetCurrentPhase() ;
		m_DecisionCycle      = agent.GetDecisionCycleCounter() ;
		
		// Have to make update in the UI thread.
		// Callback comes in the document thread.
		m_PhaseDiagram.getDisplay().asyncExec(new Runnable() {
			public void run() {
				if (getCurrentStopPhase)
					m_StopBeforePhase = getStopBeforePhase() ;
				
				m_PhaseDiagram.redraw() ;
			}}) ;
	}
		
	/************************************************************************
	* 
	* Register and unregister for Soar events for this agent.
	* (E.g. a trace window might register for the print event)
	* 
	*************************************************************************/
	protected void registerForAgentEvents(Agent agent)
	{
		if (m_StopCallback == -1)
		{
			// Update on start, stop and on init-soar.  Also listen for any time the user types "set-stop" somewhere else.
			m_StartCallback	= agent.GetKernel().RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, this, this) ;
			m_StopCallback	= agent.GetKernel().RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, this, this) ;
			m_PropertyCallback  = agent.GetKernel().RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_PROPERTY_CHANGED, this, this) ;
			m_InitCallback  = agent.GetKernel().RegisterForAgentEvent(smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED, this, this) ;
		}
	}

	protected void unregisterForAgentEvents(Agent agent)
	{
		boolean ok = true ;
		
		if (m_StartCallback != -1)
			ok = agent.GetKernel().UnregisterForSystemEvent(m_StartCallback) && ok ;
		if (m_StopCallback != -1)
			ok = agent.GetKernel().UnregisterForSystemEvent(m_StopCallback) && ok ;
		if (m_InitCallback != -1)
			ok = agent.GetKernel().UnregisterForAgentEvent(m_InitCallback) && ok ;
		if (m_PropertyCallback != -1)
			ok = agent.GetKernel().UnregisterForSystemEvent(m_PropertyCallback) && ok ;

		if (!ok)
			throw new IllegalStateException("Error unregistering callbacks in phase view") ;

		m_StopCallback = -1 ;
		m_StartCallback = -1 ;
		m_InitCallback = -1 ;
		m_PropertyCallback = -1 ;
	}

	/************************************************************************
	* 
	* ClearAgentEvents is called when the agent has already been deleted
	* (so we can't unregister but should just clear our references)
	* 
	*************************************************************************/
	protected void clearAgentEvents()
	{
		m_StartCallback = -1 ;
		m_StopCallback = -1 ;
		m_InitCallback = -1 ;
		m_PropertyCallback = -1 ;
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
		properties[0] = new PropertiesDialog.BooleanProperty("Show phase marker between phases (not within a phase)", m_ShowMarkerBetweenPhases) ;
		properties[1] = new PropertiesDialog.BooleanProperty("Use hourglass shaped cursor set", m_HourglassCursors) ;

		boolean ok = PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;

		if (ok)
		{
			m_ShowMarkerBetweenPhases = ((PropertiesDialog.BooleanProperty)properties[0]).getValue() ;
			m_HourglassCursors = ((PropertiesDialog.BooleanProperty)properties[1]).getValue() ;

			// Update the cursor set and redraw the display to match the users choices.
			selectCursorImages() ;
			this.m_PhaseDiagram.redraw() ;
		}
	}

	/************************************************************************
	* 
	* Converts this object into an XML representation.
	* 
	* For the button view there is no content beyond the list of buttons.
	* 
	*************************************************************************/
	public JavaElementXML convertToXML(String tagName, boolean storeContent)
	{
		JavaElementXML element = new JavaElementXML(tagName) ;
		
		// It's useful to record the class name to uniquely identify the type
		// of object being stored at this point in the XML tree.
		Class cl = this.getClass() ;
		element.addAttribute(JavaElementXML.kClassAttribute, cl.getName()) ;

		// Store this object's properties.
		element.addAttribute("Name", m_Name) ;
		element.addAttribute("BetweenPhases", Boolean.toString(m_ShowMarkerBetweenPhases)) ;
		element.addAttribute("HourglassCursors", Boolean.toString(m_HourglassCursors)) ;
		
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
		setValues(frame, doc, parent) ;

		m_Name   					= element.getAttributeThrows("Name") ;
		m_ShowMarkerBetweenPhases 	= element.getAttributeBooleanDefault("BetweenPhases", true) ;
		m_HourglassCursors 			= element.getAttributeBooleanDefault("HourglassCursors", true) ;
		
		// Register that this module's name is in use
		frame.registerViewName(m_Name, this) ;

		// Actually create the window
		init(frame, doc, parent) ;
	}

}

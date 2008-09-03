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

import java.io.IOException;
import java.io.InputStream;

import general.ElementXML;
import manager.Pane;
import modules.ButtonView.ButtonInfo;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.*;

import sml.Agent;
import sml.Kernel;
import sml.smlAgentEventId;
import sml.smlPhase;
import sml.smlRunEventId;
import sml.smlSystemEventId;
import debugger.MainFrame;
import doc.Document;

/************************************************************************
 * 
 * Displays the phases of Soar's kernel as it is executing.
 * Also allows the user to select where Soar will stop when running by decision.
 * 
 ************************************************************************/
public class PhaseView extends AbstractFixedView
{
	protected Canvas	m_PhaseDiagram ;
	protected String[]	m_ImageNames = { "phase-none.png",
			"phase-input.png", "phase-proposal.png", "phase-decision.png", "phase-apply.png", "phase-output.png", "phase-world.png",
			"phase-stop-cursor.png", "phase-stop-cursor-shadow.png" } ;
	
	protected Image[]	m_PhaseImages ;
	protected Image		m_StopCursor ;
	protected Image		m_StopCursorShadow ;
	protected int		m_StopCallback = -1 ;
	protected int		m_StartCallback = -1 ;
	protected int		m_InitCallback = -1 ;
	protected int		m_DecisionCycle = 0 ;
	
	protected boolean	m_DrawPhase = true ;
	protected boolean	m_DrawShadow = false ;
	
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
		
		m_StopCursor 		= m_PhaseImages[m_ImageNames.length-2] ;
		m_StopCursorShadow 	= m_PhaseImages[m_ImageNames.length-1] ;
		
		createPanel(m_Pane.getWindow()) ;
	}

	protected void createPanel(final Composite parent)
	{
		// Allow us to recreate the panel by calling this multiple times
		if (m_Container != null)
		{
			m_Container.dispose() ;
			m_Container = null ;
		}
		
		// The container lets us control the layout of the controls
		// within this window
		m_Container	   = new Composite(parent, SWT.NULL) ;
		m_Container.setLayout(null) ;	// We'll do it manually
		
		Canvas canvas = new Canvas(m_Container, SWT.NULL) ;
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
		int x = 28 ;
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
		gc.drawImage(m_PhaseImages[imageIndex], 0, 0) ;
		
		Point stop   = getCursorPosition(this.m_StopBeforePhase) ;
		Point shadow = getCursorPosition(this.m_StopBeforeShadow) ;

		if (m_DrawShadow)
		{
			// Draw the shadow cursor centered on x,y
			int x = shadow.x - m_StopCursorShadow.getImageData().width/2 ;
			int y = shadow.y - m_StopCursorShadow.getImageData().height/2 ;
			gc.drawImage(m_StopCursorShadow, x, y) ;
		}

		// If we've set the location of the stop cursor draw that in
		if (!m_DrawShadow || m_StopBeforePhase == m_StopBeforeShadow)
		{
			// Draw the cursor centered on x,y
			int x = stop.x - m_StopCursor.getImageData().width/2 ;
			int y = stop.y - m_StopCursor.getImageData().height/2 ;
			gc.drawImage(m_StopCursor, x, y) ;
		}
		
		// Draw the decision cycle counter in the corner
		String decisions = Integer.toString(m_DecisionCycle) ;
		gc.drawText(decisions, 125, 41) ;
	}
	
	public void initsoarEventHandler(int eventID, Object data, String agentName)
	{
		if (eventID == smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED.swigValue())
		{
			m_DrawPhase = true ;
			updateNow() ;
		}
	}
	
	public void systemEventHandler(int eventID, Object data, Kernel kernel)
	{
		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue())
		{
			m_DrawPhase = true ;
			updateNow() ;
		}
		
		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue())
		{
			// Don't show the phase while we're running
			m_DrawPhase = false ;
			updateNow() ;
		}
	}	
	
	public void updateNow()
	{
		Agent agent = this.getAgentFocus() ;
		if (agent == null)
			return ;

		m_NextExecutionPhase = agent.GetCurrentPhase() ;
		m_DecisionCycle      = agent.GetDecisionCycleCounter() ;
		
		// Have to make update in the UI thread.
		// Callback comes in the document thread.
		m_PhaseDiagram.getDisplay().asyncExec(new Runnable() {
			public void run() {
				m_PhaseDiagram.redraw() ;
			}}) ;
	}
	
	/********************************************************************************************
	 * 
	 * Synch up our listening to events to match those we have registered for.
	 * 
	 * @param agent
	********************************************************************************************/
	protected void updateAgentEvents(Agent agent)
	{
		if (agent == null)
			return ;
		
		unregisterForAgentEvents(agent) ;
		registerForAgentEvents(agent) ;
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
			// Update on start, stop and on init-soar
			m_StartCallback	= agent.GetKernel().RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, this, "systemEventHandler", this) ;
			m_StopCallback	= agent.GetKernel().RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, this, "systemEventHandler", this) ;
			m_InitCallback  = agent.GetKernel().RegisterForAgentEvent(smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED, this, "initsoarEventHandler", this) ;
		}
	}

	protected void unregisterForAgentEvents(Agent agent)
	{
		if (m_StartCallback != -1)
			agent.GetKernel().UnregisterForSystemEvent(m_StartCallback) ;
		if (m_StopCallback != -1)
			agent.GetKernel().UnregisterForSystemEvent(m_StopCallback) ;
		if (m_InitCallback != -1)
			agent.GetKernel().UnregisterForAgentEvent(m_InitCallback) ;
		
		m_StopCallback = -1 ;
		m_StartCallback = -1 ;
		m_InitCallback = -1 ;
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
	}

	/********************************************************************************************
	 * 
	 * 
	 * @see modules.AbstractView#showProperties()
	 ********************************************************************************************/

	public void showProperties()
	{
		// TODO Auto-generated method stub

	}

}

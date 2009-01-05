/********************************************************************************************
*
* AbstractUpdateView.java
* 
* Description:	
* 
* Created on 	Oct 15, 2007
* @author 		Jonathan Voigt and Bob Marinier
* 
********************************************************************************************/
package modules;

import java.util.ArrayList;

import general.JavaElementXML;
import helpers.CommandHistory;
import helpers.FormDataHelper;
import manager.Pane;
import menu.ParseSelectedText;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.events.KeyAdapter;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Menu;

import sml.Agent;
import sml.Kernel;
import sml.smlAgentEventId;
import sml.smlPrintEventId;
import sml.smlRunEventId;
import sml.smlSystemEventId;
import debugger.MainFrame;
import dialogs.PropertiesDialog;
import doc.Document;

/********************************************************************************************
* 
* This is a base class designed to part of a view.  This is for views that need to be regularly
* updated during a Soar run.
* 
********************************************************************************************/
public abstract class AbstractUpdateView extends AbstractView  implements Agent.RunEventInterface, Kernel.AgentEventInterface, Kernel.SystemEventInterface
{
	/** If true, run the current command again whenever Soar stops running */
	protected boolean	m_UpdateOnStop = true ;

	/** If > 0 run the current command at the end of every n'th decision */
	protected int	    m_UpdateEveryNthDecision = 0 ;
	
	/** Count the number of decisions we've seen (if we're updating every n decisions) */
	protected int		m_DecisionCounter = 0 ;
	
	protected boolean   m_Updating = false ;
	
	protected int m_StopCallback ;
	protected int m_InitCallback ;
	protected int m_DecisionCallback ;

	// The constructor must take no arguments so it can be called
	// from the loading code and the new window dialog
	public AbstractUpdateView()
	{
		m_InitCallback = -1 ;
		m_StopCallback = -1 ;
		m_DecisionCallback = -1 ;
		m_Updating = false ;
	}
	
	/** The control we're using to display the output in this case **/
	protected abstract Control getDisplayControl() ;
	
	/** 
	 * Returns the entire window, within which the display control lies.
	 * 
	 * Usually the display control is all there is, but this method allows us to define
	 * a container that surrounds the display control and includes other supporting controls.
	 * In which case this method should be overriden.
	 */
	protected Control getDisplayWindow() { return getDisplayControl() ; }
	
	/************************************************************************
	* 
	* Go from current selection (where right click occured) to the object
	* selected by the user (e.g. a production name).
	* 
	*************************************************************************/
	protected ParseSelectedText.SelectedObject getCurrentSelection(int mouseX, int mouseY)
	{
		return null ;
	}
	
	/************************************************************************
	* 
	* Given a context menu and a control, fill in the items you want to 
	* see in the menu.  The simplest is to just call "fillWindowMenu".
	* 
	* This call is made after the user has clicked to bring up the menu
	* so we can create a dymanic menu based on the current context.
	* 
	* You also have to call createContextMenu() to request a context menu
	* be attached to a specific control.
	* 
	*************************************************************************/
	@Override
	protected void fillInContextMenu(Menu contextMenu, Control control,
			int mouseX, int mouseY) {
		// Get the current selected text
		ParseSelectedText.SelectedObject selectionObject = getCurrentSelection(mouseX, mouseY) ;

		if (selectionObject == null)
			selectionObject = new ParseSelectedText.SelectedUnknown(null) ;
		
		boolean simple = true ;
		
		// For now let's dump the output into the main trace window
		// That lets us do interesting things by copying output into one of the
		// scratch windows and working with it there.
		AbstractView outputView = m_Frame.getPrimeView() ;
		if (outputView == null) outputView = this ;
		
		selectionObject.fillMenu(getDocument(), this, outputView, contextMenu, simple) ;
	}

	/********************************************************************************************
	* 
	* Return true if this view shouldn't be user resizable.  E.g. A text window would return false
	* but a bar for buttons would return true.
	* 
	********************************************************************************************/
	@Override
	public boolean isFixedSizeView()
	{
		return false ;
	}

	/************************************************************************
	* 
	* Returns true if this window can display output from commands executed through
	* the "executeAgentCommand" method.
	* 
	*************************************************************************/
	@Override
	public boolean canDisplayOutput() {
		return false;
	}

	/********************************************************************************************
	* 
	* Create the window that will display the output
	* 
	********************************************************************************************/
	protected abstract void createDisplayControl(Composite parent) ;
	
	/********************************************************************************************
	* 
	* Initialize this window and its children.
	* Should call setValues() at the start to complete initialization of the abstract view.
	* 
	********************************************************************************************/
	@Override
	public void init(MainFrame frame, Document doc, Pane parentPane) {
		setValues(frame, doc, parentPane) ;
		Composite parent = parentPane.getWindow() ;
		
		// The container lets us control the layout of the controls
		// within this window
		m_Container	   = new Composite(parent, SWT.NULL) ;
		
		// Listen for when this window is disposed and unregister for anything we registered for
		m_Container.addDisposeListener(new DisposeListener() { public void widgetDisposed(DisposeEvent e) { removeListeners() ; } } ) ;
				
		m_Updating = false ;
		
		// Create the control that will display output from the commands
		createDisplayControl(m_Container) ;
		
		getDisplayControl().setBackground(getBackgroundColor()) ;
		
		layoutControls();
	}

	/** Layout the combo box and the main display window */
	protected void layoutControls()
	{
		// I'll use forms everywhere for consistency and so it's easier
		// to extend them later if we wish to add something.
		m_Container.setLayout(new FormLayout()) ;
		
		FormData attachFull = FormDataHelper.anchorFull(0) ;
		getDisplayWindow().setLayoutData(attachFull) ;
		
		m_Container.layout() ;
		
		// Create a context menu for m_Text.
		// It will be filled in via a call to fillInContextMenu when the menu is popped up
		// (this allows for dynamic content)
		createContextMenu(getDisplayControl()) ;		
	}
	
	public abstract Color getBackgroundColor() ;
	
	// We need to remove listeners that we registered for within the debugger here.
	// Agent listeners (from Soar) are handled separately.
	protected void removeListeners()
	{
		m_Frame.removeAgentFocusListener(this) ;
	}

	/************************************************************************
	* 
	* Set the focus to this window so the user can type commands easily.
	* Return true if this window wants the focus.
	* 
	*************************************************************************/
	@Override
	public boolean setFocus() {
		return false;
	}
	@Override
	public boolean hasFocus() {
		return false;
	}

	@Override
	public void setTextFont(Font f) {
		getDisplayControl().setFont(f) ;
	}

	protected abstract void storeContent(JavaElementXML element) ;

	protected abstract void restoreContent(JavaElementXML element) ;
	
	/************************************************************************
	* 
	* Converts this object into an XML representation.
	* 
	*************************************************************************/
	@Override
	public JavaElementXML convertToXML(String title, boolean storeContent) {
		JavaElementXML element = new JavaElementXML(title) ;
		
		// It's useful to record the class name to uniquely identify the type
		// of object being stored at this point in the XML tree.
		Class cl = this.getClass() ;
		element.addAttribute(JavaElementXML.kClassAttribute, cl.getName()) ;

		if (m_Name == null)
			throw new IllegalStateException("We've created a view with no name -- very bad") ;
		
		// Store this object's properties.
		element.addAttribute("Name", m_Name) ;
		element.addAttribute("UpdateOnStop", Boolean.toString(m_UpdateOnStop)) ;
		element.addAttribute("UpdateEveryNthDecision", Integer.toString(m_UpdateEveryNthDecision)) ;
		
		if (storeContent)
			storeContent(element) ;

		element.addChildElement(this.m_Logger.convertToXML("Logger")) ;
		
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
	@Override
	public void loadFromXML(MainFrame frame, Document doc, Pane parent,
			JavaElementXML element) throws Exception {
		setValues(frame, doc, parent) ;

		m_Name			   = element.getAttribute("Name") ;
		m_UpdateOnStop	   = element.getAttributeBooleanThrows("UpdateOnStop") ;
		m_UpdateEveryNthDecision = element.getAttributeIntThrows("UpdateEveryNthDecision") ;
		
		JavaElementXML log = element.findChildByName("Logger") ;
		if (log != null)
			this.m_Logger.loadFromXML(doc, log) ;

		// Register that this module's name is in use
		frame.registerViewName(m_Name, this) ;
		
		// Actually create the window
		init(frame, doc, parent) ;

		// Restore the text we saved (if we chose to save it)
		restoreContent(element) ;
	}

	protected abstract void updateNow();	

	public void agentEventHandler(int eventID, Object data, String agentName)
	{
		// Note: We need to check the agent names match because although this is called an agentEventHandler it's
		// an event registered with the kernel -- so it's sent to all listeners, not just the agent that is reinitializing.
		if (this.m_UpdateOnStop && eventID == smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED.swigValue() &&
				this.getAgentFocus() != null && agentName.equals(this.getAgentFocus().GetAgentName()))
			updateNow() ;
	}
	
	public void systemEventHandler(int eventID, Object data, Kernel kernel)
	{
		if (this.m_UpdateOnStop && eventID == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue())
			updateNow() ;
	}	
	
	public void runEventHandler(int eventID, Object data, Agent agent, int phase)
	{
		if (eventID == smlRunEventId.smlEVENT_AFTER_DECISION_CYCLE.swigValue())
		{
			m_DecisionCounter++ ;
			
			if (this.m_UpdateEveryNthDecision > 0 && (m_DecisionCounter % m_UpdateEveryNthDecision) == 0)
				updateNow() ;
		}		
	}

	/************************************************************************
	* 
	* We're not sure why this needs to be overridden.
	* 
	*************************************************************************/
	@Override
	public String executeAgentCommand(String command, boolean echoCommand) {
		// Send the command to Soar but there's no where to display the output
		// so we just eat it.
		String result = getDocument().sendAgentCommand(getAgentFocus(), command) ;
		
		return result ;
	}

	/********************************************************************************************
	 * 
	 * Register for events of particular interest to this view
	 * 
	 ********************************************************************************************/
	protected abstract void registerForViewAgentEvents(Agent agent) ;
	protected abstract boolean unregisterForViewAgentEvents(Agent agent) ;
	protected abstract void clearViewAgentEvents() ;
	
	@Override
	protected void registerForAgentEvents(Agent agent)
	{
		if (agent == null)
			return ;
		
		if (m_StopCallback == -1)
		{
			// Update on stop, init-soar or when we make a new connection
			m_StopCallback	= agent.GetKernel().RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, this, this) ;
			m_InitCallback  = agent.GetKernel().RegisterForAgentEvent(smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED, this, this) ;
		}
		
		if (m_UpdateEveryNthDecision > 0 && m_DecisionCallback == -1)
			m_DecisionCallback = agent.RegisterForRunEvent(smlRunEventId.smlEVENT_AFTER_DECISION_CYCLE, this, this) ;
		else if (m_UpdateEveryNthDecision <= 0 && m_DecisionCallback != -1)
		{
			// This is a bit naughty, but it's helpful to unregister for the decision event
			// if we no longer are interested in updates every n decisions.
			// This allows us to just call "registerForAgentEvents" again after updating the properties
			// and get the right result.  We don't currently worry about the other events in the same way as
			// they won't generate as much traffic.
			agent.UnregisterForRunEvent(m_DecisionCallback) ;
			m_DecisionCallback = -1 ;
		}
		
		registerForViewAgentEvents(agent) ;
		
		// If this is a new remote connection, update windows marked for auto updating.
		if (m_UpdateOnStop && m_Document.isConnected() && m_Document.isRemote())
			updateNow() ;
	}
	
	/** Agent gone, so clear any callback references we have (we can't unregister because agent object already destroyed) */
	@Override
	protected void clearAgentEvents()
	{
		m_StopCallback = -1 ;
		m_InitCallback = -1 ;
		m_DecisionCallback = -1 ;
		clearViewAgentEvents() ;
	}
	
	@Override
	protected void unregisterForAgentEvents(Agent agent)
	{
		if (agent == null)
			return ;
	
		boolean ok = true ;

		if (m_StopCallback != -1)
			ok = agent.GetKernel().UnregisterForSystemEvent(m_StopCallback) && ok ;

		if (m_InitCallback != -1)
			ok = agent.GetKernel().UnregisterForAgentEvent(m_InitCallback) && ok ;
				
		if (m_DecisionCallback != -1)
			ok = agent.UnregisterForRunEvent(m_DecisionCallback) && ok ;
		
		m_StopCallback = -1 ;
		m_InitCallback = -1 ;
		m_DecisionCallback = -1 ;
		
		ok = unregisterForViewAgentEvents(agent) && ok ;
		
		if (!ok)
			throw new IllegalStateException("Problem unregistering for events") ;
	}
	
	private int propertiesStartingIndex;

	protected void initProperties(ArrayList<PropertiesDialog.Property> properties) {
		propertiesStartingIndex = properties.size();
		
		properties.add(new PropertiesDialog.IntProperty("Update automatically every n'th decision (0 => none)", m_UpdateEveryNthDecision));
	}
	
	protected void processProperties(ArrayList<PropertiesDialog.Property> properties) {
		m_UpdateEveryNthDecision = ((PropertiesDialog.IntProperty)properties.get(propertiesStartingIndex)).getValue() ;
	}
	
	@Override
	public void showProperties()
	{
		ArrayList<PropertiesDialog.Property> properties = new ArrayList<PropertiesDialog.Property>();
		initProperties(properties);
		
		if (!PropertiesDialog.showDialog(m_Frame, "Properties", properties.toArray(new PropertiesDialog.Property[0]))) {
			return;
		}
		
		processProperties(properties);
		
		reRegisterEvents();

	}
	
	protected void reRegisterEvents() {
		if (this.getAgentFocus() != null)
		{
			// Make sure we're getting the events to match the new settings
			this.unregisterForAgentEvents(this.getAgentFocus()) ;
			this.registerForAgentEvents(this.getAgentFocus()) ;
		}
	}
}




































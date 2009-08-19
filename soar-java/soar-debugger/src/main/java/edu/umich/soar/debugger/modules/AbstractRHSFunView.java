package edu.umich.soar.debugger.modules;

import java.util.ArrayList;



import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.MessageBox;

import edu.umich.soar.debugger.MainFrame;
import edu.umich.soar.debugger.dialogs.PropertiesDialog;
import edu.umich.soar.debugger.doc.Document;
import edu.umich.soar.debugger.manager.Pane;

import sml.Agent;
import sml.Kernel;
import sml.smlAgentEventId;

public abstract class AbstractRHSFunView extends AbstractUpdateView implements Kernel.AgentEventInterface, Kernel.RhsFunctionInterface {

	@Override
	public void init(MainFrame frame, Document doc, Pane parentPane) {
		super.init(frame, doc, parentPane);
	}
	
	protected String rhsFunName = new String();
	protected boolean debugMessages = true;

	protected void registerForAgentEvents(Agent agent)
	{
		super.registerForAgentEvents(agent);
		
		if (rhsFunName.length() > 0) {
			if (!this.m_Document.registerRHSFunction(rhsFunName, this, null)) {
				// failed to register function
				MessageBox errorDialog = new MessageBox(this.m_Frame.getShell(), SWT.ICON_ERROR | SWT.OK);
				errorDialog.setMessage("RHS function already registered: \"" + rhsFunName);
				errorDialog.open();
				rhsFunName = "";
			}
		}
	}

	protected void unregisterForAgentEvents(Agent agent)
	{
		super.unregisterForAgentEvents(agent);
	
		if (rhsFunName.length() > 0) {
			boolean ok = m_Document.unregisterRHSFunction(rhsFunName);

			if (!ok)
				throw new IllegalStateException("Problem unregistering for events") ;
		}
	}
	
	private int rhsFunInitSoarHandler = -1;

	@Override
	protected void registerForViewAgentEvents(Agent agent) {
		rhsFunInitSoarHandler  = agent.GetKernel().RegisterForAgentEvent(smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED, this, this) ;
	}
	@Override
	protected boolean unregisterForViewAgentEvents(Agent agent) {
		if (agent == null)
			return true;

		boolean ok = true;
		
		if (rhsFunInitSoarHandler != -1)
			ok = agent.GetKernel().UnregisterForAgentEvent(rhsFunInitSoarHandler);
		
		rhsFunInitSoarHandler = -1;
		
		return ok;
	}
	@Override
	protected void clearViewAgentEvents() {
		rhsFunInitSoarHandler = -1;
	}
	
	public abstract void onInitSoar();
	
	@Override
	public void agentEventHandler(int eventID, Object data, String agentName)
	{
		// Note: We need to check the agent names match because although this is called an agentEventHandler it's
		// an event registered with the kernel -- so it's sent to all listeners, not just the agent that is reinitializing.
		if (eventID == smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED.swigValue()) {
			onInitSoar();
			updateNow() ;
		}
	}

	private int propertiesStartingIndex;
	private String tempRHSFunName;

	protected void initProperties(ArrayList<PropertiesDialog.Property> properties) {
		super.initProperties(properties);
		propertiesStartingIndex = properties.size();
		
		properties.add(new PropertiesDialog.StringProperty("Name of RHS function to use to update this window", rhsFunName));
		properties.add(new PropertiesDialog.BooleanProperty("Debug messages", debugMessages));
	}
	
	protected void processProperties(ArrayList<PropertiesDialog.Property> properties) {
		super.processProperties(properties);
		
		tempRHSFunName = ((PropertiesDialog.StringProperty)properties.get(propertiesStartingIndex)).getValue() ;
		tempRHSFunName = tempRHSFunName.trim();
		
		debugMessages = ((PropertiesDialog.BooleanProperty)properties.get(propertiesStartingIndex + 1)).getValue() ;

	}
	
	@Override
	protected void reRegisterEvents() {
		if (this.getAgentFocus() != null)
		{
			// Make sure we're getting the events to match the new settings
			this.unregisterForAgentEvents(this.getAgentFocus()) ;
		}

		rhsFunName = tempRHSFunName;
		
		if (this.getAgentFocus() != null)
		{
			this.registerForAgentEvents(this.getAgentFocus()) ;
		}
	}
}

package modules;

import general.JavaElementXML;
import helpers.FormDataHelper;

import manager.Pane;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.events.*;

import sml.Agent;
import sml.Kernel;
import sml.smlAgentEventId;

import java.util.*;
import debugger.MainFrame;
import dialogs.PropertiesDialog;
import dialogs.ReorderButtonsDialog;
import doc.Document;

public class EmoTotalRewardView extends AbstractUpdateView implements Kernel.RhsFunctionInterface
{
	public EmoTotalRewardView()
	{
	}
	
	public String getModuleBaseName() { return "emo_total_reward" ; }
	
	public void showProperties()
	{
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[1] ;
		
		properties[0] = new PropertiesDialog.IntProperty("Update automatically every n'th decision (0 => none)", m_UpdateEveryNthDecision) ;
		
		boolean ok = PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;
		
		if (ok) {
			m_UpdateEveryNthDecision = ((PropertiesDialog.IntProperty)properties[0]).getValue() ;

			if (this.getAgentFocus() != null)
			{
				// Make sure we're getting the events to match the new settings
				this.unregisterForAgentEvents(this.getAgentFocus()) ;
				this.registerForAgentEvents(this.getAgentFocus()) ;
			}
		}
	}
	
	Text totalRewardText;

	/************************************************************************
	* 
	* Set text in a thread safe way (switches to UI thread)
	* 
	*************************************************************************/
	protected void setTextSafely(final String text)
	{
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			totalRewardText.setText(text);
			return ;
		}

		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
            	totalRewardText.setText(text);
            }
         }) ;
	}
	
	@Override
	protected void updateNow() {
		setTextSafely(Double.toString(totalRewardValue));
	}

	double totalRewardValue = 0;
	
	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {
		
		if (functionName.equals("reward")) {
			double reward = 0;
			try {
				reward = Double.parseDouble(argument);
			} catch (NumberFormatException e) {
				return "Unknown argument to " + functionName;
			}
			
			totalRewardValue += reward;
			
			return "Total reward changed to: " + totalRewardValue;
			
		} else {
			assert false;
		}
		
		return "Unknown rhs function received.";
	}
	
	int rewardCallback = -1;
	protected void registerForAgentEvents(Agent agent)
	{
		super.registerForAgentEvents(agent);
		
		if (agent == null)
			return ;

		Kernel kernel = agent.GetKernel();
		rewardCallback = kernel.AddRhsFunction("reward", this, null);
	}

	protected void unregisterForAgentEvents(Agent agent)
	{
		super.unregisterForAgentEvents(agent);
	
		if (agent == null)
			return ;
		
		boolean ok = true ;

		Kernel kernel = agent.GetKernel();

		if (rewardCallback != -1)
			ok = kernel.RemoveRhsFunction(rewardCallback);
		
		rewardCallback = -1;

		if (!ok)
			throw new IllegalStateException("Problem unregistering for events") ;
	}

	@Override
	public boolean find(String text, boolean searchDown, boolean matchCase,
			boolean wrap, boolean searchHiddenText) {
		return false;
	}
	
	int initSoarRewardResetHandler = -1;

	@Override
	protected void registerForViewAgentEvents(Agent agent) {
		initSoarRewardResetHandler  = agent.GetKernel().RegisterForAgentEvent(smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED, this, this) ;
	}
	@Override
	protected boolean unregisterForViewAgentEvents(Agent agent) {
		if (agent == null)
			return true;

		boolean ok = true;
		
		if (initSoarRewardResetHandler != -1)
			ok = agent.GetKernel().UnregisterForAgentEvent(initSoarRewardResetHandler);
		
		initSoarRewardResetHandler = -1;
		
		return ok;
	}
	@Override
	protected void clearViewAgentEvents() {
		initSoarRewardResetHandler = -1;
	}
	
	Composite rewardContainer;

	@Override
	protected void createDisplayControl(Composite parent) {
		
		rewardContainer = new Composite(parent, SWT.NULL);
		FormData attachFull = FormDataHelper.anchorFull(0) ;
		rewardContainer.setLayoutData(attachFull);
		{
			GridLayout gl = new GridLayout();
			gl.numColumns = 1;
//			gl.marginLeft = 0;
//			gl.marginRight = 0;
//			gl.marginTop = 0;
//			gl.marginBottom = 0;
			gl.verticalSpacing = 0;
//			gl.marginBottom = 0;
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			rewardContainer.setLayout(gl);
		}
		
		Label totalRewardLabel = new Label(rewardContainer, SWT.NONE);
		totalRewardLabel.setText("Total Reward:");
		{
			GridData gd = new GridData();
			gd.grabExcessHorizontalSpace = true;
			totalRewardLabel.setLayoutData(gd);
		}
		
		totalRewardText = new Text(rewardContainer, SWT.MULTI | SWT.READ_ONLY);
		totalRewardText.setText("0");
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.FILL;
			totalRewardText.setLayoutData(gd);
		}

		createContextMenu(totalRewardLabel) ;
		createContextMenu(totalRewardText) ;
	}
	
	@Override
	public Color getBackgroundColor() {
		return getMainFrame().getDisplay().getSystemColor(SWT.COLOR_WIDGET_BACKGROUND) ;
	}

	@Override
	protected Control getDisplayControl() {
		// this should return the text control
		return rewardContainer;
	}

	@Override
	protected void restoreContent(JavaElementXML element) {
		
	}

	@Override
	protected void storeContent(JavaElementXML element) {
		
	}

	@Override
	public void clearDisplay() {
	}

	@Override
	public void copy() {
		totalRewardText.copy();
	}

	@Override
	public void displayText(String text) {
	}
	
	public void resetReward() {
		totalRewardValue = 0;
		setTextSafely("0");
	}

	public void agentEventHandler(int eventID, Object data, String agentName)
	{
		// Note: We need to check the agent names match because although this is called an agentEventHandler it's
		// an event registered with the kernel -- so it's sent to all listeners, not just the agent that is reinitializing.
		if (eventID == smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED.swigValue()) {
			resetReward();
			updateNow() ;
		}
	}
}

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
		assert false;
	}
	
	Text totalRewardText;

	public general.JavaElementXML convertToXML(String title, boolean storeContent)
	{
		JavaElementXML element = new JavaElementXML(title) ;
		
		// It's useful to record the class name to uniquely identify the type
		// of object being stored at this point in the XML tree.
		Class cl = this.getClass() ;
		element.addAttribute(JavaElementXML.kClassAttribute, cl.getName()) ;

		// Store this object's properties.
		element.addAttribute("Name", m_Name) ;

		return element ;
	}

	public void loadFromXML(MainFrame frame, doc.Document doc, Pane parent, general.JavaElementXML element) throws Exception
	{
		setValues(frame, doc, parent) ;
		
		m_Name   = element.getAttributeThrows("Name") ;

		// Register that this module's name is in use
		frame.registerViewName(m_Name, this) ;

		// Actually create the window
		init(frame, doc, parent) ;
	}

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

	@Override
	protected void registerForViewAgentEvents(Agent agent) {
		//TODO: handle init-soar so that we reset the counter
	}
	@Override
	protected boolean unregisterForViewAgentEvents(Agent agent) {	
		return true;
	}
	@Override
	protected void clearViewAgentEvents() {
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
}

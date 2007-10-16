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

public class RHSFunTextView extends AbstractUpdateView implements Kernel.AgentEventInterface, Kernel.RhsFunctionInterface
{
	public RHSFunTextView()
	{
		labelText = getModuleBaseName();
	}
	
	public String getModuleBaseName() { return "rhs_fun_text" ; }
	
	// Assume this may be empty! (no function is registered)
	protected String rhsFunName = new String();
	protected String labelText = new String();
	protected Label labelTextWidget;
	
	public void showProperties()
	{
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[3] ;
		
		properties[0] = new PropertiesDialog.IntProperty("Update automatically every n'th decision (0 => none)", m_UpdateEveryNthDecision) ;
		properties[1] = new PropertiesDialog.StringProperty("Name of RHS function to use to update this window", rhsFunName) ;
		properties[2] = new PropertiesDialog.StringProperty("Label text", labelText) ;
		
		boolean ok = PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;
		
		if (ok) {
			m_UpdateEveryNthDecision = ((PropertiesDialog.IntProperty)properties[0]).getValue() ;
			labelText = ((PropertiesDialog.StringProperty)properties[2]).getValue() ;
			setLabelText(labelText);

			if (this.getAgentFocus() != null)
			{
				// Make sure we're getting the events to match the new settings
				this.unregisterForAgentEvents(this.getAgentFocus()) ;
				this.registerForAgentEvents(this.getAgentFocus()) ;
			}
			
			String tempRHSFunName = ((PropertiesDialog.StringProperty)properties[1]).getValue() ;
			tempRHSFunName = tempRHSFunName.trim();
			
			// Make sure new one is different than old one and not zero length string
			if (tempRHSFunName.length() <= 0 || tempRHSFunName.equals(rhsFunName)) {
				return;
			}

			Agent agent = m_Frame.getAgentFocus() ;
			if (agent == null) {
				return;
			}

			// Try and register this, message and return if failure
			Kernel kernel = agent.GetKernel();
			int tempRHSCallback = kernel.AddRhsFunction(tempRHSFunName, this, null);
			
			// TODO: Verify that error check here is correct, and fix registerForAgentEvents
			// BUGBUG: remove true
			if (tempRHSCallback <= 0) {
				// failed to register callback
				MessageBox errorDialog = new MessageBox(textBox.getShell(), SWT.ICON_ERROR | SWT.OK);
				errorDialog.setMessage("Failed to change RHS function name \"" + tempRHSFunName + "\".");
				errorDialog.open();
				return;
			}
			
			// unregister old rhs fun
			boolean registerOK = true ;

			if (rhsCallback != -1)
				registerOK = kernel.RemoveRhsFunction(rhsCallback);
			
			rhsCallback = -1;

			// save new one
			rhsFunName = tempRHSFunName;
			rhsCallback = tempRHSCallback;
			
			if (!registerOK)
				throw new IllegalStateException("Problem unregistering for events") ;
		
		} // Careful, returns in the previous block!
	}
	
	Text textBox;

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
			textBox.setText(text);
			return ;
		}
		
		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
            	textBox.setText(text);
            }
         }) ;
	}
	
	protected void setLabelText(final String text)
	{
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			labelTextWidget.setText(text);
			return ;
		}
		
		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
            	labelTextWidget.setText(text);
            }
         }) ;
	}
	
	/************************************************************************
	* 
	* Append text in a thread safe way (switches to UI thread)
	* 
	*************************************************************************/
	protected void appendTextSafely(final String text)
	{
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			textBox.append(text);
			return ;
		}
		
		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
            	textBox.append(text);
            }
         }) ;
	}
	
	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {

		if (!functionName.equals(rhsFunName)) {
			return "Unknown rhs function received in window " + getName() + ".";
		}
		
		String[] args = argument.split("\\s+");
		StringBuilder output = new StringBuilder();
		
		for (int index = 0; index < args.length; index += 2) {
			output.append(args[index]);
			if (index + 1 < args.length) {
				output.append(": ");
				output.append(args[index + 1]);
				output.append("\n");
			}
		}
		
		System.out.println(output);
		setTextSafely(output.toString());
		
		return "Successfully updated " + getName();
	}
	
	int rhsCallback = -1;
	protected void registerForAgentEvents(Agent agent)
	{
		super.registerForAgentEvents(agent);
		
		if (rhsFunName.length() <= 0) {
			return;
		}
		
		if (agent == null)
			return ;

		Kernel kernel = agent.GetKernel();
		rhsCallback = kernel.AddRhsFunction(rhsFunName, this, null);

		if (rhsCallback <= 0) {
			// failed to register callback
			rhsCallback = -1;
			rhsFunName = "";
			throw new IllegalStateException("Problem registering for events") ;
		}
	}

	protected void unregisterForAgentEvents(Agent agent)
	{
		super.unregisterForAgentEvents(agent);
	
		if (agent == null)
			return ;
		
		boolean ok = true ;

		Kernel kernel = agent.GetKernel();

		if (rhsCallback != -1)
			ok = kernel.RemoveRhsFunction(rhsCallback);
		
		rhsCallback = -1;

		if (!ok)
			throw new IllegalStateException("Problem unregistering for events") ;
	}

	@Override
	public boolean find(String text, boolean searchDown, boolean matchCase,
			boolean wrap, boolean searchHiddenText) {
		return false;
	}
	
	int rhsFunInitSoarHandler = -1;

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
	
	Composite rhsFunContainer;

	@Override
	protected void createDisplayControl(Composite parent) {
		
		rhsFunContainer = new Composite(parent, SWT.NULL);
		FormData attachFull = FormDataHelper.anchorFull(0) ;
		rhsFunContainer.setLayoutData(attachFull);
		{
			GridLayout gl = new GridLayout();
			gl.numColumns = 1;
			gl.verticalSpacing = 0;
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			rhsFunContainer.setLayout(gl);
		}
		
		labelTextWidget = new Label(rhsFunContainer, SWT.NONE);
		labelTextWidget.setText(labelText);
		{
			GridData gd = new GridData(SWT.FILL, SWT.NONE, true, false);
			labelTextWidget.setLayoutData(gd);
		}
		
		textBox = new Text(rhsFunContainer, SWT.MULTI | SWT.READ_ONLY | SWT.H_SCROLL | SWT.V_SCROLL);
		updateNow();
		{
			GridData gd = new GridData(SWT.FILL, SWT.FILL, true, true);
			textBox.setLayoutData(gd);
		}

		createContextMenu(labelTextWidget) ;
		createContextMenu(textBox) ;
	}
	
	@Override
	public Color getBackgroundColor() {
		return getMainFrame().getDisplay().getSystemColor(SWT.COLOR_WIDGET_BACKGROUND) ;
	}

	@Override
	protected Control getDisplayControl() {
		// this should return the text control
		return rhsFunContainer;
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
		textBox.copy();
	}

	@Override
	public void displayText(String text) {
	}
	
	public void onInitSoar() {
	}

	public void agentEventHandler(int eventID, Object data, String agentName)
	{
		// Note: We need to check the agent names match because although this is called an agentEventHandler it's
		// an event registered with the kernel -- so it's sent to all listeners, not just the agent that is reinitializing.
		if (eventID == smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED.swigValue()) {
			onInitSoar();
			updateNow() ;
		}
	}

	@Override
	protected void updateNow() {
		// we don't need to do anything here but subclasses might
	}
	
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
		element.addAttribute("RHSFunctionName", rhsFunName) ;
		element.addAttribute("LabelText", labelText) ;
				
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

		m_Name			   	= element.getAttribute("Name") ;
		m_UpdateOnStop	   	= element.getAttributeBooleanThrows("UpdateOnStop") ;
		m_UpdateEveryNthDecision = element.getAttributeIntThrows("UpdateEveryNthDecision") ;
		rhsFunName 			= element.getAttribute("RHSFunctionName");
		labelText 			= element.getAttribute("LabelText");
		
		if (rhsFunName == null) {
			rhsFunName = new String();
		}
		
		if (labelText == null) {
			labelText = new String();
		}
		
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

}

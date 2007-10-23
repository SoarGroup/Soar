package modules;

import general.JavaElementXML;
import helpers.FormDataHelper;

import manager.Pane;
import menu.ParseSelectedText;

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
	}
	
	@Override
	public void init(MainFrame frame, Document doc, Pane parentPane) {
		if (labelText.length() <= 0) {
			labelText = getModuleBaseName();
		}
		super.init(frame, doc, parentPane);
	}
	
	public String getModuleBaseName() { return "rhs_fun_text" ; }
	
	protected String rhsFunName = new String();
	
	protected String labelText = new String();
	protected Label labelTextWidget;
	protected boolean debugMessages = true;
	
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
	
	StringBuilder output;
	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {

		String[] commandLine = argument.split("\\s+");
		
		if (commandLine.length >= 1 && commandLine[0].equals("--clear")) {
			this.onInitSoar();
			return debugMessages ? m_Name + ":" + functionName + ": cleared" : null;
		}
		
		output = new StringBuilder();
		
		for (int index = 0; index < commandLine.length; index += 2) {
			output.append(commandLine[index]);
			if (index + 1 < commandLine.length) {
				output.append(": ");
				output.append(commandLine[index + 1]);
				output.append("\n");
			}
		}
		
		return debugMessages ? m_Name + ":" + functionName + ": updated " + getName() : null;
	}
	
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
		output = null;
		setTextSafely("");
	}

	@Override
	public void copy() {
		textBox.copy();
	}
	
	@Override
	protected ParseSelectedText.SelectedObject getCurrentSelection(int mouseX, int mouseY)
	{
		int pos = textBox.getCaretPosition() ;
		if (pos == -1)
			return null ;
		
		ParseSelectedText selection = new ParseSelectedText(textBox.getText(), pos) ;
		
		return selection.getParsedObject(this.m_Document, this.getAgentFocus()) ;
	}

	@Override
	public void displayText(String text) {
	}
	
	public void onInitSoar() {
		clearDisplay();
	}

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

	@Override
	protected void updateNow() {
		// we don't need to do anything here but subclasses might
		if (output != null) {
			setTextSafely(output.toString());
		}
	}
	
	public void showProperties()
	{
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[4] ;
		
		properties[0] = new PropertiesDialog.IntProperty("Update automatically every n'th decision (0 => none)", m_UpdateEveryNthDecision) ;
		properties[1] = new PropertiesDialog.StringProperty("Name of RHS function to use to update this window", rhsFunName) ;
		properties[2] = new PropertiesDialog.StringProperty("Label text", labelText) ;
		properties[3] = new PropertiesDialog.BooleanProperty("Debug messages", debugMessages) ;
		
		boolean ok = PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;
		
		if (ok) {
			m_UpdateEveryNthDecision = ((PropertiesDialog.IntProperty)properties[0]).getValue() ;
			labelText = ((PropertiesDialog.StringProperty)properties[2]).getValue() ;
			setLabelText(labelText);
			debugMessages = ((PropertiesDialog.BooleanProperty)properties[3]).getValue() ;
			String tempRHSFunName = ((PropertiesDialog.StringProperty)properties[1]).getValue() ;

			if (this.getAgentFocus() != null)
			{
				// Make sure we're getting the events to match the new settings
				this.unregisterForAgentEvents(this.getAgentFocus()) ;

				rhsFunName = tempRHSFunName.trim();

				this.registerForAgentEvents(this.getAgentFocus()) ;
				
			} else {
				rhsFunName = tempRHSFunName;
			}
			
		} // Careful, returns in the previous block!
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
		element.addAttribute("DebugMessages", Boolean.toString(debugMessages)) ;
				
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
		rhsFunName			= element.getAttribute("RHSFunctionName");
		labelText 			= element.getAttribute("LabelText");
		debugMessages		= element.getAttributeBooleanThrows("DebugMessages");

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

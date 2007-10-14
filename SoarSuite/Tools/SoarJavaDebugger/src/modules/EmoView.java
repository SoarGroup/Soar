package modules;

import general.JavaElementXML;
import helpers.FormDataHelper;

import manager.Pane;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.events.*;

import sml.Agent;
import sml.Kernel;

import java.util.*;
import debugger.MainFrame;
import dialogs.PropertiesDialog;
import dialogs.ReorderButtonsDialog;
import doc.Document;

public class EmoView extends AbstractFixedView implements Kernel.RhsFunctionInterface
{
	public EmoView()
	{
	}
	
	public String getModuleBaseName() { return "emo" ; }
	
	/********************************************************************************************
	* 
	* Initialize this window and its children.
	* Should call setValues() at the start to complete initialization of the abstract view.
	* 
	********************************************************************************************/
	public void init(MainFrame frame, Document doc, Pane parentPane)
	{
		setValues(frame, doc, parentPane) ;
		
		createEmoPanel(m_Pane.getWindow()) ;
	}
	
	public void showProperties()
	{
		assert false;
	}
	
	Label totalReward;

	protected void createEmoPanel(final Composite parent)
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
		
		{
			GridLayout layout = new GridLayout() ;
			layout.numColumns = 2;
			m_Container.setLayout(layout) ;				
		}
		
		Label totalRewardLabel = new Label(m_Container, SWT.NONE);
		totalRewardLabel.setText("Total Reward:");
		{
			GridData gd = new GridData();
			totalRewardLabel.setLayoutData(gd);
		}
		
		totalReward = new Label(m_Container, SWT.NONE);
		totalReward.setText("0");
		{
			GridData gd = new GridData();
			gd.widthHint = 50;
			totalReward.setLayoutData(gd);
		}
		
		// Create a context menu for m_Text.
		// It will be filled in via a call to fillInContextMenu when the menu is popped up
		// (this allows for dynamic content)
		createContextMenu(m_Container) ;

		// Layout the parent again, because this set of windows has changed
		// This only matters if we're recreating the windows
		parent.layout(true) ;
	}
	
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
			totalReward.setText(Double.toString(totalRewardValue));
			return ;
		}

		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
            	totalReward.setText(Double.toString(totalRewardValue));
            }
         }) ;
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
			
			//double totalRewardValue = Double.parseDouble(totalReward.getText());
			totalRewardValue += reward;
			//totalReward.setText(Double.toString(totalRewardValue));
			setTextSafely(Double.toString(totalRewardValue));
			
			//return "Total reward changed to: " + totalReward.getText();
			return "Total reward changed to: " + totalRewardValue;
			
		} else {
			assert false;
		}
		
		return "Unknown rhs function received.";
	}
	
	int rewardCallback = -1;
	protected void registerForAgentEvents(Agent agent)
	{
		Kernel kernel = agent.GetKernel();
		rewardCallback = kernel.AddRhsFunction("reward", this, null);
	}

	protected void unregisterForAgentEvents(Agent agent)
	{
		Kernel kernel = agent.GetKernel();
		kernel.RemoveRhsFunction(rewardCallback);
		rewardCallback = -1;
	}

}

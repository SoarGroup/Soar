/********************************************************************************************
*
* DebuggerWindow.java
* 
* Created on 	Nov 12, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import manager.MainWindow;
import manager.Pane;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.events.*;

import doc.* ;
import doc.events.*;
import debugger.* ;

import sml.Agent;

/********************************************************************************************
* 
* The functionality that a debugger window needs to support.
* 
********************************************************************************************/
public abstract class AbstractView implements AgentFocusListener
{	
	/** The document is used to represent the Soar process.  There is one shared document
	 *  for the entire debugger. **/
	protected Document 		m_Document ;
	
	/** The main frame that owns this window.  **/
	protected MainFrame		m_MainFrame ;
	
	/** The window that owns this view */
	protected Pane			m_Pane ;
	
	//protected Composite		m_Panel ;
	
	/********************************************************
	 * It's nice to just use a default constructor as we will dynamically create these objects
	 * using reflective methods like: class.newInstance().
	 * If we add parameters here we'll need to change that logic to look up an appropriate
	 * constructor method and call it.
	 *********************************************************/
	public AbstractView()
	{
	}
	
	public abstract void setTextFont(Font f) ;
	
	public doc.Document getDocument() 		   { return m_Document ; }
	
	public MainFrame getMainFrame()
	{
		return m_MainFrame;
	}

	public MainWindow getMainWindow()
	{
		return m_MainFrame.getMainWindow() ;
	}
	
	public Pane getPane()
	{
		return m_Pane ;
	}
	
	protected void setPane(Pane pane)
	{
		if (m_Pane != null)
			throw new IllegalStateException("Only do this once -- or you need to remove the old value from the list of views owned by pane") ;
		
		m_Pane = pane ;
		pane.addView(this) ;
	}
	
	/********************************************************************************************
	 * 
	 * Returns the agent that is associated with the main frame.
	 * A given window can choose to override this and work with a different agent.
	 * 
	 * This can return null if no agent is currently selected in the main frame.
	 * 
	********************************************************************************************/
	public Agent getAgentFocus()
	{
		return m_MainFrame.getAgentFocus() ;
	}
	
	//public abstract void registerContextMenuMouseListener(MouseListener listener) ;
		
	/********************************************************************************************
	* 
	* Initialize this window.
	* 
	********************************************************************************************/
	public abstract void Init(MainFrame frame, Document doc, Pane parentPane) ;
	
	/************************************************************************
	* 
	* Converts this object into an XML representation.
	* 
	*************************************************************************/
	public abstract general.ElementXML ConvertToXML(String title) ;

	/************************************************************************
	* 
	* Execute a command (send it to Soar) and display the output in a manner
	* appropriate to this view.
	* 
	* @param Command		The command line to execute
	* @param echoCommand	If true, display the command in the output window as well.
	* 
	* The result (if any) is also returned to the caller.
	* 
	*************************************************************************/
	public abstract String executeAgentCommand(String command, boolean echoCommand) ;

	/************************************************************************
	* 
	* Display the given text in this view (if possible).
	* 
	* This method is used to programmatically insert text that Soar doesn't generate
	* into the output window.
	* 
	*************************************************************************/
	public abstract void displayText(String text) ;

	/************************************************************************
	* 
	* Return true from a subclass if the window is a trace window.  We'll send
	* menu commands (like "source file") to this window and display the results of
	* the command here.  Multiple windows can return true in which case the first is
	* selected (currently).
	* 
	*************************************************************************/
	public boolean canBePrimeWindow() { return false ; }
	
	/************************************************************************
	* 
	* Set the focus to this window so the user can type commands easily.
	* Return true if this window wants the focus.
	* 
	*************************************************************************/
	public abstract boolean setFocus() ;
	public abstract boolean hasFocus() ;
	
	/************************************************************************
	* 
	* Create an instance of the class.  It does not have to be fully initialized
	* (it's the caller's responsibility to finish the initilization).
	* 
	*************************************************************************/
	// You need to add this method, but it can't be in the interface because it's static.
	// public static AbstractWindow createInstance() ;
	
	/************************************************************************
	* 
	* Rebuild the object from an XML representation.
	* 
	* @param frame			The top level window that owns this window
	* @param doc			The document we're rebuilding
	* @param element		The XML representation of this command
	* 
	*************************************************************************/
	public abstract void LoadFromXML(MainFrame frame, doc.Document doc, general.ElementXML element) throws Exception ;
	
	protected abstract void registerForAgentEvents(Agent agent) ;
	
	protected abstract void unregisterForAgentEvents(Agent agent) ;
	
	public void agentGettingFocus(AgentFocusEvent e)
	{
		if (e.getAgent() != null)
			registerForAgentEvents(e.getAgent()) ;
	}
	
	public void agentLosingFocus(AgentFocusEvent e)
	{
		// We may be passed "null" for the agent losing focus if the
		// agent is no longer valid to access (e.g. we shutdown the kernel).
		// We still get a notification in case we need to do any other clean-up.
		unregisterForAgentEvents(e.getAgent()) ;
	}
}



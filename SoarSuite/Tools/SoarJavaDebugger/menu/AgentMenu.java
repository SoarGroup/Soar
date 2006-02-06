/********************************************************************************************
*
* AgentMenu.java
* 
* Description:	
* 
* Created on 	Jan 31, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package menu;

import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.Shell;

import sml.Agent;

import debugger.MainFrame;
import dialogs.SelectAgentDialog;
import doc.Document;

/************************************************************************
 * 
 * A menu for controlling agent level commands--creating them, destroying them etc.
 * 
 ************************************************************************/
public class AgentMenu
{
	private BaseMenu	m_Menu ;
	
	private Document m_Document = null ;
	private MainFrame m_Frame	= null ;
	
	private AbstractAction m_SelectAgent      = new AbstractAction("Select Current Agent")			{ public void actionPerformed(ActionEvent e) { selectAgent(e) ; } } ;
	private AbstractAction m_CreateAgentSame  = new AbstractAction("Create Agent - Same Window") 	{ public void actionPerformed(ActionEvent e) { createAgent(e, false) ; } } ;
	private AbstractAction m_CreateAgentNew   = new AbstractAction("Create Agent - New Window") 	{ public void actionPerformed(ActionEvent e) { createAgent(e, true) ; } } ;
	private AbstractAction m_CreateNewWindow  = new AbstractAction("Create Window - No New Agent") 	{ public void actionPerformed(ActionEvent e) { createWindow(e) ; } } ;
	private AbstractAction m_CreateNewOnAgent = new AbstractAction("Create New Window on Agent Creation") 	{ public void actionPerformed(ActionEvent e) { createNewOnAgent(e) ; } } ;
	private AbstractAction m_CloseOnDestroy   = new AbstractAction("Close Window on Agent Destruction") 	{ public void actionPerformed(ActionEvent e) { closeOnDestroy(e) ; } } ;
	private AbstractAction m_DestroyAgent 	  = new AbstractAction("Destroy Agent") 				{ public void actionPerformed(ActionEvent e) { destroyAgent(e) ; } } ;

	/** Create this menu */
	public static AgentMenu createMenu(MainFrame frame, Document doc, String title)
	{
		AgentMenu menu = new AgentMenu() ;
		menu.m_Frame    = frame ;
		menu.m_Document = doc ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title)
	{
		BaseMenu menu = new BaseMenu(parent, title) ;
		
		menu.add(m_SelectAgent) ;
		menu.addSeparator() ;
		menu.add(m_CreateAgentNew) ;
		menu.add(m_CreateAgentSame) ;
		menu.add(m_CreateNewWindow) ;
		menu.addSeparator() ;
		menu.addCheckedItem(m_CreateNewOnAgent, isCreateNewWindowForNewAgent()) ;
		menu.addCheckedItem(m_CloseOnDestroy, isCloseWindowWhenDestroyAgent()) ;
		menu.addSeparator() ;
		menu.add(m_DestroyAgent) ;

		updateMenu() ;
		
		return menu ;
	}
		
	/** Enable/disable the menu items depending on the current state of the kernel */
	public void updateMenu()
	{
		// Can only work with agents when we have a kernel
		m_SelectAgent.setEnabled(m_Document.isConnected() && m_Document.getNumberAgents() > 0) ;
		m_CreateAgentSame.setEnabled(m_Document.isConnected()) ;
		m_CreateAgentNew.setEnabled(m_Document.isConnected()) ;
		m_DestroyAgent.setEnabled(m_Document.isConnected() && m_Frame.getAgentFocus() != null) ;
		
		// Make all menus match
		if (!m_CreateNewOnAgent.getMenuItem().isDisposed())
		{
			final boolean check = isCreateNewWindowForNewAgent() ;
			this.m_Frame.getDisplay().asyncExec(new Runnable() { public void run() { m_CreateNewOnAgent.setChecked(check, false) ; } } ) ;
			
		}
	}

	public boolean isCreateNewWindowForNewAgent()
	{
		return m_Document.getAppProperties().getAppBooleanProperty(Document.kCreateNewWindowProperty, true) ;
	}
	
	private boolean isCloseWindowWhenDestroyAgent()
	{
		return m_Document.isCloseWindowWhenDestroyAgent() ;	
	}
	
	// BUGBUG: We should change the menu items on all of the menus to match this or
	// better look up the state when the menu is popped down.  Right now turning this off on
	// one menu won't change the menus on other frames.
	private void createNewOnAgent(ActionEvent e)
	{
		m_Document.getAppProperties().setAppProperty(Document.kCreateNewWindowProperty, m_CreateNewOnAgent.isChecked()) ;
	}

	private void closeOnDestroy(ActionEvent e)
	{
		m_Document.getAppProperties().setAppProperty(Document.kCloseOnDestroyProperty, m_CloseOnDestroy.isChecked()) ;
	}

	private void selectAgent(ActionEvent e)
	{
		if (m_Document.getNumberAgents() == 0)
		{
			throw new IllegalStateException("Menu item selectAgent should have been disabled") ;
		}
				
		// Ask the user to pick an agent
		String agentName = SelectAgentDialog.showDialog(m_Frame.getWindow(), "Select the current agent", m_Document.getAgentNameArray()) ;
		
		// User cancelled
		if (agentName == null)
			return ;
		
		// Set this frame to focus on a particular agent
		Agent agent = m_Document.getAgent(agentName) ;
		m_Frame.setAgentFocus(agent) ;
	}

	private void createWindow(ActionEvent e)
	{
		// Create a new window for this agent
		Shell shell = new Shell(m_Frame.getDisplay()) ;
		
		MainFrame frame = new MainFrame(shell, m_Document) ;
		frame.initComponents() ;

		shell.open() ;
	}
	
	private void createAgent(ActionEvent e, boolean newWindow)
	{
		// Generate a default name for the agent
		int nAgents = m_Document.getNumberAgents() ;
		String generatedName = "soar" + (nAgents+1) ;
		
		// Ask for the name of this agent
		String name = m_Frame.ShowInputDialog("Create Agent", "Enter new agent name", generatedName) ;
		
		// Check if cancelled
		if (name == null || name == "")
			return ;

		if (m_Document.getAgent(name) != null)
		{
			m_Frame.ShowMessageBox("There is already an agent named " + name) ;
			return ;
		}
		
		Agent newAgent = m_Document.createAgentNoNewWindow(name) ;
		
		if (newWindow)
		{
			MainFrame frame = MainFrame.createNewFrame(m_Frame.getDisplay(), m_Document) ;		
			frame.setAgentFocus(newAgent) ;
		}
		else
		{
			// Re-use the existing window
			m_Frame.setAgentFocus(newAgent) ;
		}
	}

	private void destroyAgent(ActionEvent e)
	{
		Agent agent = m_Frame.getAgentFocus() ;
		
		if (agent == null)
		{
			throw new IllegalStateException("Menu item destroyAgent should have been disabled") ;
		}
		
		m_Document.destroyAgent(agent) ;
	}

}

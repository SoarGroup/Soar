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
import sml.Kernel;

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

	private AbstractAction m_SelectAgent      = new AbstractAction("Select Agent")					{ public void actionPerformed(ActionEvent e) { selectAgent(e) ; } } ;
	private AbstractAction m_CreateAgentSame  = new AbstractAction("Create Agent - Same Window") 	{ public void actionPerformed(ActionEvent e) { createAgent(e, false) ; } } ;
	private AbstractAction m_CreateAgentNew   = new AbstractAction("Create Agent - New Window") 	{ public void actionPerformed(ActionEvent e) { createAgent(e, true) ; } } ;
	private AbstractAction m_DestroyAgent 	  = new AbstractAction("Destroy Agent") 				{ public void actionPerformed(ActionEvent e) { destroyAgent(e) ; } } ;

	/** Create this menu */
	public static AgentMenu createMenu(MainFrame frame, Document doc, String title, char mnemonicChar)
	{
		AgentMenu menu = new AgentMenu() ;
		menu.m_Frame    = frame ;
		menu.m_Document = doc ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title, mnemonicChar) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title, char mnemonicChar)
	{
		BaseMenu menu = new BaseMenu(parent, title, mnemonicChar) ;
		
		menu.add(m_SelectAgent) ;
		menu.add(m_CreateAgentNew) ;
		menu.add(m_CreateAgentSame) ;
		//menu.add(new javax.swing.JSeparator()) ;
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
	}

	private void selectAgent(ActionEvent e)
	{
		if (m_Document.getNumberAgents() == 0)
		{
			throw new IllegalStateException("Menu item selectAgent should have been disabled") ;
		}
				
		// Ask the user to pick an agent
		String agentName = SelectAgentDialog.showDialog(m_Frame.getWindow(), "Select an agent", m_Document.getAgentNameArray()) ;
		
		// User cancelled
		if (agentName == null)
			return ;
		
		// Set this frame to focus on a particular agent
		Agent agent = m_Document.getAgent(agentName) ;
		m_Frame.setAgentFocus(agent) ;
	}

	private void createAgent(ActionEvent e, boolean newWindow)
	{
		// Ask for the name of this agent
		String name = m_Frame.ShowInputDialog("Create Agent", "Enter new agent name", "") ;
		
		// Check if cancelled
		if (name == null || name == "")
			return ;

		if (m_Document.getAgent(name) != null)
		{
			m_Frame.ShowMessageBox("There is already an agent named " + name) ;
			return ;
		}
		
		Agent newAgent = m_Document.createAgent(name) ;
		
		if (newWindow)
		{
			// Create a new window for this agent
			Shell shell = new Shell(m_Frame.getDisplay()) ;
			
			MainFrame frame = new MainFrame(shell, m_Document) ;
			frame.initComponents() ;
			frame.setVisible(true) ;

			shell.open() ;
		
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

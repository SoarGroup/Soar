/********************************************************************************************
*
* RemoteMenu.java
* 
* Description:	
* 
* Created on 	Jan 29, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package menu;

import org.eclipse.swt.widgets.Menu;

import sml.Agent;
import sml.Kernel;

import debugger.MainFrame;
import doc.Document;

/************************************************************************
 * 
 * Menu for commands related to working with a remote instance of Soar
 * (on another PC or already running in another process)
 * 
 ************************************************************************/
public class KernelMenu
{
	private BaseMenu	m_Menu ;
	
	private Document m_Document = null ;
	private MainFrame m_Frame	= null ;

	private AbstractAction m_StartKernel      = new AbstractAction("Start local Soar")				{ public void actionPerformed(ActionEvent e) { startKernelPerformed(e) ; } } ;
	private AbstractAction m_StopKernel       = new AbstractAction("Stop local Soar") 				{ public void actionPerformed(ActionEvent e) { stopKernelPerformed(e) ; } } ;
	private AbstractAction m_RemoteConnect 	  = new AbstractAction("Connect to remote Soar...") 	{ public void actionPerformed(ActionEvent e) { remoteConnectPerformed(e) ; } } ;
	private AbstractAction m_RemoteDisconnect = new AbstractAction("Disconnect from remote Soar") 	{ public void actionPerformed(ActionEvent e) { remoteDisconnectPerformed(e) ; } } ;

	/** Create this menu */
	public static KernelMenu createMenu(MainFrame frame, Document doc, String title, char mnemonicChar)
	{
		KernelMenu menu = new KernelMenu() ;
		menu.m_Frame    = frame ;
		menu.m_Document = doc ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title, mnemonicChar) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title, char mnemonicChar)
	{
		BaseMenu menu = new BaseMenu(parent, title, mnemonicChar) ;
		
		menu.add(m_RemoteConnect) ;
		menu.add(m_RemoteDisconnect) ;
		//menu.add(new javax.swing.JSeparator()) ;
		menu.add(m_StartKernel) ;
		menu.add(m_StopKernel) ;

		updateMenu() ;
		
		return menu ;		
	}
	
	/** Enable/disable the menu items depending on the current state of the kernel */
	public void updateMenu()
	{
		m_StartKernel.setEnabled(!m_Document.isConnected()) ;
		m_StopKernel.setEnabled(m_Document.isConnected() && !m_Document.isRemote()) ;
		m_RemoteConnect.setEnabled(!m_Document.isConnected()) ;
		m_RemoteDisconnect.setEnabled(m_Document.isConnected() && m_Document.isRemote()) ;
	}

	private void startKernelPerformed(ActionEvent e)
	{
		// Start a kernel and create a first agent
		Agent agent = m_Document.startLocalKernel(Kernel.GetDefaultPort()) ;
		
		// Attach this to the frame
		m_Frame.setAgentFocus(agent) ;
	}

	private void stopKernelPerformed(ActionEvent e)
	{
		m_Document.stopLocalKernel() ;
	}
	
	private void remoteConnectPerformed(ActionEvent e)
	{
		// BADBAD: Need a UI to ask for IP and port number here
		// For now just hard coding.
		try
		{
			m_Document.remoteConnect(null, Kernel.GetDefaultPort()) ;
		}
		catch (Exception ex)
		{
			m_Frame.ShowMessageBox("Error making remote connection: " + ex.getMessage()) ;
		}
	}
	
	private void remoteDisconnectPerformed(ActionEvent e)
	{
		m_Document.remoteDisconnect() ;
	}


}

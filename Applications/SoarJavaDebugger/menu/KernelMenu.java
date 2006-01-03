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

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Menu;

import sml.Agent;
import sml.Kernel;

import debugger.MainFrame;
import dialogs.KernelLocationDialog;
import dialogs.RemoteDialog;
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

	private AbstractAction m_StartKernel      = new AbstractAction("Create new &local Soar Kernel instance")	{ public void actionPerformed(ActionEvent e) { startKernelPerformed(e) ; } } ;
	private AbstractAction m_StopKernel       = new AbstractAction("Delete local &Soar Kernel instance") 	{ public void actionPerformed(ActionEvent e) { stopKernelPerformed(e) ; } } ;
	private AbstractAction m_RemoteConnect 	  = new AbstractAction("Connect to &remote Soar...") 			{ public void actionPerformed(ActionEvent e) { remoteConnectPerformed(e) ; } } ;
	private AbstractAction m_RemoteDisconnect = new AbstractAction("&Disconnect from remote Soar") 			{ public void actionPerformed(ActionEvent e) { remoteDisconnectPerformed(e) ; } } ;
	private AbstractAction m_KernelLocation   = new AbstractAction("Set the &path to the Kernel...") 			{ public void actionPerformed(ActionEvent e) { setKernelLocation() ; } } ;

	/** Create this menu */
	public static KernelMenu createMenu(MainFrame frame, Document doc, String title)
	{
		KernelMenu menu = new KernelMenu() ;
		menu.m_Frame    = frame ;
		menu.m_Document = doc ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title)
	{
		BaseMenu menu = new BaseMenu(parent, title) ;
		
		menu.add(m_RemoteConnect) ;
		menu.add(m_RemoteDisconnect) ;
		menu.addSeparator() ;
		menu.add(m_StartKernel) ;
		menu.add(m_StopKernel) ;
		menu.addSeparator() ;
		menu.add(m_KernelLocation) ;

		updateMenu() ;
		
		return menu ;		
	}
	
	/** Enable/disable the menu items depending on the current state of the kernel */
	public void updateMenu()
	{
		m_StartKernel.setEnabled(!m_Document.isConnected()) ;
		m_StopKernel.setEnabled(m_Document.isConnected() && !m_Document.isRemote()) ;
		m_RemoteConnect.setEnabled(!(m_Document.isConnected() && m_Document.isRemote())) ;
		m_RemoteDisconnect.setEnabled(m_Document.isConnected() && m_Document.isRemote()) ;
	}

	public String setKernelLocation()
	{
		String path = KernelLocationDialog.showDialog(m_Frame, "Set the path to the Soar kernel") ;
		return path ;
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
		// Stop the kernel but don't close all of our windows
		// (presumably the user is about to connect somewhere else -- or they'd
		//  have just closed down the app instead).
		m_Document.stopLocalKernel(false, true) ;
	}
	
	private void remoteConnectPerformed(ActionEvent e)
	{
		if (m_Document.isConnected())
		{
			int result = m_Frame.ShowMessageBox("Kernel running", "You are currently running a local Soar kernel.\nThis needs to be shutdown before connecting to a remote Soar.\n\nWould you like to shutdown the local kernel now?", SWT.ICON_WARNING | SWT.OK | SWT.CANCEL) ;
			
			if (result == SWT.CANCEL)
				return ;
			
			// Shut down the local kernel and then go on
			// (Keep any existing windows open)
			m_Document.stopLocalKernel(false, true) ;
		}
		
		RemoteDialog.RemoteInfo ip = RemoteDialog.showDialog(m_Frame, "Remote Soar Kernel Connection") ;
		
		// Check if the user cancelled
		if (ip == null)
			return ;
		
		try
		{
			m_Document.remoteConnect(ip.getIP(), ip.getPort(), null) ;
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

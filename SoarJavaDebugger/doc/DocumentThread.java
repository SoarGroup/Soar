/********************************************************************************************
*
* DocumentThread.java
* 
* Description:	
* 
* Created on 	Feb 21, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc;

import java.util.ArrayList;

import sml.Agent;

/************************************************************************
 * 
 * Executing Soar commands can take a long time (e.g. "run 1000").
 * 
 * If we execute them in the main UI thread then the UI will be locked until
 * Soar returns control to us (menus can't be selected etc.).
 * 
 * One solution to this is to execute the Soar commands in a separate thread
 * within the debugger.  This class does just that, queueing up commands
 * and executing them.
 * 
 ************************************************************************/
public class DocumentThread extends Thread
{
	// BUGBUG: There's a chance that the agent object is invalid
	// by the time we go to execute the command.  We need a way to detect that
	// the C++ object is no good before we try to run the command.
	private static class Command
	{
		private Agent m_Agent ;
		private String m_Command ;
		
		public Command(Agent agent, String command)
		{
			m_Agent = agent ;
			m_Command = command ;
		}
	}
	
	/** The commands waiting to be executed */
	private ArrayList m_CommandQueue = new ArrayList() ;
	
	/** A flag used when we wish to stop this thread (during system shutdown) */
	private boolean   m_AskedToStop = false ;
	
	/** The main document (which owns the Soar kernel object etc.) */
	private Document  m_Document = null ;
	
	public DocumentThread(Document doc)
	{
		m_Document = doc ;
	}
	
	/** Ask this thread to halt */
	public synchronized void askToStop()
	{
		m_AskedToStop = true ;
	}
	
	/** Schedule a command to execute later */
	public synchronized void scheduleCommandToExecute(Agent agent, String command)
	{
		m_CommandQueue.add(new Command(agent, command)) ;
	}
	
	/** Get the next command from the queue.  Returns null if there are no commands */
	private synchronized Command popNextCommand()
	{
		if (m_CommandQueue.size() == 0)
			return null ;
		
		Command command = (Command)m_CommandQueue.get(0) ;
		m_CommandQueue.remove(0) ;

		return command ;
	}
	
	public void run()
	{
		while (!m_AskedToStop)
		{
			Command command ;
			while ( (command = popNextCommand()) != null )
			{
				String result = command.m_Agent.ExecuteCommandLine(command.m_Command) ;
			}
		}
	}
}
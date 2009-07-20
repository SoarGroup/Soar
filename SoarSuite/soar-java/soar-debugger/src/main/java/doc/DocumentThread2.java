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

import sml.* ;

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
public class DocumentThread2 extends Thread
{
	/**
	 * Calls to the kernel are represented as generalized "Command" objects.
	 * 
	 * Each derived class overrides the "execute" method, issues a command to the kernel
	 * and then stores the result of that command in the m_Result variable.
	 * 
	 * m_Result is a generic Object so any value can be returned by derived classes.
	 * 
	 * @author Doug
	 *
	 */
	public abstract static class Command
	{
		protected Agent  m_Agent ;
		protected Object m_Result ;
		protected boolean m_Executed ;
		
		public Command(Agent agent)
		{
			m_Agent = agent ;
			m_Executed = false ;
		}

		/**
		 * Execute method is called from the DocumentThread when this command reaches the top of the queue.
		 * It MUST call recordResult() to report the result of the command and to signal that the command has completed.
		 */
		protected abstract void execute() ;

		public Agent getAgent() { return m_Agent ; }
		
		protected synchronized void recordResult(Object result)
		{
			m_Result = result ;
			m_Executed = true ;
		}
		
		public synchronized Object getResult()
		{
			if (!m_Executed)
				throw new IllegalStateException("Command has not been executed yet -- should check isExecutedCommand first") ;
			
			return m_Result ;
		}
	}
	
	public static class CommandExecCommandLine extends Command
	{
		protected String m_Command ;
		
		public CommandExecCommandLine(Agent agent, String command)
		{
			super(agent) ;
			m_Command = command ;
		}
		
		protected void execute() {
			// If there's no XML object to receive the response we just execute and get the string form.
			String result = m_Agent.ExecuteCommandLine(this.m_Command) ;
			recordResult(result) ;
		}
		
		public String toString() { return m_Command ; }
	}
	
	public static class CommandExecCommandLineXML extends Command
	{
		protected ClientAnalyzedXML m_Response ;
		protected String m_Command ;
		
		public CommandExecCommandLineXML(Agent agent, String command, ClientAnalyzedXML response)
		{
			super(agent) ;
			m_Command = command ;
			m_Response = response ;
		}
		
		protected void execute() {
			// If we were provided an XML object for the response, record the result in that object
			boolean success = m_Agent.ExecuteCommandLineXML(m_Command, m_Response) ;
			recordResult(success ? "true" : "false") ;
		}		

		public String toString() { return m_Command ; }
	}
	
	/** The commands waiting to be executed */
	private ArrayList m_ToExecuteQueue = new ArrayList() ;
	
	/** A flag used when we wish to stop this thread (during system shutdown) */
	private boolean   m_AskedToStop = false ;
	
	/** The main document (which owns the Soar kernel object etc.) */
	private Document  m_Document = null ;
	
	private boolean	  m_IsExecutingCommand = false ;
	
	/** True when connected to a kernel (i.e. able to execute commands */
	private boolean   m_IsConnected = false ;
	
	/** If true, print trace information as each command is scheduled and executed */
	private static boolean kTraceCommands = false ;
	
	public DocumentThread2(Document doc)
	{
		m_Document = doc ;
	}
	
	/** Ask this thread to halt */
	public synchronized void askToStop()
	{
		m_AskedToStop = true ;
	}

	/** Returns true if we're actively executing a command or are just about to */
	public synchronized boolean isBusy()
	{
		return m_IsExecutingCommand || m_ToExecuteQueue.size() > 0 ;
	}
		
	public synchronized void setExecuting(boolean state)
	{
		m_IsExecutingCommand = state ;
	}
	
	/** Schedule a command to execute later */
	public synchronized void scheduleCommandToExecute(Command command) {
		if (kTraceCommands)
			System.out.println("Scheduling " + command) ;
		
		m_ToExecuteQueue.add(command) ;
	}
	
	/** Get the next command from the queue.  Returns null if there are no commands */
	private synchronized Command popNextCommand()
	{
		if (m_ToExecuteQueue.size() == 0)
			return null ;
		
		Command command = (Command)m_ToExecuteQueue.get(0) ;
		m_ToExecuteQueue.remove(0) ;

		return command ;
	}

	/** Indicates whether currently connected to a Soar kernel (so can execute commands) */
	public boolean isConnected() 				{ return m_IsConnected ; }
	public void setConnected(boolean state)		{ m_IsConnected = state ; }
	
	public synchronized boolean isExecutedCommand(Command command)
	{
		return command.m_Executed ;
	}	

	public synchronized Object getExecutedCommandResult(Command command)
	{
		if (!command.m_Executed)
			throw new IllegalStateException("Command has not been executed yet -- should check isExecutedCommand first") ;
		
		return command.m_Result ;
	}
	

	// This function pulls waiting commands off the queue and executes them in Soar.
	// We expose this so that we can call it during the execution of a run to check for "stop-soar" commands.
	// It's not synchronized because we may be in here executing a "run" and then call back into the same
	// method to issue "stop-soar".  Instead the methods to work with the queue (e.g. popNextCommand) are synchronized.
	public void executePending(String source)
	{
		Command command ;
		while ( (command = popNextCommand()) != null )
		{
			setExecuting(true) ;
			
			// Check that we still have a kernel to work with
			if (!isConnected())
			{
				command.recordResult("disconnected") ;
				continue ;
			}
			
			// The agent may have been destroyed before we get around to executing
			// this command.
			if (!this.m_Document.isAgentValid(command.m_Agent))
			{
				command.recordResult("agent destroyed") ;
				continue ;
			}
			
			if (kTraceCommands)
				System.out.println("About to execute " + command + " from " + source) ;

			// Execute the command.  Note -- this call is not synchronized as it may be a call to "run" and we need to be able
			// to come in and interrupt that call with another later call.
			command.execute() ;

			if (kTraceCommands)
			{
				System.out.println("Finished " + command + " from " + source) ;
			}
		}
		
		setExecuting(false) ;
	}
	
	public void run()
	{
		while (!m_AskedToStop)
		{
			executePending("main") ;
			
			// The pause as we sleep here is just how quickly we respond to incoming commands from the user.
			// It won't affect the speed Soar executes.
			try { Thread.sleep(10) ; } catch (InterruptedException e) { } 
		}
	}
}
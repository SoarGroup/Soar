/********************************************************************************************
*
* MainDoc.java
* 
* Created on 	Nov 10, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc;

import doc.events.*;
import sml.* ;
import debugger.* ;
import general.AppProperties;
import manager.* ;
import modules.TraceView;
import modules.TreeTraceView;

import java.io.IOException;
import java.util.*;

import org.eclipse.swt.widgets.Display;

/********************************************************************************************
* 
* This class represents the soar process.
* 
* Debugger windows register here for interesting events sent to/from the soar process.
* 
********************************************************************************************/
public class Document
{
	/** The properties for this application (holds user preferences) */
	protected AppProperties m_AppProperties = new AppProperties("SoarDebugger.ini", "Soar Debugger Settings") ;

	/** The list of all modules (types of debugger windows) that exist.  This is a list of types, not a list of window instances. */
	private ModuleList m_ModuleList = new ModuleList() ;
	
	private SoarChangeGenerator m_SoarChangeGenerator = new SoarChangeGenerator() ;

	/** This object is used to get strings for Soar commands in a version independent way */
	private SoarCommands		m_SoarCommands = new SoarCommands(this, 8,6,0) ;

	/** Stores the pointer to the Soar kernel we are currently interacting with (can be local to this process or remote) */
	private Kernel				m_Kernel = null ;
		
	/** True if we are working with a remote kernel (not running inside our editor but in another process) */
	private boolean				m_IsRemote = false ;
	
	/** The list of all main frames in the application -- each frame has a menu bar */
	private FrameList			m_FrameList = new FrameList() ;
	
	/** We have to pump messages on this display to keep the app alive */
	private Display				m_Display = null ;
	
	/** There is only one document in the debugger.  If you want to work with another Soar process, start another debugger */
	//private static Document		s_Document ;
	
	/**
	 * We have to decide how to keep the UI thread responsive while Soar is running (possibly for a long time).
	 * There are 2 approaches.
	 * 1) We run Soar in its own thread (using the DocumentThread object).
	 * This makes the coding of the modules in the debugger more complex because each callback has to be re-routed back to the UI thread
	 * carefully if it wants to update the interface, but it does allow Soar to run at full speed no matter what the user is doing in the UI.
	 * 2) We run Soar in the UI thread and register for an event every decision cycle.  When that fires we check for UI events.
	 * This makes the coding of the modules in the debugger much simpler, but it means if the user does something like pull down a menu
	 * (or do some other blocking action in the UI) it will cause Soar to pause because it's blocked in our thread.
	 * 
	 * At this point I don't think either method clearly dominates so I'm keeping both around while I play with them.
	 * We can certainly reverse this choice later if we wish.
	 */
	public static final boolean kDocInOwnThread = true ;
	private DocumentThread 		m_DocumentThread = null ;
	
	public Document()
	{
		//Document.s_Document = this ;
		
		// Load the user's preferences from the properties file.
		// If we change the version number, it will cause all existing preferences
		// in the field to be ignored--useful if there might be a conflict from old settings.
		String version = "1.0" ;
		try
		{
			this.m_AppProperties.Load(version) ;
		} catch (IOException e)
		{
			e.printStackTrace();
		}
		
		if (this.kDocInOwnThread)
		{
			m_DocumentThread = new DocumentThread(this) ;
			m_DocumentThread.start() ;
		}
		
		// BUGBUG: The name and description should come from the classes.
		// The list should come from scanning the drive for classes in the modules folder and pulling them in.
		Module combo1 = new Module("Trace View", "Commands are entered at a prompt.  Output from the commands and trace output from runs is shown in a text window.", modules.TraceView.class) ;
		Module combo2 = new Module("Auto Update View", "The user's command is automatically executed at the end of each run.", modules.UpdateCommandView.class) ;
		Module combo3 = new Module("Keep View", "Commands are entered at a prompt and the results are displayed in a scrolling text window.  Trace output from runs is not shown.", modules.KeepCommandView.class) ;
		Module combo4 = new Module("Button Bar", "A collection of user-customizable buttons", modules.ButtonView.class) ;
		Module tree   = new Module("Tree View", "Commands are entered at a prompt and output is displayed in a tree, providing a hierarchical view of the output", TreeTraceView.class) ;

		m_ModuleList.add(combo1) ;
		m_ModuleList.add(combo2) ;
		m_ModuleList.add(combo3) ;
		m_ModuleList.add(combo4) ;
		m_ModuleList.add(tree) ;
	}
	
	/** Returns the single document instance for this debugger.  If possible pass a document pointer around instead of using this backdoor */
	//public static Document getMainDocument() { return s_Document ; }
	
	/** Gives us a frame to work with */
	public MainFrame getFirstFrame() 		 { return m_FrameList.get(0) ; }
	
	public AppProperties getAppProperties()	 { return m_AppProperties ; }
	
	public Agent	getAgentByIndex(int index) 	{ return m_Kernel.GetAgentByIndex(index) ; }
	public int		getNumberAgents() 			{ return m_Kernel.GetNumberAgents() ; }

	public Agent[]	getAgentArray()
	{
		Agent[] agents = new Agent[getNumberAgents()] ;
		
		for (int i = 0 ; i < agents.length ; i++)
		{
			agents[i] = getAgentByIndex(i) ;
		}
		
		return agents ;
	}

	public String[]	getAgentNameArray()
	{
		String[] agents = new String[getNumberAgents()] ;
		
		for (int i = 0 ; i < agents.length ; i++)
		{
			agents[i] = getAgentByIndex(i).GetAgentName() ;
		}
		
		return agents ;
	}
	
	public boolean isAgentValid(Agent agent)
	{
		if (m_Kernel == null)
			return false ;
		
		return m_Kernel.IsAgentValid(agent) ;
	}

	/********************************************************************************************
	 * 
	 * Note: This method is NOT currently going to return the same Agent object on repeated calls for the same agent name.
	 * This means you cannot test equality with "agent1 == agent2" or "agent1.equals(agent2)"
	 * So for now I'm including the isSameAgent method below.
	 * 
	 * BADBAD: Later we may wish to cache the agent objects locally and always return
	 * the same java object here (and from getAgentByIndex()).
	 * 
	 * @param name
	 * @return
	********************************************************************************************/
	public Agent	getAgent(String name)		{ return m_Kernel.GetAgent(name) ; }
		
	public static boolean isSameAgent(Agent a, Agent b)
	{
		if (a == null || b == null)
			return false ;
		
		return a.GetAgentName().equals(b.GetAgentName()) ;
	}
	
	public boolean isRunCommand(String command)
	{
		return m_Kernel.IsRunCommand(command) ;
	}
	
	/********************************************************************************************
	 * 
	 * We maintain a list of MainFrames in the debugger.  A MainFrame has a menu bar and children.
	 * 
	********************************************************************************************/
	public String addFrame(MainFrame frame)
	{
		// Generate a unique name for this frame (not used by any other frames)
		String baseName = "mainframe" ;
		String name = null ;

		for (int i = m_FrameList.size()+1 ; i < 200 && name == null ; i++)
		{
			String candidate = baseName + i ;

			// Just do a linear search of the frame list -- it'll be small
			if (!m_FrameList.isNameInUse(candidate))
				name = candidate ;
		}

		m_FrameList.add(frame) ;

		// Returns the name to use for this frame
		return name ;
	}
	
	public MainFrame getFrameByName(String frameName)
	{
		return m_FrameList.find(frameName) ;
	}
	
	public void removeFrame(MainFrame frame)
	{
		m_FrameList.remove(frame) ;
	}
	
	public int getNumberFrames()
	{
		return m_FrameList.size() ;
	}
	
	/********************************************************************************************
	 * 
	 * Shutdown.  We generally only call this when we're about to terminate the debugger.
	 * 
	********************************************************************************************/
	public void close()
	{
		// Close down any open kernel instances cleanly
		if (isConnected())
		{
			if (isRemote())
			{
				this.remoteDisconnect() ;
			}
			else
			{
				this.stopLocalKernel() ;
			}
		}
	}
		
	protected void registerStandardKernelEvents()
	{
		// Added this just for testing
//		int jSystemStartCallback = m_Kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, this, "systemEventHandler", this) ;

		int jAgentCreatedCallback = m_Kernel.RegisterForAgentEvent(smlAgentEventId.smlEVENT_AFTER_AGENT_CREATED, this, "agentEventHandler", this) ;
		int jAgentDestroyedCallback = m_Kernel.RegisterForAgentEvent(smlAgentEventId.smlEVENT_BEFORE_AGENT_DESTROYED, this, "agentEventHandler", this) ;
		
		// Register for an event that happens once each time agents are run a step to give me a chance to check for interruptions
		// and pump the UI thread along a bit.
		int jAgentsRunningCallback = m_Kernel.RegisterForAgentEvent(smlAgentEventId.smlEVENT_BEFORE_AGENTS_RUN_STEP, this, "agentEventHandler", this) ;
	}
	
	/********************************************************************************************
	 * 
	 * Start an instance of Soar running in the debugger and create a first agent.
	 * 
	 * @param portToListenOn	This port can be used by other external clients to send commands to
	 * 							the kernel running inside this debugger.  (Probably rare to do this).
	 * @return Returns the newly created agent.
	********************************************************************************************/
	public Agent startLocalKernel(int portToListenOn)
	{
		if (m_Kernel != null)
		{
			// For now I'll throw.  We could also just return true that we're already running
			// or call stopKernel() and then create a new one.  All should be fine.
			throw new IllegalStateException("We've already started the kernel") ;
		}

		// Create the kernel
		// We're use "InNewThread" so environments can remotely connect to the debugger and run
		// inside us if they wish (and without us having to do work to support that by calling
		// GetIncomingCommands() and pumping for incoming events).
		m_Kernel = Kernel.CreateKernelInNewThread("KernelSML", portToListenOn) ;

		// This is a local kernel
		m_IsRemote = false ;
		
		if (m_Kernel.HadError())
		{
			// Get the error and then kill off this kernel instance
			String errorMsg = m_Kernel.GetLastErrorDescription() ;
			m_Kernel.delete() ;
			m_Kernel = null ;
			
			throw new IllegalStateException("Error initializing kernel " + errorMsg) ;
		}

		// Let our listeners know about the new kernel
		fireSoarConnectionChanged() ;
		
		// Register for agent additions before we create our first agent
		registerStandardKernelEvents() ;
		
		// Start with an agent...without this a kernel's not much use.
		Agent agent = createAgent("debug-agent") ;
		
		return agent ;
	}

	public Agent createAgent(String agentName)
	{
		Agent agent = m_Kernel.CreateAgent(agentName) ;

		// Agent creation is unusual in that we get the errors in the kernel
		// Once the agent is created errors will be attached to that agent.
		if (m_Kernel.HadError())
			throw new IllegalStateException("Error initializing agent " + m_Kernel.GetLastErrorDescription()) ;
		
		return agent ;
	}
	
	public void destroyAgent(Agent agent)
	{
		m_Kernel.DestroyAgent(agent) ;
	}
	
	/********************************************************************************************
	 * 
	 * Shutdown our local Soar instance, deleting any agents etc.
	 * 
	********************************************************************************************/
	public boolean stopLocalKernel()
	{
		if (m_Kernel == null)
			return false ;

		// Make sure we're stopped
		if (this.kDocInOwnThread)
		{
			if (m_DocumentThread.isBusy())
			{
				if (this.getFirstFrame() != null)
					this.getFirstFrame().ShowMessageBox("Soar is currently executing.  Need to stop it first before deleting the kernel.") ;

				return false ;
			}
		}
		
		m_Kernel.delete() ;
		
		m_Kernel = null ;
		
		// Let our listeners know we killed the kernel
		fireSoarConnectionChanged() ;
		
		return true ;
	}

	/********************************************************************************************
	 * 
	 * Start an instance of Soar running in the debugger and create a first agent.
	 * 
	 * @param ipAddress		The ip address (e.g. "205.233.102.121" to connect to.  null => local ip)
	 * @param portNumber	The port to connect on.
	********************************************************************************************/
	public void remoteConnect(String ipAddress, int portNumber) throws Exception
	{
		// If we have a local kernel already running shut it down
		stopLocalKernel() ;

		m_IsRemote = true ;
		m_Kernel = Kernel.CreateRemoteConnection(true, ipAddress, portNumber) ;

		if (m_Kernel.HadError())
		{
			// Get the error and then kill off this kernel instance
			String errorMsg = m_Kernel.GetLastErrorDescription() ;
			m_Kernel.delete() ;
			m_Kernel = null ;
			
			throw new Exception("Error initializing kernel " + errorMsg) ;
		}

		// If we want to debug the communications this is helpful
		// m_Kernel.SetTraceCommunications(true) ;

		// Let our listeners know about the new kernel
		fireSoarConnectionChanged() ;

		// Register for agent additions etc.
		registerStandardKernelEvents() ;

		// Let everyone know that the list of agents has changed (because we've just connected to a new kernel)
		fireAgentListChanged() ;

		// Go through the current list of frames and assign an agent to each in turn
		int nAgents = this.getNumberAgents() ;
		int agentID = 0 ;
		
		for (int frameID = 0 ; frameID < this.m_FrameList.size() ; frameID++)
		{
			MainFrame frame = m_FrameList.get(frameID) ;
			
			if (agentID < nAgents)
			{
				Agent agent = this.getAgentByIndex(agentID) ;
				frame.setAgentFocus(agent) ;
				agentID++ ;
			}
			else
			{
				// Make sure the focus is set to nothing (not some left over agent)
				frame.setAgentFocus(null) ;
			}
		}
	}
	
	/********************************************************************************************
	 * 
	 * Disconnect from the remote kernel we were working with.
	 * This does not shutdown that kernel just stops us sending commands to and from it.
	 * 
	********************************************************************************************/
	public boolean remoteDisconnect()
	{
		if (m_Kernel == null)
			return false ;

		// NOTE: We don't check whether Soar is still running before allowing a remote disconnect
		// (unlike in the localStop call).  This is because setting an agent running and then disconnecting
		// should be OK.  We may need to handle some error conditions that occur when we do this
		// but by and large it should be acceptable and it would be good to make it work.
		
		m_Kernel.delete() ;
		
		m_Kernel = null ;
		
		// Let our listeners know we disconnected
		fireSoarConnectionChanged() ;
		
		return true ;
	}
	
	/********************************************************************************************
	 * 
	 * Returns true if we have some form of Soar instance running (local or remote).
	 * 
	********************************************************************************************/
	public boolean isConnected()
	{
		return m_Kernel != null ;
	}
	
	/********************************************************************************************
	 * 
	 * Returns true if the current connection is a remote connection.
	 * (Note: You need to check isConnected() is true as well)
	 * 
	********************************************************************************************/
	public boolean isRemote()
	{
		return m_IsRemote ;
	}

	public void systemEventHandler(int eventID, Object data, Kernel kernel)
	{
		System.out.println("Received system event") ;
	}
	
	public void agentEventHandler(int eventID, Object data, String agentName)
	{
		Agent agent = getAgent(agentName) ;

		if (eventID == smlAgentEventId.smlEVENT_BEFORE_AGENTS_RUN_STEP.swigValue())
		{
			if (!kDocInOwnThread)
			{
				// If we're running Soar in the UI thread we need to pump messages (for the OS) to allow
				// the UI to update at all and do things like handle mouse clicks intended to stop soar.
				pumpMessagesOneStep() ;
			}
			else
			{
				// If we're running Soar in its own thread, we need to check for interruption messages (stop-soar)
				// while Soar is running, otherwise the call to "run" blocks forever and we can't stop it.
				m_DocumentThread.executePending() ;
			}
				
			return ;
		}
		
		if (agent == null)
			throw new IllegalStateException("Error in agent event handler -- got event for agent that hasn't been created yet in ClientSML") ;

		if (eventID == smlAgentEventId.smlEVENT_AFTER_AGENT_CREATED.swigValue())
			this.fireAgentAdded(agent) ;
		if (eventID == smlAgentEventId.smlEVENT_BEFORE_AGENT_DESTROYED.swigValue())
			this.fireAgentRemoved(agent) ;
	}

	public ModuleList 	getModuleList() 	{ return m_ModuleList ; }
	
	public SoarCommands	getSoarCommands()	{ return m_SoarCommands ; }
	
	// Pump messages continuously until the window is closed
	public void pumpMessagesTillClosed(org.eclipse.swt.widgets.Display display)
	{
		m_Display = display ;
		
		if (getNumberFrames() == 0)
			return ;
		
		while (getNumberFrames() > 0)
		{
			if (!m_Display.readAndDispatch())
			{
				m_Display.sleep() ;
			}
		}
	}
	
	// Pump messages one step 
	public void pumpMessagesOneStep()
	{
		if (getNumberFrames() == 0 || m_Display == null)
			return ;
		
		// Pump any waiting messages
		while (m_Display.readAndDispatch()) { }
	}
	
	/** Register for all events from Soar */
	public synchronized void addSoarChangeListener(SoarChangeListener listener) 	{ m_SoarChangeGenerator.addSoarChangeListener(listener); }
	public synchronized void removeSoarChangeListener(SoarChangeListener listener)	{ m_SoarChangeGenerator.removeSoarChangeListener(listener); }

	protected synchronized void fireSoarConnectionChanged()							{ m_SoarChangeGenerator.fireSoarConnectionChanged(this, isConnected(), isRemote()) ; }
	protected synchronized void fireAgentAdded(Agent agent)							{ m_SoarChangeGenerator.fireAgentListChanged(this, SoarAgentEvent.kAgentAdded, agent) ; }
	protected synchronized void fireAgentRemoved(Agent agent)						{ m_SoarChangeGenerator.fireAgentListChanged(this, SoarAgentEvent.kAgentRemoved, agent) ; }
	
	// More generic than agent added/removed.  Use the more specific form if you can.
	protected synchronized void fireAgentListChanged()								{ m_SoarChangeGenerator.fireAgentListChanged(this, SoarAgentEvent.kListChanged, null) ; }

	public String sendAgentCommand(Agent agent, String commandLine)
	{
		if (agent == null)
			return "Error: Agent is missing.  Need to create one before executing commands" ;

		if (this.kDocInOwnThread)
		{
			// If Soar commands run in their own thread we issue the command and then pump the UI thread for messages
			// while waiting for a response.  We do this so that folks calling here can see this as a synchronous call
			// when really it's asynchronous.
			// We also need to pump the document thread in response to an "agents_run_step" event to allow interruptions
			// so that a Soar "run" command can be interrupted.
			DocumentThread.Command command = m_DocumentThread.scheduleCommandToExecute(agent, commandLine) ;
			
			while (!m_DocumentThread.isExecutedCommand(command))
			{
				pumpMessagesOneStep() ;
				
				// The pause as we sleep here is just how quickly we respond to commands finishing (or UI events).
				// It won't affect the speed Soar executes.
				try { Thread.sleep(10) ; } catch (InterruptedException e) { } 
			}

			String result = m_DocumentThread.getExecutedCommandResult(command) ;
			return result ;
		}
		else
		{
			// If we're running commands in the UI thread we can just execute it.
			// If it's a run we need to pump messages in response to an "agents_run_step" event
			// to keep the UI alive and allow folks to interrupt Soar.
			String response = agent.ExecuteCommandLine(commandLine) ;			
			return response ;
		}
	}
}

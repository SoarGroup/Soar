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
import modules.*;

import java.io.IOException;

import org.eclipse.swt.widgets.Display;

/********************************************************************************************
* 
* This class represents the soar process.
* 
* Debugger windows register here for interesting events sent to/from the soar process.
* 
********************************************************************************************/
public class Document implements Kernel.AgentEventInterface, Kernel.SystemEventInterface
{
	public static final String kCreateNewWindowProperty = "Agent.CreateNewWindow" ;
	public static final String kCloseOnDestroyProperty  = "Agent.CloseOnDestroy" ;
	private static final String kConnectionName = "java-debugger" ;
	
	/** This version is used to name the settings files uniquely, so there's no collisions if you use an older debugger.  Should be bumped with every release (and add the current to the end of kPrevVersion) */
	public static final String kVersion = "8_6_2" ;
	
	/** This list of versions will be checked, in order from first to last, when looking for settings to copy.  This only comes into play on the first launch of a new version of the debugger */
	/** There's no need to have this list get too long--3 versions should be plenty.  If we change the layout/properties in a way that's not backwards compatible, need to clear this list. **/
	public static final String[] kPrevVersions = new String[] { "8_6_1" } ;

	/** This object is used to get strings for Soar commands in a version independent way */
	private SoarCommands		m_SoarCommands = new SoarCommands(this, 8,6,2) ;

	/** The properties for this application (holds user preferences).  Version specific with debugger releases (or using an older version of debugger could conflict) */
	protected AppProperties m_AppProperties ;

	/** The list of all modules (types of debugger windows) that exist.  This is a list of types, not a list of window instances. */
	private ModuleList m_ModuleList = new ModuleList() ;
	
	private SoarChangeGenerator m_SoarChangeGenerator = new SoarChangeGenerator() ;

	/** Stores the pointer to the Soar kernel we are currently interacting with (can be local to this process or remote) */
	private Kernel				m_Kernel = null ;
		
	/** True if we are working with a remote kernel (not running inside our editor but in another process) */
	private boolean				m_IsRemote = false ;

	/** True if we are shutting down the current connection */
	private boolean				m_IsStopping = false ;

	/** The list of all main frames in the application -- each frame has a menu bar */
	private FrameList			m_FrameList = new FrameList() ;
	
	/** We have to pump messages on this display to keep the app alive */
	private Display				m_Display = null ;
	
	/** The user can choose to create a new window whenever an agent is created.  This makes life complicated for us so we can suppress it temporarily in the code when
	 *  we are making the new agents.  (It's really to support other folks, like the environment creating a new agent).  This variable is the name of the agent. */
	private String 			m_SuppressNewWindowCreation ;
	
	/** The user can choose to destroy a window whenever the agent is destroyed.  This makes life complicated for us so we can suppress it temporarily in the code when
	 *  we are switching from local to remote kernels etc.  (It's really to support other folks, like the environment destroying an agent). */
	private boolean			m_SuppressCloseWindowOnDestructionAllAgents = false ;

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
	
	public static final boolean kShutdownInSeparateThread = true ;
	
	public static String getPropertyFileName(String version) { return "SoarDebugger" + version + ".ini" ; }
	
	
	
	public Document()
	{
		// Load the user's preferences from the properties file.
		// If we change the version number, it will cause all existing preferences
		// in the field to be ignored--useful if there might be a conflict from old settings.
		String version = "1.0" ;
		try
		{
			m_AppProperties = new AppProperties(getPropertyFileName(kVersion), "Soar Debugger Settings") ;
			boolean found = this.m_AppProperties.Load(version) ;
			
			if (!found)
			{
				// If no properties exist, try to load earlier versions
				for (int i = 0 ; !found && i < kPrevVersions.length ; i++)
				{
					m_AppProperties = new AppProperties(getPropertyFileName(kPrevVersions[i]), "Soar Debugger Settings") ;
					found = m_AppProperties.Load(version) ;
				}

				// Whether we succeeded with loading an earlier version or not we need to use the new version when we save
				// so switch the filename now.
				m_AppProperties.setFilename(getPropertyFileName(kVersion)) ;
					
			}
		} catch (IOException e)
		{
			e.printStackTrace();
		}
		
		if (Document.kDocInOwnThread)
		{
			m_DocumentThread = new DocumentThread(this) ;
			m_DocumentThread.start() ;
		}
		
		// BUGBUG: The name and description should come from the classes.
		// The list should come from scanning the drive for classes in the modules folder and pulling them in.
		Module textTrace = new Module("Text Trace Window", "Commands are entered at a prompt.  Output from the commands and trace output from runs is shown in a text window.", modules.TextTraceView.class) ;
		Module update 	 = new Module("Auto Update Window", "The user's command is automatically executed at the end of each run.", modules.UpdateCommandView.class) ;
		Module keep 	 = new Module("Keep Window", "Commands are entered at a prompt and the results are displayed in a scrolling text window.  Trace output from runs is not shown.", modules.KeepCommandView.class) ;
		Module button 	 = new Module("Button Bar", "A collection of user-customizable buttons", modules.ButtonView.class) ;
		Module phase 	 = new Module("Phase Diagram", "A view that shows Soar's phases of execution", modules.PhaseView.class) ;
		Module edit 	 = new Module("Edit Production Window", "A window used to edit a production and then load it into Soar", modules.EditorView.class) ;
		Module fold 	 = new Module("Tree Trace Window", "Output from commands and trace output from runs is shown in a tree window.", FoldingTextView.class) ;

		m_ModuleList.add(fold) ;
		m_ModuleList.add(textTrace) ;
		m_ModuleList.add(update) ;
		m_ModuleList.add(keep) ;
		m_ModuleList.add(button) ;
		m_ModuleList.add(phase) ;
		m_ModuleList.add(edit) ;
	}
	
	/** Gives us a frame to work with */
	public MainFrame getFirstFrame() 		 { return m_FrameList.get(0) ; }
	
	public AppProperties getAppProperties()	 { return m_AppProperties ; }
	
	public Agent	getAgentByIndex(int index) 	{ return m_Kernel.GetAgentByIndex(index) ; }
	public int		getNumberAgents() 			{ return m_Kernel.GetNumberAgents() ; }
	
	public boolean isCloseWindowWhenDestroyAgent()
	{
		return getAppProperties().getAppBooleanProperty(Document.kCloseOnDestroyProperty, true) && !m_SuppressCloseWindowOnDestructionAllAgents ;		
	}

	private void setSuppressCloseWindow(boolean state)
	{
		m_SuppressCloseWindowOnDestructionAllAgents = state ;
	}
	
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
	
	public String getExpandedCommandLine(String commandLine)
	{
		if (m_Kernel == null)
			return commandLine ;
				
		return m_Kernel.ExpandCommandLine(commandLine) ;
	}
	
	public boolean isProductionLoaded(Agent agent, String productionName)
	{
		if (agent == null || !this.isConnected())
			return false ;
		
		return agent.IsProductionLoaded(productionName) ;
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
	public void close(boolean closingApp)
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
				// Stop the local kernel -- keeping windows open as we will
				// shut them down later in code.  That allows us to save other
				// information (like window positions etc) in a controlled manner.
				this.stopLocalKernel(closingApp, true) ;
			}
		}
	}
		
	protected void registerStandardKernelEvents()
	{
		// Added this just for testing
//		int jSystemStartCallback = m_Kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, this, "systemEventHandler", this) ;

		m_Kernel.RegisterForAgentEvent(smlAgentEventId.smlEVENT_AFTER_AGENT_CREATED, this, this) ;
		m_Kernel.RegisterForAgentEvent(smlAgentEventId.smlEVENT_BEFORE_AGENT_DESTROYED, this, this) ;
		
		// Register for an event that happens once each time agents are run a step to give me a chance to check for interruptions
		// and pump the UI thread along a bit.
		m_Kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_INTERRUPT_CHECK, this, this) ;
		
		m_Kernel.SetInterruptCheckRate(50) ;
	}

	/********************************************************************************************
	 * 
	 * Start an instance of Soar running in the debugger and create a first agent.
	 * This version uses all default settings.
	 * You can also call the version below directly which offers more control.
	 * 
	 * @return Returns the newly created agent.
	********************************************************************************************/
	public Agent startLocalKernel()
	{
		return startLocalKernel(Kernel.GetDefaultPort(), null, null) ;
	}
	
	/********************************************************************************************
	 * 
	 * Start an instance of Soar running in the debugger and create a first agent.
	 * 
	 * @param portToListenOn	This port can be used by other external clients to send commands to
	 * 							the kernel running inside this debugger.  (Probably rare to do this).
	 * @param agentName			Can pass null to get default name.
	 * @param sourcePath		Path to file of productions to source on agent creation (can be null)
	 * @return Returns the newly created agent.
	********************************************************************************************/
	public Agent startLocalKernel(int portToListenOn, String agentName, String sourcePath)
	{
		if (m_Kernel != null)
		{
			// For now I'll throw.  We could also just return true that we're already running
			// or call stopKernel() and then create a new one.  All should be fine.
			throw new IllegalStateException("We've already started the kernel") ;
		}

		MainFrame frame = getFirstFrame() ;
		String kernelSML = "SoarKernelSML" ;

		if (frame != null)
		{
			// Look up the name of the DLL to use if the user has set it (via a dialog)
			String path = frame.getAppStringProperty("Kernel.Location") ;

			if (path != null && path.length() > 0)
			{
				// Adopt this as the DLL to load
				kernelSML = path ;
				System.out.println("Loading Soar from user specified location: " + kernelSML) ;
			}
		}
		
		// Create the kernel
		// We're use "InNewThread" so environments can remotely connect to the debugger and run
		// inside us if they wish (and without us having to do work to support that by calling
		// GetIncomingCommands() and pumping for incoming events).
		m_Kernel = Kernel.CreateKernelInNewThread(kernelSML, portToListenOn) ;

		// This is a local kernel
		m_IsRemote = false ;
		
		if (m_Kernel.HadError())
		{
			// Get the error and then kill off this kernel instance
			String errorMsg = "Tried to load Soar kernel from " + kernelSML + "\n\nError message: " + m_Kernel.GetLastErrorDescription() ;
			m_Kernel.delete() ;
			m_Kernel = null ;
			
			if (frame != null)
			{
				frame.ShowMessageBox("Error loading local Soar kernel", errorMsg) ;
				return null ;
			}
		}

		m_DocumentThread.setConnected(true) ;
		
		// Let our listeners know about the new kernel
		fireSoarConnectionChanged() ;
		
		// Register for agent additions before we create our first agent
		registerStandardKernelEvents() ;
		
		// Let the rest of the world know that the debugger is up and ready now (but agent level events not ready yet)
		m_Kernel.SetConnectionInfo(kConnectionName, sml_Names.getKStatusReady(), sml_Names.getKStatusNotReady()) ;
		
		// Set the library location if the user has defined this explicitly
		String libraryPath = frame.getAppStringProperty("Kernel.Library.Location") ;
		if (libraryPath != null && libraryPath.length() > 0)
		{
			System.out.println("Setting Soar library from user specified location: " + libraryPath) ;
			m_Kernel.ExecuteCommandLine(getSoarCommands().setLibraryLocationCommand(libraryPath), null, false) ;
		}
		
		// Choose a name for the agent
		if (agentName == null)
			agentName = "soar1" ;

		if (sourcePath != null)
		{
			delayedCommand(agentName, frame, getSoarCommands().getSourceCommand(sourcePath)) ;
		}

		// Start with an agent...without this a kernel's not much use.
		Agent agent = createAgentNoNewWindow(agentName) ;
		
		return agent ;
	}

	/********************************************************************************************
	 * 
	 * There are times when we'd like to execute a command but do so after the agent has been created
	 * AND the window to show its output has been created.
	 * 
	 * Waiting for both of these to occur requires listening for the right event and placing a request
	 * into the command queue in the right way.  This method shows how to do that.
	 * 
	 * @param agentName
	 * @param frame
	 * @param sourcePath
	********************************************************************************************/
	protected void delayedCommand(final String agentName, final MainFrame frame, final String command)
	{
		// Step one is to listen for the "agent added" event.
		// This is fired internally by the debugger once the Soar agent has been created and the SWT window exists
		// (but has yet to be displayed).
		this.addSoarChangeListener(new SoarChangeListener() {
			
			// We'd like to remove this listener after it has executed once
			// but that turns out to be tricky because this event may fire on different threads
			// so instead we use a flag to ensure it only executes one time.
			boolean m_Executed = false ;
			public void soarAgentListChanged(SoarAgentEvent e)
			{
				if (m_Executed)
					return ;
				
				final Agent agent = e.getAgent() ;
				if (e.isAgentAdded() && agent.GetAgentName().equals(agentName))
				{
					// When the agent we're listening for is created, add the command
					// to the command queue, but do it on the SWT UI thread so that it
					// completes after the window has been fully displayed.
					// Without this the command will execute fine--but probably before the window is up
					// so the user won't see the command or its output (but the agent would still execute the command).
					m_Executed = true ;
					Display display = frame.getDisplay() ;
					display.asyncExec(new Runnable() { public void run()
					{
						frame.executeCommandPrimeView(command, true) ;
					}}) ;
				}
			}

			public void soarConnectionChanged(SoarConnectionEvent e) { } 
		} );		
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
	
	/** Creates an agent and supresses automatic window creation (if the user has that enabled) */
	public Agent createAgentNoNewWindow(String agentName)
	{
		m_SuppressNewWindowCreation = agentName ;
		return createAgent(agentName) ;
	}
	
	public void destroyAgent(Agent agent)
	{
		m_Kernel.DestroyAgent(agent) ;
	}

	/*
	private void forceShutdown()
	{
		// If the user tries to shutdown while Soar is running locally we have a problem.
		// The document thread is stuck inside a Run command so it's no longer checking for us
		// shutting down the thread (through askToStop).  If we issue a stop (which takes time
		// while we wait for the stop to actually get processed) we can deadlock waiting for
		// update events triggered by the stop to call synchExec on the display.  This deadlocks
		// because we're inside an event handler right now.  So for now I'm making a simple fix
		// and blasting the app to death if it we're shutting down and the document thread won't
		// stop in a reasonable ammount of time.
		if (this.m_Kernel.GetNumberAgents() > 0 && kDocInOwnThread)
		{
			m_DocumentThread.askToStop() ;
			
			int count = 5 ;
			while (m_DocumentThread.isAlive())
			{				
				try { Thread.sleep(100) ; } catch (InterruptedException e) { }
				
				count-- ;
				if (count == 0)
				{
					System.err.println("Forced system shutdown") ;
					System.exit(0) ;
				}
			}
		}
	}
	*/
	
	/********************************************************************************************
	 * 
	 * Shutdown our local Soar instance, deleting any agents etc.
	 * 
	********************************************************************************************/
	public boolean stopLocalKernel(final boolean closingApp, final boolean preserveWindows)
	{
		if (m_Kernel == null)
			return false ;

		m_IsStopping = true ;
		
		final Kernel kernelToStop = m_Kernel ;
		
		// Experimental shutdown code where we try to stop soar
		// before we delete the kernel.
		Thread shutdown = new Thread()
		{
			public void run()
			{
				if (m_Kernel.GetNumberAgents() > 0)
				{
					// We don't currently support sending commands without an agent so
					// choose the first agent
					Agent first = m_Kernel.GetAgentByIndex(0) ;
					
					//System.err.println("About to call stop-soar") ;
					sendAgentCommand(first, getSoarCommands().getStopCommand()) ;
	
					while (m_DocumentThread.isBusy())
					{
						//System.err.println("Waiting for Soar to stop") ;
						try { Thread.sleep(100) ; } catch (InterruptedException e) { }
					}
				}
				
				if (closingApp)
				{
					m_DocumentThread.askToStop() ;
	
					while (m_DocumentThread.isAlive())
					{
						//System.err.println("Waiting for doc thread to stop") ;
						try { Thread.sleep(100) ; } catch (InterruptedException e) { }
					}
				}

				// If we want to keep our windows open suppress
				// the "close on destroy" logic
				if (preserveWindows)
					setSuppressCloseWindow(true) ;
				
				// Deletes all agents and sends appropriate events
				kernelToStop.Shutdown() ;
				
				// Actually deletes the kernel
				kernelToStop.delete() ;
				
				m_Kernel = null ;
				m_IsStopping = false ;
				
				// Let our listeners know we killed the kernel
				fireSoarConnectionChanged() ;

				// Turn off suppression of this event.
				// (This returns this to the user's control)
				if (preserveWindows)
					setSuppressCloseWindow(false) ;
								
				//System.err.println("Exiting shutdown thread") ;
			}
		} ;

		// To do a truly clean shutdown we need to stop soar
		// if it's running.  That may be safest from another thread.
		// We can choose whether to actually use the other thread by calling start()
		// instead of run() here.
		// BUGBUG: Once this is resolved we should either take out this Thread object or call start() on it.
		shutdown.run() ;
		return true ;
	}

	/********************************************************************************************
	 * 
	 * Connect to a remote instance of Soar that is already running.
	 * 
	 * @param ipAddress		The ip address (e.g. "205.233.102.121" to connect to.  null => local ip)
	 * @param portNumber	The port to connect on.
	 * @param agentName		The name of the agent to assign to the first display window (null or not found => use first agent)
	********************************************************************************************/
	public void remoteConnect(String ipAddress, int portNumber, String agentName) throws Exception
	{
		// If we have a local kernel already running shut it down
		// but don't close any windows we have open (or the app will shutdown here!)
		stopLocalKernel(false, true) ;

		m_IsRemote = true ;
		
		// Explicitly ask not to be sent output link changes as we're not interested
		// in them (we're not an environment).  This boosts performance.
		boolean ignoreOutput = true ;
		
		m_Kernel = Kernel.CreateRemoteConnection(true, ipAddress, portNumber, ignoreOutput) ;

		if (m_Kernel.HadError())
		{
			// Get the error and then kill off this kernel instance
			String errorMsg = m_Kernel.GetLastErrorDescription() ;
			m_Kernel.delete() ;
			m_Kernel = null ;
			
			if (ipAddress == null)
				ipAddress = "<local-machine>" ;
			
			throw new Exception("Error initializing kernel with ip address " + ipAddress + " port " + portNumber + "\n" + errorMsg) ;
		}

		m_DocumentThread.setConnected(true) ;

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
				Agent agent = null ;

				// If we have a named agent to look for, find that one
				if (agentName != null && agentName.length() > 0 && agentID == 0)
					agent = this.getAgent(agentName) ;
				
				// If we we're given a name or the name didn't match or if this
				// is the second frame then just look up the agent by index.
				if (agent == null)
					agent = this.getAgentByIndex(agentID) ;
				
				frame.setAgentFocus(agent) ;
				agentID++ ;
			}
			else
			{
				// Make sure the focus is set to nothing (not some left over agent)
				frame.clearAgentFocus(false) ;
			}
		}
		
		// Let the rest of the world know that the debugger is up and ready now
		m_Kernel.SetConnectionInfo(kConnectionName, sml_Names.getKStatusReady(), sml_Names.getKStatusReady()) ;
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

		m_IsStopping = true ;
		
		m_DocumentThread.setConnected(false) ;
 		
		// NOTE: We don't check whether Soar is still running before allowing a remote disconnect
		// (unlike in the localStop call).  This is because setting an agent running and then disconnecting
		// should be OK.  We may need to handle some error conditions that occur when we do this
		// but by and large it should be acceptable and it would be good to make it work.

		// Shutdown our connection.
		m_Kernel.Shutdown() ;
		
		// Release all memory.
		m_Kernel.delete() ;
		
		m_Kernel = null ;
		m_IsStopping = false ;
		
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
	 * 
	********************************************************************************************/
	public boolean isRemote()
	{
		return m_Kernel != null && m_IsRemote ;
	}
	
	/********************************************************************************************
	 * 
	 * Returns true we're in the process of stopping the current connection and closing it.
	 * 
	********************************************************************************************/	
	public boolean isStopping()
	{
		return m_IsStopping ;
	}

	public void systemEventHandler(int eventID, Object data, Kernel kernel)
	{
		if (eventID == smlSystemEventId.smlEVENT_INTERRUPT_CHECK.swigValue())
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
				if (m_DocumentThread.isBusy())
					m_DocumentThread.executePending("Interrupt check") ;
			}
				
			return ;
		}
	}
	
	public void agentEventHandler(int eventID, Object data, String agentName)
	{
		final Agent agent = getAgent(agentName) ;
		
		if (eventID == smlAgentEventId.smlEVENT_AFTER_AGENT_CREATED.swigValue())
		{
			// Create a window if the user has set this mode AND we're not suppressing it.
			// (We suppress it if we're programmatically creating the agent so that the logic is simpler)
			boolean createWindow = getAppProperties().getAppBooleanProperty(kCreateNewWindowProperty, true) ;
			createWindow = createWindow && !agentName.equals(m_SuppressNewWindowCreation) ;

			// Only suppress window creation for the agent once.
			if (agentName.equals(m_SuppressNewWindowCreation))
				m_SuppressNewWindowCreation = null ;
			
			if (createWindow)
			{
				// Create a new window for this agent
				Display display = getFirstFrame().getDisplay() ;
				final Document doc = this ;
				
				display.asyncExec(new Runnable() { public void run()
				{
					MainFrame frame = MainFrame.createNewFrame(getFirstFrame().getDisplay(), doc) ;			
					frame.setAgentFocus(agent) ;
					doc.fireAgentAdded(agent) ;
					
					// Let the rest of the world know that the debugger is up and ready now for this agent
					m_Kernel.SetConnectionInfo(kConnectionName, sml_Names.getKStatusReady(), sml_Names.getKStatusReady()) ;

				} } ) ;
			}
			else
			{
				this.fireAgentAdded(agent) ;
				
				// Let the rest of the world know that the debugger is up and ready now for this agent
				m_Kernel.SetConnectionInfo(kConnectionName, sml_Names.getKStatusReady(), sml_Names.getKStatusReady()) ;
			}
		}
		
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
			try
			{
				if (!m_Display.readAndDispatch())
				{
					m_Display.sleep() ;
				}
			}
			catch (Throwable t)
			{
				System.out.println(t.toString()) ;
				t.printStackTrace() ;
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
		return sendAgentCommandGeneral(agent, commandLine, null) ;
	}

	public boolean sendAgentCommandXML(Agent agent, String commandLine, ClientAnalyzedXML response)
	{
		if (response == null)
			throw new IllegalArgumentException("Must provide an XML object to receive the response") ;
		
		String result = sendAgentCommandGeneral(agent, commandLine, response) ;

		boolean res = (result != null && result.equals("true")) ;
		return res ;
	}
	
	 /*******************************************************************************************
	 * 
	 * If response is null we execute the command and return the string form.
	 * If response is provided we execute the command and return the XML (and the string
	 * return value is either "true" or "false")
	 * 
	 * @param agent
	 * @param commandLine
	 * @param response
	 * @return
	*******************************************************************************************
	 */
	protected String sendAgentCommandGeneral(Agent agent, String commandLine, ClientAnalyzedXML response)
	{
		if (!isConnected())
			return "Error: No Soar kernel is running.  Need to start one before executing commands." ;
		
		if (agent == null)
			return "Error: Agent is missing.  Need to create one before executing commands" ;

		if (Document.kDocInOwnThread)
		{
			// If Soar commands run in their own thread we issue the command and then pump the UI thread for messages
			// while waiting for a response.  We do this so that folks calling here can see this as a synchronous call
			// when really it's asynchronous.
			// We also need to pump the document thread in response to an "agents_run_step" event to allow interruptions
			// so that a Soar "run" command can be interrupted.
			DocumentThread.Command command = m_DocumentThread.scheduleCommandToExecute(agent, commandLine, response) ;
			
			while (m_DocumentThread.isAlive() && !m_DocumentThread.isExecutedCommand(command))
			{
				pumpMessagesOneStep() ;
				
				// The pause as we sleep here is just how quickly we respond to commands finishing (or UI events).
				// It won't affect the speed Soar executes.
				try { Thread.sleep(10) ; } catch (InterruptedException e) { } 
			}

			if (!m_DocumentThread.isAlive())
				return "System shutdown" ;
			
			String result = m_DocumentThread.getExecutedCommandResult(command) ;
			return result ;
		}
		else
		{
			// If we're running commands in the UI thread we can just execute it.
			// If it's a run we need to pump messages in response to an "agents_run_step" event
			// to keep the UI alive and allow folks to interrupt Soar.
			String result = agent.ExecuteCommandLine(commandLine) ;			
			return result ;
		}
	}
}

package splintersoar.soar;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;

import erp.config.Config;
import sml.Agent;
import sml.Kernel;
import sml.smlAgentEventId;
import sml.smlUpdateEventId;
import splintersoar.LCMInfo;
import splintersoar.LogFactory;
import splintersoar.lcmtypes.splinterstate_t;
import splintersoar.ranger.RangerState;
import splintersoar.ranger.RangerStateProducer;

public class SoarInterface implements Kernel.UpdateEventInterface, Kernel.AgentEventInterface, LCMSubscriber
{
	private Logger logger;
	
	private class Configuration
	{
		String productions;
		
		Configuration( Config config )
		{
			productions = config.requireString( "soar.agent" );
		}
	}
	
	private Configuration configuration;
	
	Kernel kernel;
	Agent agent;
	InputLinkManager input;
	OutputLinkManager output;
	Waypoints waypoints;
	RangerStateProducer rangerStateProducer;
	splinterstate_t splinterState;
	LCM lcm;
	
	public SoarInterface( RangerStateProducer rangerStateProducer, Config config )
	{
		configuration = new Configuration( config );
		
		lcm = LCM.getSingleton();
		lcm.subscribe( LCMInfo.SPLINTER_STATE_CHANNEL, this );

		logger = LogFactory.createSimpleLogger( "SoarInterface", Level.INFO );

		kernel = Kernel.CreateKernelInNewThread();
		if ( kernel.HadError() )
		{
			logger.warning( "Soar error: " + kernel.GetLastErrorDescription() );
			System.exit(1);
		}

		kernel.SetAutoCommit( false );
		
		agent = kernel.CreateAgent( "soar" );
		if ( kernel.HadError() )
		{
			logger.warning( "Soar error: " + kernel.GetLastErrorDescription() );
			System.exit(1);
		}
		
		// load productions
		agent.LoadProductions( configuration.productions );
		
		waypoints = new Waypoints( agent );

		// wait for valid data
		while ( splinterState == null )
		{
			logger.info( "Waiting for valid splinter state" );
			try 
			{
				Thread.sleep( 200 );
			} catch ( InterruptedException ignored ) 
			{}
		}
		logger.info( "Have splinter state" );
		
		splinterstate_t ss = splinterState.copy();
		
		this.rangerStateProducer = rangerStateProducer;
		RangerState rangerState = rangerStateProducer.getRangerState();
		
		// rangerState could be null 
		
		input = new InputLinkManager( agent, waypoints, ss, rangerState );
		output = new OutputLinkManager( agent, waypoints );
		
		kernel.RegisterForUpdateEvent( smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null );
		kernel.RegisterForAgentEvent( smlAgentEventId.smlEVENT_BEFORE_AGENT_REINITIALIZED, this, null );
	}
	
	class SoarRunner implements Runnable
	{
		public void run()
		{
			kernel.RunAllAgentsForever();
		}
	}
	
	public void start()
	{
		Thread soarThread = new Thread( new SoarRunner() );
		stopSoar = false;
		soarThread.start();
	}
	
	boolean stopSoar = false;

	private boolean overrideEnabled = false;
	public void stop()
	{
		stopSoar = true;
	}
	
	public void shutdown()
	{
		stopSoar = true;
		try
		{
			Thread.sleep( 1000 );
		} catch ( InterruptedException ignored ) {}
		
		kernel.Shutdown();
		kernel.delete();
		logger.info( "Soar interface down" ); 
	}
	
	@Override
	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) 
	{
		if ( stopSoar )
		{
			logger.info( "Stopping Soar" ); 
			kernel.StopAllAgents();
		}
		
		updateState();
	}
	
	public void updateState()
	{
		try
		{
			RangerState rangerState = rangerStateProducer.getRangerState();
			
			splinterstate_t ss = splinterState.copy();
			
			input.update( ss, rangerState );
			SplinterInput splinterInput = output.update( ss );
			
			if ( splinterInput != null && !overrideEnabled )
			{
				lcm.publish( LCMInfo.DRIVE_COMMANDS_CHANNEL, splinterInput.generateDriveCommand( ss ) );
			}
		
			waypoints.update(); // updates input link due to output link commands
		
			agent.Commit();
		}
		catch ( NullPointerException unhandled )
		{
			logger.warning( "Unhandled null pointer exception in updateState" );
			unhandled.printStackTrace();
		}
		catch ( Throwable unhandled )
		{
			logger.warning( "Unhandled throwable in updateState" );
			unhandled.printStackTrace();
		}
	}

	public void setOverride( boolean overrideEnabled ) {
		this.overrideEnabled = overrideEnabled;
	}

	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if ( channel.equals( LCMInfo.SPLINTER_STATE_CHANNEL ) )
		{
			try 
			{
				splinterState = new splinterstate_t( ins );
			} 
			catch ( IOException ex ) 
			{
				logger.warning( "Error decoding splinterstate_t message: " + ex );
			}
		}
	}

	@Override
	public void agentEventHandler( int id, Object userData, String agentName ) {
		if ( id == smlAgentEventId.smlEVENT_BEFORE_AGENT_REINITIALIZED.swigValue() )
		{
			waypoints.beforeInitSoar();
			agent.Commit();
		}
	}

}

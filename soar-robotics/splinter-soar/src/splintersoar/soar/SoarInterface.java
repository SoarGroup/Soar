package splintersoar.soar;

import java.util.logging.Level;
import java.util.logging.Logger;

import lcm.lcm.LCM;

import erp.config.Config;
import sml.Agent;
import sml.Kernel;
import sml.smlUpdateEventId;
import splintersoar.LogFactory;
import splintersoar.lcmtypes.splinterstate_t;
import splintersoar.ranger.RangerState;
import splintersoar.ranger.RangerStateProducer;

public class SoarInterface implements Kernel.UpdateEventInterface
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

		logger = LogFactory.simpleLogger( Level.ALL );

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
		
		RangerState rangerState = rangerStateProducer.getRangerState();
		
		// rangerState could be null 
		
		input = new InputLinkManager( agent, waypoints, splinterState, rangerState );
		output = new OutputLinkManager( agent, waypoints );
		
		kernel.RegisterForUpdateEvent( smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null );
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
		
		try
		{
			RangerState rangerState = rangerStateProducer.getRangerState();
			
			input.update( splinterState, rangerState );
			SplinterInput splinterInput = output.update( splinterState );
			
			if ( !overrideEnabled )
			{
				lcm.publish( "MOTOR_COMMAND", splinterInput.generateDriveCommand( splinterState ) );
			}
		
			waypoints.update(); // updates input link due to output link commands
		
			agent.Commit();
		}
		catch ( NullPointerException unhandled )
		{
			logger.warning( "Unhandled null pointer exception in updateEventHandler" );
			unhandled.printStackTrace();
		}
		catch ( Throwable unhandled )
		{
			logger.warning( "Unhandled throwable in updateEventHandler" );
			unhandled.printStackTrace();
		}
	}

	public void setOverride( boolean overrideEnabled ) {
		this.overrideEnabled = overrideEnabled;
	}

}

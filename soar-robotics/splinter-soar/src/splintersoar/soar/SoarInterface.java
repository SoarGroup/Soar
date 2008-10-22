package splintersoar.soar;

import java.util.logging.Level;
import java.util.logging.Logger;

import erp.config.Config;
import sml.Agent;
import sml.Kernel;
import sml.smlUpdateEventId;
import splintersoar.LogFactory;
import splintersoar.orc.OrcInput;
import splintersoar.orc.OrcOutput;
import splintersoar.orc.OrcOutputProducer;
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
	OrcOutputProducer splinterOutputProducer;
	RangerStateProducer rangerStateProducer;
	
	public SoarInterface( OrcOutputProducer splinterOutputProducer, RangerStateProducer rangerStateProducer, Config config )
	{
		configuration = new Configuration( config );

		logger = LogFactory.simpleLogger( Level.ALL );

		this.splinterOutputProducer = splinterOutputProducer;
		
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

		OrcOutput splinterOutput = splinterOutputProducer.getOutput();

		// wait for valid data
		while ( splinterOutput.utime == 0 )
		{
			logger.info( "Waiting for valid splinter state" );
			try 
			{
				Thread.sleep( 200 );
			} catch ( InterruptedException ignored ) 
			{}
			splinterOutput = splinterOutputProducer.getOutput();
		}
		logger.info( "Have splinter state" );
		
		RangerState rangerState = rangerStateProducer.getRangerState();
		
		// rangerState could be null 
		
		input = new InputLinkManager( agent, waypoints, splinterOutput, rangerState );
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
			OrcOutput splinterOutput = splinterOutputProducer.getOutput();
			RangerState rangerState = rangerStateProducer.getRangerState();
			
			input.update( splinterOutput, rangerState );
			output.update( splinterOutput );
		
			waypoints.update(); // updates input link
		
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

	public OrcInput getSplinterInput() {
		return output.getSplinterInput();
	}
}

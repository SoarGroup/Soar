package splintersoar.soar;

import sml.*;
import splintersoar.*;

public class SoarInterface implements Kernel.UpdateEventInterface
{
	Kernel kernel;
	Agent agent;
	InputLinkManager input;
	OutputLinkManager output;
	Waypoints waypoints;
	
	static final double baselineMeters = 0.42545;

	public SoarInterface( SplinterState state, String productions )
	{
		kernel = Kernel.CreateKernelInNewThread();
		if ( kernel.HadError() )
		{
			SplinterSoar.logger.warning( "Soar error: " + kernel.GetLastErrorDescription() );
			System.exit(1);
		}

		kernel.SetAutoCommit( false );
		
		agent = kernel.CreateAgent( "soar" );
		if ( kernel.HadError() )
		{
			SplinterSoar.logger.warning( "Soar error: " + kernel.GetLastErrorDescription() );
			System.exit(1);
		}
		
		// load productions
		assert productions != null;
		agent.LoadProductions( productions );
		
		waypoints = new Waypoints( agent );
		input = new InputLinkManager( agent, waypoints, state );
		output = new OutputLinkManager( agent, waypoints, state );
		
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
		SplinterSoar.logger.info( "Soar interface down" ); 
	}
	
	public void setOverride( boolean enabled )
	{
		output.setOverride( enabled );
	}
	
	@Override
	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) 
	{
		if ( stopSoar )
		{
			SplinterSoar.logger.info( "Stopping Soar" ); 
			kernel.StopAllAgents();
		}
		
		try
		{
			input.update();
			output.update();
		
			waypoints.update(); // updates input link
		
			agent.Commit();
		}
		catch ( NullPointerException unhandled )
		{
			SplinterSoar.logger.warning( "Unhandled null pointer exception in updateEventHandler" );
			unhandled.printStackTrace();
		}
		catch ( Throwable unhandled )
		{
			SplinterSoar.logger.warning( "Unhandled throwable in updateEventHandler" );
			unhandled.printStackTrace();
		}
	}

}

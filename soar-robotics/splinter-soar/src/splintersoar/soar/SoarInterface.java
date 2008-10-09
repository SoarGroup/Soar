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

	public SoarInterface( SplinterState state )
	{
		kernel = Kernel.CreateKernelInNewThread();
		if ( kernel.HadError() )
		{
			System.err.println( kernel.GetLastErrorDescription() );
			System.exit(1);
		}

		kernel.SetAutoCommit( false );
		
		agent = kernel.CreateAgent( "soar" );
		if ( kernel.HadError() )
		{
			System.err.println( kernel.GetLastErrorDescription() );
			System.exit(1);
		}
		
		// load productions
		//agent.LoadProductions( "agents/simple-bot.soar" );
		agent.LoadProductions( "../agents/follower-robot.soar" );
		
		waypoints = new Waypoints( agent );
		input = new InputLinkManager( agent, waypoints, state );
		output = new OutputLinkManager( agent, waypoints, state );
		
		kernel.RegisterForUpdateEvent( smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null );
	}
	
	public void shutdown()
	{
		kernel.Shutdown();
		kernel.delete();
	}
	
	public void setOverride( OverrideInterface override )
	{
		input.setOverride( override );
	}
	
	@Override
	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) 
	{
		try
		{
			input.update();
			output.update();
		
			waypoints.update(); // updates input link
		
			agent.Commit();
		}
		catch ( NullPointerException unhandled )
		{
			System.out.println( "Unhandled null pointer exception in updateEventHandler" );
			unhandled.printStackTrace();
			assert false;
		}
		catch ( Throwable unhandled )
		{
			System.out.println( "Unhandled throwable in updateEventHandler" );
			unhandled.printStackTrace();
			assert false;
		}
	}

}

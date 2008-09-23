package	soaruorc;

import orc.util.*;
import sml.*;

public class SoaruOrc implements Kernel.UpdateEventInterface
{
	private GamePad gp = new GamePad();
	private uOrcThread uorc = new uOrcThread();
	Kernel kernel;
	Agent agent;
	FloatElement left;
	FloatElement right;

	public SoaruOrc()
	{
		kernel = Kernel.CreateKernelInNewThread();
		if ( kernel.HadError() )
		{
			System.err.println( kernel.GetLastErrorDescription() );
			System.exit(1);
		}

		agent = kernel.CreateAgent( "soar" );
		if ( kernel.HadError() )
		{
			System.err.println( kernel.GetLastErrorDescription() );
			System.exit(1);
		}
		
		// load productions
		agent.ExecuteCommandLine( "waitsnc -e" );
		
		// set up input link
		Identifier override = agent.CreateIdWME( agent.GetInputLink(), "override" );
		left = agent.CreateFloatWME( override, "left", 0 );
		right = agent.CreateFloatWME( override, "right", 0 );

		kernel.RegisterForUpdateEvent( smlUpdateEventId.smlEVENT_AFTER_ALL_GENERATED_OUTPUT, this, null );
		
		// start the bot thread
		uorc.start();
		
		kernel.RunAllAgentsForever();
			
		uorc.stopThread();
		kernel.Shutdown();
		kernel.delete();
	}

	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) 
	{
		// write input
		agent.Update( left, gp.getAxis( 1 ) * -1 );
		agent.Update( right, gp.getAxis( 3 ) * -1 );
		
		// process output
		for ( int i = 0; i < agent.GetNumberCommands(); ++i ) 
		{
			Identifier commandId = agent.GetCommand( i );
			String commandName = commandId.GetAttribute();
			
			if ( commandName.equals( "move" ) )
			{
				double leftCommand = 0;
				double rightCommand = 0;
				
				try {
					leftCommand = Double.parseDouble( commandId.GetParameterValue( "left" ) );
				} catch ( NumberFormatException e ) {
					System.out.println( "Unable to parse left: " + commandId.GetParameterValue( "left" ) );
					commandId.AddStatusError();
					continue;
				}
				
				try {
					rightCommand = Double.parseDouble( commandId.GetParameterValue( "right" ) );
				} catch ( NumberFormatException e ) {
					System.out.println( "Unable to parse right: " + commandId.GetParameterValue( "right" ) );
					commandId.AddStatusError();
					continue;
				}
				
				leftCommand = Math.max( leftCommand, -1.0 );
				leftCommand = Math.min( leftCommand, 1.0 );
				
				rightCommand = Math.max( rightCommand, -1.0 );
				rightCommand = Math.min( rightCommand, 1.0 );
				
				uorc.setPower( leftCommand, rightCommand );
				commandId.AddStatusComplete();
				continue;
			}
			
			System.out.println( "Unknown command: " + commandName );
			commandId.AddStatusError();
		}
		
		
	}
	
	public static void main(String args[])
	{
		new SoaruOrc();
	}
}

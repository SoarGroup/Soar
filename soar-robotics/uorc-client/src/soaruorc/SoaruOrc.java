package	soaruorc;

import orc.util.*;
import sml.*;

public class SoaruOrc implements Kernel.UpdateEventInterface, Kernel.SystemEventInterface
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
			System.err.writeln( kernel.GetLastErrorDescription() );
			System.exit(1);
		}

		agent = Kernel.CreateAgent();
		if ( kernel.HadError() )
		{
			System.err.writeln( kernel.GetLastErrorDescription() );
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
		
		// start soar
		kernel.RunAllAgentsForever();
			
		// shutdown
		uorc.stop();
		kernel.shutdown();
		delete kernel;
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
					leftCommand = Double.decode( commandId.GetParameterValue( "left" ) ).doubleValue();
				} catch ( NumberFormatException e ) {
					System.out.writeln( "Unable to parse left: " + commandId.GetParameterValue( "left" ) );
					commandId.AddStatusError();
					continue;
				}
				
				try {
					rightCommand = Double.decode( commandId.GetParameterValue( "right" ) ).doubleValue();
				} catch ( NumberFormatException e ) {
					System.out.writeln( "Unable to parse right: " + commandId.GetParameterValue( "right" ) );
					commandId.AddStatusError();
					continue;
				}
				
				leftCommand = Math.max( leftCommand, -1.0 );
				leftCommand = Math.min( leftCommand, 1.0 );
				
				rightCommand = Math.max( rightCommand, -1.0 );
				rightCommand = Math.min( rightCommand, 1.0 );
				
				uorc.setPower( leftCommand, rightCommand );
				commandId.AddStatusComplete();
				continue
			}
			
			System.out.writeln( "Unknown command: " + commandName );
			commandId.AddStatusError();
		}
		
		
	}
	
	public static void main(String args[])
	{
		new SoaruOrc();
	}
}

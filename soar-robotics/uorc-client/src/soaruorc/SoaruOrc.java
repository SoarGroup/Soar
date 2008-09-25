package	soaruorc;

import orc.util.*;
import sml.*;
import java.io.Console;

public class SoaruOrc implements Kernel.UpdateEventInterface
{
	private GamePad gp;
	private uOrcThread uorc;
	private Kernel kernel;
	private Agent agent;
	private StringElement override_active;
	private FloatElement override_move_left;
	private FloatElement override_move_right;
	private FloatElement self_motor_left_current;
	private FloatElement self_motor_right_current;
	
	private boolean useGamePad = true;
	private boolean useRobot = true;
	
	public SoaruOrc()
	{
		if ( useGamePad )
		{
			gp = new GamePad();
		}
		
		if ( useRobot )
		{
			uorc = new uOrcThread();
		}
		
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
		agent.LoadProductions( "agents/simple-bot.soar" );
		
		// set up input link
		// override
		//     active true
		//         move
		//             left 0
		//             right 0
		{
			Identifier override = agent.CreateIdWME( agent.GetInputLink(), "override" );
			override_active = agent.CreateStringWME( override, "active", "false" );
			Identifier move = agent.CreateIdWME( override, "move" );
			override_move_left = agent.CreateFloatWME( move, "left", 0 );
			override_move_right = agent.CreateFloatWME( move, "right", 0 );
		}
		
		// self
		//     motor
		//         left
		//             current
		//         right 
		//             current
		{
			Identifier self = agent.CreateIdWME( agent.GetInputLink(), "self" );
			Identifier self_motor = agent.CreateIdWME( self, "motor" );
			Identifier self_motor_left = agent.CreateIdWME( self_motor, "left" );
			Identifier self_motor_right = agent.CreateIdWME( self_motor, "right" );

			self_motor_left_current = agent.CreateFloatWME( self_motor_left, "current", 0 );
			self_motor_right_current = agent.CreateFloatWME( self_motor_right, "current", 0 );
		}

		kernel.RegisterForUpdateEvent( smlUpdateEventId.smlEVENT_AFTER_ALL_GENERATED_OUTPUT, this, null );
		
		// start the bot thread
		if ( useRobot )
		{
			uorc.start();
		}
		
		//System.out.printf( "%15s %15s %15s %15s\n", "left input", "right input", "left output", "right output" );
		
		// let the debugger debug
		Console console = System.console();
		while ( true )
		{
			String command = console.readLine();
			if ( command.equals( "quit" ) || command.equals( "exit" ) )
			{
				break;
			}
		}
			
		if ( useRobot )
		{
			uorc.stopThread();
		}
		kernel.Shutdown();
		kernel.delete();
		
		System.out.println( "Shutdown complete. Hit control-c to continue." );
	}

	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) 
	{
		uOrcThread.BotState state = uorc.getState();
		
		double leftInput = 0;
		double rightInput = 0;
		if ( useGamePad )
		{
			leftInput = gp.getAxis( 1 ) * -1;
			rightInput = gp.getAxis( 3 ) * -1;
		} 
		else
		{
			leftInput = Math.random();
			rightInput = Math.random();
		}

		double leftCurrent, rightCurrent;
		synchronized ( state )
		{
			leftCurrent = state.leftCurrent;
			rightCurrent = state.rightCurrent;
		}
		
		// write input
		agent.Update( override_active, "true" );
		agent.Update( override_move_left, leftInput );
		agent.Update( override_move_right, rightInput );
		agent.Update( self_motor_left_current, leftCurrent );
		agent.Update( self_motor_right_current, rightCurrent );
		
		// process output
		double leftCommand = 0;
		double rightCommand = 0;
		for ( int i = 0; i < agent.GetNumberCommands(); ++i ) 
		{
			Identifier commandId = agent.GetCommand( i );
			String commandName = commandId.GetAttribute();
			
			if ( commandName.equals( "move" ) )
			{
				//System.out.print( "move: " );
				try 
				{
					leftCommand = Double.parseDouble( commandId.GetParameterValue( "left" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.out.println( "No left on move command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.out.println( "Unable to parse left: " + commandId.GetParameterValue( "left" ) );
					commandId.AddStatusError();
					continue;
				}
				//System.out.print( leftCommand + " " );
				
				try 
				{
					rightCommand = Double.parseDouble( commandId.GetParameterValue( "right" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.out.println( "No right on move command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.out.println( "Unable to parse right: " + commandId.GetParameterValue( "right" ) );
					commandId.AddStatusError();
					continue;
				}
				//System.out.print( rightCommand + " " );
				
				leftCommand = Math.max( leftCommand, -1.0 );
				leftCommand = Math.min( leftCommand, 1.0 );
				
				rightCommand = Math.max( rightCommand, -1.0 );
				rightCommand = Math.min( rightCommand, 1.0 );
				
				//System.out.print( leftCommand + " " );
				//System.out.print( rightCommand + "\n" );
				
				if ( useRobot )
				{
					synchronized ( state )
					{
						state.left = leftCommand;
						state.right = rightCommand;
					}
				}
				commandId.AddStatusComplete();
				continue;
			}
			
			System.out.println( "Unknown command: " + commandName );
			commandId.AddStatusError();
		}
		
		//System.out.printf( "%15f %15f %15f %15f\r", leftInput, rightInput, leftCommand, rightCommand );
	}
	
	public static void main(String args[])
	{
		new SoaruOrc();
	}
}

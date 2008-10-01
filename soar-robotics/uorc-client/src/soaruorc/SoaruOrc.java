package	soaruorc;

import orc.util.*;
import sml.*;
import java.io.Console;

public class SoaruOrc implements Kernel.UpdateEventInterface
{
	private GamePad gp;

	private uOrcThread uorc;
	private uOrcThread.BotState botState;
		

	private Kernel kernel;
	private Agent agent;
	private StringElement override_active;
	private FloatElement override_move_left;
	private FloatElement override_move_right;
	private FloatElement self_motor_left_current;
	private FloatElement self_motor_left_position;
	private FloatElement self_motor_left_velocity;
	private FloatElement self_motor_right_current;
	private FloatElement self_motor_right_position;
	private FloatElement self_motor_right_velocity;
	private FloatElement self_pose_x;
	private FloatElement self_pose_y;
	private FloatElement self_pose_z;
	private FloatElement self_pose_yaw;
	
	private boolean useGamePad = true;
	private boolean useRobot = true;
	
	private static final double tickMeters = 0.0000429250;
	//private static final double baselineMeters = 0.4475;
	// revised by Jon
	private static final double baselineMeters = 0.42545;
	
	private boolean haveInitialPosition = false;
	private double leftInitialPosition;
	private double rightInitialPosition;
	
	public SoaruOrc()
	{
		if ( useGamePad )
		{
			gp = new GamePad();
		}
		
		if ( useRobot )
		{
			uorc = new uOrcThread();
			botState = uorc.getBotState();
		}
		
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
		//     geometry
		//         baseline
		//     motor
		//         left
		//             current
		//             position
		//             velocity
		//         right 
		//             current
		//             position
		//             velocity
		//     pose
		//             x
		//             y
		//             z
		//             yaw
		{
			Identifier self = agent.CreateIdWME( agent.GetInputLink(), "self" );

			{
				Identifier self_geometry = agent.CreateIdWME( self, "geometry" );
				
				agent.CreateFloatWME( self_geometry, "baseline", baselineMeters );
			}
			
			{
				Identifier self_motor = agent.CreateIdWME( self, "motor" );
				
				Identifier self_motor_left = agent.CreateIdWME( self_motor, "left" );
				self_motor_left_current = agent.CreateFloatWME( self_motor_left, "current", 0 );
				self_motor_left_position = agent.CreateFloatWME( self_motor_left, "position", 0 );
				self_motor_left_velocity = agent.CreateFloatWME( self_motor_left, "velocity", 0 );

				Identifier self_motor_right = agent.CreateIdWME( self_motor, "right" );
				self_motor_right_current = agent.CreateFloatWME( self_motor_right, "current", 0 );
				self_motor_right_position = agent.CreateFloatWME( self_motor_right, "position", 0 );
				self_motor_right_velocity = agent.CreateFloatWME( self_motor_right, "velocity", 0 );
			}
			
			{
				Identifier self_pose = agent.CreateIdWME( self, "pose" );
				
				self_pose_x = agent.CreateFloatWME( self_pose, "x", 0 );
				self_pose_y = agent.CreateFloatWME( self_pose, "y", 0 );
				self_pose_z = agent.CreateFloatWME( self_pose, "z", 0 );
				self_pose_yaw = agent.CreateFloatWME( self_pose, "yaw", 0 );
			}
		}
		
		agent.Commit();

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
		// write input
		if ( useGamePad )
		{
			double leftInput = gp.getAxis( 1 ) * -1;
			double rightInput = gp.getAxis( 3 ) * -1;
			
			agent.Update( override_active, "true" );
			agent.Update( override_move_left, leftInput );
			agent.Update( override_move_right, rightInput );
		} 
		else
		{
			agent.Update( override_active, "false" );
		}

		if ( useRobot )
		{
			double leftCurrent, leftPosition, leftVelocity;
			double rightCurrent, rightPosition, rightVelocity;
			
			synchronized ( botState )
			{
				leftCurrent = botState.leftCurrent;
				leftPosition = botState.leftPosition;
				leftVelocity = botState.leftVelocity;

				rightCurrent = botState.rightCurrent;
				rightPosition = botState.rightPosition;
				rightVelocity = botState.rightVelocity;
			}
			
			if ( ! haveInitialPosition )
			{
				leftInitialPosition = leftPosition;
				rightInitialPosition = rightPosition;
				haveInitialPosition = true;
			}
			
			leftPosition = ( leftPosition - leftInitialPosition ) * tickMeters;
			leftVelocity = leftVelocity * tickMeters;
			
			rightPosition = ( rightPosition - rightInitialPosition ) * tickMeters;
			rightVelocity = rightVelocity * tickMeters;
			
			// Equations from A Primer on Odopmetry and Motor Control, Olson 2006
			// dleft, dright: distance wheel travelled
			// dbaseline: wheelbase
			//
			// dcenter = ( dleft + dright ) / 2
			// phi = ( dright - dleft ) / dbaseline
			// thetaprime = theta + phi
			// xprime = x + ( dcenter * cos( theta ) )
			// yprime = y + ( dcenter * sin( theta ) )
			
			double dleft = leftPosition - self_motor_left_position.GetValue();
			double dright = rightPosition - self_motor_right_position.GetValue();
			double dcenter = ( dleft + dright ) / 2;
			
			double phi = ( dright - dleft ) / baselineMeters;
			
			double theta = Math.toRadians( self_pose_yaw.GetValue() );
			double thetaprime = theta + phi;
			double xprime = self_pose_x.GetValue() + ( dcenter * Math.cos( theta ) );
			double yprime = self_pose_y.GetValue() + ( dcenter * Math.sin( theta ) );

			agent.Update( self_motor_left_current, leftCurrent );
			agent.Update( self_motor_left_position, leftPosition );
			agent.Update( self_motor_left_velocity, leftVelocity );

			agent.Update( self_motor_right_current, rightCurrent );
			agent.Update( self_motor_right_position, rightPosition );
			agent.Update( self_motor_right_velocity, rightVelocity );

			agent.Update( self_pose_x, xprime );
			agent.Update( self_pose_y, yprime );
			thetaprime = Math.toDegrees( thetaprime ) % 360.0;
			if ( thetaprime < 0 )
			{
				thetaprime += 360.0;
			}
			agent.Update( self_pose_yaw, thetaprime );
		}
		
		// process output
		for ( int i = 0; i < agent.GetNumberCommands(); ++i ) 
		{
			Identifier commandId = agent.GetCommand( i );
			String commandName = commandId.GetAttribute();
			
			if ( commandName.equals( "move" ) )
			{
				double leftCommand = 0;
				double rightCommand = 0;
		
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
					synchronized ( botState )
					{
						botState.left = leftCommand / 2;
						botState.right = rightCommand / 2;
					}
				}
				commandId.AddStatusComplete();
				continue;
			}
			
			System.out.println( "Unknown command: " + commandName );
			commandId.AddStatusError();
		}
		
		//System.out.printf( "%15f %15f %15f %15f\r", leftInput, rightInput, leftCommand, rightCommand );
		agent.Commit();
	}
	
	public static void main(String args[])
	{
		new SoaruOrc();
	}
}

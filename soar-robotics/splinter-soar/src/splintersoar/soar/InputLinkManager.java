package splintersoar.soar;

import sml.*;
import splintersoar.*;

public class InputLinkManager {

	Agent agent;

	Identifier override;
	StringElement override_active;
	Identifier override_move;
	FloatElement override_move_left;
	FloatElement override_move_right;
	
	FloatElement self_motor_left_current;
	IntElement self_motor_left_position;
	FloatElement self_motor_left_velocity;
	FloatElement self_motor_right_current;
	IntElement self_motor_right_position;
	FloatElement self_motor_right_velocity;

	IntElement time_seconds;
	IntElement time_microseconds;

	FloatElement self_pose_x;
	FloatElement self_pose_y;
	FloatElement self_pose_z;
	FloatElement self_pose_yaw;
	
	SplinterState state;
	long lastTime = 0;
	
	OverrideInterface overrideInterface;

	public InputLinkManager( Agent agent, SplinterState state )
	{
		this.agent = agent;
		this.state = state;
		
		this.agent.SetBlinkIfNoChange( false );
		
		// wait for valid data
		while ( state.utime == lastTime )
		{
			SplinterSoar.logger.fine( "Waiting for valid splinter state" );
			try 
			{
				Thread.sleep( 200 );
			} catch ( InterruptedException ignored ) 
			{}
		}

		// cache state
		SplinterState stateCopy;
		synchronized ( state )
		{
			stateCopy = new SplinterState( state );
		}
		
		lastTime = stateCopy.utime;
		
		SplinterSoar.logger.fine( "Initializing input link" );
		Identifier inputLink = agent.GetInputLink();
		
		// override
		//     active [false true]
		//         ?motor
		//             left [-1.0..1.0]
		//             right [-1.0..1.0]
		{
			override = agent.CreateIdWME( inputLink, "override" );
			override_active = agent.CreateStringWME( override, "active", "false" );
		}
		
		// self
		//     geometry
		//         baseline-meters [0.0..]
		//         tick-meters [0.0..]
		//     motor
		//         left
		//             current [0.0..]
		//             position [float]
		//             velocity [float] may only be positive (and not indicate direction), check
		//         right 
		//             current [0.0..]
		//             position [float]
		//             velocity [float] may only be positive (and not indicate direction), check
		//     pose
		//             x [float]
		//             y [float]
		//             z [float]
		//             yaw [float]
		{
			Identifier self = agent.CreateIdWME( inputLink, "self" );

			{
				Identifier self_geometry = agent.CreateIdWME( self, "geometry" );
				
				agent.CreateFloatWME( self_geometry, "baseline-meters", state.baselineMeters );
				agent.CreateFloatWME( self_geometry, "tick-meters", state.tickMeters );
			}
			
			{
				Identifier self_motor = agent.CreateIdWME( self, "motor" );
				
				Identifier self_motor_left = agent.CreateIdWME( self_motor, "left" );
				self_motor_left_current = agent.CreateFloatWME( self_motor_left, "current", stateCopy.leftCurrent );
				self_motor_left_position = agent.CreateIntWME( self_motor_left, "position", stateCopy.leftPosition );
				self_motor_left_velocity = agent.CreateFloatWME( self_motor_left, "velocity", stateCopy.leftVelocity );

				Identifier self_motor_right = agent.CreateIdWME( self_motor, "right" );
				self_motor_right_current = agent.CreateFloatWME( self_motor_right, "current", stateCopy.rightCurrent );
				self_motor_right_position = agent.CreateIntWME( self_motor_right, "position", stateCopy.rightPosition );
				self_motor_right_velocity = agent.CreateFloatWME( self_motor_right, "velocity", stateCopy.rightVelocity );
			}
			
			{
				Identifier self_pose = agent.CreateIdWME( self, "pose" );
				
				self_pose_x = agent.CreateFloatWME( self_pose, "x", stateCopy.x );
				self_pose_y = agent.CreateFloatWME( self_pose, "y", stateCopy.y );
				self_pose_yaw = agent.CreateFloatWME( self_pose, "yaw", stateCopy.yaw );
			}
		}
		
		// time
		//     seconds
		//     microseconds
		{
			Identifier time = agent.CreateIdWME( inputLink, "time" );

			int seconds = (int)( System.nanoTime() / 1000000000 );
			int microseconds = (int)( System.nanoTime() % 1000000000 );
			microseconds /= 1000;
			
			time_seconds = agent.CreateIntWME( time, "seconds",  seconds);
			time_microseconds = agent.CreateIntWME( time, "microseconds", microseconds );
		}
		
		agent.Commit();
	}

	public void setOverride( OverrideInterface overrideInterface )
	{
		this.overrideInterface = overrideInterface;
	}
	
	void updateOverride()
	{
		if ( override == null )
		{
			agent.Update( override_active, "false" );
			if ( override_move != null )
			{
				agent.DestroyWME( override_move );
				override_move = null;
				override_move_left = null;
				override_move_right = null;
			}
		}
		else
		{
			agent.Update( override_active, "true" );
			if ( override_move == null )
			{
				override_move = agent.CreateIdWME( override, "move" );
				override_move_left = agent.CreateFloatWME( override_move, "left", overrideInterface.getLeft() );
				override_move_right = agent.CreateFloatWME( override_move, "right", overrideInterface.getRight() );
			}
			else
			{
				agent.Update( override_move_left, overrideInterface.getLeft() );
				agent.Update( override_move_right, overrideInterface.getRight() );
			}
		}
	}
	
	void updateTime()
	{
		int seconds = (int)( System.nanoTime() / 1000000000 );
		int microseconds = (int)( System.nanoTime() % 1000000000 );
		microseconds /= 1000;
		
		agent.Update( time_seconds, seconds );
		agent.Update( time_microseconds, microseconds );
	}
	
	public void update()
	{
		updateOverride();
		updateTime();
		
		// update robot state if we have new state
		if ( state.utime != lastTime )
		{
			// cache state
			SplinterState stateCopy;
			synchronized ( state )
			{
				stateCopy = new SplinterState( state );
			}
			
			lastTime = stateCopy.utime;
			
			agent.Update( self_motor_left_current, stateCopy.leftCurrent );
			agent.Update( self_motor_left_position, stateCopy.leftPosition );
			agent.Update( self_motor_left_velocity, stateCopy.leftVelocity );

			agent.Update( self_motor_right_current, stateCopy.rightCurrent );
			agent.Update( self_motor_right_position, stateCopy.rightPosition );
			agent.Update( self_motor_right_velocity, stateCopy.rightVelocity );

			agent.Update( self_pose_x, stateCopy.x );
			agent.Update( self_pose_y, stateCopy.y );
			double yaw = Math.toDegrees( stateCopy.yaw ) % 360.0;
			if ( yaw < 0 )
			{
				yaw += 360.0;
			}
			agent.Update( self_pose_yaw, yaw );
		}
	}
}

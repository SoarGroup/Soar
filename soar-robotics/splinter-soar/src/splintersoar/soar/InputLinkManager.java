package splintersoar.soar;

import sml.*;
import splintersoar.*;

public class InputLinkManager {

	Agent agent;
	Waypoints waypoints;

	Identifier override;
	StringElement override_active;
	Identifier override_motor;
	FloatElement override_motor_left;
	FloatElement override_motor_right;

	Identifier ranges;

	Identifier self;
	FloatElement self_motor_left_current;
	IntElement self_motor_left_position;
	FloatElement self_motor_left_velocity;
	FloatElement self_motor_right_current;
	IntElement self_motor_right_position;
	FloatElement self_motor_right_velocity;
	FloatElement self_pose_x;
	FloatElement self_pose_y;
	FloatElement self_pose_z;
	FloatElement self_pose_yaw;
	StringElement self_name;
	Identifier self_waypoints;

	Identifier time;
	IntElement time_seconds;
	IntElement time_microseconds;

	SplinterState state;
	long lastTime = 0;
	
	OverrideInterface overrideInterface;
	SoarTime soarTime;
	
	class SoarTime
	{
		int seconds;
		int microseconds;
		final static long nanosecondsPerSecond = 1000000000;
		
		SoarTime()
		{
			update();
		}
		
		void update()
		{
			long current = System.nanoTime();
			seconds = (int)( current / nanosecondsPerSecond );
			microseconds = (int)( current % nanosecondsPerSecond );
			microseconds /= 1000;
		}
	}

	public InputLinkManager( Agent agent, Waypoints waypoints, SplinterState state )
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
		
		// Please see default-robot.vsa for input link definition and comments!
		
		{
			override = agent.CreateIdWME( inputLink, "override" );
			override_active = agent.CreateStringWME( override, "active", "false" );
		}
		
		{
			ranges = agent.CreateIdWME( inputLink, "ranges" );
		}
		
		{
			self = agent.CreateIdWME( inputLink, "self" );

			{
				Identifier self_geometry = agent.CreateIdWME( self, "geometry" );
				
				agent.CreateFloatWME( self_geometry, "baseline-meters", state.baselineMeters );
				agent.CreateFloatWME( self_geometry, "tick-meters", state.tickMeters );
				agent.CreateFloatWME( self_geometry, "length", state.length );
				agent.CreateFloatWME( self_geometry, "width", state.width );
				agent.CreateIntWME( self_geometry, "ranger-slices", state.rangerSlices );
			}
			
			{
				Identifier self_motor = agent.CreateIdWME( self, "motor" );
				
				Identifier self_motor_left = agent.CreateIdWME( self_motor, "left" );
				{
					self_motor_left_current = agent.CreateFloatWME( self_motor_left, "current", stateCopy.leftCurrent );
					self_motor_left_position = agent.CreateIntWME( self_motor_left, "position", stateCopy.leftPosition );
					self_motor_left_velocity = agent.CreateFloatWME( self_motor_left, "velocity", stateCopy.leftVelocity );
				}
				
				Identifier self_motor_right = agent.CreateIdWME( self_motor, "right" );
				{
					self_motor_right_current = agent.CreateFloatWME( self_motor_right, "current", stateCopy.rightCurrent );
					self_motor_right_position = agent.CreateIntWME( self_motor_right, "position", stateCopy.rightPosition );
					self_motor_right_velocity = agent.CreateFloatWME( self_motor_right, "velocity", stateCopy.rightVelocity );
				}
			}
			
			self_name = agent.CreateStringWME( self, "name", agent.GetAgentName() );

			{
				Identifier self_pose = agent.CreateIdWME( self, "pose" );
				{
					self_pose_x = agent.CreateFloatWME( self_pose, "x", stateCopy.x );
					self_pose_y = agent.CreateFloatWME( self_pose, "y", stateCopy.y );
					self_pose_yaw = agent.CreateFloatWME( self_pose, "yaw", stateCopy.yaw );
				}
			}
			
			self_waypoints = agent.CreateIdWME( self, "waypoints" );
			waypoints.setRootIdentifier( self_waypoints );
		}
		
		{
			time = agent.CreateIdWME( inputLink, "time" );

			soarTime = new SoarTime();
			time_seconds = agent.CreateIntWME( time, "seconds",  soarTime.seconds );
			time_microseconds = agent.CreateIntWME( time, "microseconds", soarTime.microseconds );
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
			if ( override_motor != null )
			{
				agent.DestroyWME( override_motor );
				override_motor = null;
				override_motor_left = null;
				override_motor_right = null;
			}
		}
		else
		{
			agent.Update( override_active, "true" );
			if ( override_motor == null )
			{
				override_motor = agent.CreateIdWME( override, "motor" );
				override_motor_left = agent.CreateFloatWME( override_motor, "left", overrideInterface.getLeft() );
				override_motor_right = agent.CreateFloatWME( override_motor, "right", overrideInterface.getRight() );
			}
			else
			{
				//System.out.print( overrideInterface.getLeft() + "                   \r" );
				agent.Update( override_motor_left, overrideInterface.getLeft() );
				agent.Update( override_motor_right, overrideInterface.getRight() );
			}
		}
	}
	
	void updateTime()
	{
		soarTime.update();
		agent.Update( time_seconds, soarTime.seconds );
		agent.Update( time_microseconds, soarTime.microseconds );
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
			
			waypoints.setNewRobotPose( stateCopy.x, stateCopy.y, yaw );
		}
	}
}

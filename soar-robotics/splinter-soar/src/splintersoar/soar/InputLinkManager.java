package splintersoar.soar;

import sml.*;
import splintersoar.*;

public class InputLinkManager {

	Agent agent;
	Waypoints waypoints;
	Ranger ranger;

	Identifier ranges;
	
	Identifier self;
	IntElement self_motor_left_position;
	IntElement self_motor_right_position;
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
		this.waypoints = waypoints;
		
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
			ranges = agent.CreateIdWME( inputLink, "ranges" );
			ranger = new Ranger( agent, ranges, stateCopy.rangerSlices );
			if ( stateCopy.ranger != null )
			{
				ranger.update( stateCopy.rangerutime, stateCopy.ranger );
			}
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
					self_motor_left_position = agent.CreateIntWME( self_motor_left, "position", stateCopy.motorPosition[0] );
				}
				
				Identifier self_motor_right = agent.CreateIdWME( self_motor, "right" );
				{
					self_motor_right_position = agent.CreateIntWME( self_motor_right, "position", stateCopy.motorPosition[1] );
				}
			}
			
			self_name = agent.CreateStringWME( self, "name", agent.GetAgentName() );

			{
				Identifier self_pose = agent.CreateIdWME( self, "pose" );
				{
					self_pose_x = agent.CreateFloatWME( self_pose, "x", stateCopy.pos[0] );
					self_pose_y = agent.CreateFloatWME( self_pose, "y", stateCopy.pos[1] );
					self_pose_yaw = agent.CreateFloatWME( self_pose, "yaw", Math.toDegrees( stateCopy.yaw ) );
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

	public void update()
	{
		soarTime.update();
		agent.Update( time_seconds, soarTime.seconds );
		agent.Update( time_microseconds, soarTime.microseconds );
		
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
			
			agent.Update( self_motor_left_position, stateCopy.motorPosition[0] );
			agent.Update( self_motor_right_position, stateCopy.motorPosition[1] );

			agent.Update( self_pose_x, stateCopy.pos[0] );
			agent.Update( self_pose_y, stateCopy.pos[1] );
			double yaw = stateCopy.yaw % ( 2 * Math.PI );
			while ( yaw < 0 )
			{
				yaw += ( 2 * Math.PI );
			}
			agent.Update( self_pose_yaw, Math.toDegrees( yaw ) );

			ranger.update( stateCopy.rangerutime, stateCopy.ranger );
			
			waypoints.setNewRobotPose( stateCopy.pos, yaw );
		}
	}
}

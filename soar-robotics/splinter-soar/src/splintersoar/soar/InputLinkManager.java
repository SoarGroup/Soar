package splintersoar.soar;

import sml.*;
import splintersoar.*;
import splintersoar.orc.OrcOutput;
import splintersoar.ranger.RangerState;

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

	long lastTime;
	
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

	public InputLinkManager( Agent agent, Waypoints waypoints, OrcOutput splinterOutput, RangerState rangerState )
	{
		this.agent = agent;
		this.waypoints = waypoints;
		
		this.agent.SetBlinkIfNoChange( false );
		
		lastTime = splinterOutput.utime;
		
		SplinterSoar.logger.fine( "Initializing input link" );
		Identifier inputLink = agent.GetInputLink();
		
		// Please see default-robot.vsa for input link definition and comments!
		
		{
			ranges = agent.CreateIdWME( inputLink, "ranges" );
			ranger = new Ranger( agent, ranges, rangerState );
		}
		
		{
			self = agent.CreateIdWME( inputLink, "self" );

			{
				Identifier self_geometry = agent.CreateIdWME( self, "geometry" );
				
				agent.CreateFloatWME( self_geometry, "baseline-meters", splinterOutput.BASELINE_METERS );
				agent.CreateFloatWME( self_geometry, "tick-meters", splinterOutput.TICK_METERS );
				agent.CreateFloatWME( self_geometry, "length", splinterOutput.LENGTH_METERS );
				agent.CreateFloatWME( self_geometry, "width", splinterOutput.WIDTH_METERS );
			}
			
			{
				Identifier self_motor = agent.CreateIdWME( self, "motor" );
				
				Identifier self_motor_left = agent.CreateIdWME( self_motor, "left" );
				{
					self_motor_left_position = agent.CreateIntWME( self_motor_left, "position", splinterOutput.motorPosition[0] );
				}
				
				Identifier self_motor_right = agent.CreateIdWME( self_motor, "right" );
				{
					self_motor_right_position = agent.CreateIntWME( self_motor_right, "position", splinterOutput.motorPosition[1] );
				}
			}
			
			self_name = agent.CreateStringWME( self, "name", agent.GetAgentName() );

			{
				Identifier self_pose = agent.CreateIdWME( self, "pose" );
				{
					self_pose_x = agent.CreateFloatWME( self_pose, "x", splinterOutput.xyt[0] );
					self_pose_y = agent.CreateFloatWME( self_pose, "y", splinterOutput.xyt[1] );
					self_pose_yaw = agent.CreateFloatWME( self_pose, "yaw", Math.toDegrees( splinterOutput.xyt[2] ) );
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

	public void update( OrcOutput splinterOutput, RangerState rangerState )
	{
		soarTime.update();
		agent.Update( time_seconds, soarTime.seconds );
		agent.Update( time_microseconds, soarTime.microseconds );
		
		// update robot state if we have new state
		if ( splinterOutput.utime != lastTime )
		{
			lastTime = splinterOutput.utime;
			
			agent.Update( self_motor_left_position, splinterOutput.motorPosition[0] );
			agent.Update( self_motor_right_position, splinterOutput.motorPosition[1] );

			agent.Update( self_pose_x, splinterOutput.xyt[0] );
			agent.Update( self_pose_y, splinterOutput.xyt[1] );
			double yaw = splinterOutput.xyt[2] % ( 2 * Math.PI );
			while ( yaw < 0 )
			{
				yaw += ( 2 * Math.PI );
			}
			agent.Update( self_pose_yaw, Math.toDegrees( yaw ) );

			waypoints.setNewRobotPose( splinterOutput.xyt );
		}

		ranger.update( rangerState );
	}
}

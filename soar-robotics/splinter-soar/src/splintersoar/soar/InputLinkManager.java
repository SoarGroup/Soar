package splintersoar.soar;

import erp.geom.Geometry;
import erp.math.MathUtil;
import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import splintersoar.lcmtypes.splinterstate_t;
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

	public InputLinkManager( Agent agent, Waypoints waypoints, splinterstate_t splinterState, RangerState rangerState )
	{
		this.agent = agent;
		this.waypoints = waypoints;
		
		this.agent.SetBlinkIfNoChange( false );
		
		lastTime = splinterState.utime;
		
		Identifier inputLink = agent.GetInputLink();
		
		// Please see default-robot.vsa for input link definition and comments!
		
		{
			ranges = agent.CreateIdWME( inputLink, "ranges" );
			ranger = new Ranger( agent, ranges, rangerState );
		}
		
		{
			self = agent.CreateIdWME( inputLink, "self" );

			{
				Identifier self_motor = agent.CreateIdWME( self, "motor" );
				
				Identifier self_motor_left = agent.CreateIdWME( self_motor, "left" );
				{
					self_motor_left_position = agent.CreateIntWME( self_motor_left, "position", splinterState.leftodom );
				}
				
				Identifier self_motor_right = agent.CreateIdWME( self_motor, "right" );
				{
					self_motor_right_position = agent.CreateIntWME( self_motor_right, "position", splinterState.rightodom );
				}
			}
			
			self_name = agent.CreateStringWME( self, "name", agent.GetAgentName() );

			{
				Identifier self_pose = agent.CreateIdWME( self, "pose" );
				{
					self_pose_x = agent.CreateFloatWME( self_pose, "x", splinterState.pose.pos[0] );
					self_pose_y = agent.CreateFloatWME( self_pose, "y", splinterState.pose.pos[1] );
					self_pose_yaw = agent.CreateFloatWME( self_pose, "yaw", 
							Math.toDegrees( MathUtil.mod2pi( Geometry.quatToRollPitchYaw( splinterState.pose.orientation ) [2] ) ) );
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

	public void update( splinterstate_t splinterState, RangerState rangerState )
	{
		soarTime.update();
		agent.Update( time_seconds, soarTime.seconds );
		agent.Update( time_microseconds, soarTime.microseconds );
		
		// update robot state if we have new state
		if ( splinterState.utime != lastTime )
		{
			lastTime = splinterState.utime;
			
			agent.Update( self_motor_left_position, splinterState.leftodom );
			agent.Update( self_motor_right_position, splinterState.rightodom );

			agent.Update( self_pose_x, splinterState.pose.pos[0] );
			agent.Update( self_pose_y, splinterState.pose.pos[1] );
			agent.Update( self_pose_yaw, Math.toDegrees( MathUtil.mod2pi( Geometry.quatToRollPitchYaw( splinterState.pose.orientation ) [2] ) ) ) ;

			waypoints.setNewRobotPose( splinterState.pose );
		}

		ranger.update( rangerState );
	}
}

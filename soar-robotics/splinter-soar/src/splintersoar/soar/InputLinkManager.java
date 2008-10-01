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
	FloatElement self_motor_left_position;
	FloatElement self_motor_left_velocity;
	FloatElement self_motor_right_current;
	FloatElement self_motor_right_position;
	FloatElement self_motor_right_velocity;
	
	double leftInitialPosition;
	double rightInitialPosition;
	
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
		leftInitialPosition = stateCopy.leftPosition;
		rightInitialPosition = stateCopy.rightPosition;
		
		SplinterSoar.logger.fine( "Initializing input link" );
		Identifier inputLink = agent.GetInputLink();
		
		// override
		//     active [false true]
		//         ?move
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
				self_motor_left_position = agent.CreateFloatWME( self_motor_left, "position", stateCopy.leftPosition );
				self_motor_left_velocity = agent.CreateFloatWME( self_motor_left, "velocity", stateCopy.leftVelocity );

				Identifier self_motor_right = agent.CreateIdWME( self_motor, "right" );
				self_motor_right_current = agent.CreateFloatWME( self_motor_right, "current", stateCopy.rightCurrent );
				self_motor_right_position = agent.CreateFloatWME( self_motor_right, "position", stateCopy.rightPosition );
				self_motor_right_velocity = agent.CreateFloatWME( self_motor_right, "velocity", stateCopy.rightVelocity );
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
	
	public void update()
	{
		updateOverride();
		
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
			
			double leftTotalPosition = ( stateCopy.leftPosition - leftInitialPosition ) * stateCopy.tickMeters;
			double leftVelocity = stateCopy.leftVelocity * stateCopy.tickMeters;
			
			double rightTotalPosition = ( stateCopy.rightPosition - rightInitialPosition ) * stateCopy.tickMeters;
			double rightVelocity = stateCopy.rightVelocity * stateCopy.tickMeters;
			
			// Equations from A Primer on Odopmetry and Motor Control, Olson 2006
			// dleft, dright: distance wheel travelled
			// dbaseline: wheelbase
			//
			// dcenter = ( dleft + dright ) / 2
			// phi = ( dright - dleft ) / dbaseline
			// thetaprime = theta + phi
			// xprime = x + ( dcenter * cos( theta ) )
			// yprime = y + ( dcenter * sin( theta ) )
			
			double dleft = leftTotalPosition - self_motor_left_position.GetValue();
			double dright = rightTotalPosition - self_motor_right_position.GetValue();
			double dcenter = ( dleft + dright ) / 2;
			
			double phi = ( dright - dleft ) / stateCopy.baselineMeters;
			
			double theta = Math.toRadians( self_pose_yaw.GetValue() );
			double thetaprime = theta + phi;
			double xprime = self_pose_x.GetValue() + ( dcenter * Math.cos( theta ) );
			double yprime = self_pose_y.GetValue() + ( dcenter * Math.sin( theta ) );

			agent.Update( self_motor_left_current, stateCopy.leftCurrent );
			agent.Update( self_motor_left_position, leftTotalPosition );
			agent.Update( self_motor_left_velocity, leftVelocity );

			agent.Update( self_motor_right_current, stateCopy.rightCurrent );
			agent.Update( self_motor_right_position, rightTotalPosition );
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
	}
}

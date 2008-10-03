package splintersoar;

import java.io.DataInputStream;
import java.io.IOException;

import laserloc.*;
import java.util.*;

import erp.geom.Geometry;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.pose_t;

import orc.Motor;
import orc.Orc;
import orc.QuadratureEncoder;

public class OrcInterface implements LCMSubscriber
{
	Timer timer = new Timer();
	static final int UPDATE_HZ = 30;
	
	SplinterState state = new SplinterState();
	
	private Orc orc;
	private Motor leftMotor;
	private Motor rightMotor;
	private QuadratureEncoder leftOdom;
	private QuadratureEncoder rightOdom;
	
	int leftPreviousPosition = 0;
	int rightPreviousPosition = 0;
	
	public OrcInterface()
	{
		orc = Orc.makeOrc();
		
		leftMotor = new Motor( orc, 1, true );
		leftOdom = new QuadratureEncoder( orc, 1, true);
		leftPreviousPosition = leftOdom.getPosition();
		
		rightMotor = new Motor( orc, 0, false );
		rightOdom = new QuadratureEncoder( orc, 0, false);
		rightPreviousPosition = rightOdom.getPosition();
		
		timer.schedule( new UpdateTask(), 0, 1000 / UPDATE_HZ );
	}
	
	public SplinterState getState()
	{
		return state;
	}
	
	public void shutdown()
	{
		timer.cancel();
	}

	class UpdateTask extends TimerTask
	{
		@Override
		public void run()
		{
			double left, right, prevX, prevY, prevYaw;
			synchronized ( state )
			{
				left = state.left;
				right = state.right;
				
				prevX = state.x;
				prevY = state.y;
				prevYaw = state.yaw;
			}
			
			//// Communicate with orc
			// FIXME: each of the next get lines gets a new OrcStatus message. 
			// Really, we should only be getting one status message per update.
			// In the interest of not prematurely optimizing, I will leave this
			// like it is for now.
			double leftCurrent = leftMotor.getCurrent();
			int leftPosition = leftOdom.getPosition();
			double leftVelocity = leftOdom.getVelocity() * state.tickMeters;

			double rightCurrent = rightMotor.getCurrent();
			int rightPosition = rightOdom.getPosition();
			double rightVelocity = rightOdom.getVelocity() * state.tickMeters;

			// write new commands
			leftMotor.setPWM( left );
			rightMotor.setPWM( right );
			//// end orc communication

			// Equations from A Primer on Odopmetry and Motor Control, Olson 2006
			// dleft, dright: distance wheel travelled
			// dbaseline: wheelbase
			//
			// dcenter = ( dleft + dright ) / 2
			// phi = ( dright - dleft ) / dbaseline
			// thetaprime = theta + phi
			// xprime = x + ( dcenter * cos( theta ) )
			// yprime = y + ( dcenter * sin( theta ) )
			
			double dleft = ( leftPosition - leftPreviousPosition ) * state.tickMeters;
			double dright = ( rightPosition - rightPreviousPosition ) * state.tickMeters;
			double dcenter = ( dleft + dright ) / 2;
			
			double phi = ( dright - dleft ) / state.baselineMeters;
			
			double theta = prevYaw;
			double thetaprime = theta + phi;
			double xprime = prevX + ( dcenter * Math.cos( theta ) );
			double yprime = prevY + ( dcenter * Math.sin( theta ) );
			
			leftPreviousPosition = leftPosition;
			rightPreviousPosition = rightPosition;
			
			long utime = System.nanoTime() / 1000;			
			synchronized ( state )
			{
				state.utime = utime;

				state.leftCurrent = leftCurrent;
				state.leftPosition = leftPosition;
				state.leftVelocity = leftVelocity;
				
				state.rightCurrent = rightCurrent;
				state.rightPosition = rightPosition;
				state.rightVelocity = rightVelocity;
				
				state.x = xprime;
				state.y = yprime;
				state.yaw = thetaprime;
			}
		}
	}
	
	@Override
	public void messageReceived( LCM lcm, String channel, DataInputStream ins ) 
	{
		if ( channel.equals( LaserLoc.pose_channel ) )
		{
			try 
			{
				pose_t pose = new pose_t( ins );
				
				long utime = System.nanoTime() / 1000;
				double yaw = Geometry.quatToRollPitchYaw( pose.orientation )[2];

				// FIXME: possibly slow
				leftPreviousPosition = leftOdom.getPosition();
				rightPreviousPosition = rightOdom.getPosition();
				
				synchronized ( state )
				{
					state.utime = utime;
					state.x = pose.pos[0];
					state.y = pose.pos[1];
					state.yaw = yaw;
				}
			} 
			catch ( IOException ex ) 
			{
				System.out.println( "Error decoding pose message: " + ex );
			}
		}
	}

}

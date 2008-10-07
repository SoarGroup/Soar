package splintersoar;

import java.io.DataInputStream;
import java.io.IOException;

import laserloc.*;
import java.util.*;

import erp.geom.Geometry;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;
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
	
	pose_t pose;
	laser_t laserData;
	RangerData [] rangerData = new RangerData[ state.rangerSlices ];
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
		
		for ( int index = 0; index < rangerData.length; ++index)
		{
			rangerData[ index ] = new RangerData();
		}
		
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
			double left, right, targetYaw, targetYawTolerance, prevX, prevY, prevYaw;
			boolean targetYawEnabled;
			synchronized ( state )
			{
				left = state.left;
				//System.out.println( "R: " + state.left );
				right = state.right;
				targetYaw = state.targetYaw;
				targetYawTolerance = state.targetYawTolerance;
				targetYawEnabled = state.targetYawEnabled;
				
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
			if ( targetYawEnabled == false )
			{
				//System.out.print( left + "               \r" );
				leftMotor.setPWM( left );
				rightMotor.setPWM( right );
			} 
			//// end orc communication (more after yaw calculation)

			double thetaprime, xprime, yprime;
			if ( pose != null )
			{
				thetaprime = Geometry.quatToRollPitchYaw( pose.orientation )[2];
				xprime = pose.pos[0];
				yprime = pose.pos[1];
				
				pose = null;
			}
			else
			{
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
				
				thetaprime = theta + phi;
				xprime = prevX + ( dcenter * Math.cos( theta ) );
				yprime = prevY + ( dcenter * Math.sin( theta ) );
			}
			
			//// start orc communication
			if ( targetYawEnabled )
			{
				double angleOff = targetYaw - thetaprime;
				while ( angleOff < 0 )
				{
					angleOff += 360;
				}
				angleOff %= 360;
				
				if ( angleOff < targetYawTolerance )
				{
					leftMotor.setPWM( left );
					rightMotor.setPWM( right * -1 );
				}
				else if ( angleOff > targetYawTolerance )
				{
					leftMotor.setPWM( left * -1 );
					rightMotor.setPWM( right );
				}
				else
				{
					leftMotor.setPWM( 0 );
					rightMotor.setPWM( 0 );
				}
			} 
			//// end orc communication

			leftPreviousPosition = leftPosition;
			rightPreviousPosition = rightPosition;

			//// ranger update
			if ( laserData != null )
			{
				// FIXME verify this is general, I think sliceChunk must have no remainder
				assert state.rangerSlices == 5;
				assert rangerData != null;
				assert rangerData.length == state.rangerSlices;
				
				int sliceChunk = laserData.nranges / state.rangerSlices; // a round number with 180/5 (36)
				
				for ( int slice = 0, index = 0; slice < rangerData.length; ++slice )
				{
					rangerData[ slice ].start = laserData.rad0 + index * laserData.radstep;

					double smallest_range = Double.MAX_VALUE;
					
					for ( ; index < sliceChunk * state.rangerSlices; ++index )
					{
						if ( laserData.ranges[ index ] < smallest_range )
						{
							smallest_range = laserData.ranges[ index ];
						}
					}
					
					rangerData[ slice ].end = laserData.rad0 + ( index - 1 ) * laserData.radstep;
					rangerData[ slice ].distance = smallest_range;
				}
			}
			//// end ranger update
			
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
				
				if ( laserData != null )
				{
					state.ranger = Arrays.copyOf( rangerData, rangerData.length );
				}
				
				state.x = xprime;
				state.y = yprime;
				state.yaw = thetaprime;
			}

			laserData = null;
		}
	}
	
	@Override
	public void messageReceived( LCM lcm, String channel, DataInputStream ins ) 
	{
		if ( channel.equals( LaserLoc.pose_channel ) )
		{
			if ( pose != null )
			{
				return;
			}

			try 
			{
				pose = new pose_t( ins );
			} 
			catch ( IOException ex ) 
			{
				System.out.println( "Error decoding pose message: " + ex );
			}
		}
		else if ( channel.equals( "LASER_FRONT" ) )
		{
			if ( laserData != null )
			{
				return;
			}
			
			try 
			{
				laserData = new laser_t( ins );
			} 
			catch ( IOException ex ) 
			{
				System.out.println( "Error decoding LASER_FRONT message: " + ex );
			}
		}
	}

}

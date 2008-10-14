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
		
		SplinterSoar.logger.info( "Orc up" );
		timer.schedule( new UpdateTask(), 0, 1000 / UPDATE_HZ );
	}
	
	public SplinterState getState()
	{
		return state;
	}
	
	public void shutdown()
	{
		timer.cancel();
		SplinterSoar.logger.info( "Orc down" );
	}

	boolean translating = false;
	double prevYawCalcLocX = 0;
	double prevYawCalcLocY = 0;
	double yawCalcDistThreshold = 0.1;
	boolean hadSickData = false;
		
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
				leftMotor.setPWM( left );
				rightMotor.setPWM( right );
			} 
			//// end orc communication (more after yaw calculation)

			// calculation of new yaw is always based on odometry
			double dleft = ( leftPosition - leftPreviousPosition ) * state.tickMeters;
			double dright = ( rightPosition - rightPreviousPosition ) * state.tickMeters;
			double phi = ( dright - dleft ) / state.baselineMeters;
			double thetaprime = prevYaw + phi;

			// calculation of x,y
			double xprime, yprime;
			if ( pose != null )
			{
				System.out.print( "s" );
				xprime = pose.pos[0];
				yprime = pose.pos[1];
				
				hadSickData = true;
				pose = null;
			}
			else
			{
				System.out.print( "d" );
				// Equations from A Primer on Odopmetry and Motor Control, Olson 2006
				// dleft, dright: distance wheel travelled
				// dbaseline: wheelbase
				//
				// dcenter = ( dleft + dright ) / 2
				// phi = ( dright - dleft ) / dbaseline
				// thetaprime = theta + phi
				// xprime = x + ( dcenter * cos( theta ) )
				// yprime = y + ( dcenter * sin( theta ) )
				
				double dcenter = ( dleft + dright ) / 2;
				
				xprime = prevX + ( dcenter * Math.cos( prevYaw ) );
				yprime = prevY + ( dcenter * Math.sin( prevYaw ) );
			}
			
			// if we start moving forward or backward, mark that location
			// if we continue to move forward and move past a certain distance, recalculate yaw
			if ( hadSickData )
			{
				if ( translating )
				{
					if ( ( left >= 0 && right <= 0 ) || ( left <= 0 && right >= 0 ) )
					{
						translating = false;
						System.out.println( "Stopped translating" );
					}
				
					double deltaX = xprime - prevYawCalcLocX;
					double deltaY = yprime - prevYawCalcLocY;
					
					double distanceTravelled = deltaX * deltaX + deltaY * deltaY;
					distanceTravelled = Math.sqrt( distanceTravelled );
				
					if ( distanceTravelled > yawCalcDistThreshold )
					{
						double newThetaPrime = Math.atan2( deltaY, deltaX );
					
						prevYawCalcLocX = xprime;
						prevYawCalcLocY = yprime;
					
						System.out.format( "Correcting yaw from  %.3f to %.3f%n", Math.toDegrees( thetaprime ), Math.toDegrees( newThetaPrime ) );
					
						thetaprime = newThetaPrime;
						hadSickData = false;
					}
				}
				else
				{
					if ( ( left < 0 && right < 0 ) || ( left > 0 && right > 0 ) )
					{
						prevYawCalcLocX = xprime;
						prevYawCalcLocY = yprime;
						translating = true;
						System.out.println( "Started translating" );
					}
				}
			}
			
			//// start orc communication
			if ( targetYawEnabled )
			{
				double relativeBearingValue = targetYaw - thetaprime;
				if ( relativeBearingValue > Math.PI )
				{
					relativeBearingValue -= 2 * Math.PI;
				} else if ( relativeBearingValue < Math.PI * -1 )
				{
					relativeBearingValue += 2 * Math.PI;
				}
				
				if ( relativeBearingValue < ( 0 - targetYawTolerance ) )
				{
					leftMotor.setPWM( left );
					rightMotor.setPWM( right * -1 );
				}
				else if ( relativeBearingValue > targetYawTolerance )
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

					rangerData[ slice ].distance = Double.MAX_VALUE;
					
					for ( ; index < ( sliceChunk * slice ) + sliceChunk; ++index )
					{
						if ( laserData.ranges[ index ] < rangerData[ slice ].distance )
						{
							rangerData[ slice ].distance = laserData.ranges[ index ];
						}
					}
					
					rangerData[ slice ].end = laserData.rad0 + ( index - 1 ) * laserData.radstep;
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
					state.rangerutime = utime;
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
				SplinterSoar.logger.warning( "Error decoding pose message: " + ex );
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
				SplinterSoar.logger.warning( "Error decoding LASER_FRONT message: " + ex );
			}
		}
	}

}

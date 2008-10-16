package splintersoar;

import java.io.DataInputStream;
import java.io.IOException;

import laserloc.*;
import java.util.*;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;
import lcmtypes.pose_t;

import orc.Motor;
import orc.Orc;
import orc.QuadratureEncoder;

import erp.geom.*;

public class OrcInterface implements LCMSubscriber
{
	Timer timer = new Timer();
	static final int UPDATE_HZ = 30;
	
	SplinterState state = new SplinterState();
	
	private Orc orc;
	private Motor [] motor = new Motor[2];
	private QuadratureEncoder [] odom = new QuadratureEncoder[2];
	
	private double [] command = { 0, 0 };

	LCM lcm;
	
	pose_t pose;
	laser_t laserData;
	pose_t odometry = new pose_t();
	RangerData [] rangerData = new RangerData[ state.rangerSlices ];
	int [] previousMotorPosition = { 0, 0 };
	
	double [] initialPosition = new double[3];
	boolean haveInitialCoords = false;

	boolean testing = false;
	
	public OrcInterface( boolean testing )
	{
		this.testing = testing;
		if ( testing )
		{
			return;
		}
		
		lcm = LCM.getSingleton();
		
		Arrays.fill( initialPosition, 0 );
		
		orc = Orc.makeOrc();
		
		motor[0] = new Motor( orc, 1, true );
		odom[0] = new QuadratureEncoder( orc, 1, true);
		previousMotorPosition[0] = odom[0].getPosition();
		
		motor[1] = new Motor( orc, 0, false );
		odom[1] = new QuadratureEncoder( orc, 0, false);
		previousMotorPosition[1] = odom[1].getPosition();
		
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
	double [] prevYawCalcLoc = new double[3]; 
	double yawCalcDistThreshold = 0.1;
	boolean hadSickData = false;
		
	class UpdateTask extends TimerTask
	{
		@Override
		public void run()
		{
			double targetYaw, targetYawTolerance, prevYaw;
			double [] throttle = new double[2];
			double [] previousPosition = new double[3];
			boolean targetYawEnabled;
			synchronized ( state )
			{
				System.arraycopy( state.throttle, 0, throttle, 0, state.throttle.length );
				targetYaw = state.targetYaw;
				targetYawTolerance = state.targetYawTolerance;
				targetYawEnabled = state.targetYawEnabled;
				
				System.arraycopy( state.pos, 0, previousPosition, 0, state.pos.length );
				prevYaw = state.yaw;
			}
			
			//// Communicate with orc
			// FIXME: each of the next get lines gets a new OrcStatus message. 
			// Really, we should only be getting one status message per update.
			// In the interest of not prematurely optimizing, I will leave this
			// like it is for now.
			int [] motorPosition = { odom[0].getPosition(), odom[1].getPosition() };

			// write new commands
			if ( targetYawEnabled == false )
			{
				commandMotors( throttle );
			} 
			//// end orc communication (more after yaw calculation)

			// calculation of new yaw is always based on odometry
			double dleft = ( motorPosition[0] - previousMotorPosition[0] ) * state.tickMeters;
			double dright = ( motorPosition[1] - previousMotorPosition[1] ) * state.tickMeters;
			double phi = ( dright - dleft ) / state.baselineMeters;
			double thetaprime = prevYaw + phi;
			double dcenter = ( dleft + dright ) / 2;

			// calculation of x,y
			double [] newPosition;
			if ( pose != null )
			{
				if ( haveInitialCoords == false )
				{
					System.arraycopy( pose.pos, 0, initialPosition, 0, pose.pos.length );
					haveInitialCoords = true;
				}
				
				//System.out.print( "*" );
				newPosition = Geometry.subtract( pose.pos, initialPosition );
				
				hadSickData = true;
				pose = null;
			}
			else
			{
				//System.out.print( "." );
				// Equations from A Primer on Odopmetry and Motor Control, Olson 2006
				// dleft, dright: distance wheel travelled
				// dbaseline: wheelbase
				//
				// dcenter = ( dleft + dright ) / 2
				// phi = ( dright - dleft ) / dbaseline
				// thetaprime = theta + phi
				// xprime = x + ( dcenter * cos( theta ) )
				// yprime = y + ( dcenter * sin( theta ) )
				
				newPosition = new double[3];
				newPosition[0] = previousPosition[0] + ( dcenter * Math.cos( prevYaw ) );
				newPosition[1] = previousPosition[1] + ( dcenter * Math.sin( prevYaw ) );
				newPosition[2] = 0;
			}
			
			// if we start moving forward or backward, mark that location
			// if we continue to move forward and move past a certain distance, recalculate yaw
			if ( hadSickData )
			{
				if ( translating )
				{
					if ( ( throttle[0] >= 0 && throttle[1] <= 0 ) || ( throttle[0] <= 0 && throttle[1] >= 0 ) )
					{
						translating = false;
						System.out.println( "Stopped translating" );
					}
				
					double [] deltaPos = Geometry.subtract( newPosition, prevYawCalcLoc );
					double distanceTravelled = Geometry.magnitude( deltaPos );
				
					if ( distanceTravelled > yawCalcDistThreshold )
					{
						double newThetaPrime = Math.atan2( deltaPos[1], deltaPos[0] );
						System.out.format( "Correcting yaw delta %.3f%n", Math.toDegrees( newThetaPrime ) - Math.toDegrees( thetaprime ) );
						thetaprime = newThetaPrime;
					
						System.arraycopy( newPosition, 0, prevYawCalcLoc, 0, newPosition.length ); 
						hadSickData = false;
					}
				}
				else
				{
					if ( ( throttle[0] < 0 && throttle[1] < 0 ) || ( throttle[0] > 0 && throttle[1] > 0 ) )
					{
						System.arraycopy( newPosition, 0, prevYawCalcLoc, 0, newPosition.length );
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
					double [] actualThrottle = Arrays.copyOf( throttle, throttle.length );
					actualThrottle[1] *= -1;
					commandMotors( actualThrottle );
				}
				else if ( relativeBearingValue > targetYawTolerance )
				{
					double [] actualThrottle = Arrays.copyOf( throttle, throttle.length );
					actualThrottle[0] *= -1;
					commandMotors( actualThrottle );
				}
				else
				{
					commandMotors( new double [] { 0, 0 } );
				}
			} 
			//// end orc communication

			System.arraycopy( motorPosition, 0, previousMotorPosition, 0, motorPosition.length );

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

			// Odometry debug
			odometry.pos[0] = dcenter;
			odometry.pos[1] = phi;
			odometry.utime = utime;
			lcm.publish( "ODOM_DEBUG", odometry );

			synchronized ( state )
			{
				state.utime = utime;
				
				System.arraycopy( motorPosition, 0, state.motorPosition, 0, motorPosition.length );
				
				if ( laserData != null )
				{
					state.ranger = Arrays.copyOf( rangerData, rangerData.length );
					state.rangerutime = utime;
				}
				
				System.arraycopy( newPosition, 0, state.pos, 0, newPosition.length );
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

	public static double maxThrottleAccelleration = 2.0;
	
	private long lastutime = 0;
	private void commandMotors( double [] throttle )
	{
		assert throttle[0] <= 1;
		assert throttle[0] >= -1;
		assert throttle[1] <= 1;
		assert throttle[1] >= -1;
		
		if ( lastutime == 0 )
		{
			lastutime = System.nanoTime();
			return;
		}
		long elapsed = System.nanoTime() - lastutime; 
		double elapsedsec = elapsed / 1000000000.0;
		
		double [] delta = Geometry.subtract( throttle, command );

		boolean [] capped = { false, false };
		if ( delta[0] > 0 )
		{
			double newDelta = Math.min( delta[0], elapsedsec * maxThrottleAccelleration );
			if ( delta[0] != newDelta )
			{
				capped[0] = true;
				delta[0] = newDelta;
			}
			//System.out.format( "delta1: %10.3f %10.3f%n", delta[0], delta[1] );
		}
		else if ( delta[0] < 0 )
		{
			double newDelta = Math.max( delta[0], -1 * elapsedsec * maxThrottleAccelleration );
			if ( delta[0] != newDelta )
			{
				capped[0] = true;
				delta[0] = newDelta;
			}
			//System.out.format( "delta2: %10.3f %10.3f%n", delta[0], delta[1] );
		}
		if ( delta[1] > 0 )
		{
			double newDelta = Math.min( delta[1], elapsedsec * maxThrottleAccelleration );
			if ( delta[1] != newDelta )
			{
				capped[1] = true;
				delta[1] = newDelta;
			}
			//System.out.format( "delta3: %10.3f %10.3f%n", delta[0], delta[1] );
		}
		else if ( delta[1] < 0 )
		{
			double newDelta = Math.max( delta[1], -1 * elapsedsec * maxThrottleAccelleration );
			if ( delta[1] != newDelta )
			{
				capped[1] = true;
				delta[1] = newDelta;
			}
			//System.out.format( "delta4: %10.3f %10.3f%n", delta[0], delta[1] );
		}

		command[0] += delta[0];
		command[1] += delta[1];

		//System.out.format( "0: %10.3f %10.3f %10.3f %10.3f %s%n", throttle[0], delta[0], delta[0] / elapsedsec, command[0], capped[0] ? "capped" : "" );
		//System.out.format( "1: %10.3f %10.3f %10.3f %10.3f %s%n", throttle[1], delta[1], delta[1] / elapsedsec, command[1], capped[1] ? "capped" : "" );
		
		lastutime += elapsed;
		
		if ( !testing )
		{
			motor[0].setPWM( command[0] );
			motor[1].setPWM( command[1] );
		}
	}
	
	public static void main( String [] args )
	{
		OrcInterface orc = new OrcInterface( true );
		
		double [] throttle = { -1, -1 };
		while ( true )
		{
			try
			{
				Thread.sleep( 10 );
			} 
			catch ( InterruptedException ignored )
			{}

			orc.commandMotors( throttle );
		}
	}
	
}

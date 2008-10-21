package splintersoar.orc;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.Timer;
import java.util.TimerTask;

import laserloc.LaserLoc;
import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.pose_t;

import orc.Motor;
import orc.Orc;
import orc.OrcStatus;
import splintersoar.SplinterSoar;
import splintersoar.pf.ParticleFilter;

import erp.geom.Geometry;
import erp.math.MathUtil;

public class OrcInterface implements LCMSubscriber, OrcOutputProducer
{
	private Timer timer = new Timer();
	static final long UPDATE_HZ = 30;
	
	private Orc orc;
	private Motor [] motor = new Motor[2];
	private int [] ports = { 1, 0 };
	private boolean [] invert = { true, false };

	private OrcInputProducer inputProducer;
	private OrcOutput previousOutput = new OrcOutput( 0 );

	private double [] command = { 0, 0 };

	private LCM lcm;
	
	private pose_t laserxy;
	
	private double [] initialxy = null;

	public OrcInterface( OrcInputProducer inputProducer )
	{
		this.inputProducer = inputProducer;
		
		lcm = LCM.getSingleton();
		lcm.subscribe( laserloc.LaserLoc.pose_channel, this );
		
		orc = Orc.makeOrc();
		motor[0] = new Motor( orc, ports[0], invert[0] );
		motor[1] = new Motor( orc, ports[1], invert[1] );
		
		SplinterSoar.logger.info( "Orc up" );
		timer.schedule( new PFUpdateTask(), 0, 1000 / UPDATE_HZ );
	}
	
	@Override
	public OrcOutput getOutput()
	{
		synchronized( this )
		{
			return previousOutput.copy();
		}
	}
	
	public void shutdown()
	{
		timer.cancel();
		SplinterSoar.logger.info( "Orc down" );
	}

    public static double [] subtract( double a[], double b[], int len )
    {
		double r[] = new double[len];
	
		for (int i = 0; i < len; i++)
		{
		    r[i] = a[i] - b[i];
		}
	
		return r;
    }

	class PFUpdateTask extends TimerTask
	{		
		int runs = 0;
		long statustimestamp = 0;
		final long STATUS_UPDATE_NANOSECS = 5 * 1000000000L; // 5 seconds
		ParticleFilter pf = new ParticleFilter();
		
		public void run()
		{
			boolean moving;
			OrcOutput currentOutput;
			{
				// Get OrcStatus
				OrcStatus currentStatus = orc.getStatus();
	
				// calculate location if moving
				moving = ( currentStatus.qeiVelocity[0] != 0 ) || ( currentStatus.qeiVelocity[1] != 0 );
	
				// assemble output
				currentOutput = new OrcOutput( currentStatus.utime );
				currentOutput.motorPosition[0] = currentStatus.qeiPosition[ports[0]] * (invert[0] ? -1 : 1);
				currentOutput.motorPosition[1] = currentStatus.qeiPosition[ports[1]] * (invert[1] ? -1 : 1);
			}

			OrcInput input = inputProducer.getInput();
			if ( input == null )
			{
				SplinterSoar.logger.finest( "No input, using default" );
				input = new OrcInput();
			}
			
			if ( moving )
			{				
				double dleft = ( currentOutput.motorPosition[0] - previousOutput.motorPosition[0] ) * previousOutput.TICK_METERS;
				double dright = ( currentOutput.motorPosition[1] - previousOutput.motorPosition[1] ) * previousOutput.TICK_METERS;
				double phi = ( dright - dleft ) / previousOutput.BASELINE_METERS;
				double dcenter = ( dleft + dright ) / 2;

				phi = MathUtil.mod2pi( phi );

				double [] deltaxyt = { dcenter * Math.cos( previousOutput.xyt[2] ), dcenter * Math.sin( previousOutput.xyt[2] ), phi };
	
				// laser update
				double [] adjustedlaserxy = null;
				if ( laserxy != null )
				{
					if ( initialxy == null )
					{
						initialxy = Arrays.copyOf( laserxy.pos, 2 );
						System.out.format( "initialxy: %5.2f, %5.2f%n", initialxy[0], initialxy[1] );
					}
					
					// FIXME can delete when laserxy is really 2 coords instead of overloaded pose
					adjustedlaserxy = subtract( laserxy.pos, initialxy, 2 );

					laserxy = null;
				}
	
				// adjustedlaserxyt could be null
				currentOutput.xyt = pf.update( deltaxyt, adjustedlaserxy );
				
				currentOutput.xyt[2] = MathUtil.mod2pi( currentOutput.xyt[2] );
				System.out.format( "%10.6f %10.6f %10.6f%n", currentOutput.xyt[0], currentOutput.xyt[1], Math.toDegrees( currentOutput.xyt[2] ) );

			}
			
			// if target yaw command, write it out now since we have location
			if ( input.targetYawEnabled )
			{
				double relativeBearingValue = input.targetYaw - currentOutput.xyt[2];
				relativeBearingValue = erp.math.MathUtil.mod2pi( relativeBearingValue );
				
				if ( relativeBearingValue < ( 0 - input.targetYawTolerance ) )
				{
					double [] actualThrottle = { input.throttle[0], input.throttle[1] * -1 };
					commandMotors( currentOutput.utime, actualThrottle );
				}
				else if ( relativeBearingValue > input.targetYawTolerance )
				{
					double [] actualThrottle = { input.throttle[0] * -1, input.throttle[1] };
					commandMotors( currentOutput.utime, actualThrottle );
				}
				else
				{
					commandMotors( currentOutput.utime, new double [] { 0, 0 } );
				}
			} 
			else
			{
				commandMotors( currentOutput.utime, input.throttle );
			}

			// status message
			runs += 1;
			long nanotime = System.nanoTime();
			if ( statustimestamp == 0 )
			{
				statustimestamp = nanotime;
			}
			else if ( nanotime - statustimestamp > STATUS_UPDATE_NANOSECS ) 
			{
				double updatesPerSecond = this.runs / ( ( nanotime - statustimestamp ) / 1000000000.0 );
				System.out.format( "Orc updates running at %6.2f per sec%n", updatesPerSecond );
				statustimestamp = nanotime;
				runs = 0;
			}

			synchronized ( this )
			{
				previousOutput = currentOutput;
			}
		}
	}

	@Override
	public void messageReceived( LCM lcm, String channel, DataInputStream ins ) 
	{
		if ( channel.equals( LaserLoc.pose_channel ) )
		{
			if ( laserxy != null )
			{
				return;
			}

			try 
			{
				laserxy = new pose_t( ins );
			} 
			catch ( IOException ex ) 
			{
				SplinterSoar.logger.warning( "Error decoding laserxy message: " + ex );
			}
		}
	}

	public static double maxThrottleAccellerationPeruSec = 2.0 / 1000000;
	
	private long lastutime = 0;
	private void commandMotors( long utime, double [] throttle )
	{
		assert throttle[0] <= 1;
		assert throttle[0] >= -1;
		assert throttle[1] <= 1;
		assert throttle[1] >= -1;
		
		if ( lastutime == 0 )
		{
			lastutime = utime;
			return;
		}
		
		long elapsed = utime - lastutime; 
		
		double [] delta = Geometry.subtract( throttle, command );

		if ( delta[0] > 0 )
		{
			double newDelta = Math.min( delta[0], elapsed * maxThrottleAccellerationPeruSec );
			if ( delta[0] != newDelta )
			{
				delta[0] = newDelta;
			}
		}
		else if ( delta[0] < 0 )
		{
			double newDelta = Math.max( delta[0], -1 * elapsed * maxThrottleAccellerationPeruSec );
			if ( delta[0] != newDelta )
			{
				delta[0] = newDelta;
			}
		}
		if ( delta[1] > 0 )
		{
			double newDelta = Math.min( delta[1], elapsed * maxThrottleAccellerationPeruSec );
			if ( delta[1] != newDelta )
			{
				delta[1] = newDelta;
			}
		}
		else if ( delta[1] < 0 )
		{
			double newDelta = Math.max( delta[1], -1 * elapsed * maxThrottleAccellerationPeruSec );
			if ( delta[1] != newDelta )
			{
				delta[1] = newDelta;
			}
		}

		command[0] += delta[0];
		command[1] += delta[1];
		
		lastutime += elapsed;
		
		motor[0].setPWM( command[0] );
		motor[1].setPWM( command[1] );
	}
}

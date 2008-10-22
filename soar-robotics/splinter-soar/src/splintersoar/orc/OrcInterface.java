package splintersoar.orc;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.Timer;
import java.util.TimerTask;
import java.util.logging.Level;
import java.util.logging.Logger;

import laserloc.LaserLoc;
import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;

import orc.Motor;
import orc.Orc;
import orc.OrcStatus;
import splintersoar.LogFactory;
import splintersoar.lcmtypes.coords_t;
import splintersoar.pf.ParticleFilter;

import erp.config.Config;
import erp.geom.Geometry;
import erp.math.MathUtil;

public class OrcInterface implements LCMSubscriber, OrcOutputProducer
{
	private Logger logger;
	
	private class Configuration
	{
		int updateHz = 30;
		int [] ports = { 1, 0 };
		boolean [] invert = { true, false };
		long statusUpdatePeriodNanos = 5 * 1000000000L;
		double maxThrottleAccellerationPeruSec = 2.0 / 1000000;
		
		Configuration( Config config )
		{
			updateHz = config.getInt( "orc.updateHz", configuration.updateHz );
			ports = config.getInts( "orc.ports", configuration.ports );
			invert = config.getBooleans( "orc.invert", configuration.invert );
			
			int statusUpdatePeriodSeconds = config.getInt( "orc.statusUpdatePeriodSeconds", 5 );
			statusUpdatePeriodNanos = statusUpdatePeriodSeconds * 1000000000L;
			
			double maxThrottleAccelleration = config.getDouble( "orc.maxThrottleAccelleration", 2.0 );
			maxThrottleAccellerationPeruSec = maxThrottleAccelleration  / 1000000;
		}
	}
	
	private Configuration configuration;
	
	private Timer timer = new Timer();

	private Orc orc;
	private Motor [] motor = new Motor[2];

	private OrcInputProducer inputProducer;
	private OrcOutput previousOutput = new OrcOutput( 0 );

	private double [] command = { 0, 0 };

	private LCM lcm;
	
	private coords_t laserxy;
	
	private double [] initialxy = null;

	public OrcInterface( Config config, OrcInputProducer inputProducer )
	{
		configuration = new Configuration( config );
		
		logger = LogFactory.createSimpleLogger( Level.ALL );

		this.inputProducer = inputProducer;
		
		lcm = LCM.getSingleton();
		lcm.subscribe( laserloc.LaserLoc.coords_channel, this );
		
		orc = Orc.makeOrc();
		motor[0] = new Motor( orc, configuration.ports[0], configuration.invert[0] );
		motor[1] = new Motor( orc, configuration.ports[1], configuration.invert[1] );
		
		logger.info( "Orc up" );
		timer.schedule( new PFUpdateTask(), 0, 1000 / configuration.updateHz );
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
		logger.info( "Orc down" );
	}

	class PFUpdateTask extends TimerTask
	{		
		int runs = 0;
		long statustimestamp = 0;
		long statusUpdatePeriodNanos;
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
				currentOutput.motorPosition[0] = currentStatus.qeiPosition[configuration.ports[0]] * (configuration.invert[0] ? -1 : 1);
				currentOutput.motorPosition[1] = currentStatus.qeiPosition[configuration.ports[1]] * (configuration.invert[1] ? -1 : 1);
			}

			OrcInput input = inputProducer.getInput();
			if ( input == null )
			{
				logger.finest( "No input, using default" );
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
						initialxy = Arrays.copyOf( laserxy.xy, 2 );
						logger.finest( String.format( "initialxy: %5.2f, %5.2f%n", initialxy[0], initialxy[1] ) );
					}
					
					adjustedlaserxy = Geometry.subtract( laserxy.xy, initialxy );

					laserxy = null;
				}
	
				// adjustedlaserxyt could be null
				currentOutput.xyt = pf.update( deltaxyt, adjustedlaserxy );
				
				currentOutput.xyt[2] = MathUtil.mod2pi( currentOutput.xyt[2] );
				logger.finest( String.format( "%10.6f %10.6f %10.6f%n", currentOutput.xyt[0], currentOutput.xyt[1], Math.toDegrees( currentOutput.xyt[2] ) ) );
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
			else if ( nanotime - statustimestamp > statusUpdatePeriodNanos ) 
			{
				double updatesPerSecond = this.runs / ( ( nanotime - statustimestamp ) / 1000000000.0 );
				logger.fine( String.format( "Orc updates running at %6.2f per sec%n", updatesPerSecond ) );
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
		if ( channel.equals( LaserLoc.coords_channel ) )
		{
			if ( laserxy != null )
			{
				return;
			}

			try 
			{
				laserxy = new coords_t( ins );
			} 
			catch ( IOException ex ) 
			{
				logger.warning( "Error decoding laserxy message: " + ex );
			}
		}
	}

	private long throttleAcceluTime = 0;
	private void commandMotors( long utime, double [] throttle )
	{
		assert throttle[0] <= 1;
		assert throttle[0] >= -1;
		assert throttle[1] <= 1;
		assert throttle[1] >= -1;
		
		if ( throttleAcceluTime == 0 )
		{
			throttleAcceluTime = utime;
			return;
		}
		
		long elapsed = utime - throttleAcceluTime; 
		
		double [] delta = Geometry.subtract( throttle, command );

		if ( delta[0] > 0 )
		{
			double newDelta = Math.min( delta[0], elapsed * configuration.maxThrottleAccellerationPeruSec );
			if ( delta[0] != newDelta )
			{
				delta[0] = newDelta;
			}
		}
		else if ( delta[0] < 0 )
		{
			double newDelta = Math.max( delta[0], -1 * elapsed * configuration.maxThrottleAccellerationPeruSec );
			if ( delta[0] != newDelta )
			{
				delta[0] = newDelta;
			}
		}
		if ( delta[1] > 0 )
		{
			double newDelta = Math.min( delta[1], elapsed * configuration.maxThrottleAccellerationPeruSec );
			if ( delta[1] != newDelta )
			{
				delta[1] = newDelta;
			}
		}
		else if ( delta[1] < 0 )
		{
			double newDelta = Math.max( delta[1], -1 * elapsed * configuration.maxThrottleAccellerationPeruSec );
			if ( delta[1] != newDelta )
			{
				delta[1] = newDelta;
			}
		}

		command[0] += delta[0];
		command[1] += delta[1];
		
		throttleAcceluTime += elapsed;
		
		motor[0].setPWM( command[0] );
		motor[1].setPWM( command[1] );
	}
}

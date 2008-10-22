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
import lcmtypes.pose_t;

import orc.Motor;
import orc.Orc;
import orc.OrcStatus;
import splintersoar.LogFactory;
import splintersoar.lcmtypes.splinterstate_t;
import splintersoar.lcmtypes.xy_t;
import splintersoar.pf.ParticleFilter;

import erp.config.Config;
import erp.geom.Geometry;
import erp.lcmtypes.differential_drive_command_t;
import erp.math.MathUtil;

public class OrcInterface implements LCMSubscriber
{
	private Logger logger;
	
	private class Configuration
	{
		int updateHz = 30;
		int [] ports = { 1, 0 };
		boolean [] invert = { true, false };
		double maxThrottleAccellerationPeruSec = 2.0 / 1000000;
		// geometry, configuration
		double baselineMeters = 0.383;
		double tickMeters = 0.000043225;
		double lengthMeters = 0.64;
		double widthMeters = 0.42;
		String splinterPoseChannel = "SPLINTER_POSE";
		String driveCommandChannel = "DRIVE_COMMANDS";
		
		Configuration( Config config )
		{
			updateHz = config.getInt( "orc.updateHz", updateHz );
			ports = config.getInts( "orc.ports", ports );
			invert = config.getBooleans( "orc.invert", invert );
			
			double maxThrottleAccelleration = config.getDouble( "orc.maxThrottleAccelleration", 2.0 );
			maxThrottleAccellerationPeruSec = maxThrottleAccelleration  / 1000000;

			baselineMeters = config.getDouble( "orc.baselineMeters", baselineMeters );
			tickMeters = config.getDouble( "orc.tickMeters", tickMeters );
			lengthMeters = config.getDouble( "orc.lengthMeters", lengthMeters );
			widthMeters = config.getDouble( "orc.widthMeters", widthMeters );
			splinterPoseChannel = config.getString( "orc.splinterPoseChannel", splinterPoseChannel );
			driveCommandChannel = config.getString( "orc.driveCommandChannel", driveCommandChannel );
		}
	}
	
	private Configuration configuration;
	
	private Timer timer = new Timer();

	private Orc orc;
	private Motor [] motor = new Motor[2];

	private splinterstate_t previousState = new splinterstate_t();

	private double [] command = { 0, 0 };

	private LCM lcm;
	
	private xy_t laserxy;
	
	private double [] initialxy = null;
	
	differential_drive_command_t driveCommand;

	public OrcInterface( Config config )
	{
		configuration = new Configuration( config );

		previousState.pose = new pose_t();
		previousState.pose.orientation = new double [4];
		previousState.pose.pos = new double [3];
		
		logger = LogFactory.simpleLogger( );

		lcm = LCM.getSingleton();
		lcm.subscribe( laserloc.LaserLoc.coords_channel, this );
		lcm.subscribe( configuration.driveCommandChannel, this);
		
		orc = Orc.makeOrc();
		motor[0] = new Motor( orc, configuration.ports[0], configuration.invert[0] );
		motor[1] = new Motor( orc, configuration.ports[1], configuration.invert[1] );
		
		logger.info( "Orc up" );
		timer.schedule( new PFUpdateTask(), 0, 1000 / configuration.updateHz );
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
			splinterstate_t currentState = new splinterstate_t();
			{
				// Get OrcStatus
				OrcStatus currentStatus = orc.getStatus();
	
				// calculate location if moving
				moving = ( currentStatus.qeiVelocity[0] != 0 ) || ( currentStatus.qeiVelocity[1] != 0 );
	
				// assemble output
				currentState.utime = currentStatus.utime;
				currentState.leftodom = currentStatus.qeiPosition[configuration.ports[0]] * (configuration.invert[0] ? -1 : 1);
				currentState.rightodom = currentStatus.qeiPosition[configuration.ports[1]] * (configuration.invert[1] ? -1 : 1);
			}

			// update pose
			if ( moving )
			{				
				double dleft = ( currentState.leftodom - previousState.leftodom ) * configuration.tickMeters;
				double dright = ( currentState.rightodom - previousState.rightodom ) * configuration.tickMeters;
				double phi = ( dright - dleft ) / configuration.baselineMeters;
				double dcenter = ( dleft + dright ) / 2;

				phi = MathUtil.mod2pi( phi );

				double theta = Geometry.quatToRollPitchYaw( previousState.pose.orientation )[2];
				theta = MathUtil.mod2pi( theta );
				double [] deltaxyt = { dcenter * Math.cos( theta ), dcenter * Math.sin( theta ), phi };
	
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
	
				// adjustedlaserxy could be null
				currentState.pose = pf.update( deltaxyt, adjustedlaserxy );
				
				logger.finest( String.format( "%10.6f %10.6f %10.6f", 
						currentState.pose.pos[0], currentState.pose.pos[1], 
						Math.toDegrees( Geometry.quatToRollPitchYaw( previousState.pose.orientation )[2] ) ) );
			}
			else
			{
				currentState.pose = previousState.pose.copy();
			}
			currentState.pose.utime = currentState.utime;
			
			// publish pose
			lcm.publish( configuration.splinterPoseChannel, currentState );

			// command motors
			commandMotors( currentState.utime );
			
			previousState = currentState;
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
				laserxy = new xy_t( ins );
			} 
			catch ( IOException ex ) 
			{
				logger.warning( "Error decoding laserxy message: " + ex );
			}
		}
		else if ( channel.equals( configuration.driveCommandChannel ) )
		{
			try 
			{
				synchronized( this )
				{
					driveCommand = new differential_drive_command_t( ins );
				}
			} 
			catch ( IOException ex ) 
			{
				logger.warning( "Error decoding differential_drive_command_t message: " + ex );
			}
		}
	}

	private long throttleAcceluTime = 0;
	private void commandMotors( long utime )
	{
		differential_drive_command_t newDriveCommand = null;
		
		synchronized( this )
		{
			if ( driveCommand != null )
			{
				newDriveCommand = driveCommand.copy();
			}
		}
		if ( newDriveCommand == null )
		{
			return;
		}
		//logger.finest( String.format("Got input %f %f", newDriveCommand.left, newDriveCommand.right)  );
		
		if ( throttleAcceluTime == 0 )
		{
			throttleAcceluTime = utime;
			return;
		}
		
		long elapsed = ( utime - throttleAcceluTime );
		double maxAccel = elapsed *  configuration.maxThrottleAccellerationPeruSec;
		
		if ( newDriveCommand.left_enabled )
		{
			newDriveCommand.left = Math.min( newDriveCommand.left, 1.0 );
			newDriveCommand.left = Math.max( newDriveCommand.left, -1.0 );
			
			double delta = newDriveCommand.left - command[0];
			if ( delta > 0 )
			{
				delta = Math.min( delta, maxAccel );
			} 
			else if ( delta < 0 )
			{
				delta = Math.max( delta, -1 * maxAccel );
			}

			command[0] += delta;
		}
		
		if ( newDriveCommand.right_enabled )
		{
			newDriveCommand.right = Math.min( newDriveCommand.right, 1.0 );
			newDriveCommand.right = Math.max( newDriveCommand.right, -1.0 );

			double delta = newDriveCommand.right - command[1];
			if ( delta > 0 )
			{
				delta = Math.min( delta, maxAccel );
			} 
			else if ( delta < 0 )
			{
				delta = Math.max( delta, -1 * maxAccel );
			}
			command[1] += delta;
		}
		
		throttleAcceluTime += elapsed;
		
		motor[0].setPWM( command[0] );
		motor[1].setPWM( command[1] );
	}
}

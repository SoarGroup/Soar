package splintersoar;

import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;

import laserloc.LaserLoc;
import lcm.lcm.LCM;

import erp.config.Config;
import erp.config.ConfigFile;

import orc.util.GamePad;

import splintersoar.orc.OrcInterface;
import splintersoar.orc.OrcInput;
import splintersoar.orc.OrcInputProducer;
import splintersoar.ranger.RangerManager;
import splintersoar.soar.SoarInterface;

public class SplinterSoar implements OrcInputProducer
{
	SoarInterface soar;
	OrcInterface orc;
	RangerManager ranger;
	LaserLoc laserloc;
	LCM lcm;
	GamePad gamePad;
	
	boolean running = true;

	private Logger logger;
	
	public SplinterSoar( String args [] )
	{
		if ( args.length != 1 ) 
		{
		    System.out.println("Usage: splintersoar <configfile>");
		    return;
		}

		Config config;

		try 
		{
		    config = ( new ConfigFile( args[0] ) ).getConfig();
		} 
		catch ( IOException ex ) 
		{
		    logger.severe( "Couldn't open config file: " + args[0] );
		    return;
		}
		
		logger = LogFactory.simpleLogger( Level.ALL );
		
		logger.info( "Starting orc interface" );
		orc = new OrcInterface( config, this );

		logger.info( "Starting ranger" );
		ranger = new RangerManager();

		logger.info( "Starting Soar interface" );
		soar = new SoarInterface( orc, ranger, config );
		
		logger.info( "Creating game pad for override" );
		gamePad = new GamePad();
		
		Runtime.getRuntime().addShutdownHook( new ShutdownHook() );
		
		logger.info( "Ready" );
		while ( running )
		{
			try 
			{
				updateOverride();
				updateStartStop();
				Thread.sleep( 50 );
			} 
			catch ( InterruptedException ignored ) 
			{}
		}
	}
	
	boolean overrideEnabled = false;
	boolean overrideButton = false;
	double [] throttle = { 0, 0 };
	boolean tankMode = false;
	
	private void updateOverride()
	{
		boolean currentOverrideButton = gamePad.getButton( 0 );
		// change on trailing edge
		if ( overrideButton && !currentOverrideButton )
		{
			synchronized( throttle )
			{
				overrideEnabled = !overrideEnabled;
			
				if ( overrideEnabled )
				{
					throttle = new double [] { 0, 0 };
				}
			}
			
			logger.info( "Override " + ( overrideEnabled ? "enabled" : "disabled" ) );
		}
		overrideButton = currentOverrideButton;
		
		if ( overrideEnabled )
		{
			double [] newThrottle = new double[2];
			
			if ( tankMode ) 
			{
				newThrottle[0] = gamePad.getAxis( 1 ) * -1;
				newThrottle[1] = gamePad.getAxis( 3 ) * -1;
			}
			else
			{
				double fwd = -1 * gamePad.getAxis( 3 ); // +1 = forward, -1 = back
				double lr  = -1 * gamePad.getAxis( 2 );   // +1 = left, -1 = right

				newThrottle[0] = fwd - lr;
				newThrottle[1] = fwd + lr;

				double max = Math.max( Math.abs( newThrottle[0] ), Math.abs( newThrottle[1] ) );
				if ( max > 1 ) 
				{
					newThrottle[0] /= max;
					newThrottle[1] /= max;
				}
			}
			
			if ( ( newThrottle[0] != throttle[0] ) || ( newThrottle[1] != throttle[1] ) )
			{
				synchronized( throttle )
				{
					System.arraycopy( newThrottle, 0, throttle, 0, newThrottle.length );
				}
			}
		}
	}

	boolean startStopButton = false;
	boolean startStop = false;
	
	private void updateStartStop()
	{
		boolean currentStartStopButton = gamePad.getButton( 1 );
		// change on trailing edge
		if ( startStopButton && !currentStartStopButton )
		{
			startStop = !startStop;
			
			if ( startStop )
			{
				logger.info( "Starting Soar" );
				soar.start();
			}
			else
			{
				logger.info( "Stop Soar requested" ); 
				soar.stop();
			}
		}
		startStopButton = currentStartStopButton;
	}
	
	public class ShutdownHook extends Thread
	{
		@Override
		public void run()
		{
			running = false;
			
			soar.shutdown();
			orc.shutdown();
			
			System.out.flush();
			System.err.println( "Terminated" );
			System.err.flush();
		}
	}
	
	public static void main( String args[] )
	{
		new SplinterSoar( args );
	}

	@Override
	public OrcInput getInput() {
		if ( overrideEnabled )
		{
			OrcInput input = new OrcInput();
			input.targetYawEnabled = false;
			synchronized ( throttle )
			{
				System.arraycopy( throttle, 0, input.throttle, 0, throttle.length );
			}
			return input;
		} 
		else
		{
			if ( soar == null )
			{
				return null;
			}
			return soar.getSplinterInput();
		}
	}

}

package splintersoar;

import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Date;
import java.util.logging.*;

import erp.config.Config;
import erp.config.ConfigFile;

import orc.util.GamePad;

import splintersoar.orc.OrcInterface;
import splintersoar.orc.OrcInput;
import splintersoar.orc.OrcInputProducer;
import splintersoar.ranger.RangerManager;
import splintersoar.soar.*;
import laserloc.*;
import lcm.lcm.*;

public class SplinterSoar implements OrcInputProducer
{
	SoarInterface soar;
	OrcInterface orc;
	RangerManager ranger;
	LaserLoc laserloc;
	LCM lcm;
	GamePad gamePad;
	
	boolean running = true;

	public static final Logger logger = Logger.getLogger(Logger.GLOBAL_LOGGER_NAME);
	
	public class TextFormatter extends Formatter {
		SimpleDateFormat format = new SimpleDateFormat("HH:mm:ss");
		
		public TextFormatter() {
			super();
		}

		public String format(LogRecord record) {
			Date d = new Date(record.getMillis());
			StringBuilder output = new StringBuilder();
			output.append( format.format( d ) );
			output.append( " " );
			output.append( record.getLevel().getName() );
			output.append( " " );
			output.append( record.getMessage() );
			output.append( java.lang.System.getProperty("line.separator") );
			return output.toString();
		}

	}

	public SplinterSoar( String args [] )
	{
		ConsoleHandler handler = new ConsoleHandler();
		handler.setFormatter(new TextFormatter());
		handler.setLevel( Level.ALL );
		logger.setLevel( Level.ALL );
		logger.addHandler(handler);
		logger.setUseParentHandlers( false );

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
		    SplinterSoar.logger.severe( "Couldn't open config file: " + args[0] );
		    return;
		}
		
		String agent = config.requireString( "soar.agent" );
		
		logger.info( "Starting orc interface" );
		orc = new OrcInterface( this );

		logger.info( "Starting ranger" );
		ranger = new RangerManager();

		logger.info( "Starting Soar interface with agent: " + agent );
		soar = new SoarInterface( orc, ranger, agent );
		
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
				SplinterSoar.logger.info( "Stop Soar requested" ); 
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
			
			System.err.println( "Terminated" );
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

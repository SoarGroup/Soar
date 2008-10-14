package splintersoar;

import java.util.logging.*;

import orc.util.GamePad;

import splintersoar.soar.*;
import laserloc.*;
import lcm.lcm.*;

public class SplinterSoar
{
	SoarInterface soar;
	OrcInterface orc;
	LaserLoc laserloc;
	LCM lcm;
	GamePad gamePad;
	
	boolean running = true;

	public static final Logger logger = Logger.getLogger("splintersoar");
	
	public SplinterSoar( String args [] )
	{
		logger.info( "Starting orc interface" );
		orc = new OrcInterface();

		logger.info( "Starting laser localizer" );
		laserloc = new LaserLoc( false );
		
		logger.info( "Subscribing orc to POSE channel" );
		lcm = LCM.getSingleton();
		lcm.subscribe( LaserLoc.pose_channel, orc );

		logger.info( "Subscribing orc to LASER_FRONT channel" );
		lcm.subscribe( "LASER_FRONT", orc );

		if ( args.length > 0 )
		{
			logger.info( "Starting Soar interface with agent: " + args[ 0 ] );
			soar = new SoarInterface( orc.getState(), args[ 0 ] );
		}
		else
		{
			logger.info( "Starting Soar interface with default agent" );
			soar = new SoarInterface( orc.getState(), null );
		}
		
		logger.info( "Creating game pad for override" );
		gamePad = new GamePad();
		
		Runtime.getRuntime().addShutdownHook( new ShutdownHook() );
		
		logger.info( "Ready" );
		while ( running )
		{
			try 
			{
				updateOverride();
				if ( overrideEnabled )
				{
					Thread.sleep( 50 );
				} 
				else
				{
					Thread.sleep( 500 );
				}
			} 
			catch ( InterruptedException ignored ) 
			{}
		}
	}
	
	boolean overrideEnabled = false;
	boolean overrideButton = false;
	double left = 0;
	double right = 0;
	private void updateOverride()
	{
		boolean currentOverrideButton = gamePad.getButton( 0 );
		// change on trailing edge
		if ( overrideButton && !currentOverrideButton )
		{
			overrideEnabled = !overrideEnabled;
			soar.setOverride( overrideEnabled );
			
			if ( overrideEnabled )
			{
				logger.info( "Override enabled" );
				left = 0;
				right = 0;
				commitOverrideCommand();
			}
			else
			{
				logger.info( "Override disabled" );
			}
		}
		overrideButton = currentOverrideButton;
		
		if ( overrideEnabled )
		{
			double newLeft = gamePad.getAxis( 1 ) * -1;
			double newRight = gamePad.getAxis( 3 ) * -1;
			
			if ( ( newLeft != left ) || ( newRight != right ) )
			{
				left = newLeft;
				right = newRight;
				commitOverrideCommand();
			}
		}
	}
	
	private void commitOverrideCommand()
	{
		synchronized ( orc.getState() )
		{
			orc.getState().left = left;
			orc.getState().right = right;
			orc.getState().targetYawEnabled = false;
		}
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

}

package splintersoar;

import java.util.logging.*;

import splintersoar.soar.*;
import laserloc.*;
import lcm.lcm.*;

public class SplinterSoar
{
	SoarInterface soar;
	OrcInterface orc;
	LaserLoc laserloc;
	LCM lcm;
	
	boolean running = true;

	public static final Logger logger = Logger.getLogger("splintersoar");
	
	public SplinterSoar()
	{
		logger.info( "Starting orc interface" );
		orc = new OrcInterface();

		logger.info( "Starting laser localizer" );
		laserloc = new LaserLoc( false );
		
		logger.info( "Subscribing orc to POSE channel" );
		lcm = LCM.getSingleton();
		lcm.subscribe( LaserLoc.pose_channel, orc );

		//logger.info( "Subscribing orc to LASER_FRONT channel" );
		//lcm.subscribe( "LASER_FRONT", orc );

		logger.info( "Starting Soar interface" );
		soar = new SoarInterface( orc.getState() );

		logger.info( "Creating and using game pad for override" );
		soar.setOverride( new GamePadManager() );
		
		Runtime.getRuntime().addShutdownHook( new ShutdownHook() );
		
		logger.info( "Ready" );
		while ( running )
		{
			try 
			{
				Thread.sleep( 500 );
			} 
			catch ( InterruptedException ignored ) 
			{}
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
			
			System.out.println( "Terminated" );
		}
	}
	
	public static void main( String args[] )
	{
		new SplinterSoar();
	}

}

package splintersoar;

import java.util.logging.*;

import splintersoar.soar.*;
import splintersoar.orc.*;
import splintersoar.gamepad.*;

public class SplinterSoar 
{
	SoarInterface soar;
	SplinterInterface splinter;

	public static final Logger logger = Logger.getLogger("splintersoar");
	
	public SplinterSoar()
	{
		logger.info( "Starting splinter interface" );
		splinter = new OrcInterface();
		//splinter = new LCMInterface();

		logger.info( "Starting Soar interface" );
		soar = new SoarInterface( splinter.getState() );

		logger.info( "Creating and using game pad for override" );
		soar.setOverride( new GamePadManager() );
		
		logger.info( "Shutting down Soar interface" );
		soar.shutdown();

		logger.info( "Shutting down splinter interface" );
		splinter.shutdown();
		
		logger.info( "Exiting" );
	}
	
	public static void main( String args[] )
	{
		new SplinterSoar();
	}

}

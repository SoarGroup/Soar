package laserloc;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.logging.Level;
import java.util.logging.Logger;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;

import splintersoar.LogFactory;
import splintersoar.lcmtypes.xy_t;

import erp.config.Config;
import erp.config.ConfigFile;

public class LaserLoc implements LCMSubscriber
{
	static
	{
		initializeLaserData();
	}

	public static final String laser_channel = "LASER_LOC";
	public static final String coords_channel = "COORDS";

	private class Configuration
	{
		boolean testing = false;
		double laser_x = 0; // if we assume the laser is at the origin facing up the y-axis, the next 3 constants are all 0
		double laser_y = 0;
		double laser_yaw_adjust = 0; // amount to adjust for laser's yaw = 90 - laser's yaw = 0 if laser is facing positive y directly
		double laser_dist_adjustment = 0; // radius of tube?
		long update_period = 5; // nanoseconds between status updates
		long activity_timeout = 5;
		double [] maxRanges;

		Configuration( Config config )
		{
			testing = config.getBoolean( "testing", testing );
			laser_x = config.getDouble( "laser_x", laser_x );
			laser_y = config.getDouble( "laser_y", laser_y );
			laser_yaw_adjust = config.getDouble( "laser_yaw_adjust", laser_yaw_adjust );
			laser_dist_adjustment = config.getDouble( "laser_dist_adjustment", laser_dist_adjustment );
			update_period = config.getInt( "update_period", (int)update_period );
			update_period *= 1000000000;
			activity_timeout = config.getInt( "activity_timeout", (int)activity_timeout );
			activity_timeout *= 1000000000;
			
			try {
				Config mapConfig = new ConfigFile( "map.txt" ).getConfig();
				maxRanges = mapConfig.getDoubles( "map" );
			} catch (IOException e) {
			}
		}
	}
	
	private Configuration configuration;
	
	// Assumptions:
		// robot_z is constant
		// robot starts at a known angle
		// this level only reports laser data (next layer up decided whether to use odometry or not)
		// distances in meters
	
	// regular state
	laser_t laser_data;
	
	int droppedLocPackets = 0;
	long lastStatusUpdate = System.nanoTime();

	LCM lcm;

	boolean inactive = true;
	long nanolastactivity = System.nanoTime();
	long currentTimeout;

	private Logger logger;
	RoomMapper mapper;
	
	public LaserLoc( Config config )
	{
		configuration = new Configuration( config );
		
		logger = LogFactory.simpleLogger( );
		if ( !configuration.testing )
		{
			lcm = LCM.getSingleton();
			lcm.subscribe( laser_channel, this );
			lcm.subscribe( "MOTOR_COMMANDS", this );

		}

		if ( configuration.maxRanges == null )
		{
			logger.warning( "No map file found, using infinite maximums" );
			Arrays.fill( configuration.maxRanges, Double.MAX_VALUE );
		}
		
		currentTimeout = configuration.activity_timeout;
		
		printHeaderLine();
	}
	
	public void printHeaderLine()
	{
		logger.fine( String.format( "%10s %10s", "x", "y" ) );
	}

	private void updatePose()
	{
		long nanotime = System.nanoTime();

		if ( nanotime - nanolastactivity > currentTimeout )
		{
			inactive = true;
			logger.warning( String.format("no activity in last " + ( currentTimeout / 1000000000 ) + " seconds" ) );
			nanolastactivity = nanotime;
			currentTimeout += configuration.activity_timeout;
		}
		
		// occasionally print out status update
		long nanoelapsed = nanotime - lastStatusUpdate;
		if ( nanoelapsed > configuration.update_period )
		{
			if ( droppedLocPackets > 0 )
			{
				double dropRate = (double) droppedLocPackets / (nanoelapsed / 1000000000);
				logger.warning( String.format( "LaserLoc: dropping %5.1f %s packets/sec", dropRate, laser_channel ) );
				printHeaderLine();
			}
			
			droppedLocPackets = 0;
			lastStatusUpdate = nanotime;
		}
		
		if ( laser_data == null )
		{
			try 
			{
				Thread.sleep( 50 );
			} 
			catch ( InterruptedException ignored ) 
			{
			}
			return;
		}
		
		nanolastactivity = nanotime;
		currentTimeout = configuration.activity_timeout;
		
		if ( inactive )
		{
			logger.info( "receiving data" );
			inactive = false;
		}
		
		xy_t estimated_coords = getRobotXY( laser_data );

		logger.fine( String.format( "%10.3f %10.3f", estimated_coords.xy[ 0 ], estimated_coords.xy[ 1 ] ) );

		estimated_coords.utime = laser_data.utime;

		if ( lcm != null )
		{
			lcm.publish( coords_channel, estimated_coords );
		}

		laser_data = null;
	}

	private xy_t getRobotXY( laser_t laser_data )
	{
		assert laser_data != null;
			
		double smallest_range = Double.MAX_VALUE;
		int smallest_range_index = -1;
		for ( int index = 0; index < laser_data.nranges; ++index )
		{
			if ( laser_data.ranges[ index ] < configuration.maxRanges[index] )
			{
				if ( laser_data.ranges[ index ] < smallest_range )
				{
					smallest_range = laser_data.ranges[ index ];
					smallest_range_index = index;
				}
			}
		}
		assert smallest_range_index != -1;
		
		double laser_angle = configuration.laser_yaw_adjust + laser_data.rad0 + laser_data.radstep * smallest_range_index;
		
		double laser_dist = smallest_range + configuration.laser_dist_adjustment;
		
		xy_t new_coords = new xy_t();
		new_coords.xy[ 0 ] = configuration.laser_x + laser_dist * Math.cos( laser_angle );
		new_coords.xy[ 1 ] = configuration.laser_y + laser_dist * Math.sin( laser_angle );
		
		return new_coords;
	}

	@Override
	public void messageReceived( LCM lcm, String channel, DataInputStream ins ) 
	{
		if ( channel.equals( laser_channel ) )
		{
			if ( laser_data != null )
			{
				droppedLocPackets += 1;
				return;
			}
			
			try 
			{
				laser_data = new laser_t( ins );
			} 
			catch ( IOException ex ) 
			{
				System.err.println( "Error decoding laser message: " + ex );
			}
		}
		
	}

	static laser_t[][] test_data;
	private static void initializeLaserData()
	{
		test_data = new laser_t[2][];
		for ( int test_index = 0; test_index < test_data.length; ++test_index )
		{
			switch ( test_index )
			{
			case 0:
				initializeTestData0();
				break;
				
			case 1:
				initializeTestData1();
				break;
			default:
				assert false;
			}
		}
	}
	
	private static void initializeTestData0() {
		final int THIS_TEST = 0;
		
		float rad0 = (float)Math.PI / -2;
		float radstep = (float)Math.PI / 180; // one degree
		float max = 10;
		long time = -1;
		int nranges = 180;
		
		float start_range = 1;

		test_data[ THIS_TEST ] = new laser_t[ 100 ]; 
		for ( int index = 0; index < test_data[ THIS_TEST ].length; ++index )
		{
			laser_t data = new laser_t();
			data.rad0 = rad0;
			data.radstep = radstep;
			data.utime = ++time;
			data.nranges = nranges;
			data.ranges = new float[ nranges ];
			Arrays.fill( data.ranges, max );

			data.ranges[ 20 ] = start_range + 0.01f * index;
			
			test_data[ THIS_TEST ][ index ] = data;
		}
	}

	private static void initializeTestData1() {
		final int THIS_TEST = 1;

		float rad0 = (float)Math.PI / -2;
		float radstep = (float)Math.PI / 180; // one degree
		float max = 10;
		long time = -1;
		int nranges = 180;

		int start_range_index = 60;
		float data_range = 1;
		
		test_data[ THIS_TEST ] = new laser_t[ 60 ]; 
		for ( int index = 0; index < test_data[ THIS_TEST ].length; ++index )
		{
			laser_t data = new laser_t();
			data.rad0 = rad0;
			data.radstep = radstep;
			data.utime = ++time;
			data.nranges = nranges;
			data.ranges = new float[ nranges ];
			Arrays.fill( data.ranges, max );

			data.ranges[ start_range_index + index ] = data_range;
			
			test_data[ THIS_TEST ][ index ] = data;
		}
	}

	public static void main(String[] args) 
	{
		if ( args.length != 1 ) 
		{
		    System.out.println("Usage: laserloc <configfile>");
		    return;
		}

		Config config;

		try 
		{
		    config = ( new ConfigFile( args[0] ) ).getConfig();
		} 
		catch ( IOException ex ) 
		{
			System.err.println( "Couldn't open config file: " + args[0] );
		    return;
		}
		
		// set up constants
		LaserLoc lloc = null;
		if ( config.getBoolean( "testing", false ) )
		{
			for ( int current_test_index = 0; current_test_index < test_data.length; ++current_test_index )
			{
				lloc = new LaserLoc( config );
				lloc.logger.info( "Starting test " + ( current_test_index + 1 ) );

				for ( int current_test_data_index = 0; current_test_data_index < test_data[ current_test_index ].length; ++current_test_data_index )
				{
					lloc.laser_data = test_data[ current_test_index ][ current_test_data_index ];
					lloc.updatePose();
				}
				
			}
			if (lloc == null)
			{
				System.err.println( "No tests to run." );
			}
			else
			{
				lloc.logger.info( "All tests done." );
			}
			System.exit( 0 );
		}
		
		// Not testing
		lloc = new LaserLoc( config );

		while ( true )
		{
			lloc.updatePose();
		}
	}

}























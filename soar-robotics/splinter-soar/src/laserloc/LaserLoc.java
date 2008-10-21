package laserloc;

import java.io.DataInputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Calendar;

import splintersoar.SplinterSoar;
import splintersoar.lcmtypes.coords_t;

import lcm.lcm.*;
import lcmtypes.*;
import erp.config.Config;
import erp.config.ConfigFile;
import erp.geom.*;

// TODO: don't assert
public class LaserLoc implements LCMSubscriber
{
	static
	{
		initializeLaserData();
	}

	public static final String laser_channel = "LASER_LOC";
	public static final String pose_channel = "POSE";

	// Assumptions:
		// robot_z is constant
		// robot starts at a known angle
		// this level only reports laser data (next layer up decided whether to use odometry or not)
		// distances in meters
	
	// Configuration:
	private static double robot_starting_yaw = 0;
	private static double laser_x = 0; // if we assume the laser is at the origin facing up the y-axis, the next 3 constants are all 0
	private static double laser_y = 0;
	private static double laser_yaw_adjust = 0; // amount to adjust for laser's yaw = 90 - laser's yaw = 0 if laser is facing positive y directly
	private static double laser_dist_adjustment = 0; // radius of tube?
	private static double translation_threshold = 0; // minimum translation distance to update x,y location, 0.05 total guess
	private static int update_period = 5 * 1000000000; // nanoseconds between status updates
	private static boolean verbose = false;
	// end constants
	
	// regular state
	laser_t laser_data;
	coords_t estimated_coords;
	
	int droppedLocPackets = 0;
	long lastStatusUpdate = System.nanoTime();

	LCM lcm;
	
	public LaserLoc()
	{
		printHeaderLine();
	}
	
	public void setLCM( LCM lcm )
	{
		this.lcm = lcm;
	}
	
	public void printHeaderLine()
	{
		if ( verbose )
		{
			System.out.format( "%10s %10s%n", "x", "y" );
		}
	}

	boolean inactive = true;
    SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss");
	long nanolastactivity = System.nanoTime();
	final long ACTIVITY_TIMEOUT = 5 * 1000000000;
	long currentTimeout = ACTIVITY_TIMEOUT;
	private void updatePose()
	{
		long nanotime = System.nanoTime();

		if ( nanotime - nanolastactivity > currentTimeout )
		{
			inactive = true;
			System.out.println( sdf.format(Calendar.getInstance().getTime()) + ": no activity in last " + ( currentTimeout / 1000000000 ) + " seconds" );
			nanolastactivity = nanotime;
			currentTimeout += ACTIVITY_TIMEOUT;
		}
		
		// occasionally print out status update
		long nanoelapsed = nanotime - lastStatusUpdate;
		if ( nanoelapsed > update_period )
		{
			if ( droppedLocPackets > 0 )
			{
				float dropRate = droppedLocPackets / (nanoelapsed / 1000000000);
				System.err.format( "LaserLoc: dropping %5.1f %s packets/sec%n", dropRate, laser_channel );
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
		currentTimeout = ACTIVITY_TIMEOUT;
		
		if ( inactive )
		{
			System.out.println( sdf.format(Calendar.getInstance().getTime()) + ": receiving data" );
			inactive = false;
		}
		
		if ( estimated_coords == null )
		{
			// initialize
			estimated_coords = getRobotXY( laser_data );
			printOldPose();

			estimated_coords.utime = laser_data.utime;
			if ( lcm != null )
			{
				lcm.publish( pose_channel, estimated_coords );
			}

			laser_data = null;
			return;
		}
		
		coords_t new_estimated_coords = getRobotXY( laser_data );
		if ( new_estimated_coords == null )
		{
			laser_data = null;
			return;
		}
		
		double translation_dist = Geometry.distance( estimated_coords.xy, new_estimated_coords.xy );
		
		// only update location if moved enough. Don't want Soar to thrash on constantly changing x,y due to noise
		// this also means this loop can run as fast as it can and we'll still get reasonable updates
		// (i.e., we don't have to worry about robot not moving enough between updates)
		boolean movedEnough = translation_dist >= translation_threshold;
		
		if( movedEnough )
		{
			System.arraycopy( new_estimated_coords.xy, 0, estimated_coords.xy, 0, new_estimated_coords.xy.length );
			
			estimated_coords.utime = laser_data.utime;
			if ( lcm != null )
			{
				lcm.publish( pose_channel, estimated_coords );
			}

			printOldPose();
		}
		laser_data = null;
	}

	private void printOldPose() 
	{
		if ( verbose )
		{
			System.out.format( "%10.3f %10.3f%n", 
					estimated_coords.xy[ 0 ], 
					estimated_coords.xy[ 1 ] );
		}
	}

	private coords_t getRobotXY( laser_t laser_data )
	{
		assert laser_data != null;
			
		double smallest_range = Double.MAX_VALUE;
		int smallest_range_index = -1;
		for ( int index = 0; index < laser_data.nranges; ++index )
		{
			if ( laser_data.ranges[ index ] < smallest_range )
			{
				smallest_range = laser_data.ranges[ index ];
				smallest_range_index = index;
			}
		}
		assert smallest_range_index != -1;
		
		double laser_angle = laser_yaw_adjust + laser_data.rad0 + laser_data.radstep * smallest_range_index;
		
		double laser_dist = smallest_range + laser_dist_adjustment;
		
		coords_t new_coords = new coords_t();
		new_coords.xy[ 0 ] = laser_x + laser_dist * Math.cos( laser_angle );
		new_coords.xy[ 1 ] = laser_y + laser_dist * Math.sin( laser_angle );
		
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
		    SplinterSoar.logger.severe( "Couldn't open config file: " + args[0] );
		    return;
		}
		
		// set up constants
		LaserLoc.robot_starting_yaw = config.getDouble( "robot_starting_yaw", 0 );
		LaserLoc.laser_x = config.getDouble( "laser_x", 0 );
		LaserLoc.laser_y = config.getDouble( "laser_y", 0 );
		LaserLoc.laser_yaw_adjust = config.getDouble( "laser_yaw_adjust", 0 );
		LaserLoc.laser_dist_adjustment = config.getDouble( "laser_dist_adjustment", 0 );
		LaserLoc.translation_threshold = config.getDouble( "translation_threshold", 0 );
		LaserLoc.update_period = config.getInt( "update_period", 5 );
		LaserLoc.verbose = config.getBoolean( "verbose", false );
		
		LaserLoc lloc;
		if ( config.getBoolean( "testing", false ) )
		{
			for ( int current_test_index = 0; current_test_index < test_data.length; ++current_test_index )
			{
				SplinterSoar.logger.info( "Starting test " + ( current_test_index + 1 ) );
				lloc = new LaserLoc();

				for ( int current_test_data_index = 0; current_test_data_index < test_data[ current_test_index ].length; ++current_test_data_index )
				{
					lloc.laser_data = test_data[ current_test_index ][ current_test_data_index ];
					lloc.updatePose();
				}
				
			}
			SplinterSoar.logger.info( "All tests done." );
			System.exit( 0 );
		}
		
		// Not testing
		lloc = new LaserLoc();

		LCM lcm = LCM.getSingleton();
		lcm.subscribe( laser_channel, lloc );
		lloc.setLCM( lcm );

		while ( true )
		{
			lloc.updatePose();
		}
	}

}























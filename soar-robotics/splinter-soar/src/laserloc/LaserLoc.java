package laserloc;

import java.awt.Point;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.Arrays;

import lcm.lcm.*;
import lcmtypes.*;
import erp.geom.*;

// TODO: don't assert
public class LaserLoc implements LCMSubscriber
{
	static
	{
		initializeLaserData();
	}

	// Assumptions:
		// robot_z is constant
		// robot starts at a known angle
		// this level only reports laser data (next layer up decided whether to use odometry or not)
		// distances in meters
	
	// Constants:
	public static double robot_starting_yaw = 0;
	public static double laser_x = 0; // if we assume the laser is at the origin facing up the y-axis, the next 3 constants are all 0
	public static double laser_y = 0;
	public static double laser_yaw_adjust = 0; // amount to adjust for laser's yaw = 90 - laser's yaw = 0 if laser is facing positive y directly
	public static double laser_dist_adjustment = 0; // radius of tube?
	public static double translation_threshold = 0.1; // minimum translation distance to update x,y location, 0.05 total guess
	public static double duration_threshold = 2; // maximum time between updates in seconds
	public static String laser_channel = "LASER_LOC";
	public static String pose_channel = "POSE";
	public static int update_period = 5; // seconds between status updates

	// Even though this comes in each message, it may help the compiler optimize 
	// if we make it a separate constant (can assert on it not matching the message)
	public static double laser_delta_angle = 0; // angle between laser indexes. 
	// end constants
	
	// regular state
	laser_t laser_data;
	pose_t estimated_pose;
	
	int droppedLocPackets = 0;
	long lastStatusUpdate = System.nanoTime();

	static boolean testing = false;
	static boolean verbose = false;
	
	LCM lcm;
	
	public LaserLoc( boolean verbose )
	{
		// ?? initialize?
		
		if ( !testing )
		{
			lcm = LCM.getSingleton();
			lcm.subscribe( laser_channel, this );
		}

		printHeaderLine();
	}
	
	public void printHeaderLine()
	{
		System.out.format( "%10s %10s %10s %10s %10s%n", "x", "y", "vx", "vy", "vmag", "yaw" );
	}
	
	private void updatePose()
	{
		// occasionally print out status update
		long utime = System.nanoTime() / 1000;
		long elapsed = utime - lastStatusUpdate;
		if ( elapsed > update_period )
		{
			if ( droppedLocPackets > 0 )
			{
				float dropRate = droppedLocPackets / elapsed;
				System.err.format( "LaserLoc: dropping %5.1f %s packets/sec%n", dropRate, laser_channel );
				printHeaderLine();
			}
			
			droppedLocPackets = 0;
			lastStatusUpdate = System.nanoTime();
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
		
		if ( estimated_pose == null )
		{
			// initialize
			estimated_pose = getRobotXY( laser_data );
			estimated_pose.orientation = Geometry.rollPitchYawToQuat( new double[] { 0, 0, robot_starting_yaw } );
			if ( verbose )
			{
				printOldPose();
			}

			estimated_pose.utime = utime;
			if ( !testing )
			{
				lcm.publish( pose_channel, estimated_pose );
			}

			laser_data = null;
			return;
		}
		
		pose_t new_estimated_pose = getRobotXY( laser_data );
		if ( new_estimated_pose == null )
		{
			laser_data = null;
			return;
		}
		
		double translation_dist = Point.distance( estimated_pose.pos[ 0 ], estimated_pose.pos[ 1 ], new_estimated_pose.pos[ 0 ], new_estimated_pose.pos[ 1 ] );
		
		// only update location if moved enough. Don't want Soar to thrash on constantly changing x,y due to noise
		// this also means this loop can run as fast as it can and we'll still get reasonable updates
		// (i.e., we don't have to worry about robot not moving enough between updates)
		boolean movedEnough = translation_dist >= translation_threshold;
		boolean enoughTimePassed = ( new_estimated_pose.utime - estimated_pose.utime ) > ( duration_threshold * 1000000 );
		
		if( movedEnough || enoughTimePassed )
		{
			estimated_pose.vel[0] = ( new_estimated_pose.pos[ 0 ] - estimated_pose.pos[ 0 ] ) / elapsed;
			estimated_pose.vel[1] = ( new_estimated_pose.pos[ 1 ] - estimated_pose.pos[ 1 ] ) / elapsed;
			estimated_pose.vel[2] = ( new_estimated_pose.pos[ 2 ] - estimated_pose.pos[ 2 ] ) / elapsed;
			
			double estimated_pose_yaw = Math.atan2( new_estimated_pose.pos[ 1 ] - estimated_pose.pos[ 1 ], new_estimated_pose.pos[ 0 ] - estimated_pose.pos[ 0 ] );
			estimated_pose.orientation = Geometry.rollPitchYawToQuat( new double[] { 0, 0, estimated_pose_yaw } );
			
			// TODO: figure out best way to clone
			estimated_pose.pos[ 0 ] =  new_estimated_pose.pos[ 0 ];
			estimated_pose.pos[ 1 ] =  new_estimated_pose.pos[ 1 ];
			estimated_pose.pos[ 2 ] =  new_estimated_pose.pos[ 2 ];
			
			estimated_pose.utime = utime;
			if ( !testing )
			{
				lcm.publish( pose_channel, estimated_pose );
			}

			printOldPose();
		}
		else
		{
			if ( verbose )
			{
				System.out.println( "Skipping update (beneath threshold)" );
			}
		}
		laser_data = null;
	}

	private void printOldPose() 
	{
		System.out.format( "%10.3f %10.3f %10.3g %10.3g %10.3g %10.3f%n", 
				estimated_pose.pos[ 0 ], 
				estimated_pose.pos[ 1 ], 
				estimated_pose.vel[ 0 ], 
				estimated_pose.vel[ 1 ], 
				Geometry.magnitude( estimated_pose.vel ),
				Math.toDegrees( Geometry.quatToRollPitchYaw( estimated_pose.orientation )[ 2 ] ) );
	}

	private pose_t getRobotXY( laser_t laser_data )
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
		
		pose_t new_pose = new pose_t();
		new_pose.pos[ 0 ] = laser_x + laser_dist * Math.cos( laser_angle );
		new_pose.pos[ 1 ] = laser_y + laser_dist * Math.sin( laser_angle );
		new_pose.pos[ 2 ] = 0;
		
		return new_pose;
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
		// TODO: configuration of constants from command line params
		// TODO: testing switch from command line params

		LaserLoc lloc;
		if ( testing )
		{
			for ( int current_test_index = 0; current_test_index < test_data.length; ++current_test_index )
			{
				System.out.println( "Starting test " + ( current_test_index + 1 ) );
				lloc = new LaserLoc( true );

				for ( int current_test_data_index = 0; current_test_data_index < test_data[ current_test_index ].length; ++current_test_data_index )
				{
					lloc.laser_data = test_data[ current_test_index ][ current_test_data_index ];
					lloc.updatePose();
				}
				
			}
			System.out.println( "All tests done." );
			System.exit( 0 );
		}
		
		// Not testing
		lloc = new LaserLoc( false );
		while ( true )
		{
			lloc.updatePose();
		}
	}

}























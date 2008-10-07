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
	public static double translation_threshold = 0.05; // minimum translation distance to update x,y location, 0.05 total guess
	public static String laser_channel = "LASER";
	public static String pose_channel = "POSE";

	// Even though this comes in each message, it may help the compiler optimize 
	// if we make it a separate constant (can assert on it not matching the message)
	public static double laser_delta_angle = 0; // angle between laser indexes. 
	// end constants
	
	// regular state
	pose_t estimated_pose;

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
	}
	
	private void updatePose( laser_t laser_data )
	{
		if ( laser_data == null )
		{
			assert false;
			return;
		}
		
		if ( estimated_pose == null )
		{
			// initialize
			estimated_pose = getRobotXY( laser_data );
			estimated_pose.orientation = Geometry.rollPitchYawToQuat( new double[] { 0, 0, robot_starting_yaw } );
			if ( verbose )
			{
				System.out.print( "initial: " );
				printOldPose();
			}
			return;
		}
		
		pose_t new_estimated_pose = getRobotXY( laser_data );
		if ( new_estimated_pose == null )
		{
			return;
		}
		
		double translation_dist = Point.distance( estimated_pose.pos[ 0 ], estimated_pose.pos[ 1 ], new_estimated_pose.pos[ 0 ], new_estimated_pose.pos[ 1 ] );
		//System.out.print( "Moved " + translation_dist + " meters. " );
		
		// only update location if moved enough. Don't want Soar to thrash on constantly changing x,y due to noise
		// this also means this loop can run as fast as it can and we'll still get reasonable updates
		// (i.e., we don't have to worry about robot not moving enough between updates)
		if( translation_dist >= translation_threshold )
		{
			
			double estimated_pose_yaw = Math.atan2( new_estimated_pose.pos[ 1 ] - estimated_pose.pos[ 1 ], new_estimated_pose.pos[ 0 ] - estimated_pose.pos[ 0 ] );
			estimated_pose.orientation = Geometry.rollPitchYawToQuat( new double[] { 0, 0, estimated_pose_yaw } );
			
			// TODO: figure out best way to clone
			estimated_pose.pos[ 0 ] =  new_estimated_pose.pos[ 0 ];
			estimated_pose.pos[ 1 ] =  new_estimated_pose.pos[ 1 ];
			
			if ( !testing )
			{
				estimated_pose.utime = System.nanoTime() / 1000;
				lcm.publish( "POSE", estimated_pose );
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
	}

	private void printOldPose() 
	{
		System.out.println( "x,y,a: " + estimated_pose.pos[ 0 ] + "," + estimated_pose.pos[ 1 ] + "," + Math.toDegrees( Geometry.quatToRollPitchYaw( estimated_pose.orientation )[2] ) );
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
		
		return new_pose;
	}

	@Override
	public void messageReceived( LCM lcm, String channel, DataInputStream ins ) 
	{
		if ( channel.equals( laser_channel ) )
		{
	         try 
	         {
	        	 updatePose( new laser_t( ins ) );
	          } 
	         catch ( IOException ex ) 
	          {
	             System.out.println( "Error decoding laser message: " + ex );
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
					System.out.print( "Test data " + current_test_data_index + ": ");
					lloc.updatePose( test_data[ current_test_index ][ current_test_data_index ] );
				}
				
			}
			System.out.println( "All tests done." );
			System.exit( 0 );
		}
		
		// Not testing
		new LaserLoc( false );
		while ( true )
		{
			try
			{
				Thread.sleep( 500 );
			} 
			catch ( InterruptedException ignored )
			{}
		}
	}

}























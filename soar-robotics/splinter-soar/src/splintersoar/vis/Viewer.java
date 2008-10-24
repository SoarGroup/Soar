package splintersoar.vis;

import java.awt.BorderLayout;
import java.awt.Color;
import java.io.DataInputStream;
import java.io.IOException;

import javax.swing.JFrame;

import jmat.LinAlg;

import splintersoar.LCMInfo;
import splintersoar.lcmtypes.particles_t;
import splintersoar.lcmtypes.splinterstate_t;
import splintersoar.lcmtypes.waypoints_t;
import splintersoar.lcmtypes.xy_t;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;

import erp.vis.VisCanvas;
import erp.vis.VisChain;
import erp.vis.VisData;
import erp.vis.VisDataLineStyle;
import erp.vis.VisDataPointStyle;
import erp.vis.VisRobot;
import erp.vis.VisWorld;

public class Viewer implements LCMSubscriber
{
	JFrame jf;
	VisWorld vw = new VisWorld();
	VisCanvas vc = new VisCanvas(vw);
	
	LCM lcm;
	splinterstate_t splinterPose;
	waypoints_t waypoints;
	xy_t laserxy;
	particles_t particles;
	laser_t laser_front;
	
	public Viewer()
	{
		lcm = LCM.getSingleton();
		lcm.subscribe( LCMInfo.SPLINTER_STATE_CHANNEL, this );
		lcm.subscribe( LCMInfo.WAYPOINTS_CHANNEL, this );
		lcm.subscribe( LCMInfo.COORDS_CHANNEL, this );
		lcm.subscribe( LCMInfo.PARTICLES_CHANNEL, this );
		lcm.subscribe( LCMInfo.LASER_FRONT_CHANNEL, this );


		jf = new JFrame("RoomMapper");
		jf.setLayout(new BorderLayout());
		jf.add(vc, BorderLayout.CENTER);
		jf.setSize(600, 500);
		jf.setVisible(true);
		
		VisWorld.Buffer vb = vw.getBuffer("splinter");
		VisData vd = new VisData(new double[2], new double[] { 0.2, 0 },
				new VisDataLineStyle(Color.red, 1));
		
		vc.setDrawGrid(true);
		vc.setDrawGround(true);
		
		while ( true )
		{
			splinterstate_t sp = null;
			if ( splinterPose != null )
			{
				sp = splinterPose.copy();
				vb.addBuffered( new VisChain( LinAlg.quatPosToMatrix( sp.pose.orientation, sp.pose.pos),
						new VisRobot(Color.blue)));
			}
			
			if ( laserxy != null )
			{
				xy_t xy;
				xy = laserxy.copy();
				vb.addBuffered(new VisData( xy.xy, new VisDataPointStyle(Color.black, 4)));
			}
			
			if ( waypoints != null )
			{
				waypoints_t wp;
				wp = waypoints.copy();
				for ( int index = 0; index < wp.nwaypoints; ++index )
				{
					vb.addBuffered(new VisData( wp.locations[index].xy, new VisDataPointStyle(Color.green, 10)));
				}
			}
			
			if ( particles != null )
			{
				particles_t p;
				p = particles.copy();
				for ( double[] pxyt : p.particle ) {
					vb.addBuffered(new VisChain(LinAlg.xytToMatrix( pxyt ), vd));
				}
			}
			
			if ( laser_front != null )
			{
				laser_t lf;
				lf = laser_front.copy();
				
				VisData points = new VisData();
				points.add( new VisDataLineStyle( Color.green, 3, true) );
				
			    for (int i = 0; i < lf.nranges; i++) {
					if (lf.ranges[i] > 50)
						continue;

					double yaw = 0;
					double [] offset = new double [] { 0, 0, 0 }; 
					if (sp != null)
					{
						offset = sp.pose.pos;
						yaw = LinAlg.quatToRollPitchYaw( sp.pose.orientation )[2];
					}
					double theta = lf.rad0 + i * lf.radstep + yaw;
					double [] xyz = LinAlg.add( new double [] { lf.ranges[i] * Math.cos(theta), lf.ranges[i] * Math.sin(theta), 0 }, offset );
					points.add( xyz );
				}
			    
			    vb.addBuffered( points );
			}
			
			vb.switchBuffer();
		}
	}

	@Override
	public void messageReceived( LCM lcm, String channel, DataInputStream ins ) 
	{
		if ( channel.equals( LCMInfo.SPLINTER_STATE_CHANNEL ) )
		{
			try 
			{
				splinterPose = new splinterstate_t( ins );
			} 
			catch ( IOException ex ) 
			{
				System.err.println( "Error decoding splinterstate_t message: " + ex );
			}
		}
		else if ( channel.equals( LCMInfo.WAYPOINTS_CHANNEL ) )
		{
			try 
			{
				waypoints = new waypoints_t( ins );
			} 
			catch ( IOException ex ) 
			{
				System.err.println( "Error decoding waypoints_t message: " + ex );
			}
		}
		else if ( channel.equals( LCMInfo.COORDS_CHANNEL ) )
		{
			try 
			{
				laserxy = new xy_t( ins );
			} 
			catch ( IOException ex ) 
			{
				System.err.println( "Error decoding xy_t message: " + ex );
			}
		}
		else if ( channel.equals( LCMInfo.PARTICLES_CHANNEL ) )
		{
			try 
			{
				particles = new particles_t( ins );
			} 
			catch ( IOException ex ) 
			{
				System.err.println( "Error decoding particles_t message: " + ex );
			}
		}
		else if ( channel.equals( LCMInfo.LASER_FRONT_CHANNEL ) )
		{
			try 
			{
				laser_front = new laser_t( ins );
			} 
			catch ( IOException ex ) 
			{
				System.err.println( "Error decoding laser_t message: " + ex );
			}
		}
	}
	
	public static void main ( String args [] )
	{
		new Viewer();
	}
}

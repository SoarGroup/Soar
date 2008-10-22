package splintersoar.vis;

import java.awt.BorderLayout;
import java.awt.Color;
import java.io.DataInputStream;
import java.io.IOException;

import javax.swing.JFrame;

import jmat.LinAlg;

import splintersoar.lcmtypes.splinterstate_t;
import splintersoar.lcmtypes.waypoints_t;
import splintersoar.lcmtypes.xy_t;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;

import erp.vis.VisCanvas;
import erp.vis.VisChain;
import erp.vis.VisData;
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
	
	public Viewer()
	{
		lcm = LCM.getSingleton();
		lcm.subscribe( "SPLINTER_POSE", this );
		lcm.subscribe( "WAYPOINTS", this );
		lcm.subscribe( "COORDS", this );


		jf = new JFrame("RoomMapper");
		jf.setLayout(new BorderLayout());
		jf.add(vc, BorderLayout.CENTER);
		jf.setSize(600, 500);
		jf.setVisible(true);
		
		VisWorld.Buffer vb = vw.getBuffer("splinter");

		while ( true )
		{
			if ( splinterPose != null )
			{
				splinterstate_t sp;
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
					vb.addBuffered(new VisData( wp.locations[index].xy, new VisDataPointStyle(Color.green, 3)));
				}
			}
			
			vb.switchBuffer();
		}
	}

	@Override
	public void messageReceived( LCM lcm, String channel, DataInputStream ins ) 
	{
		if ( channel.equals( "SPLINTER_POSE" ) )
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
		else if ( channel.equals( "WAYPOINTS" ) )
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
		else if ( channel.equals( "COORDS" ) )
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
	}
	
	public static void main ( String args [] )
	{
		new Viewer();
	}
}

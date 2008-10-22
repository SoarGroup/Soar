package splintersoar.vis;

import java.awt.BorderLayout;
import java.awt.Color;
import java.io.DataInputStream;
import java.io.IOException;

import javax.swing.JFrame;

import jmat.LinAlg;

import splintersoar.lcmtypes.splinterstate_t;
import splintersoar.lcmtypes.waypoints_t;

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
	
	public Viewer()
	{
		lcm = LCM.getSingleton();
		lcm.subscribe( "SPLINTER_POSE", this );


		jf = new JFrame("RoomMapper");
		jf.setLayout(new BorderLayout());
		jf.add(vc, BorderLayout.CENTER);
		jf.setSize(600, 500);
		jf.setVisible(true);
		
		VisWorld.Buffer vb = vw.getBuffer("splinter");

		while ( true )
		{
			try {
				this.wait();
			} catch (InterruptedException e) {}
			
			if ( splinterPose != null )
			{
				splinterstate_t sp;
				synchronized ( splinterPose )
				{
					sp = splinterPose.copy();
				}
				vb.addBuffered( new VisChain( LinAlg.quatPosToMatrix( sp.pose.orientation, sp.pose.pos),
						new VisRobot(Color.blue)));
			}
			
			if ( waypoints != null )
			{
				waypoints_t wp;
				synchronized ( waypoints )
				{
					wp = waypoints.copy();
				}
				for ( int index = 0; index < wp.nwaypoints; ++index )
				{
					vb.addBuffered(new VisData( wp.locations[index], new VisDataPointStyle(Color.green, 3)));
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
				synchronized ( splinterPose )
				{
					splinterPose = new splinterstate_t( ins );
					splinterPose.notify();
				}
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
				synchronized ( waypoints )
				{
					waypoints = new waypoints_t( ins );
					waypoints.notify();
				}
			} 
			catch ( IOException ex ) 
			{
				System.err.println( "Error decoding waypoints_t message: " + ex );
			}
		}
	}
	
	public static void main ( String args [] )
	{
		new Viewer();
	}
}

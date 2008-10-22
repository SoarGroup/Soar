package laserloc;

import java.awt.BorderLayout;
import java.awt.Color;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.Arrays;

import javax.swing.JFrame;

import erp.vis.VisCanvas;
import erp.vis.VisData;
import erp.vis.VisDataPointStyle;
import erp.vis.VisWorld;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;

public class RoomMapper implements LCMSubscriber
{
	
	LCM lcm;
	laser_t laser_data;
	float[] ranges; 
	float radstep;
	
	JFrame jf;
	VisWorld vw = new VisWorld();
	VisCanvas vc = new VisCanvas(vw);
	
	public RoomMapper( boolean testing )
	{
		if ( !testing )
		{
			lcm = LCM.getSingleton();
			lcm.subscribe( LaserLoc.laser_channel, this );
		
			int readings = 75 * 5; // 5 seconds at 75 Hz
			while ( readings-- > 0 )
			{
				update();
			}
		} else {
			ranges = new float[180];
			Arrays.fill( ranges, 3.0f );
			radstep = (float)Math.toRadians( 1 );
		}
		
		jf = new JFrame("RoomMapper");
		jf.setLayout(new BorderLayout());
		jf.add(vc, BorderLayout.CENTER);
		jf.setSize(600, 500);
		jf.setVisible(true);
		
		VisWorld.Buffer vb = vw.getBuffer("map");
		
		displayRanges(vb, false);
		
		float bufferMeters = .25f;
		for ( int index = 0; index < ranges.length; ++index )
		{
			ranges[index] -= bufferMeters;
		}

		displayRanges(vb, true);
		
		vb.switchBuffer();
	}
	
	void displayRanges(VisWorld.Buffer vb, boolean buffer)
	{
		double angle = Math.toRadians( -90 );
		for( int index = 0; index < ranges.length; ++index )
		{
			double [] xy = new double[2];
			xy[0] = ranges[index] * Math.cos( angle );
			xy[1] = ranges[index] * Math.sin( angle );
			
			System.err.println( Arrays.toString( xy ));
			vb.addBuffered(new VisData( xy, new VisDataPointStyle(buffer ? Color.red : Color.black, 2)));
			
			angle += radstep;
		}
		
	}
	
	void update()
	{
		while ( laser_data == null )
		{
			try {
				Thread.sleep( 500 );
			} catch (InterruptedException e) {}
		}
		
		radstep = laser_data.radstep;
		
		if ( ranges == null )
		{
			ranges = new float[ laser_data.nranges ];
			Arrays.fill( ranges, Float.MAX_VALUE );
		}
		
		for ( int index = 0; index < laser_data.nranges; ++index )
		{
			ranges[index] = Math.min( ranges[index], laser_data.ranges[index]);
		}
	}
	
	@Override
	public void messageReceived( LCM lcm, String channel, DataInputStream ins ) 
	{
		if ( channel.equals( LaserLoc.laser_channel ) )
		{
			if ( laser_data != null )
			{
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

	public static void main ( String [] args )
	{
		new RoomMapper( true );
	}

}

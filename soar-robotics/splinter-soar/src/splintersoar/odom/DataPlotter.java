package splintersoar.odom;

import java.awt.BorderLayout;
import java.awt.Color;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
//import java.util.ArrayList;
import java.util.Arrays;

import javax.swing.JFrame;

import erp.geom.Geometry;
import erp.math.MathUtil;
import erp.vis.VisCanvas;
import erp.vis.VisData;
import erp.vis.VisDataPointStyle;
import erp.vis.VisWorld;

public class DataPlotter {

	public static void main( String [] args )
	{
		if (args.length < 1)
		{
			System.out.println( "Usage: dataplotter <odometry-data>");
			System.exit(1);
		}
		DataPlotter dp = new DataPlotter( args[0] );
		
		dp.process();
	}

	BufferedReader br;
	JFrame jf;
	VisWorld vw = new VisWorld();
	VisCanvas vc = new VisCanvas(vw);

	public DataPlotter( String file )
	{
		File datafile = new File( file );
		FileReader fr = null;
		try {
			fr = new FileReader( datafile );
		} catch (IOException e) {
			e.printStackTrace();
			System.exit( 1 );
		}
		br = new BufferedReader( fr );

		//System.out.println( "file opened");
		jf = new JFrame("Odom Plotter");
		jf.setLayout(new BorderLayout());
		jf.add(vc, BorderLayout.CENTER);
		jf.setSize(600, 500);
		jf.setVisible(true);
	}

	public void process()
	{
		VisWorld.Buffer vb = vw.getBuffer("truth");
		
		try {
			Thread.sleep( 1000 );
		} catch (InterruptedException e) {
		}
		
		//ArrayList<VisData> points = new ArrayList<VisData>();

		try {
			boolean mark = false;
			int count = -600;
			while ( br.ready() ) 
			{
				String line = br.readLine();
				if ( line.length() < 2 )
				{
					mark = true;
					continue;
				}
				String [] odomString = line.split(",");
				int [] odometry = new int[2];
				odometry[0] = Integer.parseInt( odomString[0] );
				odometry[1] = Integer.parseInt( odomString[1] );

				//System.out.println( odometry[0] + "," + odometry[1] );
				
				updatePosition( odometry );
				double [] xy = Arrays.copyOf( position, 2);
				if ( mark == false )
				{
					//points.add(new VisData( xy, new VisDataPointStyle(Color.black, 1)));
					vb.addBuffered(new VisData( xy, new VisDataPointStyle(Color.black, 1)));
				} 
				else
				{
					//points.add(new VisData( xy, new VisDataPointStyle(Color.red, 4)));
					vb.addBuffered(new VisData( xy, new VisDataPointStyle(Color.red, 4)));
					mark = false;
				}
				
				//for ( VisData data : points)
				//{
				//	vb.addBuffered(data);
				//}
				//vb.switchBuffer();

				//try {
				//	Thread.sleep( 30 );
				//} catch (InterruptedException e) {
				//}
				
				if ( --count == 0 )
				{
					break;
				}
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.exit(1);
		}
		
		vb.switchBuffer();

		System.out.println( "Done.");
		while ( true )
		{
			try {
				Thread.sleep( 500 );
			} catch (InterruptedException e) {
			}
		}
	}
	
	int [] previousOdometry;
	double [] position;
	void updatePosition( int [] odometry )
	{
		if ( previousOdometry == null ) {
			previousOdometry = Arrays.copyOf( odometry, odometry.length );
			position = new double [] { 0, 0, 0 };
			return;
		}
		
		//double tickmeters = 0.0000429250;
		double tickmeters = 0.000043225;
		//double baselinemeters = 0.42545;
		double baselinemeters = 0.383;
		double dleft = ( odometry[0] - previousOdometry[0] ) * tickmeters;
		double dright = ( odometry[1] - previousOdometry[1] ) * tickmeters;
		double phi = ( dright - dleft ) / baselinemeters;
		phi = MathUtil.mod2pi( phi );
		//System.out.println( dleft + "," + dright + "," + phi );
		double dcenter = ( dleft + dright ) / 2;
		
		double [] deltaxyt = { dcenter * Math.cos( position[2] ), dcenter * Math.sin( position[2] ), phi };
		position = Geometry.add( position, deltaxyt );
		position[2] = MathUtil.mod2pi( position[2] );
		//System.out.println( position[0] + "," + position[1] + "," + Math.toDegrees(position[2]) );
		previousOdometry = Arrays.copyOf( odometry, odometry.length );
	}
}

package splintersoar.laserloc;

import java.awt.BorderLayout;
import java.awt.Color;
import java.io.DataInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.util.Arrays;

import javax.swing.JFrame;

import splintersoar.LCMInfo;

import vis.VisCanvas;
import vis.VisData;
import vis.VisDataPointStyle;
import vis.VisWorld;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;

/**
 * @author voigtjr
 * Looks at SICK data and collects a boundary. Results are used by the laser localizer
 * to help it determine if what it is seeing is a beacon or a static object.
 */
public class RoomMapper implements LCMSubscriber {

	/**
	 * Sick data from the localizing sick comes in on H1
	 */
	LCM lcmH1;
	/**
	 * Sick data
	 */
	laser_t laserData;
	/**
	 * The values we've observed.
	 */
	float[] ranges;
	float radstep;

	JFrame jf;

	public RoomMapper(boolean display) {
		try {
			lcmH1 = new LCM(LCMInfo.H1_NETWORK);

		} catch (IOException e) {
			e.printStackTrace();
			System.exit(1);
		}
		lcmH1.subscribe(LCMInfo.LASER_LOC_CHANNEL, this);

		int readings = 75 * 5; // 5 seconds at 75 Hz
		while (readings-- > 0) {
			update();
		}

		if (display) {
			VisWorld vw = new VisWorld();
			VisCanvas vc = new VisCanvas(vw);

			jf = new JFrame("RoomMapper");
			jf.setLayout(new BorderLayout());
			jf.add(vc, BorderLayout.CENTER);
			jf.setSize(600, 500);
			jf.setVisible(true);

			VisWorld.Buffer vb = vw.getBuffer("map");

			displayRanges(vb, false);

			float bufferMeters = .25f;
			for (int index = 0; index < ranges.length; ++index) {
				ranges[index] -= bufferMeters;
			}

			displayRanges(vb, true);
			System.out.println("Number of ranges: " + ranges.length );

			vb.switchBuffer();
		}

		FileOutputStream out;
		try {
			out = new FileOutputStream("map.txt");
			PrintStream p = new PrintStream(out);

			p.print("map=");
			p.print(Arrays.toString(ranges));
			p.print(";");
			p.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
	}

	void displayRanges(VisWorld.Buffer vb, boolean buffer) {
		double angle = Math.toRadians(-90);
		for (int index = 0; index < ranges.length; ++index) {
			double[] xy = new double[2];
			xy[0] = ranges[index] * Math.cos(angle);
			xy[1] = ranges[index] * Math.sin(angle);

			vb.addBuffered(new VisData(xy, new VisDataPointStyle(buffer ? Color.red : Color.black, 2)));

			angle += radstep;
		}
	}

	void update() {
		while (laserData == null) {
			try {
				Thread.sleep(500);
			} catch (InterruptedException e) {
			}
		}

		radstep = laserData.radstep;
		
		if (laserData.nranges != 180)
		{
			System.err.println("Number of ranges is not 180!");
			System.exit(1);
		}
		
		if (ranges == null) {
			ranges = new float[laserData.nranges];
			Arrays.fill(ranges, Float.MAX_VALUE);
		}

		for (int index = 0; index < laserData.nranges; ++index) {
			ranges[index] = Math.min(ranges[index], laserData.ranges[index]);
		}
	}

	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(LCMInfo.LASER_LOC_CHANNEL)) {
			if (laserData != null) {
				return;
			}

			try {
				laserData = new laser_t(ins);
			} catch (IOException ex) {
				System.err.println("Error decoding laser message: " + ex);
			}
		}

	}

	public static void main(String[] args) {
		new RoomMapper(true);
	}

}

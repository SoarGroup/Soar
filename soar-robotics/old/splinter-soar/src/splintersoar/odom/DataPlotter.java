package splintersoar.odom;

import java.awt.BorderLayout;
import java.awt.Color;
import java.io.BufferedReader;
import java.io.Console;
import java.io.File;
import java.io.FileReader;
import java.io.IOException; //import java.util.ArrayList;
import java.util.ArrayList;
import java.util.Arrays;

import javax.swing.JFrame;

import jmat.LinAlg;
import jmat.MathUtil;

import splintersoar.Configuration;
import vis.VisCanvas;
import vis.VisData;
import vis.VisDataPointStyle;
import vis.VisWorld;

/**
 * @author voigtjr
 * Tool used to plot odometry data for calibration.
 */
public class DataPlotter {

	public static void main(String[] args) {
		if (args.length < 1) {
			System.out.println("Usage: dataplotter <odometry-data>");
			System.exit(1);
		}
		DataPlotter dp = new DataPlotter(args[0]);

		dp.run();
	}
	
	Configuration cnf = new Configuration(null);
	
	public void run() {
		plot();

		Console console = System.console();
		while (true) {
			try {
				String input = console.readLine("plotter> ");
				
				input = input.toLowerCase();
				input = input.trim();
				String [] args = input.split(" ");
				
				if (args[0].equals("plot")) {
					if (args.length == 1) {
						plot();
					} else {
						plot(Integer.parseInt(args[1]));
					}
				} else if (args[0].equals("file")) {
					readFile(args[1]);
					plot();
				} else if (args[0].equals("dec")) {
					if (args[1].equals("baseline")) {
						cnf.orc.baselineMeters -= Double.parseDouble(args[2]);
					} else if (args[1].equals("tick")) {
						cnf.orc.tickMeters -= Double.parseDouble(args[2]);
					}
					plot();
				} else if (args[0].equals("inc")) {
					if (args[1].equals("baseline")) {
						cnf.orc.baselineMeters += Double.parseDouble(args[2]);
					} else if (args[1].equals("tick")) {
						cnf.orc.tickMeters += Double.parseDouble(args[2]);
					}
					plot();
				} else if (args[0].equals("quit") || args[0].equals("exit")) {
					jf.dispose();
					System.exit(1);
				}
			} catch (ArrayIndexOutOfBoundsException ex) {
				System.err.println("Syntax error.");
			} catch (Throwable t) {
				System.err.println("Unhandled exception.");
				t.printStackTrace();
				System.exit(1);
			}
		}
	}

	ArrayList<int[]> data;
	JFrame jf;
	VisWorld vw = new VisWorld();
	VisCanvas vc = new VisCanvas(vw);
	
	public DataPlotter(String file) {
		readFile(file);

		jf = new JFrame("Odom Plotter");
		jf.setLayout(new BorderLayout());
		jf.add(vc, BorderLayout.CENTER);
		jf.setSize(600, 500);
		jf.setVisible(true);
		
		vc.setDrawGrid(true);
		vc.setDrawGround(true);
	}

	public void readFile(String file) {
		File datafile = new File(file);
		FileReader fr = null;
		try {
			fr = new FileReader(datafile);
		} catch (IOException e) {
			e.printStackTrace();
			System.exit(1);
		}
		
		 data = new ArrayList<int[]>();

		BufferedReader br = new BufferedReader(fr);
		try {
			while (br.ready()) {
				String line = br.readLine();

				if (line.length() < 2) {
					data.add(null);
					continue;
				}
				
				String[] odomString = line.split(",");
				int[] odometry = new int[2];
				odometry[0] = Integer.parseInt(odomString[0]);
				odometry[1] = Integer.parseInt(odomString[1]);
				
				data.add(odometry);
			}
		} catch (IOException e) {
			e.printStackTrace();
			System.exit(1);
		}
	}
	
	public void plot() {
		plot(-1);
	}
	
	public void plot(int count) {
		VisWorld.Buffer vb = vw.getBuffer("truth");

		int[] previousOdometry = null;
		double[] position = new double[] { 0, 0, 0 };

		double z = 0;
		boolean mark = false;
		for(int[] odometry : data) {
			if (odometry == null) {
				mark = true;
				odometry = Arrays.copyOf(previousOdometry, previousOdometry.length);
			}
			
			if (previousOdometry == null) {
				if (odometry == null) {
					continue;
				}
				previousOdometry = Arrays.copyOf(odometry, odometry.length);
			} else {
				updatePosition(odometry, previousOdometry, position);
			}
			
			double[] xy = new double[] { position[0], position[1], z };
			if (mark) {
				vb.addBuffered(new VisData(xy, new VisDataPointStyle(Color.red, 4)));
				mark = false;
			} else {
				vb.addBuffered(new VisData(xy, new VisDataPointStyle(Color.black, 1)));
			}

			if (--count == 0) {
				break;
			}
			z += 0.000005;
		}

		vb.switchBuffer();
		System.out.println(String.format("baseline: %7.5f, tick: %10.8f", cnf.orc.baselineMeters, cnf.orc.tickMeters));
	}

	void updatePosition(int[] odometry, int[] previousOdometry, double[] position) {
		double dleft = (odometry[0] - previousOdometry[0]) * cnf.orc.tickMeters;
		double dright = (odometry[1] - previousOdometry[1]) * cnf.orc.tickMeters;
		double phi = (dright - dleft) / cnf.orc.baselineMeters;
		phi = MathUtil.mod2pi(phi);
		double dcenter = (dleft + dright) / 2;

		double[] deltaxyt = { dcenter * Math.cos(position[2]), dcenter * Math.sin(position[2]), phi };
		LinAlg.add(position, deltaxyt, position);
		position[2] = MathUtil.mod2pi(position[2]);
		
		System.arraycopy(odometry, 0, previousOdometry, 0, odometry.length);
	}
}

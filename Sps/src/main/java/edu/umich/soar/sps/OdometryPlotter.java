package edu.umich.soar.sps;

import java.awt.BorderLayout;
import java.awt.Color;
import java.io.BufferedReader;
import java.io.Console;
import java.io.File;
import java.io.FileReader;
import java.io.IOException; //import java.util.ArrayList;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Queue;

import javax.swing.JFrame;


import edu.umich.soar.sps.lcmtypes.odom_t;
import edu.umich.soar.sps.splinter.Splinter;

import lcmtypes.pose_t;

import vis.VisCanvas;
import vis.VisData;
import vis.VisDataPointStyle;
import vis.VisWorld;

/**
 * @author voigtjr
 * Tool used to plot odometry data for calibration.
 */
final class OdometryPlotter {
	private final JFrame jf;
	private final VisWorld vw = new VisWorld();
	private final VisCanvas vc = new VisCanvas(vw);
	
	private ArrayList<odom_t> data;
	private double baselineMeters = Splinter.DEFAULT_BASELINE;
	private double tickMeters = Splinter.DEFAULT_TICKMETERS;

	public static void main(String[] args) {
		if (args.length < 1) {
			System.err.println("Usage: OdometryPlotter <odometry-data>");
			System.exit(1);
		}
		
		OdometryPlotter p = null;
		if (args.length > 1) {
			p = new OdometryPlotter(args[0], args[1]);
		} else {
			p = new OdometryPlotter(args[0], null);
		}
		p.run();
	}
	
	private OdometryPlotter(String odom, String tags) {
		readOdom(odom, tags);

		jf = new JFrame("Odometry Plotter");
		jf.setLayout(new BorderLayout());
		jf.add(vc, BorderLayout.CENTER);
		jf.setSize(600, 500);
		jf.setVisible(true);
		
		vc.setDrawGrid(true);
		vc.setDrawGround(true);
	}

	private void readOdom(String odomPath, String tagsPath) {
		try {
			Queue<Long> tags = new LinkedList<Long>();
			if (tagsPath != null) {
				BufferedReader tfbr = new BufferedReader(new FileReader(new File(tagsPath)));
	
				while (tfbr.ready()) {
					tags.add(Long.parseLong(tfbr.readLine()));
				}
			}
			
			BufferedReader ofbr = new BufferedReader(new FileReader(new File(odomPath)));
			data = new ArrayList<odom_t>();

			while (ofbr.ready()) {
				String line = ofbr.readLine();
				String[] odomString = line.split(",");

				odom_t odom = new odom_t();
				odom.utime = Long.parseLong(odomString[0]);
				odom.left = Integer.parseInt(odomString[1]);
				odom.right = Integer.parseInt(odomString[2]);
				
				if (tags.size() > 0 && odom.utime > tags.peek()) {
					tags.poll();
					data.add(null); // nulls are marks
				}
				data.add(odom);
			}
			
			while (tags.size() > 0) {
				tags.poll();
				data.add(null);
			}
			
		} catch (IOException e) {
			System.err.println(e.getMessage());
			e.printStackTrace();
			System.exit(1);
		}
	}
	
	public void run() {
		plot();

		Console console = System.console();
		if (console != null) {
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
					} else if (args[0].equals("files")) {
						readOdom(args[1], args[2]);
						plot();
					} else if (args[0].equals("dec")) {
						if (args[1].equals("baseline")) {
							baselineMeters -= Double.parseDouble(args[2]);
						} else if (args[1].equals("tick")) {
							tickMeters -= Double.parseDouble(args[2]);
						}
						plot();
					} else if (args[0].equals("inc")) {
						if (args[1].equals("baseline")) {
							baselineMeters += Double.parseDouble(args[2]);
						} else if (args[1].equals("tick")) {
							tickMeters += Double.parseDouble(args[2]);
						}
						plot();
					} else if (args[0].equals("set")) {
						if (args[1].equals("baseline")) {
							baselineMeters = Double.parseDouble(args[2]);
						} else if (args[1].equals("tick")) {
							tickMeters = Double.parseDouble(args[2]);
						}
						plot();
					} else if (args[0].equals("help")) {
						System.out.println("plot files dec inc set quit");
						
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
		} else {
			System.err.println("Error: no console.");
		}
	}

	private void plot() {
		plot(-1);
	}
	
	private void plot(int count) {
		VisWorld.Buffer vb = vw.getBuffer("truth");

		Odometry odometry = new Odometry(tickMeters, baselineMeters);
		odom_t oldOdom = null;
		pose_t pose = new pose_t();

		double z = 0;
		boolean mark = false;
		for(odom_t newOdom : data) {
			if (oldOdom == null) {
				if (newOdom == null) {
					continue;
				}
				oldOdom = newOdom.copy();
			} else if (newOdom != null) {
				odometry.propagate(newOdom, oldOdom, pose);
				oldOdom = newOdom.copy();
			} else {
				mark = true;
				newOdom = oldOdom.copy();
			}
			
			double[] xy = new double[] { pose.pos[0], pose.pos[1], z };
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
		System.out.println(String.format("baseline: %7.5f, tick: %10.8f", baselineMeters, tickMeters));
	}
}

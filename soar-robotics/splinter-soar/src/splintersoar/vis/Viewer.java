package splintersoar.vis;

import java.awt.BorderLayout;
import java.awt.Color;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.Arrays;

import javax.swing.JFrame;

import april.config.Config;
import april.config.ConfigFile;

import jmat.LinAlg;

import splintersoar.LCMInfo;
import lcmtypes.particles_t;
import lcmtypes.splinterstate_t;
import lcmtypes.waypoints_t;
import lcmtypes.xy_t;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;

import vis.VisCanvas;
import vis.VisChain;
import vis.VisData;
import vis.VisDataLineStyle;
import vis.VisDataPointStyle;
import vis.VisRobot;
import vis.VisWorld;

/**
 * @author voigtjr
 * Simple viewer for splinter/Soar LCM traffic.
 */
public class Viewer implements LCMSubscriber {
	JFrame jf;
	VisWorld vw = new VisWorld();
	VisCanvas vc = new VisCanvas(vw);

	LCM lcmH1;
	LCM lcmGG;
	splinterstate_t splinterPose;
	waypoints_t waypoints;
	xy_t laserxy;
	particles_t particles;
	laser_t laserFront;
	laser_t laserLoc;
	double [] initialxy;

	public Viewer() {
		try {
			lcmH1 = new LCM(LCMInfo.H1_NETWORK);

		} catch (IOException e) {
			e.printStackTrace();
			System.exit(1);
		}

		try {
			lcmGG = new LCM(LCMInfo.GG_NETWORK);

		} catch (IOException e) {
			e.printStackTrace();
			System.exit(1);
		}
		
		lcmGG.subscribe(LCMInfo.SPLINTER_STATE_CHANNEL, this);
		lcmGG.subscribe(LCMInfo.WAYPOINTS_CHANNEL, this);
		lcmGG.subscribe(LCMInfo.COORDS_CHANNEL, this);
		lcmGG.subscribe(LCMInfo.PARTICLES_CHANNEL, this);
		lcmGG.subscribe(LCMInfo.LASER_FRONT_CHANNEL, this);
		lcmH1.subscribe(LCMInfo.LASER_LOC_CHANNEL, this);
		
		jf = new JFrame("Viewer");
		jf.setLayout(new BorderLayout());
		jf.add(vc, BorderLayout.CENTER);
		jf.setSize(600, 500);
		jf.setVisible(true);

		VisWorld.Buffer vb = vw.getBuffer("splinter");
		VisData vd = new VisData(new double[2], new double[] { 0.2, 0 }, new VisDataLineStyle(Color.red, 1));

		vc.setDrawGrid(true);
		vc.setDrawGround(true);

		makePoints(vb);
		
		while (true) {
			splinterstate_t sp = null;
			if (splinterPose != null) {
				sp = splinterPose;
				vb.addBuffered(new VisChain(LinAlg.quatPosToMatrix(sp.pose.orientation, sp.pose.pos), new VisRobot(Color.blue)));
			}

			if (laserxy != null) {
				xy_t xy;
				xy = laserxy.copy();

				if (initialxy == null /*&& sp != null*/) {
					initialxy = new double [] { xy.xy[0], xy.xy[1] };
					makePoints(vb);
				}

				if (initialxy != null) {
					LinAlg.subtract(xy.xy, initialxy, xy.xy);
				}

				vb.addBuffered(new VisData(xy.xy, new VisDataPointStyle(Color.black, 4)));
			}

			if (waypoints != null) {
				waypoints_t wp;
				wp = waypoints;
				for (int index = 0; index < wp.nwaypoints; ++index) {
					vb.addBuffered(new VisData(wp.locations[index].xy, new VisDataPointStyle(Color.red, 10)));
				}
			}

			if (particles != null) {
				particles_t p;
				p = particles;
				for (float[] pxyt : p.particle) {
					double [] dpxyt = new double [] { pxyt[0], pxyt[1], pxyt[2] };
					vb.addBuffered(new VisChain(LinAlg.xytToMatrix(dpxyt), vd));
				}
			}

			if (laserFront != null) {
				laser_t lf;
				lf = laserFront;

				VisData points = new VisData();
				points.add(new VisDataLineStyle(Color.green, 2, true));

				for (int i = 0; i < lf.nranges; i++) {
					if (lf.ranges[i] > 50)
						continue;

					double yaw = 0;
					double[] offset = new double[] { 0, 0 };
					if (sp != null) {
						offset = Arrays.copyOf(sp.pose.pos, 2);
						yaw = LinAlg.quatToRollPitchYaw(sp.pose.orientation)[2];
					}
					double theta = lf.rad0 + i * lf.radstep + yaw;
					double[] xy = LinAlg.add(new double[] { lf.ranges[i] * Math.cos(theta), lf.ranges[i] * Math.sin(theta) }, offset);
					points.add(xy);
				}

				vb.addBuffered(points);
			}

			if (laserLoc != null) {
				laser_t ll;
				ll = laserLoc;

				VisData points = new VisData();
				points.add(new VisDataLineStyle(Color.orange, 2, true));

				for (int i = 0; i < ll.nranges; i++) {
					if (ll.ranges[i] > 50)
						continue;

					double theta = ll.rad0 + i * ll.radstep;
					double [] xy = new double [] { ll.ranges[i] * Math.cos(theta), ll.ranges[i] * Math.sin(theta) };
					if (initialxy != null) {
						LinAlg.subtract(xy, initialxy, xy);
					}
					points.add(xy);
				}

				vb.addBuffered(points);
			}

			vb.switchBuffer();
		}
	}
	
	void makePoints(VisWorld.Buffer vb) {
		try {
			Config mapConfig = new ConfigFile("map.txt").getConfig();
			double [] maxRanges = mapConfig.getDoubles("map");

			VisData points = new VisData();
			points.add(new VisDataLineStyle(Color.black, 2, true));

			for (int i = 0; i < maxRanges.length; i++) {
				if (maxRanges[i] > 50)
					continue;

				double theta = -(Math.PI / 2) + i * (Math.PI / 180);
				double [] xy = new double [] { maxRanges[i] * Math.cos(theta), maxRanges[i] * Math.sin(theta) };
				if (initialxy != null) {
					LinAlg.subtract(xy, initialxy, xy);
				}
				points.add(xy);
			}

			vb.addBuffered(points);
		} catch (IOException e) {
		}

	}

	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(LCMInfo.SPLINTER_STATE_CHANNEL)) {
			try {
				splinterPose = new splinterstate_t(ins);
			} catch (IOException ex) {
				System.err.println("Error decoding splinterstate_t message: " + ex);
			}
		} else if (channel.equals(LCMInfo.WAYPOINTS_CHANNEL)) {
			try {
				waypoints = new waypoints_t(ins);
			} catch (IOException ex) {
				System.err.println("Error decoding waypoints_t message: " + ex);
			}
		} else if (channel.equals(LCMInfo.COORDS_CHANNEL)) {
			try {
				laserxy = new xy_t(ins);
			} catch (IOException ex) {
				System.err.println("Error decoding xy_t message: " + ex);
			}
		} else if (channel.equals(LCMInfo.PARTICLES_CHANNEL)) {
			try {
				particles = new particles_t(ins);
			} catch (IOException ex) {
				System.err.println("Error decoding particles_t message: " + ex);
			}
		} else if (channel.equals(LCMInfo.LASER_FRONT_CHANNEL)) {
			try {
				laserFront = new laser_t(ins);
			} catch (IOException ex) {
				System.err.println("Error decoding laser_t message: " + ex);
			}
		} else if (channel.equals(LCMInfo.LASER_LOC_CHANNEL)) {
			try {
				laserLoc = new laser_t(ins);
			} catch (IOException ex) {
				System.err.println("Error decoding laser_t message: " + ex);
			}
		}
	}

	public static void main(String args[]) {
		new Viewer();
	}
}

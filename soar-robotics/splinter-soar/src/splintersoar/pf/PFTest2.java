package splintersoar.pf;

import java.util.*;
import java.awt.*;
import java.io.DataInputStream;
import java.io.IOException;

import javax.swing.*;
import erp.vis.*;
import erp.math.*;
import jmat.*;
import lcm.lcm.*;
import lcmtypes.pose_t;

public class PFTest2 implements LCMSubscriber {
	static class Particle {
		double xyt[];
		double weight;
	}

	public static void main(String args[]) {

		PFTest2 pf = new PFTest2();

		LCM lcm = LCM.getSingleton();
		lcm.subscribe("ODOM_DEBUG", pf);
		lcm.subscribe("POSE", pf);

		while (true) {
			pf.step();
			try {
				Thread.sleep(10);
			} catch (InterruptedException ex) {
			}
		}
	}

	double[] pfxyt;
	double[] lcmxyt = new double[3];
	double[] laserxy;
	double[] oldlaserxy = new double[2];

	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals("ODOM_DEBUG")) {
			try {
				pose_t rawOdom = new pose_t(ins);

				lcmxyt[2] += rawOdom.pos[1];
				lcmxyt[0] = lcmxyt[0] + (rawOdom.pos[0] * Math.cos(lcmxyt[2]));
				lcmxyt[1] = lcmxyt[1] + (rawOdom.pos[0] * Math.sin(lcmxyt[2]));
			} catch (IOException ex) {
				System.err.println("Error decoding odom message: " + ex);
			}

			if (pfxyt == null) {
				pfxyt = Arrays.copyOf(lcmxyt, lcmxyt.length);

				Arrays.fill(lcmxyt, 0);
			}
		} else if (channel.equals("POSE")) {
			if (laserxy != null) {
				return;
			}

			try {
				pose_t laserlocation = new pose_t(ins);
				laserxy = new double[] { laserlocation.pos[0],
						laserlocation.pos[1] };
			} catch (IOException ex) {
				System.err.println("Error decoding laser message: " + ex);
			}
		}
	}

	JFrame jf;
	VisWorld vw = new VisWorld();
	VisCanvas vc = new VisCanvas(vw);

	// double time;
	// double xyt_truth[];

	Random rand = new Random();

	ArrayList<Particle> particles = new ArrayList<Particle>();

	public PFTest2() {
		jf = new JFrame("PF Test2");
		jf.setLayout(new BorderLayout());
		jf.add(vc, BorderLayout.CENTER);
		jf.setSize(600, 400);
		jf.setVisible(true);

		init();
	}

	void init() {
		for (int i = 0; i < 100; i++) {
			Particle p = new Particle();
			p.xyt = new double[] { (1 - 2 * rand.nextFloat()) * 5,
					(1 - 2 * rand.nextFloat()) * 5,
					2 * rand.nextFloat() * Math.PI };
			particles.add(p);
		}
	}

	void step() {
		// time += 0.01;
		update();

		try {
			Thread.sleep(33);
		} catch (InterruptedException ignored) {
		}
	}

	boolean started = false;

	void update() {

		if (!started) {
			if (pfxyt == null) {
				return;
			}
			started = true;
			System.out.println("Started");
		}

		double[] pfxytcopy = new double[3];
		if (pfxyt != null) {
			System.arraycopy(pfxyt, 0, pfxytcopy, 0, pfxyt.length);
			pfxyt = null;
		}
		double thetadeltadegrees = Math.toDegrees(pfxytcopy[2]);
		System.out.format("%10.6f%n", thetadeltadegrees);

		// //////////////////////////////////////////////////
		// Simulate motion of real robot.

		// drive in a big circle
		// double theta = time;
		// double radius = 5;

		// double xyt_truth_new[] = new double[] { radius * Math.cos(theta),
		// radius * Math.sin(theta), theta + Math.PI / 2 };
		// if (xyt_truth == null)
		// xyt_truth = LinAlg.copy(xyt_truth_new);

		VisWorld.Buffer vb = vw.getBuffer("truth");
		// vb.addBuffered(new VisChain(LinAlg.xytToMatrix(xyt_truth_new),
		// new VisRobot(Color.blue)));

		// //////////////////////////////////////////////////
		// Compute Odometry (perfect)

		// AT = B
		// T = inv(A)*B
		// double odom_xyt_truth[];
		// odom_xyt_truth = LinAlg.xytInvMul31(xyt_truth, xyt_truth_new);

		// double odom_xyt_noisy[] = new double[3];
		// odom_xyt_noisy[0] = odom_xyt_truth[0] + rand.nextGaussian() * 0.01;
		// odom_xyt_noisy[1] = odom_xyt_truth[1] + rand.nextGaussian() * 0.01;
		// odom_xyt_noisy[2] = odom_xyt_truth[2] + rand.nextGaussian() * 0.001;

		// //////////////////////////////////////////////////
		// propagate odometry
		for (Particle p : particles) {
			p.xyt = LinAlg.xytMultiply(p.xyt, pfxytcopy);
		}

		VisData vd = new VisData(new double[2], new double[] { 0.2, 0 },
				new VisDataLineStyle(Color.red, 1));

		for (Particle p : particles) {
			vb.addBuffered(new VisChain(LinAlg.xytToMatrix(p.xyt), vd));
		}

		// //////////////////////////////////////////////////
		// simulate lidar.
		// double laser_xy[] = new double[] {
		// xyt_truth[0] + rand.nextGaussian() * 0.05,
		// xyt_truth[1] + rand.nextGaussian() * 0.05 };
		if (laserxy != null) {
			System.arraycopy(laserxy, 0, oldlaserxy, 0, laserxy.length);
			laserxy = null;
		}

		vb.addBuffered(new VisData(oldlaserxy, new VisDataPointStyle(
				Color.black, 4)));

		// //////////////////////////////////////////////////
		// score particles.
		double totalweight = 0;
		for (Particle p : particles) {
			double dist2 = LinAlg.sq(p.xyt[0] - oldlaserxy[0])
					+ LinAlg.sq(p.xyt[1] - oldlaserxy[1]);

			p.weight = Math.exp(-dist2 / 0.05);
			totalweight += p.weight;
		}

		Particle bestParticle = null;

		// normalize weights
		for (Particle p : particles) {
			p.weight /= totalweight;
			if (bestParticle == null || p.weight > bestParticle.weight)
				bestParticle = p;
		}

		Particle fitParticle = new Particle();
		fitParticle.xyt = new double[3];

		for (Particle p : particles) {
			fitParticle.xyt[0] += p.weight * p.xyt[0];
			fitParticle.xyt[1] += p.weight * p.xyt[1];
			fitParticle.xyt[2] += p.weight
					* MathUtil.mod2pi(bestParticle.xyt[2], p.xyt[2]);
		}

		vb.addBuffered(new VisChain(LinAlg.xytToMatrix(fitParticle.xyt),
				new VisRobot(Color.yellow)));

		// render laser data on top
		vb.addBuffered(new VisData(oldlaserxy, new VisDataPointStyle(
				Color.black, 4)));

		// //////////////////////////////////////////////////
		// resample.
		ArrayList<Particle> newParticles = new ArrayList<Particle>();

		for (int i = 0; i < particles.size(); i++) {
			double tw = rand.nextFloat();
			Particle p = null;
			for (int j = 0; j < particles.size(); j++) {
				tw -= particles.get(j).weight;
				if (tw < 0) {
					p = particles.get(j);
					break;
				}
			}

			if (p == null)
				continue;

			Particle np = new Particle();
			np.xyt = new double[] { p.xyt[0] + rand.nextGaussian() * 0.05,
					p.xyt[1] + rand.nextGaussian() * 0.05,
					p.xyt[2] + rand.nextGaussian() * 0.05 };
			newParticles.add(np);
		}
		particles = newParticles;

		// /////////////////////////////////////////////////
		// clean up
		// xyt_truth = xyt_truth_new;
		vb.switchBuffer();

	}

}

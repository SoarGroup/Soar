package splintersoar.pf;

import java.util.*;
import java.awt.*;
import javax.swing.*;
import vis.*;
import jmat.*;

public class PFTest {
	static class Particle {
		double xyt[];
		double weight;
	}

	public static void main(String args[]) {
		PFTest pf = new PFTest();

		while (true) {
			pf.step();
			try {
				Thread.sleep(10);
			} catch (InterruptedException ex) {
			}
		}
	}

	JFrame jf;
	VisWorld vw = new VisWorld();
	VisCanvas vc = new VisCanvas(vw);

	double time;
	double xyt_truth[];

	Random rand = new Random();

	ArrayList<Particle> particles = new ArrayList<Particle>();

	public PFTest() {
		jf = new JFrame("PF Test");
		jf.setLayout(new BorderLayout());
		jf.add(vc, BorderLayout.CENTER);
		jf.setSize(600, 400);
		jf.setVisible(true);

		init();
	}

	void init() {
		for (int i = 0; i < 100; i++) {
			Particle p = new Particle();
			p.xyt = new double[] { (1 - 2 * rand.nextFloat()) * 5, (1 - 2 * rand.nextFloat()) * 5, 2 * rand.nextFloat() * Math.PI };
			particles.add(p);
		}
	}

	void step() {
		time += 0.01;
		update();
	}

	void update() {
		// //////////////////////////////////////////////////
		// Simulate motion of real robot.

		// drive in a big circle
		double theta = time;
		double radius = 5;

		double xyt_truth_new[] = new double[] { radius * Math.cos(theta), radius * Math.sin(theta), theta + Math.PI / 2 };
		if (xyt_truth == null)
			xyt_truth = LinAlg.copy(xyt_truth_new);

		VisWorld.Buffer vb = vw.getBuffer("truth");
		vb.addBuffered(new VisChain(LinAlg.xytToMatrix(xyt_truth_new), new VisRobot(Color.blue)));

		// //////////////////////////////////////////////////
		// Compute Odometry (perfect)

		// AT = B
		// T = inv(A)*B
		double odom_xyt_truth[];
		odom_xyt_truth = LinAlg.xytInvMul31(xyt_truth, xyt_truth_new);

		double odom_xyt_noisy[] = new double[3];
		odom_xyt_noisy[0] = odom_xyt_truth[0] + rand.nextGaussian() * 0.01;
		odom_xyt_noisy[1] = odom_xyt_truth[1] + rand.nextGaussian() * 0.01;
		odom_xyt_noisy[2] = odom_xyt_truth[2] + rand.nextGaussian() * 0.001;

		// //////////////////////////////////////////////////
		// propagate odometry
		for (Particle p : particles) {
			p.xyt = LinAlg.xytMultiply(p.xyt, odom_xyt_noisy);
		}

		VisData vd = new VisData(new double[2], new double[] { 0.2, 0 }, new VisDataLineStyle(Color.red, 1));

		for (Particle p : particles) {
			vb.addBuffered(new VisChain(LinAlg.xytToMatrix(p.xyt), vd));
		}

		// //////////////////////////////////////////////////
		// simulate lidar.
		double laser_xy[] = new double[] { xyt_truth[0] + rand.nextGaussian() * 0.05, xyt_truth[1] + rand.nextGaussian() * 0.05 };

		vb.addBuffered(new VisData(laser_xy, new VisDataPointStyle(Color.black, 4)));

		// //////////////////////////////////////////////////
		// score particles.
		double totalweight = 0;
		for (Particle p : particles) {
			double dist2 = LinAlg.sq(p.xyt[0] - laser_xy[0]) + LinAlg.sq(p.xyt[1] - laser_xy[1]);

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
			fitParticle.xyt[2] += p.weight * MathUtil.mod2pi(bestParticle.xyt[2], p.xyt[2]);
		}

		vb.addBuffered(new VisChain(LinAlg.xytToMatrix(fitParticle.xyt), new VisRobot(Color.yellow)));

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
			np.xyt = new double[] { p.xyt[0] + rand.nextGaussian() * 0.05, p.xyt[1] + rand.nextGaussian() * 0.05, p.xyt[2] + rand.nextGaussian() * 0.05 };
			newParticles.add(np);
		}
		particles = newParticles;

		// /////////////////////////////////////////////////
		// clean up
		xyt_truth = xyt_truth_new;
		vb.switchBuffer();

	}

}

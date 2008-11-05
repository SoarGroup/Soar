package splintersoar.pf;

import java.io.IOException;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.Logger;

import jmat.LinAlg;
import jmat.MathUtil;

import splintersoar.Configuration;
import splintersoar.LCMInfo;
import splintersoar.LogFactory;
import lcmtypes.particles_t;

import lcm.lcm.LCM;
import lcmtypes.pose_t;

/**
 * @author voigtjr
 * Experimental attempt to use a particle filter to help localization. Takes as input noisy odometry deltas and optional
 * noisy laser localization coordinates. Returns possible pose.
 */
public class ParticleFilter {
	static class Particle {
		double xyt[];
		double weight;
	}

	Random rand = new Random();

	ArrayList<Particle> particles = new ArrayList<Particle>();
	particles_t particleslcm;
	LCM lcmGG;
	
	Logger logger;
	Configuration cnf;

	public ParticleFilter(Configuration cnf) {
		this.cnf = cnf;
		
		logger = LogFactory.createSimpleLogger("ParticleFilter", cnf.loglevel);

		try {
			logger.info(String.format("Using %s for %s provider URL.", LCMInfo.GG_NETWORK, LCMInfo.PARTICLES_CHANNEL));
			lcmGG = new LCM(LCMInfo.GG_NETWORK);

		} catch (IOException e) {
			logger.severe("Error creating lcmGG.");
			e.printStackTrace();
			System.exit(1);
		}
		
		init();
	}

	void init() {
		final int size = cnf.pf.numParticles;
		logger.info(String.format("Initialzing %d particles", size));

		particleslcm = new particles_t();
		particleslcm.nparticles = size;
		particleslcm.particle = new float[size][];

		for (int i = 0; i < 100; i++) {
			Particle p = new Particle();
			if (cnf.pf.useRandomInitial) {
				p.xyt = new double[] { (1 - 2 * rand.nextFloat()) * 5, (1 - 2 * rand.nextFloat()) * 5, 2 * rand.nextFloat() * Math.PI };
			} else {
				p.xyt = new double[] { 0, 0, 0 };
			}
			particles.add(p);

			particleslcm.particle[i] = new float[] { (float)p.xyt[0], (float)p.xyt[1], (float)p.xyt[2] };
			
			if (logger.isLoggable(Level.FINEST)) {
				logger.finest(String.format("New particle xyt: %5.3f %5.3f %5.3f", p.xyt[0], p.xyt[1], p.xyt[2]));
			}
		}

		particleslcm.utime = System.nanoTime() / 1000;
		lcmGG.publish(LCMInfo.PARTICLES_CHANNEL, particleslcm);
	}

	public void propagate(double[] deltast) {
		assert deltast != null;

		// //////////////////////////////////////////////////
		// propagate odometry
		particleslcm = new particles_t();
		particleslcm.nparticles = particles.size();
		particleslcm.particle = new float[particles.size()][];
		int i = 0;
		for (Particle p : particles) {
			p.xyt[0] += deltast[0] * Math.cos(p.xyt[2]);
			p.xyt[1] += deltast[0] * Math.sin(p.xyt[2]);
			p.xyt[2] += deltast[1];
			p.xyt[2] = MathUtil.mod2pi(p.xyt[2]);
			
			particleslcm.particle[i] = new float[] { (float)p.xyt[0], (float)p.xyt[1], (float)p.xyt[2] };
			++i;
		}
		particleslcm.utime = System.nanoTime() / 1000;
		lcmGG.publish(LCMInfo.PARTICLES_CHANNEL, particleslcm);
	}
	
	public pose_t update(double[] laserxy) {
		assert laserxy != null;

		// //////////////////////////////////////////////////
		// use new lidar reading

		// //////////////////////////////////////////////////
		// score particles.
		double totalweight = 0;
		for (Particle p : particles) {
			double dist2 = LinAlg.sq(p.xyt[0] - laserxy[0]) + LinAlg.sq(p.xyt[1] - laserxy[1]);

			p.weight = Math.exp(-dist2 / cnf.pf.xySigma);
			totalweight += p.weight;
		}

		Particle bestParticle = null;

		// normalize weights
		for (Particle p : particles) {
			p.weight /= totalweight;
			if (bestParticle == null || p.weight > bestParticle.weight)
				bestParticle = p;
		}
		
		if (logger.isLoggable(Level.FINE)) {
			logger.fine(String.format("bestParticle %5.3f %5.3f %5.3f %5.3f", bestParticle.xyt[0], bestParticle.xyt[1], bestParticle.xyt[2], bestParticle.weight));
		}

		Particle fitParticle = new Particle();
		fitParticle.xyt = new double[3];

		for (Particle p : particles) {
			fitParticle.xyt[0] += p.weight * p.xyt[0];
			fitParticle.xyt[1] += p.weight * p.xyt[1];
			fitParticle.xyt[2] += p.weight * MathUtil.mod2pi(bestParticle.xyt[2], p.xyt[2]);
		}

		if (logger.isLoggable(Level.FINE)) {
			logger.fine(String.format("fitParticle %5.3f %5.3f %5.3f", fitParticle.xyt[0], fitParticle.xyt[1], fitParticle.xyt[2]));
		}

		// //////////////////////////////////////////////////
		// resample.
		ArrayList<Particle> newParticles = new ArrayList<Particle>();

		particleslcm = new particles_t();
		particleslcm.nparticles = particles.size();
		particleslcm.particle = new float[particles.size()][];

		for (int i = 0; i < particles.size() - cnf.pf.standingPopulation; i++) {
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
			np.xyt = new double[] { p.xyt[0] + rand.nextGaussian() * cnf.pf.xySigma, p.xyt[1] + rand.nextGaussian() * cnf.pf.xySigma, p.xyt[2] + rand.nextGaussian() * cnf.pf.thetaSigma };
			newParticles.add(np);

			particleslcm.particle[i] = new float[] { (float)np.xyt[0], (float)np.xyt[1], (float)np.xyt[2] };
			
			if (logger.isLoggable(Level.FINEST)) {
				logger.finest(String.format("Resampled particle xyt: %5.3f %5.3f %5.3f", np.xyt[0], np.xyt[1], np.xyt[2]));
			}
		}

		// standing population
		for (int i = cnf.pf.numParticles - cnf.pf.standingPopulation; i < cnf.pf.numParticles; i++) {
			Particle p = new Particle();
			p.xyt = new double[] { (1 - 2 * rand.nextFloat()) * 5, (1 - 2 * rand.nextFloat()) * 5, 2 * rand.nextFloat() * Math.PI };
			newParticles.add(p);

			particleslcm.particle[i] = new float[] { (float)p.xyt[0], (float)p.xyt[1], (float)p.xyt[2] };
			
			if (logger.isLoggable(Level.FINEST)) {
				logger.finest(String.format("Standing particle xyt: %5.3f %5.3f %5.3f", p.xyt[0], p.xyt[1], p.xyt[2]));
			}
		}

		particles = newParticles;

		particleslcm.utime = System.nanoTime() / 1000;
		lcmGG.publish(LCMInfo.PARTICLES_CHANNEL, particleslcm);

		fitParticle.xyt[2] = MathUtil.mod2pi(fitParticle.xyt[2]);

		pose_t pose = new pose_t();
		pose.pos = new double[] { fitParticle.xyt[0], fitParticle.xyt[1], 0 };
		pose.orientation = LinAlg.rollPitchYawToQuat(new double[] { 0, 0, fitParticle.xyt[2] });

		return pose;
	}

}

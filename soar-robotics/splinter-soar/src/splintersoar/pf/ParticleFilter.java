package splintersoar.pf;

import java.util.*;

import splintersoar.lcmtypes.particles_t;

import erp.geom.Geometry;
import erp.math.*;
import jmat.*;
import lcm.lcm.LCM;
import lcmtypes.pose_t;

public class ParticleFilter {
	static class Particle {
		double xyt[];
		double weight;
	}
	
	Random rand = new Random();

	ArrayList<Particle> particles = new ArrayList<Particle>();
	particles_t particleslcm;
	LCM lcm;
	
	public ParticleFilter() {
		lcm = LCM.getSingleton();
		
		init();
	}

	void init() {
		final int size = 100;
		
		particleslcm = new particles_t();
		particleslcm.nparticles = size;
		particleslcm.particle = new double [size][];
		
		for (int i = 0; i < 100; i++) {
			Particle p = new Particle();
			p.xyt = new double[] { (1 - 2 * rand.nextFloat()) * 5,
					(1 - 2 * rand.nextFloat()) * 5,
					2 * rand.nextFloat() * Math.PI };
			particles.add(p);
			
			particleslcm.particle[i] = new double [] { p.xyt[0], p.xyt[1], p.xyt[2] };
		}
		
		particleslcm.utime = System.nanoTime() / 1000;
		lcm.publish( "PARTICLES", particleslcm );
	}

	double [] oldlaserxy = { 0, 0 };
	
	public pose_t update( double [] deltaxyt, double [] laserxy ) {

		assert deltaxyt != null;
		
		// //////////////////////////////////////////////////
		// propagate odometry
		for (Particle p : particles) {
			p.xyt = LinAlg.xytMultiply( p.xyt, deltaxyt );
		}
		
		// //////////////////////////////////////////////////
		// use new lidar reading
		if ( laserxy != null )
		{
			System.arraycopy( laserxy, 0, oldlaserxy, 0, laserxy.length );
		}
		
		//System.out.println( oldlaserxy[0] + " " + oldlaserxy[1]);	 

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

		// //////////////////////////////////////////////////
		// resample.
		ArrayList<Particle> newParticles = new ArrayList<Particle>();

		particleslcm = new particles_t();
		particleslcm.nparticles = particles.size();
		particleslcm.particle = new double [particles.size()][];

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
					p.xyt[2] + rand.nextGaussian() * 0.01 };
			newParticles.add(np);
			
			particleslcm.particle[i] = new double [] { np.xyt[0], np.xyt[1], np.xyt[2] };
		}
		particles = newParticles;

		particleslcm.utime = System.nanoTime() / 1000;
		lcm.publish( "PARTICLES", particleslcm );
		
		fitParticle.xyt[2] = MathUtil.mod2pi( fitParticle.xyt[2] );
		
		pose_t pose = new pose_t();
		pose.pos = new double[] { fitParticle.xyt[0], fitParticle.xyt[1], 0 };
		pose.orientation = Geometry.rollPitchYawToQuat( new double [] { 0, 0, fitParticle.xyt[2] } );
		
		return pose;
	}

}

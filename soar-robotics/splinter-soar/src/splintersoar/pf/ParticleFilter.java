package splintersoar.pf;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Random;

import erp.geom.Geometry;
import erp.math.MathUtil;

public class ParticleFilter {
	
	Random random = new Random();

	static final int population = 100;
	static final double xyError = 0.05; // measurement error ( 5 cm ) ??
	static final double thetaError = 0.05; // ?? possibly should be different
	
	ArrayList<Particle> particles;
	
	void createStartingPopulation( double [] initialPos, double initalTheta )
	{
		System.out.println( "createStartingPopulation" );

		particles = new ArrayList< Particle >();
		
		while ( particles.size() < population )
		{
			Particle particle = new Particle();
			
			particle.pos[0] = initialPos[0] + random.nextGaussian() * xyError;
			particle.pos[1] = initialPos[1] + random.nextGaussian() * xyError;
			particle.theta = initalTheta + random.nextGaussian() * thetaError;
			
			particles.add( particle );
			
			System.out.println( particle );
		}
	}
	
	void laserReading( double [] laserPos )
	{
		System.out.println( "laserReading" );

		assert laserPos != null;
		
		Iterator< Particle > iter = particles.iterator();
		
		double total = 0;
		
		while ( iter.hasNext() )
		{
			Particle particle = iter.next();
			
			double squaredDistance = Geometry.squaredDistance( laserPos, particle.pos );
			particle.probability = Math.exp( -squaredDistance / Math.pow( xyError, 2 ) );
			total += particle.probability;

			System.out.println( particle );
		}		
		
		iter = particles.iterator();
		double cumulative = 0;
		while ( iter.hasNext() )
		{
			Particle particle = iter.next();
			
			cumulative += particle.probability / total;
			particle.cumulative = cumulative;
		}
	}
	
	void odometryReading( double deltaS, double deltaTheta )
	{
		Iterator< Particle > iter = particles.iterator();
		
		while ( iter.hasNext() )
		{
			Particle particle = iter.next();
//
//			particle.theta += deltaTheta + noise;
//			
//			particle.pos[0] += ( Math.cos( particle.theta ) * deltaS ) + noise;
//			particle.pos[1] += ( Math.sin( particle.theta ) * deltaS ) + noise;
//			
//			particle.pos[0] += deltaPos[0];
//			particle.pos[1] += deltaPos[1];
//			
//			particle.theta = MathUtil.mod2pi( particle.theta );
		}
	} 
	
	public static void main( String [] args )
	{
	}
}

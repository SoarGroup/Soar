package edu.umich.tanksoar;
/* File: MissileBucket.java
 * Date: Sep 10, 2004
 */


import java.util.Random;
/**
 * This class represents a set of missiles lying on the ground in the TankSoar simulation.
 * It keeps track of the number of missiles that are stored and the <code>TankSoarJSquare</code>
 * in which they are stored.
 * @author John Duchi
 */
public class MissileBucket {

	/** The random number generator for MissileBuckets; may be seeded through the
	 * <code>seedRandomNumber()</code> method. For example, could be used to
	 * assure deterministic missile placement. */
	private static Random missileRandom = null;
	
	/** The number of missiles that are in one set of missiles on the ground. */
	public final static int NumMissiles = 7;
	
	/** The <code>TSEmpty</code> square that contains these missiles. */
	private TSEmpty mySquare = null;
	/** The <code>TankSoarJControl</code> running the simulation. */
	private TankSoarJControl myTC;
	
	/**
	 * Constructs a new <code>MissileBucket</code> in the given <code>TSEmpty</code>, adding
	 * the <code>MissileBucket</code> to the square's occupants. Client does <b>not</b>
	 * need to add this to any occupant lists.
	 * @param containerSquare The <code>TSEmpty</code> that contains these missiles.
	 * @param tc The <code>TankSoarJControl</code> running the simulation of which this
	 * <code>MissileBucket</code> is becoming a part.
	 */
	public MissileBucket(TSEmpty containerSquare, TankSoarJControl tc)
  {
		mySquare = containerSquare;
		myTC = tc;
		mySquare.addOccupant(this);
	}
	
	/**
	 * Method called on this <code>MissileBucket</code> to indicate that the specified
	 * <code>Tank</code> has picked up the missiles. Will remove the <code>MissileBucket</code>
	 * from its square in the simulation by calling <code>TankSoarJControl.pickedUpMissiles(MissileBucket)</code>.
	 * @param pickingUp The <code>Tank</code> that is picking up the missiles.
	 * @see TankSoarJControl#pickedUpMissiles(MissileBucket)
	 */
	public void pickUpMissiles(Tank pickingUp){
		pickingUp.gotMissiles(NumMissiles);
		mySquare.removeOccupant(this);
		myTC.pickedUpMissiles(this);
	}
	
	/**
	 * Gives to the caller the <code>TSEmpty</code> in which this instance of
	 * <code>MissileBucket</code> exists.
	 * @return The <code>TSEmpty</code> that has as an occupant this <code>MissileBucket</code>.
	 */
	public TSEmpty getContainingSquare(){
		return(mySquare);
	}
	
	/**
	 * Seeds the random number generator so that deterministically random generation
	 * can be used.
	 */
	public static void seedRandomNumber(){
		if(missileRandom == null){
			missileRandom = new Random(43);
		} else {
			missileRandom.setSeed(43);
		}
	}
	
	/**
	 * Seeds the random number generator so that there is no deterministically
	 * random generation, using <code>Random.setSeed(long)</code> with 
	 * <code>System.currentTimeMillis()</code>.
	 */
	public static void unseedRandomNumber(){
		if(missileRandom == null){
			missileRandom = new Random();
		} else {
			missileRandom.setSeed(System.currentTimeMillis());
		}
	}
	
	/**
	 * Returns the next pseudorandom, uniformly distributed double from 0.0 to 1.0 in
	 * <code>MissileBucket</code>'s random number generator. If the random number
	 * generated has not been seeded, uses a seed based on the current time.
	 * @return A random double greater than or equal to 0.0 but strictly less than 1.0.
	 * @see java.util.Random#nextDouble()
	 */
	public static double getNextRandomDouble(){
		if(missileRandom == null){
			missileRandom = new Random();
		}
		return(missileRandom.nextDouble());
	}
	
	/**
	 * <p> To be specific, returns <code>true</code> if <code>(o instanceof MissileBucket)</code>,
	 * since all <code>MissileBucket</code>s contain the same number of missiles. The containing
	 * <code>TSEmpty</code> is not significant, though if a client cares, he or she can check
	 * that the containing squares of two <code>MissileBucket</code>s are equal.
	 */
	public boolean equals(Object o){
		return(o != null && o instanceof MissileBucket);
	}
}

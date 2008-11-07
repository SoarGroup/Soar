package splintersoar.laserloc;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.logging.Level;
import java.util.logging.Logger;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;

import splintersoar.Configuration;
import splintersoar.LCMInfo;
import splintersoar.LogFactory;
import lcmtypes.xy_t;

/**
 * @author voigtjr
 * Laser localizer class. Takes as input SICK data and generates x,y position of beacon.
 */
public class LaserLoc extends Thread implements LCMSubscriber {

	private Configuration cnf;

	/**
	 * The data that comes in from the sick
	 */
	laser_t laserData;

	/**
	 * Keep track of how many packets are being ignored, for debugging.
	 */
	int droppedLocPackets = 0;
	/**
	 * Used when deciding when to print status message about packet loss.
	 */
	long lastStatusUpdate = System.nanoTime();

	/**
	 * H1 network for the sick laser data
	 */
	LCM lcmH1;
	/**
	 * GG network for the result of this class, the coordinates.
	 */
	LCM lcmGG;

	/**
	 * Used to print out a warning when no sick data is being received.
	 */
	boolean inactive = true;
	/**
	 * State for inactivity warning message
	 */
	long nanolastactivity = System.nanoTime();
	/**
	 * Timeout before inactivity warning messages are generated.
	 */
	long currentTimeout;

	private Logger logger;

	public LaserLoc(Configuration cnf) {
		this.cnf = cnf;

		logger = LogFactory.createSimpleLogger("LaserLoc", cnf.loglevel);
		
		try {
			logger.info(String.format("Using %s for %s provider URL.", LCMInfo.H1_NETWORK, LCMInfo.LASER_LOC_CHANNEL));
			lcmH1 = new LCM(LCMInfo.H1_NETWORK);

		} catch (IOException e) {
			logger.severe("Error creating lcmH1.");
			e.printStackTrace();
			System.exit(1);
		}
		lcmH1.subscribe(LCMInfo.LASER_LOC_CHANNEL, this);

		try {
			logger.info(String.format("Using %s for %s provider URL.", LCMInfo.GG_NETWORK, LCMInfo.COORDS_CHANNEL));
			lcmGG = new LCM(LCMInfo.GG_NETWORK);

		} catch (IOException e) {
			logger.severe("Error creating lcmGG.");
			e.printStackTrace();
			System.exit(1);
		}
		
		if (cnf.lloc.maxRanges == null) {
			logger.warning("No map file found, using infinite maximums");
			cnf.lloc.maxRanges = new double[180];
			Arrays.fill(cnf.lloc.maxRanges, Double.MAX_VALUE);
		}

		currentTimeout = cnf.lloc.activityTimeoutNanos;
	}

	/**
	 * Main update function, called in a loop.
	 */
	private void updatePose() {
		long nanotime = System.nanoTime();

		if (nanotime - nanolastactivity > currentTimeout) {
			inactive = true;
			logger.warning(String.format("no activity in last %1.0f seconds", (currentTimeout / 1000000000.0)));
			currentTimeout += cnf.lloc.activityTimeoutNanos;
		}

		// occasionally print out status update
		long nanoelapsed = nanotime - lastStatusUpdate;
		if (nanoelapsed > cnf.lloc.updatePeriodNanos) {
			if (droppedLocPackets > 0) {
				double dropRate = (double) droppedLocPackets / (nanoelapsed / 1000000000.0);
				logger.fine(String.format("LaserLoc: dropping %5.1f packets/sec", dropRate));
			}

			droppedLocPackets = 0;
			lastStatusUpdate = nanotime;
		}

		// If no laser data, sleep for a bit.
		if (laserData == null) {
			try {
				Thread.sleep(50);
			} catch (InterruptedException ignored) {
			}
			return;
		}

		nanolastactivity = nanotime;
		currentTimeout = cnf.lloc.activityTimeoutNanos;

		if (inactive) {
			logger.info("receiving data");
			inactive = false;
		}

		laser_t ld = laserData.copy();
		laserData = null;
		if (lastutime == ld.utime) {
			if (logger.isLoggable(Level.FINE))
				logger.fine("Skipping message, time hasn't changed");

			try {
				Thread.sleep(50);
			} catch (InterruptedException ignored) {
			}
			return;
		}
		lastutime = ld.utime;

		xy_t estimatedCoords = getRobotXY(ld);
		if (estimatedCoords == null)
			return;

		if (logger.isLoggable(Level.FINE))
			logger.fine(String.format("publishing %10.3f %10.3f", estimatedCoords.xy[0], estimatedCoords.xy[1]));

		lcmGG.publish(LCMInfo.COORDS_CHANNEL, estimatedCoords);
	}
	
	long lastutime = 0;

	/**
	 * @param ld Data from front sick.
	 * @return Estimate of splinter location.
	 * 
	 * Takes the laser data, looks for the closest thing within the 
	 * bounds of the "room," returns its location.
	 */
	private xy_t getRobotXY(laser_t ld) {
		int nranges = Math.min( cnf.lloc.maxRanges.length, ld.nranges);
		if (cnf.lloc.maxRanges.length != ld.nranges)
		{
			logger.fine(String.format("maxRanges array not equal in size to nranges, %d %d", cnf.lloc.maxRanges.length, ld.nranges));
		}
		
		double smallestRange = Double.MAX_VALUE;
		int smallestRangeIndex = -1;
		for (int index = 0; index < nranges; ++index) {
			if (ld.ranges[index] < cnf.lloc.maxRanges[index]) {
				if (ld.ranges[index] < smallestRange) {
					smallestRange = ld.ranges[index];
					smallestRangeIndex = index;
				}
			}
		}

		if (logger.isLoggable(Level.FINEST)) {
			logger.finest(String.format("picked index %d range %4.2f under maxRange %4.2f", smallestRangeIndex, smallestRange, cnf.lloc.maxRanges[smallestRangeIndex]));
		}
		if (smallestRangeIndex == -1)
		{
			logger.warning("did not find smallest range (is there nothing in view?");
			return null;
		}
		
		double laserAngle = cnf.lloc.laserxyt[2] + ld.rad0 + ld.radstep * smallestRangeIndex;

		double laserDist = smallestRange + cnf.lloc.tubeRadius;

		xy_t newCoords = new xy_t();
		newCoords.utime = ld.utime;
		newCoords.xy[0] = cnf.lloc.laserxyt[0] + laserDist * Math.cos(laserAngle);
		newCoords.xy[1] = cnf.lloc.laserxyt[1] + laserDist * Math.sin(laserAngle);
		
		return newCoords;
	}

	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(LCMInfo.LASER_LOC_CHANNEL)) {
			if (laserData != null) {
				droppedLocPackets += 1;
				return;
			}

			try {
				laserData = new laser_t(ins);
			} catch (IOException ex) {
				System.err.println("Error decoding laser message: " + ex);
			}
		}

	}

	@Override
	public void run() {
		while (true) {
			updatePose();
		}
	}
}

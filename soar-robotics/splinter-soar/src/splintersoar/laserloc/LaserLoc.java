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

	// Assumptions:
	// robot_z is constant
	// robot starts at a known angle
	// this level only reports laser data (next layer up decided whether to use
	// odometry or not)
	// distances in meters

	// regular state
	laser_t laserData;

	int droppedLocPackets = 0;
	long lastStatusUpdate = System.nanoTime();

	LCM lcmH1;
	LCM lcmGG;

	boolean inactive = true;
	long nanolastactivity = System.nanoTime();
	long currentTimeout;

	private Logger logger;
	RoomMapper mapper;

	public LaserLoc(Configuration cnf) {
		this.cnf = cnf;

		logger = LogFactory.createSimpleLogger("LaserLoc", Level.INFO);
		
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
				logger.warning(String.format("LaserLoc: dropping %5.1f packets/sec", dropRate));
			}

			droppedLocPackets = 0;
			lastStatusUpdate = nanotime;
		}

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

	private xy_t getRobotXY(laser_t ld) {
		int nranges = Math.min( cnf.lloc.maxRanges.length, ld.nranges);
		if (cnf.lloc.maxRanges.length != ld.nranges)
		{
			logger.fine(String.format("maxRanges array not equal in size to nranges, %d %d", cnf.lloc.maxRanges.length, ld.nranges));
		}
		
		double smallestRange = Double.MAX_VALUE;
		int smallestRangeIndex = -1;
		// throwing out high/low ranges
		for (int index = 1; index < nranges-2; ++index) {
			if (ld.ranges[index] < cnf.lloc.maxRanges[index]) {
				if (ld.ranges[index] < smallestRange) {
					smallestRange = ld.ranges[index];
					smallestRangeIndex = index;
				}
			}
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

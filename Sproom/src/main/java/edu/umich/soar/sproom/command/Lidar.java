package edu.umich.soar.sproom.command;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import lcm.lcm.LCM;
import lcm.lcm.LCMDataInputStream;
import lcm.lcm.LCMSubscriber;
import april.lcmtypes.laser_t;
import edu.umich.soar.sproom.HzChecker;
import edu.umich.soar.sproom.SharedNames;

/**
 * Merges messages from a SICK and/or simulated laser sensor in to a single,
 * "lower resolution" reading of usually 5 ranges. Merges by taking the minimum
 * reading from the set of sensors.
 * 
 * Will use an old reading up to a configurable duration so that the sensor 
 * doesn't "jitter" when old and new readings aren't available at the same time.
 *
 * @author voigtjr@gmail.com
 */
public class Lidar {
	private static final Log logger = LogFactory.getLog(Lidar.class);
	
	private final long cacheTime;
	private laser_t simLaser;
	private laser_t sickLaser;

	private final laser_t laserLowRes = new laser_t();
	private final CommandConfig c = CommandConfig.CONFIG;
	private int chunkRanges = 0;
	private final LCM lcm = LCM.getSingleton();
	private final HzChecker simChecker = HzChecker.newInstance("Lidar/sim");
	private final HzChecker sickChecker = HzChecker.newInstance("Lidar/sick");
	
	private LCMSubscriber subscriber = new LCMSubscriber() {
		@Override
		public void messageReceived(LCM lcm, String channel, LCMDataInputStream ins) {
			if (channel.equals(SharedNames.SIM_LASER_CHANNEL)) {
				try {
					simLaser = new laser_t(ins);
					simChecker.tick();
				} catch (IOException e) {
					logger.error("Error decoding SIM_LASER_CHANNEL message: " + e.getMessage());
				}
			} else if (channel.equals(SharedNames.SICK_LASER_CHANNEL)) {
				try {
					sickLaser = new laser_t(ins);
					sickChecker.tick();
				} catch (IOException e) {
					logger.error("Error decoding SICK_LASER_CHANNEL message: " + e.getMessage());
				}
			} else {
				logger.error("Unknown message channel: " + channel);
			}
		}
	};
	
	public Lidar(long cacheTime) {
		this.cacheTime = cacheTime;
		laserLowRes.utime = 0;
		laserLowRes.nranges = 0;
		laserLowRes.ranges = new float[0];
		laserLowRes.nintensities = 0;
		laserLowRes.intensities = new float[0];
		laserLowRes.rad0 = 0;
		laserLowRes.radstep = 0;
		
		lcm.subscribe(SharedNames.SIM_LASER_CHANNEL, subscriber);
		lcm.subscribe(SharedNames.SICK_LASER_CHANNEL, subscriber);
	}
	
	private static long simLast = -1;	// value of last reading's utime
	private static long sickLast = -1;	
	private static long simSeen = -1;	// System.nanotime() when we last saw a fresh reading
	private static long sickSeen = -1;  
	
	private class MergedLaser {
		laser_t sim = simLaser;
		laser_t sick = sickLaser; 
		laser_t auth;

		MergedLaser() {
			if (!isValid()) {
				return;
			}
			
			// TODO make this not hard-wired, careful with bounds-checking getRange()
			// hard-wired at least 180 ranges
			
			// if we only have one, it is authoritative
			if (sim == null) {
				if (sick.nranges < 180) {
					sick = null;
				} else {
					auth = sick;
				}
			} else if (sick == null) {
				if (sim.nranges < 180) {
					sim = null;
				} else {
					auth = sim;
				}
			}
		}
		
		boolean isValid() {
			return sick != null || sim != null;
		}
		
		int getNRanges() {
			return auth == null ? sim.nranges : auth.nranges;
		}
		
		float getRad0() {
			return auth == null ? sim.rad0 : auth.rad0;
		}

		float getRadStep() {
			return auth == null ? sim.radstep : auth.radstep;
		}

		float getRange(int index) {
			if (index >= 180) {
				return Integer.MAX_VALUE;
			}
			
			if (auth != null) {
				return auth.ranges[index];
			}
			// want min here, the closest range reported
			return Math.min(sim.ranges[index], sick.ranges[index]);
		}
	}
	
	public laser_t getLaserLowRes() {
		long now = System.nanoTime();
		//StringBuilder trace = new StringBuilder(Long.toString(now));
		
		// throw out readings if we haven't seen a new one in cacheTime seconds
		if (simLaser != null) {
			if (simLaser.utime > simLast) {
				//trace.append(" simgood");
				simLast = simLaser.utime;
				simSeen = now;
			} else if (now - simSeen > cacheTime) {
				//trace.append(" simold");
				simLaser = null;
				simLast = -1;
			}
		}
		if (sickLaser != null) {
			if (sickLaser.utime > sickLast) {
				//trace.append(" sickgood");
				sickLast = sickLaser.utime;
				sickSeen = now;
			} else if (now - sickSeen > cacheTime) {
				//trace.append(" sickold");
				sickLaser = null;
				sickLast = -1;
			}
		}
		
		MergedLaser ml = new MergedLaser();
		if (ml.isValid()) {
			//trace.append(" mlvalid ");
			laserLowRes.utime = now;
			if (laserLowRes.nranges != c.getRangeCount()) {
				laserLowRes.nranges = c.getRangeCount();
				laserLowRes.ranges = new float[c.getRangeCount()];
				
				chunkRanges = ml.getNRanges() / laserLowRes.nranges;
				
				laserLowRes.rad0 = ml.getRad0() + ml.getRadStep() * (chunkRanges / 2.0f);
				laserLowRes.radstep = ml.getNRanges() * ml.getRadStep() / laserLowRes.nranges;
				
				logger.debug(String.format("%d nranges (%d per chunk), rad0 %1.3f, radstep %1.3f", laserLowRes.nranges, chunkRanges, laserLowRes.rad0, laserLowRes.radstep));
			}
			
			for (int slice = 0, index = 0; slice < laserLowRes.nranges; ++slice) {
				
				float distance = Float.MAX_VALUE;

				for (; index < (chunkRanges * slice) + chunkRanges; ++index) {
					distance = Math.min(ml.getRange(index), distance);
				}

				if (logger.isTraceEnabled()) {
					logger.trace(String.format("%d: %1.3f", slice, distance));
				}
				
				laserLowRes.ranges[slice] = distance;
			}
		}
		
		lcm.publish(SharedNames.LASER_LOWRES_CHANNEL, laserLowRes);
		//System.err.println(trace);
		return laserLowRes.copy();
	}
}

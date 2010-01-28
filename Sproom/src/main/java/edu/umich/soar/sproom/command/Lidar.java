package edu.umich.soar.sproom.command;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import lcm.lcm.LCM;
import lcm.lcm.LCMDataInputStream;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;
import edu.umich.soar.sproom.SharedNames;

public class Lidar {
	private static final Log logger = LogFactory.getLog(Lidar.class);

	private laser_t laser;
	private final laser_t laserLowRes = new laser_t();
	private final CommandConfig c = CommandConfig.CONFIG;
	private int chunkRanges = 0;
	private final LCM lcm = LCM.getSingleton();
	
	public Lidar() {
		laserLowRes.utime = 0;
		laserLowRes.nranges = 0;
		laserLowRes.ranges = new float[0];
		laserLowRes.nintensities = 0;
		laserLowRes.intensities = new float[0];
		laserLowRes.rad0 = 0;
		laserLowRes.radstep = 0;
		
		lcm.subscribe(SharedNames.LASER_CHANNEL, new LCMSubscriber() {
			@Override
			public void messageReceived(LCM lcm, String channel, LCMDataInputStream ins) {
				if (channel.equals(SharedNames.LASER_CHANNEL)) {
					try {
						laser = new laser_t(ins);
					} catch (IOException e) {
						logger.error("Error decoding laser_t message: " + e.getMessage());
					}
				}
			}
		});
	}
	
	public laser_t getLaserLowRes() {
		laser_t laserReading = laser; // grab reference
		
		if (laserReading == null) {
			return null;
		}
		
		if (laserReading.utime != laserLowRes.utime) {
			laserLowRes.utime = laserReading.utime;
			if (laserLowRes.nranges != c.getRangeCount()) {
				laserLowRes.nranges = c.getRangeCount();
				laserLowRes.ranges = new float[c.getRangeCount()];
				
				// FIXME this will be a bit off if the integer division isn't a whole number
				chunkRanges = laserReading.nranges / laserLowRes.nranges;
				
				laserLowRes.rad0 = laserReading.rad0 + laserReading.radstep * (chunkRanges / 2.0f);
				laserLowRes.radstep = laserReading.nranges * laserReading.radstep / laserLowRes.nranges;
				
				logger.debug(String.format("%d nranges (%d per chunk), rad0 %1.3f, radstep %1.3f", laserLowRes.nranges, chunkRanges, laserLowRes.rad0, laserLowRes.radstep));
			}
			
			for (int slice = 0, index = 0; slice < laserLowRes.nranges; ++slice) {
				
				float distance = Float.MAX_VALUE;

				for (; index < (chunkRanges * slice) + chunkRanges; ++index) {
					distance = Math.min(laser.ranges[index], distance);
				}

				if (logger.isTraceEnabled()) {
					logger.trace(String.format("%d: %1.3f", slice, distance));
				}
				
				laserLowRes.ranges[slice] = distance;
			}
		}
		
		lcm.publish(SharedNames.LASER_LOWRES_CHANNEL, laserLowRes);
		return laserLowRes.copy();
	}
}

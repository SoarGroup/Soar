package edu.umich.soar.sps.control;

import java.io.IOException;

import org.apache.log4j.Logger;

import edu.umich.soar.sps.SharedNames;

import lcm.lcm.LCM;
import lcm.lcm.LCMDataInputStream;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;
import sml.Agent;
import sml.Identifier;

final class RangerIL implements LCMSubscriber {
	private static final Logger logger = Logger.getLogger(RangerIL.class);

	private long utimeLast = 0;
	private final RangeIL[] slices;
	private laser_t laser;
	
	RangerIL(Agent agent, Identifier ranges, int count) {
		this.slices = new RangeIL[count];
		for (int index = 0; index < count; ++index) {
			slices[index] = new RangeIL(agent, ranges, index - (count / 2));
		}
		
		LCM.getSingleton().subscribe(SharedNames.LASER_CHANNEL, this);
	}

	void update(boolean useFloatYawWmes) {
		if (laser == null) {
			return;
		}

		if (laser.utime == utimeLast) {
			return;
		}
		utimeLast = laser.utime;

		updateSlices(laser, useFloatYawWmes);
	}

	void updateSlices(laser_t laser, boolean useFloatYawWmes) {
		// FIXME verify this is general, I think sliceChunk must have no remainder
		int sliceChunk = laser.nranges / slices.length; // a round number with 180/5 (36)

		for (int slice = 0, index = 0; slice < slices.length; ++slice) {
			
			double start = laser.rad0 + index * laser.radstep;
			double distance = Double.MAX_VALUE;

			for (; index < (sliceChunk * slice) + sliceChunk; ++index) {
				if (laser.ranges[index] < distance) {
					distance = laser.ranges[index];
				}
			}

			double end = laser.rad0 + (index - 1) * laser.radstep;
			
			if (logger.isTraceEnabled()) {
				System.out.println(String.format("%d: %1.3f %1.3f %1.2f", slice, start, end, distance));
			}
			
			slices[slice].update(start, end, distance, useFloatYawWmes);
		}
	}
	
	public void messageReceived(LCM lcm, String channel, LCMDataInputStream ins) {
		if (channel.equals(SharedNames.LASER_CHANNEL)) {
			try {
				laser = new laser_t(ins);
			} catch (IOException e) {
				logger.error("Error decoding laser_t message: " + e.getMessage());
			}
		}
	}
}

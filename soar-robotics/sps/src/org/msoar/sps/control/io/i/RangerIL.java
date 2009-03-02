package org.msoar.sps.control.io.i;

import lcmtypes.laser_t;
import sml.Agent;
import sml.Identifier;

class RangerIL {
	private long utimeLast = 0;
	private RangeIL[] slices;

	RangerIL(Agent agent, Identifier ranges, int count) {
		this.slices = new RangeIL[count];
		for (int index = 0; index < count; ++index) {
			slices[index] = new RangeIL(agent, ranges, index - (count / 2));
		}
	}

	void update(laser_t laser, boolean useFloatYawWmes) {
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
			
			slices[slice].update(start, end, distance, useFloatYawWmes);
		}
	}

}

package splintersoar.ranger;

import lcmtypes.laser_t;

/**
 * @author voigtjr
 * Coarse ranger data for Soar to use. Essentially divides up ranger data and uses minimums in each sector.
 */
public class RangerState {
	public class RangerData {
		public double start = 0;
		public double end = 0;
		public double distance = 0;
	}

	public long utime = 0;
	public RangerData[] ranger = { new RangerData(), new RangerData(), new RangerData(), new RangerData(), new RangerData() };

	public RangerState(laser_t laserData) {
		utime = laserData.utime;

		// FIXME verify this is general, I think sliceChunk must have no
		// remainder
		assert ranger.length == 5; // for now assert that it is 5

		int sliceChunk = laserData.nranges / ranger.length; // a round number
		// with 180/5 (36)

		for (int slice = 0, index = 0; slice < ranger.length; ++slice) {
			ranger[slice].start = laserData.rad0 + index * laserData.radstep;
			ranger[slice].distance = Double.MAX_VALUE;

			for (; index < (sliceChunk * slice) + sliceChunk; ++index) {
				if (laserData.ranges[index] < ranger[slice].distance) {
					ranger[slice].distance = laserData.ranges[index];
				}
			}

			ranger[slice].end = laserData.rad0 + (index - 1) * laserData.radstep;
		}
	}
}

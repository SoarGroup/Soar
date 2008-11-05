package splintersoar.ranger;

import lcmtypes.laser_t;

/**
 * @author voigtjr
 * Coarse ranger data for Soar to use. Essentially divides up ranger data and uses minimums in each sector.
 * 
 * TODO: This entire class could really (and should really) just be a laser_t.
 */
public class RangerState {
	public class RangerData {
		public float start = 0;
		public float end = 0;
		public float distance = 0;
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
			ranger[slice].distance = Float.MAX_VALUE;

			for (; index < (sliceChunk * slice) + sliceChunk; ++index) {
				if (laserData.ranges[index] < ranger[slice].distance) {
					ranger[slice].distance = laserData.ranges[index];
				}
			}

			ranger[slice].end = laserData.rad0 + (index - 1) * laserData.radstep;
		}
	}
	
	public laser_t toLaserT() {
		laser_t lcmData = new laser_t();
		lcmData.utime = utime;
		lcmData.nranges = ranger.length;
		lcmData.ranges = new float[ranger.length];
		for(int i = 0; i < ranger.length; ++i) {
			lcmData.ranges[i] = ranger[i].distance;
		}
		
		lcmData.rad0 = (ranger[0].start - ranger[0].end) / 2;
		lcmData.radstep = (ranger[ranger.length - 1].end - ranger[0].start) / ranger.length;
		return lcmData;
	}
}

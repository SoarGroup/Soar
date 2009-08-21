package edu.umich.soar.gridmap2d.map;

import java.util.Arrays;

import lcmtypes.pose_t;
import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.world.RoomWorld;

public class RoomBarrier {
	public int id = -1;
	public boolean gateway = false;

	public int[] left;
	public int[] right;

	public Direction direction;

	private pose_t centerpoint;

	public pose_t centerpoint() {
		// IMPORTANT! Assumes left/right won't change
		if (centerpoint != null) {
			return centerpoint.copy();
		}

		int m = 0;
		int n = 0;
		centerpoint = new pose_t();

		if (Arrays.equals(left, right) == false) {
			switch (direction) {
			case NORTH:
			case SOUTH:
				// horizontal
				m = left[0];
				n = right[0];
				centerpoint.pos[1] = left[1] * RoomWorld.CELL_SIZE;
				break;
			case EAST:
			case WEST:
				// vertical
				m = left[1];
				n = right[1];
				centerpoint.pos[0] = left[0] * RoomWorld.CELL_SIZE;
				break;
			}
		} else {
			// single block
			centerpoint.pos[0] = left[0] * RoomWorld.CELL_SIZE;
			centerpoint.pos[1] = left[1] * RoomWorld.CELL_SIZE;

			switch (direction) {
			case NORTH:
				centerpoint.pos[1] += RoomWorld.CELL_SIZE;
			case SOUTH:
				centerpoint.pos[0] += RoomWorld.CELL_SIZE / 2;
				break;
			case WEST:
				centerpoint.pos[0] += RoomWorld.CELL_SIZE;
			case EAST:
				centerpoint.pos[1] += RoomWorld.CELL_SIZE / 2;
				break;
			}
			return centerpoint;
		}
		int numberOfBlocks = n - m;
		int[] upperLeft = left;
		// take abs, also note that if negative then we need to calculate center
		// from the upper-left block
		// which this case is the "right"
		if (numberOfBlocks < 0) {
			numberOfBlocks *= -1;
			upperLeft = right;
		}
		numberOfBlocks += 1; // endpoints 0,0 and 0,3 represent 4 blocks
		assert numberOfBlocks > 1;

		if (left[0] == right[0]) {
			// vertical
			// add half to y
			centerpoint.pos[1] = upperLeft[1] * RoomWorld.CELL_SIZE;
			centerpoint.pos[1] += (numberOfBlocks / 2.0) * RoomWorld.CELL_SIZE;

			// if west, we gotta add a cell size to x
			if (direction == Direction.WEST) {
				centerpoint.pos[0] += RoomWorld.CELL_SIZE;
			}

		} else {
			// horizontal
			// add half to x
			centerpoint.pos[0] = upperLeft[0] * RoomWorld.CELL_SIZE;
			centerpoint.pos[0] += (numberOfBlocks / 2.0) * RoomWorld.CELL_SIZE;

			// if north, we gotta add a cell size to y
			if (direction == Direction.NORTH) {
				centerpoint.pos[1] += RoomWorld.CELL_SIZE;
			}
		}
		return centerpoint;
	}

	@Override
	public String toString() {
		String output = new String(Integer.toString(id));
		output += " (" + direction.id() + ")";
		pose_t center = centerpoint();
		output += " (" + Integer.toString(left[0]) + ","
				+ Integer.toString(left[1]) + ")-("
				+ Double.toString(center.pos[0]) + ","
				+ Double.toString(center.pos[1]) + ")-("
				+ Integer.toString(right[0]) + "," + Integer.toString(right[1])
				+ ")";
		if (gateway) {
			output += " (gateway)";
		}
		return output;
	}
}

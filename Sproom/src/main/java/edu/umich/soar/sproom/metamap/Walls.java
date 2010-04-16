/**
 * 
 */
package edu.umich.soar.sproom.metamap;

import java.util.HashMap;

import april.jmat.LinAlg;

public class Walls {
	private final HashMap<WallDir, double[]> walls = new HashMap<WallDir, double[]>(4);
	
	Walls(double[] pos, double[] xySize) {
		double[] midpoint = LinAlg.scale(xySize, 0.5);
		walls.put(WallDir.NORTH, new double[] { pos[0] + midpoint[0], pos[1] + xySize[1], 0 });
		walls.put(WallDir.SOUTH, new double[] { pos[0] + midpoint[0], pos[1], 0 });
		walls.put(WallDir.EAST, new double[] { pos[0] + xySize[0], pos[1] + midpoint[1], 0 });
		walls.put(WallDir.WEST, new double[] { pos[0], pos[1] + midpoint[1], 0 });
	}
	
	public double[] getPos(WallDir dir) {
		double[] temp = new double[3];
		System.arraycopy(walls.get(dir), 0, temp, 0, temp.length);
		return temp;
	}
}
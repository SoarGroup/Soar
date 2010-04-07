/**
 * 
 */
package edu.umich.soar.sproom.metamap;

import april.config.Config;

public class UnitConverter {
	private final double[] origin;
	private final double scale;
	
	public static UnitConverter getInstance(Config config) {
		if (config.getString("metadata.units", "meters").equals("pixels")) {
			int[] origin = config.getInts("metadata.origin");
			double scale = config.requireDouble("metadata.scale");
			return new UnitConverter(origin, scale);
		}
		return new UnitConverter();
	}
	
	private UnitConverter() {
		origin = null; // in meters, no conversion necessary
		scale = 1;
	}
	
	private UnitConverter(int[] origin, double scale) {
		this.origin = new double[] { origin[0] * scale, origin[1] * scale }; // pixels -> meters
		this.scale = scale;
	}
	
	public double[] getPos(Config config, String nick) {
		return getPos(config, nick, new double[] {0, 0});
	}
	
	public double[] getPos(Config config, String nick, double[] size) {
		if (origin == null) {
			return config.getDoubles("metadata." + nick + ".pos");
		}
		int[] loc = config.getInts("metadata." + nick + ".pos");
		double[] pos = new double[] { loc[0] * scale, loc[1] * scale };
		return new double[] { pos[0] - origin[0], ((pos[1] + size[1]) - origin[1]) * -1 };
	}
	
	public double[] getSize(Config config, String nick) {
		if (origin == null) {
			return config.getDoubles("metadata." + nick + ".size");
		}
		int[] size = config.getInts("metadata." + nick + ".size");
		
		return new double[] { size[0] * scale, size[1] * scale };
	}
	
}
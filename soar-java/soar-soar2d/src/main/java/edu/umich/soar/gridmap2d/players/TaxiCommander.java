package edu.umich.soar.gridmap2d.players;

import edu.umich.soar.gridmap2d.map.TaxiMap;

public interface TaxiCommander extends Commander {
	public void update(TaxiMap taxiMap) throws Exception;
}

package org.msoar.gridmap2d.players;

import org.msoar.gridmap2d.map.TaxiMap;

public interface TaxiCommander extends Commander {
	public void update(TaxiMap taxiMap) throws Exception;
}

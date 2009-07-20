package org.msoar.gridmap2d.players;

import org.msoar.gridmap2d.map.EatersMap;

public interface EaterCommander extends Commander {
	public void update(EatersMap eatersMap) throws Exception;
}

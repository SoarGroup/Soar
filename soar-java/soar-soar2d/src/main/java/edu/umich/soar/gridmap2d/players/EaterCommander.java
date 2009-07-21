package edu.umich.soar.gridmap2d.players;

import edu.umich.soar.gridmap2d.map.EatersMap;

public interface EaterCommander extends Commander {
	public void update(EatersMap eatersMap) throws Exception;
}

package edu.umich.soar.gridmap2d.players;

import edu.umich.soar.gridmap2d.map.TankSoarMap;

public interface TankCommander extends Commander {
	public void update(TankSoarMap tankSoarMap);
	public void playersChanged(Player[] players);
	public void commit();
	public void fragged();
}

package org.msoar.gridmap2d.players;

import org.msoar.gridmap2d.map.CellObject;
import org.msoar.gridmap2d.map.RoomMap;

public interface RoomCommander extends Commander {
	public void update(RoomMap roomMap) throws Exception;
	public void rotateComplete();
	public void updateGetStatus(boolean success);
	public void updateDropStatus(boolean success);
	public void receiveMessage(Player player, String message);
	public void carry(CellObject object);
	public void drop();
}

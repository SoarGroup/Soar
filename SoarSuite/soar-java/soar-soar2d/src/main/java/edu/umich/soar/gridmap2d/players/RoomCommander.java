package edu.umich.soar.gridmap2d.players;

import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.RoomMap;

public interface RoomCommander extends Commander {
	public void update(RoomMap roomMap) throws Exception;
	public void rotateComplete();
	public void updateGetStatus(boolean success);
	public void updateDropStatus(boolean success);
	public void receiveMessage(Player player, String message);
	public void carry(CellObject object);
	public void drop();
}

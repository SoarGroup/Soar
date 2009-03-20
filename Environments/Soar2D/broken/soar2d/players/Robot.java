package broken.soar2d.players;

import soar2d.Names;
import soar2d.Soar2D;
import soar2d.map.BookMap;
import soar2d.map.CellObject;
import soar2d.world.World;

public class Robot {
	private CommandInfo move;

	public Robot(String playerId) {
		super(playerId);
	}
	
	public CommandInfo getMove() {
		return move;
	}

	public boolean getHumanMove() {
		move = Soar2D.wm.getHumanCommand(this);

		if (move == null) {
			return false;
		}
		
		return true;
	}

	int inId = -1;
	String inType = null;
	
	public void update(int [] location) {
		World world = Soar2D.simulation.world;
		BookMap map = (BookMap)world.getMap();
		CellObject obj = map.getInObject(location);
		
		inId = obj.getIntProperty(Names.kPropertyNumber);
		inType = new String(obj.getProperty(Names.kPropertyID));
		
		//System.out.println("Type: " + inType + ", ID: " + inId);
	}
}

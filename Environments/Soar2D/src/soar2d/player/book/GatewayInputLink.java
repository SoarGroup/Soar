package soar2d.player.book;

import java.util.ArrayList;

import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import soar2d.map.BookMap.Barrier;
import soar2d.world.PlayersManager;
import soar2d.world.World;

class GatewayInputLink extends BarrierInputLink {
	ArrayList<IntElement> toList = new ArrayList<IntElement>();
	FloatElement range;
	
	GatewayInputLink(SoarRobot robot, Identifier parent) {
		super(robot, parent);
	}
	
	@Override
	void initialize(Barrier barrier, World world) {
		PlayersManager players = world.getPlayers();
		
		super.initialize(barrier, world);

		double dx = centerpoint.x - players.getFloatLocation(robot).x;
		dx *= dx;
		double dy = centerpoint.y - players.getFloatLocation(robot).y;
		dy *= dy;
		double r = Math.sqrt(dx + dy);

		range = robot.agent.CreateFloatWME(center, "range", r);
	}
	
	void addDest(int id) {
		IntElement dest = robot.agent.CreateIntWME(parent, "to", id);
		toList.add(dest);
	}
}


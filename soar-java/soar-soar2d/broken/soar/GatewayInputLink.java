package broken.soar;

import java.util.ArrayList;
import java.util.List;

import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import soar2d.map.BookMap.Barrier;
import soar2d.world.PlayersManager;
import soar2d.world.World;

class GatewayInputLink extends BarrierInputLink {
	List<IntElement> toList = new ArrayList<IntElement>();
	FloatElement range;
	
	GatewayInputLink(SoarRobot robot, Identifier parent) {
		super(robot, parent);
	}
	
	@Override
	void initialize(Barrier barrier, World world) {
		PlayersManager players = world.getPlayers();
		
		super.initialize(barrier, world);

		double dx = centerpoint[0] - players.getFloatLocation(robot)[0];
		dx *= dx;
		double dy = centerpoint[1] - players.getFloatLocation(robot)[1];
		dy *= dy;
		double r = Math.sqrt(dx + dy);

		range = robot.agent.CreateFloatWME(center, "range", r);
	}
	
	void addDest(int id) {
		IntElement dest = robot.agent.CreateIntWME(parent, "to", id);
		toList.add(dest);
	}
}


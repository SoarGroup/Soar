package broken.soar;

import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import soar2d.Soar2D;
import soar2d.map.BookMap.Barrier;
import soar2d.world.PlayersManager;
import soar2d.world.World;

class BarrierInputLink {
	SoarRobot robot;
	IntElement id;
	Identifier parent, left, right, center, angleOff;
	IntElement leftRow, leftCol, rightRow, rightCol;
	FloatElement x, y, yaw;
	StringElement direction;
	
	double [] centerpoint;
	
	BarrierInputLink(SoarRobot robot, Identifier parent) {
		this.robot = robot;
		this.parent = parent;
	}
	
	void initialize(Barrier barrier, World world) {
		PlayersManager players = world.getPlayers();

		id = robot.agent.CreateIntWME(parent, "id", barrier.id);
		left = robot.agent.CreateIdWME(parent, "left");
		{
			leftRow = robot.agent.CreateIntWME(left, "row", barrier.left[1]);
			leftCol = robot.agent.CreateIntWME(left, "col", barrier.left[0]);
		}
		right = robot.agent.CreateIdWME(parent, "right");
		{
			rightRow = robot.agent.CreateIntWME(right, "row", barrier.right[1]);
			rightCol = robot.agent.CreateIntWME(right, "col", barrier.right[0]);
		}
		center = robot.agent.CreateIdWME(parent, "center");
		{
			centerpoint = barrier.centerpoint();
			if (Soar2D.config.roomConfig().continuous) {
				x = robot.agent.CreateFloatWME(center, "x", centerpoint[0]);
				y = robot.agent.CreateFloatWME(center, "y", centerpoint[1]);
			}
			angleOff = robot.agent.CreateIdWME(center, "angle-off");
			yaw = robot.agent.CreateFloatWME(angleOff, "yaw", players.angleOff(robot, centerpoint));
		}
		direction = robot.agent.CreateStringWME(parent, "direction", barrier.direction.id());
	}
}


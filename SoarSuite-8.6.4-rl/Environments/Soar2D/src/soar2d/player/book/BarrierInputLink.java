package soar2d.player.book;

import java.awt.geom.Point2D;

import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import soar2d.Direction;
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
	
	Point2D.Double centerpoint;
	
	BarrierInputLink(SoarRobot robot, Identifier parent) {
		this.robot = robot;
		this.parent = parent;
	}
	
	void initialize(Barrier barrier, World world) {
		PlayersManager players = world.getPlayers();

		id = robot.agent.CreateIntWME(parent, "id", barrier.id);
		left = robot.agent.CreateIdWME(parent, "left");
		{
			leftRow = robot.agent.CreateIntWME(left, "row", barrier.left.y);
			leftCol = robot.agent.CreateIntWME(left, "col", barrier.left.x);
		}
		right = robot.agent.CreateIdWME(parent, "right");
		{
			rightRow = robot.agent.CreateIntWME(right, "row", barrier.right.y);
			rightCol = robot.agent.CreateIntWME(right, "col", barrier.right.x);
		}
		center = robot.agent.CreateIdWME(parent, "center");
		{
			centerpoint = barrier.centerpoint();
			if (Soar2D.bConfig.getContinuous()) {
				x = robot.agent.CreateFloatWME(center, "x", centerpoint.x);
				y = robot.agent.CreateFloatWME(center, "y", centerpoint.y);
			}
			angleOff = robot.agent.CreateIdWME(center, "angle-off");
			yaw = robot.agent.CreateFloatWME(angleOff, "yaw", players.angleOff(robot, centerpoint));
		}
		direction = robot.agent.CreateStringWME(parent, "direction", Direction.stringOf[barrier.direction]);
	}
}


package soar2d.player.book;

import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import soar2d.Soar2D;
import soar2d.player.Player;
import soar2d.world.PlayersManager;
import soar2d.world.World;

class PlayerInputLink { // FIXME should share code with OIL
	SoarRobot robot;
	Identifier parent;
	
	IntElement area;
	Identifier position;
	Identifier angleOff;
	FloatElement yaw;
	FloatElement x, y;
	IntElement row, col;
	FloatElement range;
	StringElement name;
	
	int cycleTouched;
	
	PlayerInputLink(SoarRobot robot, Identifier parent) {
		this.robot = robot;
		this.parent = parent;
	}
	
	void initialize(Player player, World world, double range, double angleOffDouble) {
		PlayersManager players = world.getPlayers();

		this.name = robot.agent.CreateStringWME(parent, "name", player.getName());
		this.area = robot.agent.CreateIntWME(parent, "area", player.getLocationId());
		this.angleOff = robot.agent.CreateIdWME(parent, "angle-off");
		this.yaw = robot.agent.CreateFloatWME(angleOff, "yaw", angleOffDouble);
		this.position = robot.agent.CreateIdWME(parent, "position");
		{
			this.col = robot.agent.CreateIntWME(position, "col", players.getLocation(player).x);
			this.row = robot.agent.CreateIntWME(position, "row", players.getLocation(player).y);
			if (Soar2D.config.bConfig.getContinuous()) {
				this.x = robot.agent.CreateFloatWME(position, "x", players.getFloatLocation(player).x);
				this.y = robot.agent.CreateFloatWME(position, "y", players.getFloatLocation(player).y);
			}
		}
		this.range = robot.agent.CreateFloatWME(parent, "range", range);
		
		touch(world.getWorldCount());
	}
	
	void touch(int cycle) {
		cycleTouched = cycle;
	}
}


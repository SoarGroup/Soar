package broken.soar;

import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import soar2d.Soar2D;
import soar2d.map.GridMap;
import soar2d.world.World;

class ObjectInputLink {
	SoarRobot robot;
	Identifier parent;
	
	IntElement area;
	StringElement type;
	Identifier position;
	Identifier angleOff;
	FloatElement yaw;
	FloatElement x, y;
	IntElement row, col;
	StringElement visible;
	FloatElement range;
	IntElement id;
	
	int cycleTouched;
	
	ObjectInputLink(SoarRobot robot, Identifier parent) {
		this.robot = robot;
		this.parent = parent;
	}
	
	void initialize(GridMap.BookObjectInfo info, World world, double range, double angleOffDouble) {
		this.id = robot.agent.CreateIntWME(parent, "id", info.object.getIntProperty("object-id"));
		this.type = robot.agent.CreateStringWME(parent, "type", info.object.getProperty("id"));
		this.area = robot.agent.CreateIntWME(parent, "area", info.area);
		this.angleOff = robot.agent.CreateIdWME(parent, "angle-off");
		this.yaw = robot.agent.CreateFloatWME(angleOff, "yaw", angleOffDouble);
		this.position = robot.agent.CreateIdWME(parent, "position");
		{
			this.col = robot.agent.CreateIntWME(position, "col", info.location[0]);
			this.row = robot.agent.CreateIntWME(position, "row", info.location[1]);
			if (Soar2D.config.roomConfig().continuous) {
				this.x = robot.agent.CreateFloatWME(position, "x", info.floatLocation[0]);
				this.y = robot.agent.CreateFloatWME(position, "y", info.floatLocation[1]);
			}
		}
		this.range = robot.agent.CreateFloatWME(parent, "range", range);
		this.visible = robot.agent.CreateStringWME(parent, "visible", "yes");
		
		touch(world.getWorldCount());
	}
	
	void touch(int cycle) {
		cycleTouched = cycle;
		if (visible.GetValue().equals("no")) {
			robot.agent.Update(visible, "yes");
		}
	}
	
	void makeInvisible() {
		if (visible.GetValue().equals("yes")) {
			robot.agent.Update(visible, "no");
		}
	}
}


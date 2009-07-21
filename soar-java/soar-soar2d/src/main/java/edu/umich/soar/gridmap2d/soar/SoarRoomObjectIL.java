package edu.umich.soar.gridmap2d.soar;


import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.world.RoomWorld;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;

class SoarRoomObjectIL {
	Agent agent;
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
	
	SoarRoomObjectIL(Agent agent, Identifier parent) {
		this.agent = agent;
		this.parent = parent;
	}
	
	void initialize(RoomMap.RoomObjectInfo info, RoomWorld world, double range, double angleOffDouble) {
		this.id = agent.CreateIntWME(parent, "id", info.object.getIntProperty("object-id", -1));
		this.type = agent.CreateStringWME(parent, "type", info.object.getProperty("id"));
		this.area = agent.CreateIntWME(parent, "area", info.area);
		this.angleOff = agent.CreateIdWME(parent, "angle-off");
		this.yaw = agent.CreateFloatWME(angleOff, "yaw", angleOffDouble);
		this.position = agent.CreateIdWME(parent, "position");
		{
			this.col = agent.CreateIntWME(position, "col", info.location[0]);
			this.row = agent.CreateIntWME(position, "row", info.location[1]);
			this.x = agent.CreateFloatWME(position, "x", info.floatLocation[0]);
			this.y = agent.CreateFloatWME(position, "y", info.floatLocation[1]);
		}
		this.range = agent.CreateFloatWME(parent, "range", range);
		this.visible = agent.CreateStringWME(parent, "visible", "yes");
		
		touch(Gridmap2D.simulation.getWorldCount());
	}
	
	void touch(int cycle) {
		cycleTouched = cycle;
		if (visible.GetValue().equals("no")) {
			visible.Update("yes");
		}
	}
	
	void makeInvisible() {
		if (visible.GetValue().equals("yes")) {
			visible.Update("no");
		}
	}
}


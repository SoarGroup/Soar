package org.msoar.gridmap2d.soar;

import org.msoar.gridmap2d.Gridmap2D;
import org.msoar.gridmap2d.players.RoomPlayer;
import org.msoar.gridmap2d.world.RoomWorld;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;

class SoarRoomPlayerIL { // FIXME should share code with OIL
	Agent agent;
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
	
	SoarRoomPlayerIL(Agent agent, Identifier parent) {
		this.agent = agent;
		this.parent = parent;
	}
	
	void initialize(RoomPlayer target, RoomWorld world, double range, double angleOffDouble) {
		this.name = agent.CreateStringWME(parent, "name", target.getName());
		this.area = agent.CreateIntWME(parent, "area", target.getState().getLocationId());
		this.angleOff = agent.CreateIdWME(parent, "angle-off");
		this.yaw = agent.CreateFloatWME(angleOff, "yaw", angleOffDouble);
		this.position = agent.CreateIdWME(parent, "position");
		{
			this.col = agent.CreateIntWME(position, "col", target.getLocation()[0]);
			this.row = agent.CreateIntWME(position, "row", target.getLocation()[1]);
			this.x = agent.CreateFloatWME(position, "x", target.getState().getFloatLocation()[0]);
			this.y = agent.CreateFloatWME(position, "y", target.getState().getFloatLocation()[1]);
		}
		this.range = agent.CreateFloatWME(parent, "range", range);
		
		touch(Gridmap2D.simulation.getWorldCount());
	}
	
	void touch(int cycle) {
		cycleTouched = cycle;
	}
}


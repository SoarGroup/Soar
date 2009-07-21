package edu.umich.soar.gridmap2d.soar;


import edu.umich.soar.gridmap2d.map.GridMapUtil.Barrier;
import edu.umich.soar.gridmap2d.players.RoomPlayer;
import edu.umich.soar.gridmap2d.world.RoomWorld;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;

class SoarRoomBarrierIL {
	RoomPlayer player;
	Agent agent;
	IntElement id;
	Identifier parent, left, right, center, angleOff;
	IntElement leftRow, leftCol, rightRow, rightCol;
	FloatElement x, y, yaw;
	StringElement direction;
	
	double [] centerpoint;
	
	SoarRoomBarrierIL(RoomPlayer player, Agent agent, Identifier parent) {
		this.player = player;
		this.agent = agent;
		this.parent = parent;
	}
	
	void initialize(Barrier barrier, RoomWorld world) {
		id = agent.CreateIntWME(parent, "id", barrier.id);
		left = agent.CreateIdWME(parent, "left");
		{
			leftRow = agent.CreateIntWME(left, "row", barrier.left[1]);
			leftCol = agent.CreateIntWME(left, "col", barrier.left[0]);
		}
		right = agent.CreateIdWME(parent, "right");
		{
			rightRow = agent.CreateIntWME(right, "row", barrier.right[1]);
			rightCol = agent.CreateIntWME(right, "col", barrier.right[0]);
		}
		center = agent.CreateIdWME(parent, "center");
		{
			centerpoint = barrier.centerpoint();
			x = agent.CreateFloatWME(center, "x", centerpoint[0]);
			y = agent.CreateFloatWME(center, "y", centerpoint[1]);
			angleOff = agent.CreateIdWME(center, "angle-off");
			yaw = agent.CreateFloatWME(angleOff, "yaw", player.getState().angleOff(centerpoint));
		}
		direction = agent.CreateStringWME(parent, "direction", barrier.direction.id());
	}
}


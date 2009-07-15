package org.msoar.gridmap2d.soar;

import java.util.ArrayList;

import org.msoar.gridmap2d.map.GridMapUtil.Barrier;
import org.msoar.gridmap2d.players.RoomPlayer;
import org.msoar.gridmap2d.world.RoomWorld;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;

class SoarRoomGatewayIL extends SoarRoomBarrierIL {
	ArrayList<IntElement> toList = new ArrayList<IntElement>();
	FloatElement range;
	
	SoarRoomGatewayIL(RoomPlayer player, Agent agent, Identifier parent) {
		super(player, agent, parent);
	}
	
	@Override
	void initialize(Barrier barrier, RoomWorld world) {
		super.initialize(barrier, world);

		double dx = centerpoint[0] - player.getState().getFloatLocation()[0];
		dx *= dx;
		double dy = centerpoint[1] - player.getState().getFloatLocation()[1];
		dy *= dy;
		double r = Math.sqrt(dx + dy);

		range = agent.CreateFloatWME(center, "range", r);
	}
	
	void addDest(int id) {
		IntElement dest = agent.CreateIntWME(parent, "to", id);
		toList.add(dest);
	}
}


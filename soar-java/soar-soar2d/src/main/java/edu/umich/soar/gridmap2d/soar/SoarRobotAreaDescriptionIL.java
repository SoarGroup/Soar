package edu.umich.soar.gridmap2d.soar;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import jmat.LinAlg;

import lcmtypes.pose_t;

import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.map.GridMapUtil.Barrier;
import edu.umich.soar.robot.OffsetPose;

public class SoarRobotAreaDescriptionIL {
	
	private final Identifier areaDescription;
	private final OffsetPose opose;
	private final List<BarrierIL> walls = new ArrayList<BarrierIL>();
	private final List<GatewayIL> gateways = new ArrayList<GatewayIL>();
	
	SoarRobotAreaDescriptionIL(Identifier areaDescription, int locationId, OffsetPose opose, RoomMap roomMap) {
		this.areaDescription = areaDescription;
		this.opose = opose;
		
		areaDescription.CreateIntWME("id", locationId);
		
		// create new area information
		List<Barrier> barrierList = roomMap.getRoomBarrierList(locationId);
		assert barrierList != null;
		assert barrierList.size() > 0;
		
		for (Barrier barrier : barrierList) {
			if (barrier.gateway) {
				// gateway
				GatewayIL gateway = new GatewayIL(areaDescription.CreateIdWME("gateway"));
				gateway.initialize(barrier);
				
				// add destinations
				List<Integer> gatewayDestList = roomMap.getGatewayDestinationList(barrier.id);
				Iterator<Integer> destIter = gatewayDestList.iterator();
				while(destIter.hasNext()) {
					gateway.addDest(destIter.next().intValue());
				}
				
				addGateway(gateway);
				
			} else {
				// wall
				BarrierIL wall = new BarrierIL(areaDescription.CreateIdWME("wall"));
				wall.initialize(barrier);
				addWall(wall);
			}
		}
	}
	
	void destroy() {
		areaDescription.DestroyWME();
	}
	
	private void addWall(BarrierIL wall) {
		walls.add(wall);
	}
	
	private void addGateway(GatewayIL gateway) {
		gateways.add(gateway);
	}
	
	void update() {
		pose_t player = opose.getPose();
		
		// barrier angle offs and range
		for(BarrierIL barrier : walls) {
			barrier.angleOff.Update(SoarRobot.angleOff(player, barrier.centerpoint));
		}

		for(GatewayIL gateway : gateways) {
			gateway.angleOff.Update(SoarRobot.angleOff(player, gateway.centerpoint));
			gateway.range.Update(LinAlg.distance(opose.getPose().pos, gateway.centerpoint.pos));
		}
	}

	private class BarrierIL {
		protected Identifier parent;
		FloatElement angleOff;
		
		protected pose_t centerpoint;
		
		private BarrierIL(Identifier parent) {
			this.parent = parent;
		}
		
		protected void initialize(Barrier barrier) {
			parent.CreateIntWME("id", barrier.id);
			centerpoint = barrier.centerpoint();
			LinAlg.scale(centerpoint.pos, SoarRobot.PIXELS_2_METERS);
			parent.CreateFloatWME("x", centerpoint.pos[0]);
			parent.CreateFloatWME("y", centerpoint.pos[1]);
			angleOff = parent.CreateFloatWME("angle-off", SoarRobot.angleOff(opose.getPose(), centerpoint));
			parent.CreateStringWME("direction", barrier.direction.id());
		}
	}

	private class GatewayIL extends BarrierIL {
		private ArrayList<IntElement> toList = new ArrayList<IntElement>();
		private FloatElement range;
		
		private GatewayIL(Identifier parent) {
			super(parent);
		}
		
		@Override
		protected void initialize(Barrier barrier) {
			super.initialize(barrier);
			range = parent.CreateFloatWME("range", LinAlg.distance(opose.getPose().pos, centerpoint.pos));
		}
		
		private void addDest(int id) {
			IntElement dest = parent.CreateIntWME("to", id);
			toList.add(dest);
		}
	}



}

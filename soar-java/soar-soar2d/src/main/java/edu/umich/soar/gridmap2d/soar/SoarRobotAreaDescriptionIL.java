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
import edu.umich.soar.robot.PointRelationship;

public class SoarRobotAreaDescriptionIL {
	
	private final Identifier areaDescription;
	private final OffsetPose opose;
	private final List<BarrierIL> barriers = new ArrayList<BarrierIL>();
	
	SoarRobotAreaDescriptionIL(Identifier areaDescription, int locationId, OffsetPose opose, RoomMap roomMap) {
		this.areaDescription = areaDescription;
		this.opose = opose;
		
		areaDescription.CreateIntWME("id", locationId);
		
		// create new area information
		List<Barrier> barrierList = roomMap.getRoomBarrierList(locationId);
		assert barrierList != null;
		assert barrierList.size() > 0;
		
		pose_t player = opose.getPose();
		
		for (Barrier barrier : barrierList) {
			if (barrier.gateway) {
				// gateway
				GatewayIL gateway = new GatewayIL(areaDescription.CreateIdWME("gateway"));
				gateway.initialize(player, barrier);
				
				// add destinations
				List<Integer> gatewayDestList = roomMap.getGatewayDestinationList(barrier.id);
				Iterator<Integer> destIter = gatewayDestList.iterator();
				while(destIter.hasNext()) {
					gateway.addDest(destIter.next().intValue());
				}
				
				barriers.add(gateway);
				
			} else {
				// wall
				BarrierIL wall = new BarrierIL(areaDescription.CreateIdWME("wall"));
				wall.initialize(player, barrier);
				barriers.add(wall);
			}
		}
	}
	
	void destroy() {
		areaDescription.DestroyWME();
	}
	
	void update() {
		pose_t player = opose.getPose();
		
		for(BarrierIL barrier : barriers) {
			barrier.update(player);
		}
	}

	private class BarrierIL {
		protected Identifier parent;
		private FloatElement distance;
		private FloatElement yaw;
		private FloatElement relativeBearing;
		private FloatElement absRelativeBearing;
		
		protected pose_t centerpoint;
		
		private BarrierIL(Identifier parent) {
			this.parent = parent;
		}
		
		protected void initialize(pose_t player, Barrier barrier) {
			parent.CreateIntWME("id", barrier.id);

			parent.CreateStringWME("direction", barrier.direction.id());

			centerpoint = barrier.centerpoint();
			LinAlg.scaleEquals(centerpoint.pos, SoarRobot.PIXELS_2_METERS);

			parent.CreateFloatWME("x", centerpoint.pos[0]);
			parent.CreateFloatWME("y", centerpoint.pos[1]);
			parent.CreateFloatWME("z", centerpoint.pos[2]);
			
			PointRelationship r = PointRelationship.calculate(player, centerpoint.pos);
			
			distance = parent.CreateFloatWME("distance", r.getDistance());
			yaw = parent.CreateFloatWME("yaw", Math.toDegrees(r.getYaw()));
			relativeBearing = parent.CreateFloatWME("relative-bearing", Math.toDegrees(r.getRelativeBearing()));
			absRelativeBearing = parent.CreateFloatWME("abs-relative-bearing", Math.abs(Math.toDegrees(r.getRelativeBearing())));
		}
		
		protected void update(pose_t player) {
			PointRelationship r = PointRelationship.calculate(player, centerpoint.pos);
			
			this.distance.Update(r.getDistance());
			this.yaw.Update(Math.toDegrees(r.getYaw()));
			this.relativeBearing.Update(Math.toDegrees(r.getRelativeBearing()));
			this.absRelativeBearing.Update(Math.abs(Math.toDegrees(r.getRelativeBearing())));
		}
	}

	private class GatewayIL extends BarrierIL {
		private ArrayList<IntElement> toList = new ArrayList<IntElement>();
		
		private GatewayIL(Identifier parent) {
			super(parent);
		}
		
		private void addDest(int id) {
			IntElement dest = parent.CreateIntWME("to", id);
			toList.add(dest);
		}
	}
}

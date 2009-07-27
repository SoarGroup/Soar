package edu.umich.soar.gridmap2d.soar;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import lcmtypes.pose_t;

import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.map.GridMapUtil.Barrier;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.RoomPlayer;
import edu.umich.soar.gridmap2d.world.RoomWorld;

public class SoarRobotAreaDescriptionIL {
	
	private final Identifier areaDescription;
	private final List<BarrierIL> walls = new ArrayList<BarrierIL>();
	private final List<GatewayIL> gateways = new ArrayList<GatewayIL>();
	private final Map<RoomPlayer, PlayerIL> players = new HashMap<RoomPlayer, PlayerIL>();
	private final Map<Integer, ObjectIL> objects = new HashMap<Integer, ObjectIL>();
	
	SoarRobotAreaDescriptionIL(Identifier areaDescription, RoomPlayer player, RoomWorld world) {
		this.areaDescription = areaDescription;
		
		RoomMap roomMap = (RoomMap)world.getMap();
		
		// create new area information
		List<Barrier> barrierList = roomMap.getRoomBarrierList(player.getState().getLocationId());
		assert barrierList != null;
		assert barrierList.size() > 0;
		
		for (Barrier barrier : barrierList) {
			if (barrier.gateway) {
				// gateway
				GatewayIL gateway = new GatewayIL(player, areaDescription.CreateIdWME("gateway"));
				gateway.initialize(barrier, world);
				
				// add destinations
				List<Integer> gatewayDestList = roomMap.getGatewayDestinationList(barrier.id);
				Iterator<Integer> destIter = gatewayDestList.iterator();
				while(destIter.hasNext()) {
					gateway.addDest(destIter.next().intValue());
				}
				
				addGateway(gateway);
				
			} else {
				// wall
				BarrierIL wall = new BarrierIL(player, areaDescription.CreateIdWME("wall"));
				wall.initialize(barrier, world);
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
	
	private void addOrUpdatePlayer(RoomPlayer self, RoomPlayer target, RoomWorld world, double angleOffDouble) {
		PlayerIL pIL = players.get(target);

		pose_t targetPose = target.getState().getPose();
		pose_t selfPose = self.getState().getPose();
		double dx = targetPose.pos[0] - selfPose.pos[0];
		dx *= dx;
		double dy = targetPose.pos[1] - selfPose.pos[1];
		dy *= dy;
		double range = Math.sqrt(dx + dy);
		
		if (pIL == null) {
			// create new player
			Identifier parent = areaDescription.CreateIdWME("player");
			pIL = new PlayerIL(parent);
			pIL.initialize(target, world, range, angleOffDouble);
			players.put(target, pIL);
		
		} else {
			pIL.area.Update(target.getState().getLocationId());
			pIL.row.Update(target.getLocation()[1]);
			pIL.col.Update(target.getLocation()[0]);
			pIL.x.Update(targetPose.pos[0]);
			pIL.y.Update(targetPose.pos[1]);
			pIL.range.Update(range);
			pIL.angleOff.Update(angleOffDouble);
			pIL.touch(Gridmap2D.simulation.getWorldCount());
		}
	}
	
	private void addOrUpdateObject(RoomPlayer self, RoomMap.RoomObjectInfo objectInfo, RoomWorld world, double angleOffDouble) {
		ObjectIL oIL = objects.get(objectInfo.object.getIntProperty("object-id", -1));

		pose_t selfPose = self.getState().getPose();
		double dx = objectInfo.floatLocation[0] - selfPose.pos[0];
		dx *= dx;
		double dy = objectInfo.floatLocation[1] - selfPose.pos[1];
		dy *= dy;
		double range = Math.sqrt(dx + dy);
		
		if (oIL == null) {
			// create new object
			Identifier parent = areaDescription.CreateIdWME("object");
			oIL = new ObjectIL(parent);
			oIL.initialize(objectInfo, world, range, angleOffDouble);
			objects.put(objectInfo.object.getIntProperty("object-id", -1), oIL);
		
		} else {
			oIL.area.Update(objectInfo.area);
			oIL.x.Update(objectInfo.location[0]);
			oIL.y.Update(objectInfo.location[1]);
			oIL.range.Update(range);
			oIL.angleOff.Update(angleOffDouble);
			oIL.touch(Gridmap2D.simulation.getWorldCount());
		}
	}
	
	private void purge(int cycle) {
		Iterator<ObjectIL> oiter = objects.values().iterator();
		while (oiter.hasNext()) {
			ObjectIL oIL = oiter.next();
			if (oIL.cycleTouched < cycle) {
				if (oIL.cycleTouched > cycle - 3) {
					oIL.makeInvisible();
				} else {
					oIL.destroy();
					oiter.remove();
				}
			}
		}
		
		Iterator<PlayerIL> piter = players.values().iterator();
		while (piter.hasNext()) {
			PlayerIL pIL = piter.next();
			if (pIL.cycleTouched < cycle) {
				pIL.destroy();
				piter.remove();
			}
		}
	}

	void update(RoomPlayer player, RoomWorld world) {
		RoomMap roomMap = (RoomMap)world.getMap();
		
		// barrier angle offs and range
		for(BarrierIL barrier : walls) {
			barrier.angleOff.Update(player.getState().angleOff(barrier.centerpoint));
		}
		
		for(GatewayIL gateway : gateways) {
			
			gateway.angleOff.Update(player.getState().angleOff(gateway.centerpoint));
			pose_t selfPose = player.getState().getPose();
			double dx = gateway.centerpoint[0] - selfPose.pos[0];
			dx *= dx;
			double dy = gateway.centerpoint[1] - selfPose.pos[1];
			dy *= dy;
			double r = Math.sqrt(dx + dy);
			
			gateway.range.Update(r);

		}
		
		// objects
		Set<CellObject> roomObjects = roomMap.getRoomObjects();
		for (CellObject obj : roomObjects) {
			RoomMap.RoomObjectInfo info = roomMap.getRoomObjectInfo(obj);
			if (info.area == player.getState().getLocationId()) {
				double maxAngleOff = 180 / 2;
				double angleOff = player.getState().angleOff(info.floatLocation);
				if (Math.abs(angleOff) <= maxAngleOff) {
					addOrUpdateObject(player, info, world, angleOff);
				}
			}
		}
		
		// players
		if (world.getPlayers().length > 1) {
			for (Player temp : world.getPlayers()) {
				RoomPlayer rTarget = (RoomPlayer)temp;
				if (rTarget.equals(player)) {
					continue;
				}
				pose_t rTargetPose = rTarget.getState().getPose();
				addOrUpdatePlayer(player, rTarget, world, player.getState().angleOff(rTargetPose.pos));
			}
		}

		purge(Gridmap2D.simulation.getWorldCount());
	}

	private static class PlayerIL {
		private Identifier parent;
		
		private IntElement area;
		private Identifier position;
		private FloatElement angleOff;
		private FloatElement x, y;
		private IntElement row, col;
		private FloatElement range;
		
		private int cycleTouched;
		
		private PlayerIL(Identifier parent) {
			this.parent = parent;
		}
		
		private void initialize(RoomPlayer target, RoomWorld world, double range, double angleOffDouble) {
			parent.CreateStringWME("name", target.getName());
			this.area = parent.CreateIntWME("area", target.getState().getLocationId());
			this.angleOff = parent.CreateFloatWME("angle-off", angleOffDouble);
			this.position = parent.CreateIdWME("position");
			{
				pose_t targetPose = target.getState().getPose();
				this.col = position.CreateIntWME("col", target.getLocation()[0]);
				this.row = position.CreateIntWME("row", target.getLocation()[1]);
				this.x = position.CreateFloatWME("x", targetPose.pos[0]);
				this.y = position.CreateFloatWME("y", targetPose.pos[1]);
			}
			this.range = parent.CreateFloatWME("range", range);
			
			touch(Gridmap2D.simulation.getWorldCount());
		}
		
		private void destroy() {
			parent.DestroyWME();
		}
		
		private void touch(int cycle) {
			cycleTouched = cycle;
		}
	}

	private static class BarrierIL {
		protected RoomPlayer player;
		protected Identifier parent;
		FloatElement angleOff;
		
		protected double [] centerpoint;
		
		private BarrierIL(RoomPlayer player, Identifier parent) {
			this.player = player;
			this.parent = parent;
		}
		
		protected void initialize(Barrier barrier, RoomWorld world) {
			parent.CreateIntWME("id", barrier.id);
			centerpoint = barrier.centerpoint();
			parent.CreateFloatWME("x", centerpoint[0]);
			parent.CreateFloatWME("y", centerpoint[1]);
			angleOff = parent.CreateFloatWME("angle-off", player.getState().angleOff(centerpoint));
			parent.CreateStringWME("direction", barrier.direction.id());
		}
	}

	private static class GatewayIL extends BarrierIL {
		private ArrayList<IntElement> toList = new ArrayList<IntElement>();
		private FloatElement range;
		
		private GatewayIL(RoomPlayer player, Identifier parent) {
			super(player, parent);
		}
		
		@Override
		protected void initialize(Barrier barrier, RoomWorld world) {
			super.initialize(barrier, world);

			pose_t playerPose = player.getState().getPose();
			double dx = centerpoint[0] - playerPose.pos[0];
			dx *= dx;
			double dy = centerpoint[1] - playerPose.pos[1];
			dy *= dy;
			double r = Math.sqrt(dx + dy);

			range = parent.CreateFloatWME("range", r);
		}
		
		private void addDest(int id) {
			IntElement dest = parent.CreateIntWME("to", id);
			toList.add(dest);
		}
	}

	private static class ObjectIL {
		private Identifier parent;
		
		private IntElement area;
		private Identifier position;
		private FloatElement angleOff;
		private FloatElement x, y;
		private StringElement visible;
		private FloatElement range;
		
		private int cycleTouched;
		
		private ObjectIL(Identifier parent) {
			this.parent = parent;
		}
		
		private void initialize(RoomMap.RoomObjectInfo info, RoomWorld world, double range, double angleOffDouble) {
			parent.CreateIntWME("id", info.object.getIntProperty("object-id", -1));
			parent.CreateStringWME("type", info.object.getProperty("id"));
			this.area = parent.CreateIntWME("area", info.area);
			this.angleOff = parent.CreateFloatWME("angle-off", angleOffDouble);
			this.position = parent.CreateIdWME("position");
			{
				this.x = position.CreateFloatWME("x", info.floatLocation[0]);
				this.y = position.CreateFloatWME("y", info.floatLocation[1]);
			}
			this.range = parent.CreateFloatWME("range", range);
			this.visible = parent.CreateStringWME("visible", "yes");
			
			touch(Gridmap2D.simulation.getWorldCount());
		}
		
		private void destroy() {
			parent.DestroyWME();
		}
		
		private void touch(int cycle) {
			cycleTouched = cycle;
			if (visible.GetValue().equals("no")) {
				visible.Update("yes");
			}
		}
		
		private void makeInvisible() {
			if (visible.GetValue().equals("yes")) {
				visible.Update("no");
			}
		}
	}


}

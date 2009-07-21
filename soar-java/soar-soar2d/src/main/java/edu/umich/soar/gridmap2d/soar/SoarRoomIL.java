package edu.umich.soar.gridmap2d.soar;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;


import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.map.GridMapUtil.Barrier;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.RoomPlayer;
import edu.umich.soar.gridmap2d.world.RoomWorld;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;

public class SoarRoomIL {

	private Agent agent;
	private SelfIL selfIL = new SelfIL();
	private List<SoarRoomBarrierIL> walls = new ArrayList<SoarRoomBarrierIL>();
	private List<SoarRoomGatewayIL> gateways = new ArrayList<SoarRoomGatewayIL>();
	private Map<RoomPlayer, SoarRoomPlayerIL> players = new HashMap<RoomPlayer, SoarRoomPlayerIL>();
	private Map<Integer, SoarRoomObjectIL> objects = new HashMap<Integer, SoarRoomObjectIL>();
	protected HashSet<SoarRoomMessageIL> messages = new HashSet<SoarRoomMessageIL>();
	private int oldLocationId;
	private StringElement continuous;

	private Identifier il;
	private Identifier areaDescription;

	SoarRoomIL(Agent agent) {
		this.agent = agent;
		il = agent.GetInputLink();
		oldLocationId = -1;
	}
	
	void create() {
		continuous = agent.CreateStringWME(il, "continuous", "true");
		selfIL.create();
	}
	
	void update(RoomPlayer player, RoomWorld world, RoomMap roomMap) {
		if (player.getMoved()) {
			// check if we're in a new location
			if (oldLocationId != player.getState().getLocationId()) {
				oldLocationId = player.getState().getLocationId();
				
				// destroy old area information
				if (areaDescription != null) {
					destroyAreaDescription();
				}

				// create new area information
				List<Barrier> barrierList = roomMap.getRoomBarrierList(oldLocationId);
				assert barrierList != null;
				assert barrierList.size() > 0;
				
				// this is, in fact, a room
				createAreaDescription();
				
				for (Barrier barrier : barrierList) {
					if (barrier.gateway) {
						// gateway
						SoarRoomGatewayIL gateway = new SoarRoomGatewayIL(player, agent, createGatewayId());
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
						SoarRoomBarrierIL wall = new SoarRoomBarrierIL(player, agent, createWallId());
						wall.initialize(barrier, world);
						addWall(wall);
					}
				}
			} else {
				// barrier angle offs and range
				for(SoarRoomBarrierIL barrier : walls) {
					agent.Update(barrier.yaw, player.getState().angleOff(barrier.centerpoint));
				}
				
				for(SoarRoomGatewayIL gateway : gateways) {
					
					agent.Update(gateway.yaw, player.getState().angleOff(gateway.centerpoint));
					double dx = gateway.centerpoint[0] - player.getState().getFloatLocation()[0];
					dx *= dx;
					double dy = gateway.centerpoint[1] - player.getState().getFloatLocation()[1];
					dy *= dy;
					double r = Math.sqrt(dx + dy);
					
					agent.Update(gateway.range, r);

				}
			}

			// update the clock
			agent.Update(selfIL.cycle, Gridmap2D.simulation.getWorldCount());
		}
		
		selfIL.update(player);
		
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
				addOrUpdatePlayer(player, rTarget, world, player.getState().angleOff(rTarget.getState().getFloatLocation()));
			}
		}

		purge(Gridmap2D.simulation.getWorldCount());
	}
	
	void createAreaDescription() {
		assert areaDescription == null;
		areaDescription = agent.CreateIdWME(il, "area-description");
	}
	
	void destroyAreaDescription() {

		if (areaDescription == null) {
			return;
		}
		agent.DestroyWME(areaDescription);
		areaDescription = null;
		walls = new ArrayList<SoarRoomBarrierIL>();
		gateways = new ArrayList<SoarRoomGatewayIL>();	
	}
	
	Identifier createWallId() {
		assert areaDescription != null;
		return agent.CreateIdWME(areaDescription, "wall");
	}
	
	Identifier createGatewayId() {
		assert areaDescription != null;
		return agent.CreateIdWME(areaDescription, "gateway");
	}
	
	void addWall(SoarRoomBarrierIL wall) {
		walls.add(wall);
	}
	
	void addGateway(SoarRoomGatewayIL gateway) {
		gateways.add(gateway);
	}
	
	void addOrUpdatePlayer(RoomPlayer self, RoomPlayer target, RoomWorld world, double angleOffDouble) {
		SoarRoomPlayerIL pIL = players.get(target);

		double dx = target.getState().getFloatLocation()[0] - self.getState().getFloatLocation()[0];
		dx *= dx;
		double dy = target.getState().getFloatLocation()[1] - self.getState().getFloatLocation()[1];
		dy *= dy;
		double range = Math.sqrt(dx + dy);
		
		if (pIL == null) {
			// create new player
			Identifier parent = agent.CreateIdWME(il, "player");
			pIL = new SoarRoomPlayerIL(agent, parent);
			pIL.initialize(target, world, range, angleOffDouble);
			players.put(target, pIL);
		
		} else {
			pIL.area.Update(target.getState().getLocationId());
			pIL.row.Update(target.getLocation()[1]);
			pIL.col.Update(target.getLocation()[0]);
			pIL.x.Update(target.getState().getFloatLocation()[0]);
			pIL.y.Update(target.getState().getFloatLocation()[1]);
			pIL.range.Update(range);
			pIL.yaw.Update(angleOffDouble);
			pIL.touch(Gridmap2D.simulation.getWorldCount());
		}
	}
	
	void addOrUpdateObject(RoomPlayer self, RoomMap.RoomObjectInfo objectInfo, RoomWorld world, double angleOffDouble) {
		SoarRoomObjectIL oIL = objects.get(objectInfo.object.getIntProperty("object-id", -1));

		double dx = objectInfo.floatLocation[0] - self.getState().getFloatLocation()[0];
		dx *= dx;
		double dy = objectInfo.floatLocation[1] - self.getState().getFloatLocation()[1];
		dy *= dy;
		double range = Math.sqrt(dx + dy);
		
		if (oIL == null) {
			// create new object
			Identifier parent = agent.CreateIdWME(il, "object");
			oIL = new SoarRoomObjectIL(agent, parent);
			oIL.initialize(objectInfo, world, range, angleOffDouble);
			objects.put(objectInfo.object.getIntProperty("object-id", -1), oIL);
		
		} else {
			oIL.area.Update(objectInfo.area);
			oIL.row.Update(objectInfo.location[1]);
			oIL.col.Update(objectInfo.location[0]);
			oIL.x.Update(objectInfo.location[0]);
			oIL.y.Update(objectInfo.location[1]);
			oIL.range.Update(range);
			oIL.yaw.Update(angleOffDouble);
			oIL.touch(Gridmap2D.simulation.getWorldCount());
		}
	}
	
	void purge(int cycle) {
		Iterator<SoarRoomObjectIL> oiter = objects.values().iterator();
		while (oiter.hasNext()) {
			SoarRoomObjectIL oIL = oiter.next();
			if (oIL.cycleTouched < cycle) {
				if (oIL.cycleTouched > cycle - 3) {
					oIL.makeInvisible();
				} else {
					oIL.parent.DestroyWME();
					oiter.remove();
				}
			}
		}
		
		Iterator<SoarRoomPlayerIL> piter = players.values().iterator();
		while (piter.hasNext()) {
			SoarRoomPlayerIL pIL = piter.next();
			if (pIL.cycleTouched < cycle) {
				pIL.parent.DestroyWME();
				piter.remove();
			}
		}
		
		Iterator<SoarRoomMessageIL> miter = messages.iterator();
		while (miter.hasNext()) {
			SoarRoomMessageIL mIL = miter.next();
			if (mIL.cycleCreated < cycle - 3) {
				mIL.parent.DestroyWME();
				miter.remove();
			}
		}
	}

	public SoarRoomObjectIL getOIL(int getId) {
		return objects.get(getId);
	}

	public void addMessage(Player player, String message) {
		Identifier parent = agent.CreateIdWME(il, "message");
		SoarRoomMessageIL mIL = new SoarRoomMessageIL(agent, parent);
		mIL.initialize(player.getName(), message);
		messages.add(mIL);
	}

	public void carry(CellObject object) {
		selfIL.carry(object);
	}

	public void drop() {
		selfIL.drop();
	}
	
	public void destroy() {
		oldLocationId = -1;
		continuous.DestroyWME();
		continuous = null;
		
		destroyAreaDescription();
		
		for (SoarRoomObjectIL thing : objects.values()) {
			thing.parent.DestroyWME();
		}
		objects = new HashMap<Integer, SoarRoomObjectIL>();
		
		for (SoarRoomPlayerIL thing : players.values()) {
			thing.parent.DestroyWME();
		}
		players = new HashMap<RoomPlayer, SoarRoomPlayerIL>();
		
		for (SoarRoomMessageIL thing : messages) {
			thing.parent.DestroyWME();
		}
		messages = new HashSet<SoarRoomMessageIL>();
	}

	private class SelfIL {
		Identifier self;
		Identifier angle;
		FloatElement yaw;
		IntElement area;
		StringElement blockedForward;
		StringElement blockedBackward;
		StringElement blockedLeft;
		StringElement blockedRight;
		Identifier collision;
		StringElement collisionX;
		StringElement collisionY;
		IntElement cycle;
		IntElement score;
		Identifier position;
		FloatElement x;
		FloatElement y;
		IntElement row;
		IntElement col;
		FloatElement random;
		FloatElement time;
		Identifier velocity;
		FloatElement speed;
		FloatElement dx;
		FloatElement dy;
		FloatElement rotation;
		Identifier carry;
		StringElement carryType;
		IntElement carryId;

		void create() {
			self = agent.CreateIdWME(il, "self");
			angle = agent.CreateIdWME(self, "angle");
			yaw = agent.CreateFloatWME(angle, "yaw", 0);
			{
				Identifier blocked = agent.CreateIdWME(self, "blocked");
				blockedForward = agent.CreateStringWME(blocked, "forward", "false");
				blockedBackward = agent.CreateStringWME(blocked, "backward", "false");
				blockedLeft = agent.CreateStringWME(blocked, "left", "false");
				blockedRight = agent.CreateStringWME(blocked, "right", "false");
			}
			area = agent.CreateIntWME(self, "area", -1);
			collision = agent.CreateIdWME(self, "collision");
			collisionX = agent.CreateStringWME(collision, "x", "false");
			collisionY = agent.CreateStringWME(collision, "y", "false");
			cycle = agent.CreateIntWME(self, "cycle", 0);
			score = agent.CreateIntWME(self, "score", 0);
			position = agent.CreateIdWME(self, "position");
			{
				x = agent.CreateFloatWME(position, "x", 0);
				y = agent.CreateFloatWME(position, "y", 0);
				row = agent.CreateIntWME(position, "row", 0);
				col = agent.CreateIntWME(position, "col", 0);
			}
			random = agent.CreateFloatWME(self, "random", 0);
			time = agent.CreateFloatWME(self, "time", 0);
			velocity = agent.CreateIdWME(self, "velocity");
			{
				speed = agent.CreateFloatWME(velocity, "speed", 0);
				dx = agent.CreateFloatWME(velocity, "dx", 0);
				dy = agent.CreateFloatWME(velocity, "dy", 0);
				rotation = agent.CreateFloatWME(velocity, "rotation", 0);
			}
		}
		
		void update(RoomPlayer player) {
			area.Update(player.getState().getLocationId());

			// update the location
			col.Update(player.getLocation()[0]);
			row.Update(player.getLocation()[1]);
			
			double [] floatLocation = player.getState().getFloatLocation();
			agent.Update(selfIL.x, floatLocation[0]);
			agent.Update(selfIL.y, floatLocation[1]);
			
			// heading
			double heading = player.getState().getHeading();
			if (heading < 0) {
				heading += 2 * Math.PI;
			}
			agent.Update(selfIL.yaw, heading);
		}
		
		public void carry(CellObject object) {
			assert carry == null;
			carry = agent.CreateIdWME(self, "carry");
			carryType = agent.CreateStringWME(carry, "type", object.getProperty("id"));
			carryId = agent.CreateIntWME(carry, "id", object.getIntProperty("object-id", -1));
		}

		public void drop() {
			assert carry != null;
			carry.DestroyWME();
			carry = null;
		}
		
		public void destroy() {
			assert self != null;
			self.DestroyWME();
			self = null;
			carry = null;
		}
	}

}

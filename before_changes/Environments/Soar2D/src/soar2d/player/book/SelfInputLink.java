package soar2d.player.book;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import soar2d.Direction;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.map.GridMap;
import soar2d.player.Player;
import soar2d.world.PlayersManager;
import soar2d.world.World;

class SelfInputLink {
	protected SoarRobot robot;
	protected StringElement continuous;
	protected Identifier self;
	protected Identifier angle;
	protected FloatElement yaw;
	protected IntElement area;
	protected Identifier areaDescription;
	protected ArrayList<BarrierInputLink> wallsIL = new ArrayList<BarrierInputLink>();
	protected ArrayList<GatewayInputLink> gatewaysIL = new ArrayList<GatewayInputLink>();
	protected HashMap<Integer, ObjectInputLink> objectsIL = new HashMap<Integer, ObjectInputLink>();
	protected HashMap<Player, PlayerInputLink> playersIL = new HashMap<Player, PlayerInputLink>();
	protected HashSet<MessageInputLink> messagesIL = new HashSet<MessageInputLink>();
	protected Identifier collision;
	protected StringElement collisionX;
	protected StringElement collisionY;
	protected IntElement cycle;
	protected IntElement score;
	protected Identifier position;
	protected FloatElement x;
	protected FloatElement y;
	protected IntElement row;
	protected IntElement col;
	protected FloatElement random;
	protected FloatElement time;
	protected Identifier velocity;
	protected FloatElement speed;
	protected FloatElement dx;
	protected FloatElement dy;
	protected FloatElement rotation;
	protected Identifier carry;
	protected StringElement carryType;
	protected IntElement carryId;
	protected StringElement direction;
	protected StringElement blockedForward;
	protected StringElement blockedBackward;
	protected StringElement blockedLeft;
	protected StringElement blockedRight;

	SelfInputLink(SoarRobot robot) {
		this.robot = robot;
	}
	
	void initialize() {
		Identifier il = robot.agent.GetInputLink();

		assert il != null;
		assert self == null;

		continuous = robot.agent.CreateStringWME(il, "continuous", Soar2D.config.bConfig.getContinuous() ? "true" : "false");
		
		self = robot.agent.CreateIdWME(il, "self");
		angle = robot.agent.CreateIdWME(self, "angle");
		{
			if (Soar2D.config.bConfig.getZeroIsEast() == false) {
				yaw = robot.agent.CreateFloatWME(angle, "yaw", Direction.toDisplayRadians(robot.getHeadingRadians()));
			} else {
				yaw = robot.agent.CreateFloatWME(angle, "yaw", robot.getHeadingRadians());
			}
			if (Soar2D.config.bConfig.getContinuous() == false) {
				direction = robot.agent.CreateStringWME(angle, "direction", Direction.stringOf[robot.getFacingInt()]);
			}
		}
		if (Soar2D.config.bConfig.getContinuous() == false) {
			Identifier blocked = robot.agent.CreateIdWME(self, "blocked");
			blockedForward = robot.agent.CreateStringWME(blocked, "forward", "false");
			blockedBackward = robot.agent.CreateStringWME(blocked, "backward", "false");
			blockedLeft = robot.agent.CreateStringWME(blocked, "left", "false");
			blockedRight = robot.agent.CreateStringWME(blocked, "right", "false");
		}
		area = robot.agent.CreateIntWME(self, "area", -1);
		if (Soar2D.config.bConfig.getContinuous()) {
			collision = robot.agent.CreateIdWME(self, "collision");
			{
				collisionX = robot.agent.CreateStringWME(collision, "x", "false");
				collisionY = robot.agent.CreateStringWME(collision, "y", "false");
			}
		}
		cycle = robot.agent.CreateIntWME(self, "cycle", 0);
		score = robot.agent.CreateIntWME(self, "score", 0);
		position = robot.agent.CreateIdWME(self, "position");
		{
			if (Soar2D.config.bConfig.getContinuous()) {
				x = robot.agent.CreateFloatWME(position, "x", 0);
				y = robot.agent.CreateFloatWME(position, "y", 0);
			}
			row = robot.agent.CreateIntWME(position, "row", 0);
			col = robot.agent.CreateIntWME(position, "col", 0);
		}
		random = robot.agent.CreateFloatWME(self, "random", robot.random);
		time = robot.agent.CreateFloatWME(self, "time", 0);
		if (Soar2D.config.bConfig.getContinuous()) {
			velocity = robot.agent.CreateIdWME(self, "velocity");
			{
				speed = robot.agent.CreateFloatWME(velocity, "speed", 0);
				dx = robot.agent.CreateFloatWME(velocity, "dx", 0);
				dy = robot.agent.CreateFloatWME(velocity, "dy", 0);
				rotation = robot.agent.CreateFloatWME(velocity, "rotation", 0);
			}
		}
	}
	
	void createAreaDescription() {
		assert areaDescription == null;
		areaDescription = robot.agent.CreateIdWME(robot.agent.GetInputLink(), "area-description");
	}
	
	Identifier createWallId() {
		assert areaDescription != null;
		return robot.agent.CreateIdWME(areaDescription, "wall");
	}
	
	Identifier createGatewayId() {
		assert areaDescription != null;
		return robot.agent.CreateIdWME(areaDescription, "gateway");
	}
	
	void addWall(BarrierInputLink wall) {
		wallsIL.add(wall);
	}
	
	void addGateway(GatewayInputLink gateway) {
		gatewaysIL.add(gateway);
	}
	
	void addOrUpdatePlayer(Player player, World world, double angleOffDouble) {
		PlayerInputLink pIL = playersIL.get(player);
		PlayersManager players = world.getPlayers();

		double dx = players.getFloatLocation(player).x - players.getFloatLocation(robot).x;
		dx *= dx;
		double dy = players.getFloatLocation(player).y - players.getFloatLocation(robot).y;
		dy *= dy;
		double range = Math.sqrt(dx + dy);
		
		if (pIL == null) {
			// create new player
			Identifier parent = robot.agent.CreateIdWME(robot.agent.GetInputLink(), "player");
			pIL = new PlayerInputLink(robot, parent);
			pIL.initialize(player, world, range, angleOffDouble);
			playersIL.put(player, pIL);
		
		} else {
			if (pIL.area.GetValue() != player.getLocationId()) {
				robot.agent.Update(pIL.area, player.getLocationId());
			}
			if (pIL.row.GetValue() != players.getLocation(player).y) {
				robot.agent.Update(pIL.row, players.getLocation(player).y);
			}
			if (pIL.col.GetValue() != players.getLocation(player).x) {
				robot.agent.Update(pIL.col, players.getLocation(player).x);
			}
			if (Soar2D.config.bConfig.getContinuous()) {
				if (pIL.x.GetValue() != players.getFloatLocation(player).x) {
					robot.agent.Update(pIL.x, players.getFloatLocation(player).x);
				}
				if (pIL.y.GetValue() != players.getFloatLocation(player).y) {
					robot.agent.Update(pIL.y, players.getFloatLocation(player).y);
				}
			}
			if (pIL.range.GetValue() != range) {
				robot.agent.Update(pIL.range, range);
			}
			if (pIL.yaw.GetValue() != angleOffDouble) {
				robot.agent.Update(pIL.yaw, angleOffDouble);
			}
			pIL.touch(world.getWorldCount());
		}
	}
	
	void addOrUpdateObject(GridMap.BookObjectInfo objectInfo, World world, double angleOffDouble) {
		ObjectInputLink oIL = objectsIL.get(objectInfo.object.getIntProperty("object-id"));
		PlayersManager players = world.getPlayers();

		double dx = objectInfo.floatLocation.x - players.getFloatLocation(robot).x;
		dx *= dx;
		double dy = objectInfo.floatLocation.y - players.getFloatLocation(robot).y;
		dy *= dy;
		double range = Math.sqrt(dx + dy);
		
		if (oIL == null) {
			// create new object
			Identifier parent = robot.agent.CreateIdWME(robot.agent.GetInputLink(), "object");
			oIL = new ObjectInputLink(robot, parent);
			oIL.initialize(objectInfo, world, range, angleOffDouble);
			objectsIL.put(objectInfo.object.getIntProperty("object-id"), oIL);
		
		} else {
			if (oIL.area.GetValue() != objectInfo.area) {
				robot.agent.Update(oIL.area, objectInfo.area);
			}
			if (oIL.row.GetValue() != objectInfo.location.y) {
				robot.agent.Update(oIL.row, objectInfo.location.y);
			}
			if (oIL.col.GetValue() != objectInfo.location.x) {
				robot.agent.Update(oIL.col, objectInfo.location.x);
			}
			if (Soar2D.config.bConfig.getContinuous()) {
				if (oIL.x.GetValue() != objectInfo.floatLocation.x) {
					robot.agent.Update(oIL.x, objectInfo.location.x);
				}
				if (oIL.y.GetValue() != objectInfo.floatLocation.y) {
					robot.agent.Update(oIL.y, objectInfo.location.y);
				}
			}
			if (oIL.range.GetValue() != range) {
				robot.agent.Update(oIL.range, range);
			}
			if (oIL.yaw.GetValue() != angleOffDouble) {
				robot.agent.Update(oIL.yaw, angleOffDouble);
			}
			oIL.touch(world.getWorldCount());
		}
	}
	
	void purge(int cycle) {
		Iterator<ObjectInputLink> oiter = objectsIL.values().iterator();
		while (oiter.hasNext()) {
			ObjectInputLink oIL = oiter.next();
			if (oIL.cycleTouched < cycle) {
				if (oIL.cycleTouched > cycle - 3) {
					oIL.makeInvisible();
				} else {
					robot.agent.DestroyWME(oIL.parent);
					oiter.remove();
				}
			}
		}
		
		Iterator<PlayerInputLink> piter = playersIL.values().iterator();
		while (piter.hasNext()) {
			PlayerInputLink pIL = piter.next();
			if (pIL.cycleTouched < cycle) {
				robot.agent.DestroyWME(pIL.parent);
				piter.remove();
			}
		}
		
		Iterator<MessageInputLink> miter = messagesIL.iterator();
		while (miter.hasNext()) {
			MessageInputLink mIL = miter.next();
			if (mIL.cycleCreated < cycle - 3) {
				robot.agent.DestroyWME(mIL.parent);
				miter.remove();
			}
		}
	}
	
	ObjectInputLink getOIL(int id) {
		return objectsIL.get(id);
	}
	
	void destroyAreaDescription() {

		if (areaDescription == null) {
			return;
		}
		robot.agent.DestroyWME(areaDescription);
		areaDescription = null;
		wallsIL = new ArrayList<BarrierInputLink>();
		gatewaysIL = new ArrayList<GatewayInputLink>();	
	}
	
	void destroy() {
		assert self != null;
		robot.agent.DestroyWME(self);
		self = null;
		carry = null;
		robot.agent.DestroyWME(continuous);
		continuous = null;
		
		destroyAreaDescription();

		{
			Iterator<ObjectInputLink> iter = objectsIL.values().iterator();
			while (iter.hasNext()) {
				ObjectInputLink thing = iter.next();
				robot.agent.DestroyWME(thing.parent);
			}
			objectsIL = new HashMap<Integer, ObjectInputLink>();
		}

		{
			Iterator<PlayerInputLink> iter = playersIL.values().iterator();
			while (iter.hasNext()) {
				PlayerInputLink thing = iter.next();
				robot.agent.DestroyWME(thing.parent);
			}
			playersIL = new HashMap<Player, PlayerInputLink>();
		}
		
		{
			Iterator<MessageInputLink> iter = messagesIL.iterator();
			while (iter.hasNext()) {
				MessageInputLink thing = iter.next();
				robot.agent.DestroyWME(thing.parent);
			}
			messagesIL = new HashSet<MessageInputLink>();
		}
	}
	
	void carry(CellObject object) {
		assert carry == null;
		carry = robot.agent.CreateIdWME(self, "carry");
		carryType = robot.agent.CreateStringWME(carry, "type", object.getProperty("id"));
		carryId = robot.agent.CreateIntWME(carry, "id", object.getIntProperty("object-id"));
	}
	
	void drop() {
		assert carry != null;
		robot.agent.DestroyWME(carry);
		carry = null;
	}

	void addMessage(Player player, String message, World world) {
		Identifier parent = robot.agent.CreateIdWME(robot.agent.GetInputLink(), "message");
		MessageInputLink mIL = new MessageInputLink(robot, parent);
		mIL.initialize(player.getName(), message, world);
		messagesIL.add(mIL);
	}
}


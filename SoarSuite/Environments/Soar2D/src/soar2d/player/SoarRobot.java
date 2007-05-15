package soar2d.player;

import java.awt.geom.Point2D;
import java.awt.Point;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.logging.Level;
import java.lang.Math;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import soar2d.Direction;
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.World;
import soar2d.world.CellObject;
import soar2d.world.GridMap;
import soar2d.world.GridMap.Barrier;

class SelfInputLink {
	SoarRobot robot;
	Identifier self;
	Identifier angle;
	FloatElement yaw;
	IntElement area;
	Identifier areaDescription;
	ArrayList<WallInputLink> walls = new ArrayList<WallInputLink>();
	ArrayList<DoorInputLink> doors = new ArrayList<DoorInputLink>();
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
	
	SelfInputLink(SoarRobot robot) {
		this.robot = robot;
	}
	
	void initialize() {
		Identifier il = robot.agent.GetInputLink();

		assert il != null;
		assert self == null;

		self = robot.agent.CreateIdWME(il, "self");
		angle = robot.agent.CreateIdWME(self, "angle");
		{
			yaw = robot.agent.CreateFloatWME(angle, "yaw", robot.getHeadingRadians());
		}
		area = robot.agent.CreateIntWME(self, "area", -1);
		collision = robot.agent.CreateIdWME(self, "collision");
		collisionX = robot.agent.CreateStringWME(self, "collision", "false");
		collisionY = robot.agent.CreateStringWME(self, "collision", "false");
		cycle = robot.agent.CreateIntWME(self, "cycle", 0);
		score = robot.agent.CreateIntWME(self, "score", 0);
		position = robot.agent.CreateIdWME(self, "position");
		{
			x = robot.agent.CreateFloatWME(position, "x", 0);
			y = robot.agent.CreateFloatWME(position, "y", 0);
			row = robot.agent.CreateIntWME(position, "row", 0);
			col = robot.agent.CreateIntWME(position, "col", 0);
		}
		random = robot.agent.CreateFloatWME(self, "random", robot.random);
		time = robot.agent.CreateFloatWME(self, "time", 0);
		velocity = robot.agent.CreateIdWME(self, "velocity");
		{
			speed = robot.agent.CreateFloatWME(velocity, "speed", 0);
			dx = robot.agent.CreateFloatWME(velocity, "dx", 0);
			dy = robot.agent.CreateFloatWME(velocity, "dy", 0);
		}
	}
	
	void createAreaDescription() {
		assert areaDescription == null;
		areaDescription = robot.agent.CreateIdWME(self, "area-description");
	}
	
	Identifier createWallId() {
		assert areaDescription != null;
		return robot.agent.CreateIdWME(areaDescription, "wall");
	}
	
	Identifier createDoorId() {
		assert areaDescription != null;
		return robot.agent.CreateIdWME(areaDescription, "door");
	}
	
	void addWall(WallInputLink wall) {
		walls.add(wall);
	}
	
	void addDoor(DoorInputLink door) {
		doors.add(door);
	}
	
	void destroyAreaDescription() {
		walls = new ArrayList<WallInputLink>();
		doors = new ArrayList<DoorInputLink>();

		if (areaDescription == null) {
			return;
		}
		robot.agent.DestroyWME(areaDescription);
		areaDescription = null;
		
	}
	
	void destroy() {
		assert self != null;
		robot.agent.DestroyWME(self);
		self = areaDescription = null;
		destroyAreaDescription();
	}
}

class WallInputLink {
	SoarRobot robot;
	IntElement id;
	Identifier parent, left, right, center;
	IntElement leftRow, leftCol, rightRow, rightCol;
	FloatElement x, y, angleOff;
	StringElement direction;
	
	WallInputLink(SoarRobot robot, Identifier parent) {
		this.robot = robot;
		this.parent = parent;
	}
	
	void initialize(Barrier barrier) {
		id = robot.agent.CreateIntWME(parent, "id", barrier.id);
		left = robot.agent.CreateIdWME(parent, "left");
		{
			leftRow = robot.agent.CreateIntWME(left, "row", barrier.left.y);
			leftCol = robot.agent.CreateIntWME(left, "col", barrier.left.x);
		}
		right = robot.agent.CreateIdWME(parent, "right");
		{
			rightRow = robot.agent.CreateIntWME(right, "row", barrier.right.y);
			rightCol = robot.agent.CreateIntWME(right, "col", barrier.right.x);
		}
		center = robot.agent.CreateIdWME(parent, "center");
		{
			x = robot.agent.CreateFloatWME(center, "x", barrier.centerpoint().x);
			y = robot.agent.CreateFloatWME(center, "y", barrier.centerpoint().y);
			angleOff = robot.agent.CreateFloatWME(center, "angle-off", 0);
		}
		direction = robot.agent.CreateStringWME(parent, "direction", Direction.stringOf[barrier.direction]);
	}
}

class DoorInputLink extends WallInputLink {
	ArrayList<IntElement> toList = new ArrayList<IntElement>();
	
	DoorInputLink(SoarRobot robot, Identifier parent) {
		super(robot, parent);
	}
	
	void addDest(int id) {
		IntElement dest = robot.agent.CreateIntWME(parent, "to", id);
		toList.add(dest);
	}
}

//class ObjectInputLink {
//	SoarRobot robot;
//	Identifier object, position;
//	FloatElement angleOff, x, y, range;
//	IntElement area, row, col;
//	StringElement type, visible;
//
//	ObjectInputLink(SoarRobot robot) {
//		this.robot = robot;
//	}
//	
//	void initialize() {
//		Identifier il = robot.agent.GetInputLink();
//		assert il != null;
//		
//		object = robot.agent.CreateIdWME(il, "object");
//		angleOff = robot.agent.CreateFloatWME(object, "angle-off", 0.0);
//		area = robot.agent.CreateIntWME(object, "area", -1);
//		type = robot.agent.CreateStringWME(object, "type", "None");
//		position = robot.agent.CreateIdWME(object, "position");
//		{
//			x = robot.agent.CreateFloatWME(position, "x", 0);
//			y = robot.agent.CreateFloatWME(position, "y", 0);
//			row = robot.agent.CreateIntWME(position, "row", 0);
//			col = robot.agent.CreateIntWME(position, "col", 0);
//		}
//		range = robot.agent.CreateFloatWME(object, "range", 0.0);
//		visible = robot.agent.CreateStringWME(object, "visible", "false");
//	}
//}

public class SoarRobot extends Robot {
	Agent agent;	// the soar agent
	float random;	// a random number, guaranteed to change every frame
	private boolean fragged = true;
	
	SelfInputLink selfIL;
	
	private ArrayList<String> shutdownCommands;	// soar commands to run before this agent is destroyed

	private int locationId = -1;
	
	/**
	 * @param agent a valid soar agent
	 * @param playerConfig the rest of the player config
	 */
	public SoarRobot(Agent agent, PlayerConfig playerConfig) {
		super(playerConfig);
		this.agent = agent;
		this.shutdownCommands = playerConfig.getShutdownCommands();
		
		previousLocation = new java.awt.Point(-1, -1);
		
		random = 0;
		generateNewRandom();

		// create and initialize self input link
		selfIL = new SelfInputLink(this);
		selfIL.initialize();
		
		if (!agent.Commit()) {
			Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
			Soar2D.control.stopSimulation();
		}
	}
	
	/**
	 * create a new random number
	 * make sure it is different from current
	 */
	private void generateNewRandom() {
		float newRandom;
		do {
			newRandom = Simulation.random.nextFloat();
		} while (this.random == newRandom);
		this.random = newRandom;
	}
	
	public void update(World world, java.awt.Point location) {
		
		// check to see if we've moved
		super.update(world, location);
		
		// if we've been fragged, set move to true
		if (fragged) {
			moved = true;
		}
		
		GridMap map = world.getMap();
		
		// if we moved
		if (moved) {
			// check if we're in a new location
			ArrayList<CellObject> locationObjects = map.getAllWithProperty(location, Names.kPropertyNumber);
			if (locationObjects.size() != 1) {
				logger.warning("Location objects greater or less than 1");
				locationId = -1;
				agent.Update(selfIL.area, locationId);

				// destroy old area information
				if (selfIL.areaDescription != null) {
					agent.DestroyWME(selfIL.areaDescription);
					selfIL.areaDescription = null;
				}

			} else {
				int newLocationId = locationObjects.get(0).getIntProperty(Names.kPropertyNumber);

				if (newLocationId != locationId) {
					locationId = newLocationId;
					agent.Update(selfIL.area, locationId);

					// destroy old area information
					if (selfIL.areaDescription != null) {
						agent.DestroyWME(selfIL.areaDescription);
						selfIL.areaDescription = null;
					}
					
					// create new area information
					ArrayList<Barrier> barrierList = world.getMap().getRoomBarrierList(locationId);
					assert barrierList != null;
					assert barrierList.size() > 0;
					
					// this is, in fact, a room
					selfIL.createAreaDescription();
					
					Iterator<Barrier> iter = barrierList.iterator();
					while(iter.hasNext()) {
						Barrier barrier = iter.next();
						if (barrier.door) {
							// door
							DoorInputLink door = new DoorInputLink(this, selfIL.createDoorId());
							door.initialize(barrier);
							
							// add destinations
							ArrayList<Integer> doorDestList = world.getMap().getDoorDestinationList(barrier.id);
							Iterator<Integer> destIter = doorDestList.iterator();
							while(destIter.hasNext()) {
								door.addDest(destIter.next().intValue());
							}
							
							Point2D.Double myVector = new Point2D.Double();
							myVector.x = world.getFloatLocation(this).x;
							myVector.y = world.getFloatLocation(this).y;
							
							Point2D.Double doorVector = new Point2D.Double();
							doorVector.x = barrier.centerpoint().x;
							doorVector.y = barrier.centerpoint().y;
							
							// translate door so i'm the origin
							doorVector.x -= myVector.x;
							doorVector.y -= myVector.y;
							
							// make door unit vector
							double doorVectorLength = Math.sqrt(Math.pow(doorVector.x, 2) + Math.pow(doorVector.y, 2));
							assert doorVectorLength > 0;
							doorVector.x /= doorVectorLength;
							doorVector.y /= doorVectorLength;
							
							// make my facing vector
							myVector.x = Math.cos(getHeadingRadians());
							myVector.y = Math.sin(getHeadingRadians());
							
							double dotProduct = (doorVector.x * myVector.x) + (doorVector.y * myVector.y);
							double crossProduct = (doorVector.x * myVector.y) - (doorVector.y * myVector.x);
							
							// calculate inverse cosine of that for angle
							if (crossProduct < 0) {
								agent.Update(door.angleOff, Math.acos(dotProduct));
							} else {
								agent.Update(door.angleOff, Math.acos(dotProduct) * -1);
							}

							selfIL.addDoor(door);
							
						} else {
							// wall
							WallInputLink wall = new WallInputLink(this, selfIL.createWallId());
							wall.initialize(barrier);
							selfIL.addWall(wall);
						}
					}
				}
			}

			// update the location
			agent.Update(selfIL.col, location.x);
			agent.Update(selfIL.row, location.y);
			
			Point2D.Double floatLocation = world.getFloatLocation(this);
			agent.Update(selfIL.x, floatLocation.x);
			agent.Update(selfIL.y, floatLocation.y);
			
			// and heading
			agent.Update(selfIL.yaw, getHeadingRadians());
			
			// update the clock
			agent.Update(selfIL.cycle, world.getWorldCount());
		}
		
		// velocity
		agent.Update(selfIL.speed, getSpeed());
		agent.Update(selfIL.dx, getVelocity().x);
		agent.Update(selfIL.dy, getVelocity().y);
		
		// update the random no matter what
		float oldrandom = random;
		do {
			random = Simulation.random.nextFloat();
		} while (random == oldrandom);
		agent.Update(selfIL.random, random);
		
		// commit everything
		if (!agent.Commit()) {
			Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
			Soar2D.control.stopSimulation();
		}
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Eater#getMove()
	 */
	public MoveInfo getMove() {
		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			if (logger.isLoggable(Level.FINER)) logger.finer(getName() + " issued no command.");
			return new MoveInfo();
		}

		// go through the commands
		MoveInfo move = new MoveInfo();
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();
			
			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (move.move) {
					logger.warning(getName() + " multiple move commands issued");
					IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
					continue;
				}
				
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				
				if (direction == null) {
					logger.warning(getName() + " move command missing direction parameter");
					IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
					continue;
				}
				
				if (direction.equalsIgnoreCase(Names.kForwardID)) {
					move.forward = true;
					
				} else if (direction.equalsIgnoreCase(Names.kBackwardID)) {
					move.backward = true;
				
				} else if (direction.equalsIgnoreCase(Names.kStopID)) {
					move.forward = true;
					move.backward = true;
				
				} else {
					logger.warning(getName() + "unrecognized move direction: " + direction);
					IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
					continue;
				}
				move.move = true;

			} else if (commandName.equalsIgnoreCase(Names.kRotateID)) {
				if (move.rotate) {
					logger.warning(getName() + " multiple rotate commands issued");
					IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
					continue;
				}
				
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				
				if (direction == null) {
					logger.warning(getName() + " rotate command missing direction parameter");
					IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
					continue;
				}
				
				if (direction.equalsIgnoreCase(Names.kLeftID)) {
					move.rotateDirection = Names.kRotateLeft;
					
				} else if (direction.equalsIgnoreCase(Names.kRightID)) {
					move.rotateDirection = Names.kRotateRight;
				
				} else if (direction.equalsIgnoreCase(Names.kStopID)) {
					move.rotateDirection = Names.kRotateStop;
				
				} else {
					logger.warning(getName() + "unrecognized rotate direction: " + direction);
					IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
					continue;
					
				}

				move.rotate = true;

			} else if (commandName.equalsIgnoreCase(Names.kStopSimID)) {
				if (move.stopSim) {
					logger.warning(getName() + " multiple stop-sim commands issued");
					IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
					continue;
				}
				move.stopSim = true;
				
			} else if (commandName.equalsIgnoreCase(Names.kRotateAbsoluteID)) {
				if (move.rotateAbsolute) {
					logger.warning(getName() + " multiple rotate-absolute commands issued");
					IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
					continue;
				}
				
				String yawString = commandId.GetParameterValue("yaw");
				if (yawString == null) {
					logger.warning(getName() + " rotate-absolute command missing yaw parameter");
					IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
					continue;
				}
				
				try {
					move.rotateAbsoluteHeading = Double.parseDouble(yawString);
				} catch (NumberFormatException e) {
					logger.warning(getName() + " rotate-absolute yaw parameter improperly formatted");
					IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
					continue;
				}

				move.rotateAbsolute = true;
				
			} else if (commandName.equalsIgnoreCase(Names.kRotateRelativeID)) {
				if (move.rotateRelative) {
					logger.warning(getName() + " multiple rotate-relative commands issued");
					IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
					continue;
				}
				
				String amountString = commandId.GetParameterValue("amount");
				if (amountString == null) {
					logger.warning(getName() + " rotate-relative command missing amount parameter");
					IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
					continue;
				}
				
				try {
					move.rotateRelativeAmount = Double.parseDouble(amountString);
				} catch (NumberFormatException e) {
					logger.warning(getName() + " rotate-relative amount parameter improperly formatted");
					IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
					continue;
				}

				move.rotateRelative = true;
				
			} else {
				logger.warning("Unknown command: " + commandName);
				IOLinkUtility.CreateOrAddStatus(agent, commandId, "error");
				continue;
			}
			
			IOLinkUtility.CreateOrAddStatus(agent, commandId, "complete");

		}
		agent.ClearOutputLinkChanges();
		if (!agent.Commit()) {
			Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
			Soar2D.control.stopSimulation();
		}
		return move;
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Player#reset()
	 */
	public void reset() {
		super.reset();
		fragged = false;
		moved = true;
		locationId = -1;
		
		if (agent == null) {
			return;
		}
		
		selfIL.destroy();

		if (!agent.Commit()) {
			Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
			Soar2D.control.stopSimulation();
		}

		agent.InitSoar();

		selfIL.initialize();
		agent.ClearOutputLinkChanges();
		if (!agent.Commit()) {
			Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
			Soar2D.control.stopSimulation();
		}
		
	}

	public void fragged() {
		fragged = true;
	}

	/* (non-Javadoc)
	 * @see soar2d.player.Eater#shutdown()
	 */
	public void shutdown() {
		assert agent != null;
		if (shutdownCommands != null) { 
			// execute the pre-shutdown commands
			Iterator<String> iter = shutdownCommands.iterator();
			while(iter.hasNext()) {
				String command = iter.next();
				String result = getName() + ": result: " + agent.ExecuteCommandLine(command, true);
				Soar2D.logger.info(getName() + ": shutdown command: " + command);
				if (agent.HadError()) {
					Soar2D.control.severeError(result);
				} else {
					Soar2D.logger.info(getName() + ": result: " + result);
				}
			}
		}
	}

}

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
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.World;
import soar2d.world.CellObject;
import soar2d.world.GridMap;
import soar2d.world.GridMap.Barrier;

class SelfInputLink {
	SoarRobot robot;
	Identifier self, angle, areaDescription, position, velocity;
	FloatElement yaw, x, y, random, speed, dx, dy;
	IntElement area, cycle, score, row, col;
	StringElement name, type;
	
	ArrayList<WallInputLink> walls = new ArrayList<WallInputLink>();
	ArrayList<DoorInputLink> doors = new ArrayList<DoorInputLink>();
	
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
			yaw = robot.agent.CreateFloatWME(angle, "yaw", 0.0);
		}
		area = robot.agent.CreateIntWME(self, "area", -1);
		cycle = robot.agent.CreateIntWME(self, "cycle", 0);
		score = robot.agent.CreateIntWME(self, "score", 0);
		name = robot.agent.CreateStringWME(self, "name", robot.agent.GetAgentName());
		position = robot.agent.CreateIdWME(self, "position");
		{
			x = robot.agent.CreateFloatWME(position, "x", 0);
			y = robot.agent.CreateFloatWME(position, "y", 0);
			row = robot.agent.CreateIntWME(position, "row", 0);
			col = robot.agent.CreateIntWME(position, "col", 0);
		}
		random = robot.agent.CreateFloatWME(self, "random", robot.random);
		velocity = robot.agent.CreateIdWME(self, "velocity");
		{
			speed = robot.agent.CreateFloatWME(velocity, "speed", 0.0);
			dx = robot.agent.CreateFloatWME(velocity, "dx", 0.0);
			dy = robot.agent.CreateFloatWME(velocity, "dy", 0.0);
		}
	}
	
	void createAreaDescription(String typeString) {
		assert areaDescription == null;
		areaDescription = robot.agent.CreateIdWME(self, "area-description");
		type = robot.agent.CreateStringWME(areaDescription, "type", typeString);
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
	Identifier parent, left, right;
	IntElement leftRow, leftCol, rightRow, rightCol;
	
	WallInputLink(SoarRobot robot, Identifier parent) {
		this.robot = robot;
		this.parent = parent;
	}
	
	void initialize(int idInt, Point leftPoint, Point rightPoint) {
		id = robot.agent.CreateIntWME(parent, "id", idInt);
		left = robot.agent.CreateIdWME(parent, "left");
		{
			leftRow = robot.agent.CreateIntWME(left, "row", leftPoint.y);
			leftCol = robot.agent.CreateIntWME(left, "col", leftPoint.x);
		}
		right = robot.agent.CreateIdWME(parent, "right");
		{
			rightRow = robot.agent.CreateIntWME(right, "row", rightPoint.y);
			rightCol = robot.agent.CreateIntWME(right, "col", rightPoint.x);
		}
	}
}

class DoorInputLink extends WallInputLink {
	ArrayList<IntElement> toList = new ArrayList<IntElement>();
	
	DoorInputLink(SoarRobot robot, Identifier parent) {
		super(robot, parent);
	}
	
	void initialize(int idInt, Point leftPoint, Point rightPoint) {
		super.initialize(idInt, leftPoint, rightPoint);
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
					
					selfIL.createAreaDescription("room");

					// create new area information
					ArrayList<Barrier> barrierList = world.getMap().getRoomBarrierList(locationId);
					if (barrierList.size() > 0) {
						// this is, in fact, a room
						
						Iterator<Barrier> iter = barrierList.iterator();
						while(iter.hasNext()) {
							Barrier barrier = iter.next();
							if (barrier.direction == null) {
								// door
								DoorInputLink door = new DoorInputLink(this, selfIL.createDoorId());
								door.initialize(barrier.id, barrier.left, barrier.right);
								selfIL.addDoor(door);
								
							} else {
								// wall
								WallInputLink wall = new WallInputLink(this, selfIL.createWallId());
								wall.initialize(barrier.id, barrier.left, barrier.right);
								//agent.CreateStringWME(barrierWME, Names.kDirectionID, barrier.direction);
								selfIL.addWall(wall);
							}
						}

					} else {
						// we're in a door
						// TODO: create door and pass it
						selfIL.createAreaDescription("door");
					}
					
				}
			}

			// update the location
			agent.Update(selfIL.col, location.x);
			agent.Update(selfIL.row, location.y);
			
			Point2D.Float floatLocation = world.getFloatLocation(this);
			agent.Update(selfIL.x, floatLocation.x);
			agent.Update(selfIL.y, floatLocation.y);
			
			// and heading
			double heading = Math.toDegrees(getHeadingRadians());
			int headingInt = (int)heading;
			agent.Update(selfIL.yaw, headingInt);
			
			// update the clock
			agent.Update(selfIL.cycle, world.getWorldCount());
		}
		
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
		// see move info for details
		MoveInfo move = new MoveInfo();
		boolean moveWait = false;
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();
			
			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (move.move || moveWait) {
					logger.warning(getName() + "multiple move commands detected");
					continue;
				}
				
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				if (direction != null) {
					if (direction.equals(Names.kNone)) {
						// legal wait
						moveWait = true;
						commandId.AddStatusComplete();
						continue;
					} else {
						if (direction.equalsIgnoreCase(Names.kForwardID)) {
							move.forward = true;
							
						} else if (direction.equalsIgnoreCase(Names.kBackwardID)) {
							move.backward = true;
						
						} else {
							logger.warning(getName() + "unknown move direction: " + direction);
							continue;
							
						}
						commandId.AddStatusComplete();
						continue;
					}
				}
				
			} else if (commandName.equalsIgnoreCase(Names.kRotateID)) {
				if (move.rotate) {
					logger.warning(getName() + "multiple rotate commands detected, ignoring");
					continue;
				}
				move.rotate = true;

				String direction = commandId.GetParameterValue(Names.kDirectionID);
				if (direction != null) {
					if (direction.equalsIgnoreCase(Names.kLeftID)) {
						move.rotateDirection = Names.kRotateLeft;
						
					} else if (direction.equalsIgnoreCase(Names.kRightID)) {
						move.rotateDirection = Names.kRotateRight;
					
					} else {
						logger.warning(getName() + "unknown move direction: " + direction);
						continue;
						
					}
					commandId.AddStatusComplete();
					continue;
				}

			} else if (commandName.equalsIgnoreCase(Names.kStopSimID)) {
				if (move.stopSim) {
					logger.warning(getName() + "multiple stop commands detected, ignoring");
					continue;
				}
				move.stopSim = true;
				commandId.AddStatusComplete();
				continue;
				
			} else {
				logger.warning("Unknown command: " + commandName);
				continue;
			}
			
			logger.warning("Improperly formatted command: " + commandName);
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

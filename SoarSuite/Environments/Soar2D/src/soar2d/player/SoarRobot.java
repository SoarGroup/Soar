package soar2d.player;

import java.awt.geom.Point2D;
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

public class SoarRobot extends Robot {
	private Agent agent;	// the soar agent
	private float random;	// a random number, guaranteed to change every frame
	private boolean fragged = true;
	
	private IntElement xWME;	// our current x location
	private IntElement yWME;	// our current y location
	private FloatElement floatxWME;	// our current x exact location
	private FloatElement floatyWME;	// our current y exact location
	private FloatElement randomWME;	// the wme for the random number
	private IntElement clockWME;	// the world count
	private IntElement headingWME;	// current heading, degrees
	private IntElement inWME;		// ID of current room
	
	private Identifier roomWME; // location identifier
	//private ArrayList<Identifier> barrierWMEs; // location identifier

	
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
		
		Identifier il = agent.GetInputLink();
		
		agent.CreateStringWME(il, Names.kNameID, getName());
		xWME = agent.CreateIntWME(il, Names.kXID, 0);
		yWME = agent.CreateIntWME(il, Names.kYID, 0);
		floatxWME = agent.CreateFloatWME(il, Names.kFloatXID, 0);
		floatyWME = agent.CreateFloatWME(il, Names.kFloatYID, 0);
		headingWME = agent.CreateIntWME(il, Names.kHeadingID, 0);
		inWME = agent.CreateIntWME(il, Names.kInID, -1);
		
		clockWME = agent.CreateIntWME(il, Names.kClockID, 0);
		
		random = 0;
		generateNewRandom();
		randomWME = agent.CreateFloatWME(agent.GetInputLink(), Names.kRandomID, random);
		
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
	
	private void destroyLocation() {
		if (roomWME == null) {
			return;
		}
		agent.DestroyWME(roomWME);
		
		roomWME = null;
		//barrierWMEs = null;
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Eater#update(soar2d.World, java.awt.Point)
	 */
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
				locationId = -1;
				destroyLocation();

				agent.Update(inWME, locationId);

			} else {
				int newLocationId = locationObjects.get(0).getIntProperty(Names.kPropertyNumber);

				if (newLocationId != locationId) {
					locationId = newLocationId;

					destroyLocation();

					// CREATE/UPDATE NEW LOCATION INFO
					ArrayList<Barrier> barrierList = world.getMap().getRoomBarrierList(locationId);
					if (barrierList.size() > 0) {
						// this is, in fact, a room
						roomWME = agent.CreateIdWME(agent.GetInputLink(), Names.kRoomID);
						//barrierWMEs = new ArrayList<Identifier>();
						
						agent.CreateIntWME(roomWME, Names.kIdID, locationId);
						Iterator<Barrier> iter = barrierList.iterator();
						while(iter.hasNext()) {
							Barrier barrier = iter.next();
							Identifier barrierWME = null;
							if (barrier.direction == null) {
								// door
								barrierWME = agent.CreateIdWME(roomWME, Names.kDoorID);
								
							} else {
								// wall
								barrierWME = agent.CreateIdWME(roomWME, Names.kWallID);
								agent.CreateStringWME(barrierWME, Names.kDirectionID, barrier.direction);
							}
							agent.CreateIntWME(barrierWME, Names.kIdID, barrier.id);
							Identifier leftWME = agent.CreateIdWME(barrierWME, Names.kLeftID);
							agent.CreateIntWME(leftWME, Names.kXID, barrier.left.x);
							agent.CreateIntWME(leftWME, Names.kYID, barrier.left.y);
							Identifier rightWME = agent.CreateIdWME(barrierWME, Names.kRightID);
							agent.CreateIntWME(rightWME, Names.kXID, barrier.right.x);
							agent.CreateIntWME(rightWME, Names.kYID, barrier.right.y);
							
							//barrierWMEs.add(barrierWME);
						}
					} else {
						// we're in a door
					}

					agent.Update(inWME, locationId);
				}
			}

			// update the location
			agent.Update(xWME, location.x);
			agent.Update(yWME, location.y);
			
			Point2D.Float floatLocation = world.getFloatLocation(this);
			agent.Update(floatxWME, floatLocation.x);
			agent.Update(floatyWME, floatLocation.y);
			
			// and heading
			double heading = Math.toDegrees(getHeadingRadians());
			int headingInt = (int)heading;
			agent.Update(headingWME, headingInt);
			
			// update the clock
			agent.Update(clockWME, world.getWorldCount());
		}
		
		// update the random no matter what
		float oldrandom = random;
		do {
			random = Simulation.random.nextFloat();
		} while (random == oldrandom);
		agent.Update(randomWME, random);
		
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

		if (!agent.Commit()) {
			Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
			Soar2D.control.stopSimulation();
		}

		agent.InitSoar();
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

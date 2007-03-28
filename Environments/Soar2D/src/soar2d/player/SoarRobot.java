package soar2d.player;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.logging.Level;

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

public class SoarRobot extends Robot {
	private Agent agent;	// the soar agent
	private float random;	// a random number, guaranteed to change every frame
	private boolean fragged = false;
	
	private IntElement xWME;	// our current x location
	private IntElement yWME;	// our current y location
	private FloatElement randomWME;	// the wme for the random number
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
				// DESTROY OLD LOCATION INFO
			} else {
				int newLocationId = locationObjects.get(0).getIntProperty(Names.kPropertyNumber);

				if (newLocationId != locationId) {
					// DESTROY OLD LOCATION INFO
					locationId = newLocationId;
					
					// CREATE/UPDATE NEW LOCATION INFO
				}
			}
			
			// update the location
			agent.Update(xWME, location.x);
			agent.Update(yWME, location.y);
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

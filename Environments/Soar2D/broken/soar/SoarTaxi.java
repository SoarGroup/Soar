package broken.soar;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import soar2d.Direction;
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.map.TaxiMap;
import soar2d.players.CommandInfo;
import soar2d.players.Taxi;
import soar2d.world.World;

/**
 * @author voigtjr
 *
 */
public class SoarTaxi extends Taxi {
	private static Logger logger = Logger.getLogger(SoarTaxi.class);

	/**
	 * the soar agent
	 */
	private Agent agent;
	/**
	 * a random number, guaranteed to change every frame
	 */
	private float random;
	
	private Identifier self;
	
	private IntElement xWME;
	private IntElement yWME;
	
	private FloatElement randomWME;
	private IntElement reward;
	private StringElement passenger;
	private StringElement destination;
	private IntElement fuel;
	private StringElement formerDestination;
	
	private Identifier view;
	
	private StringElement northType;
	private StringElement northWall;
	private StringElement northPassenger;
	
	private StringElement southType;
	private StringElement southWall;
	private StringElement southPassenger;
	
	private StringElement eastType;
	private StringElement eastWall;
	private StringElement eastPassenger;
	
	private StringElement westType;
	private StringElement westWall;
	private StringElement westPassenger;
	
	private Identifier cell;
	
	private StringElement cellType;
	private StringElement cellPassenger;

	/**
	 * soar commands to run before this agent is destroyed
	 */
	private String [] shutdownCommands;

	InputLinkMetadata metadata;

	/**
	 * @param agent a valid soar agent
	 * @param playerConfig the rest of the player config
	 */
	public SoarTaxi(Agent agent, String playerId) {
		super(playerId);

		this.agent = agent;
		this.shutdownCommands = playerConfig.shutdown_commands;
		
		initInputLink();
		
		if (!agent.Commit()) {
			error(Names.Errors.commitFail + this.getName());
			Soar2D.control.stopSimulation();
		}
	}
	
	private void error(String message) {
		logger.error(message);
		Soar2D.control.errorPopUp(message);
	}
	
	private void initInputLink() {
		self = agent.CreateIdWME(agent.GetInputLink(), "self");
		
		Identifier position = agent.CreateIdWME(self, "position");
		xWME = agent.CreateIntWME(position, Names.kXID, 0);
		yWME = agent.CreateIntWME(position, Names.kYID, 0);

		reward = agent.CreateIntWME(self, "reward", 0);
		
		random = 0;
		generateNewRandom();
		randomWME = agent.CreateFloatWME(self, Names.kRandomID, random);
		
		passenger = agent.CreateStringWME(self, "passenger", "false");
		formerDestination = agent.CreateStringWME(self, "former-destination", "none");

		fuel = agent.CreateIntWME(self, "fuel", 0);
		
		view = agent.CreateIdWME(agent.GetInputLink(), "view");

		Identifier north = agent.CreateIdWME(view, "north");
		northType = agent.CreateStringWME(north, "type", "none");
		northPassenger = agent.CreateStringWME(north, "passenger", "false");
		northWall = agent.CreateStringWME(north, "wall", "false");
		
		Identifier south = agent.CreateIdWME(view, "south");
		southType = agent.CreateStringWME(south, "type", "none");
		southPassenger = agent.CreateStringWME(south, "passenger", "false");
		southWall = agent.CreateStringWME(south, "wall", "false");

		Identifier east = agent.CreateIdWME(view, "east");
		eastType = agent.CreateStringWME(east, "type", "none");
		eastPassenger = agent.CreateStringWME(east, "passenger", "false");
		eastWall = agent.CreateStringWME(east, "wall", "false");

		Identifier west = agent.CreateIdWME(view, "west");
		westType = agent.CreateStringWME(west, "type", "none");
		westPassenger = agent.CreateStringWME(west, "passenger", "false");
		westWall = agent.CreateStringWME(west, "wall", "false");

		cell = agent.CreateIdWME(agent.GetInputLink(), "cell");
		cellType = agent.CreateStringWME(cell, "type", "none");
		cellPassenger = agent.CreateStringWME(cell, "passenger", "false");
		
		metadata = InputLinkMetadata.load(agent);
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
	
	public void update(int [] location) {
		World world = Soar2D.simulation.world;

		// check to see if we've moved
		super.update(location);
		
		// if we moved, update the location
		if (moved) {
			agent.Update(xWME, location[0]);
			int newY = Soar2D.simulation.world.getMap().getSize() - 1 - location[1];
			agent.Update(yWME, newY);
		}
		
		if (pointsChanged()) {
			agent.Update(reward, getPointsDelta());
		} else {
			agent.Update(reward, 0);
		}
		
		TaxiMap xMap = (TaxiMap)world.getMap();
		if (xMap.isPassengerCarried()) {
			if (!Boolean.parseBoolean(passenger.GetValueAsString())) {
				agent.Update(passenger, "true");
			}
			if (destination == null) {
				destination = agent.CreateStringWME(self, "destination", xMap.getPassengerDestination());
			}
			
		} else {
			if (Boolean.parseBoolean(passenger.GetValueAsString())) {
				agent.Update(passenger, "false");
			}
			if (destination != null) {
				agent.DestroyWME(destination);
				destination = null;
			}
		}

		String dest = xMap.getPassengerFormerDestination();
		if (dest == null) {
			if (!formerDestination.GetValueAsString().equals("none")) {
				agent.Update(formerDestination, "none");
			}
		} else {
			if (!formerDestination.GetValueAsString().equals(dest)) {
				agent.Update(formerDestination, dest);
			}
		}

		if (fuel.GetValue() != xMap.getFuel()) {
			agent.Update(fuel, xMap.getFuel());
		}
		
		// cell
		if (!cellType.GetValueAsString().equals(xMap.getStringType(location))) {
			agent.Update(cellType, xMap.getStringType(location));
		}
		
		if (xMap.getObject(location, "passenger") != null) {
			if (!Boolean.parseBoolean(cellPassenger.GetValueAsString())) {
				agent.Update(cellPassenger, "true");
			}
			
		} else {
			if (Boolean.parseBoolean(cellPassenger.GetValueAsString())) {
				agent.Update(cellPassenger, "false");
			}
		}
		
		// view
		updateView(location, xMap, Direction.NORTH, northType, northPassenger, northWall);
		updateView(location, xMap, Direction.SOUTH, southType, southPassenger, southWall);
		updateView(location, xMap, Direction.EAST, eastType, eastPassenger, eastWall);
		updateView(location, xMap, Direction.WEST, westType, westPassenger, westWall);
		
		// update the random no matter what
		float oldrandom = random;
		do {
			random = Simulation.random.nextFloat();
		} while (random == oldrandom);
		agent.Update(randomWME, random);
		
		// commit everything
		if (!agent.Commit()) {
			error(Names.Errors.commitFail + this.getName());
			Soar2D.control.stopSimulation();
		}
		//this.resetPointsChanged();
	}
	
	private void updateView(int [] location, TaxiMap xMap, Direction direction, StringElement type, StringElement passenger, StringElement wall) {
		int [] tempLocation = Direction.translate(location, direction, new int[2]);
		
		
		if (!type.GetValueAsString().equals(xMap.getStringType(tempLocation))) {
			agent.Update(type, xMap.getStringType(tempLocation));
		}
		
		if (xMap.isInBounds(tempLocation)) {
			if (xMap.getObject(tempLocation, "passenger") != null) {
				if (!Boolean.parseBoolean(passenger.GetValueAsString())) {
					agent.Update(passenger, "true");
				}
				
			} else {
				if (Boolean.parseBoolean(passenger.GetValueAsString())) {
					agent.Update(passenger, "false");
				}
			}
		} else {
			if (Boolean.parseBoolean(passenger.GetValueAsString())) {
				agent.Update(passenger, "false");
			}
		}

		if (xMap.wall(location, direction)) {
			if (!Boolean.parseBoolean(wall.GetValueAsString())) {
				agent.Update(wall, "true");
			}
			
		} else {
			if (Boolean.parseBoolean(wall.GetValueAsString())) {
				agent.Update(wall, "false");
			}
		}
	}
	
	public CommandInfo getMove() {
		if (Soar2D.config.generalConfig().force_human) {
			return super.getMove();
		}
		
		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			if (logger.isDebugEnabled()) {
				logger.debug(getName() + " issued no command.");
			}
			return new CommandInfo();
		}

		// go through the commands
		// see move info for details
		CommandInfo move = new CommandInfo();
		boolean moveWait = false;
		if (agent.GetNumberCommands() > 1) {
			logger.warn(getName() + ": " + agent.GetNumberCommands() 
					+ " commands detected, all but the first will be ignored");
		}
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();
			
			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (move.move || moveWait) {
					logger.warn(getName() + ": multiple move commands detected");
					commandId.AddStatusError();
					continue;
				}
				move.move = true;
				
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				if (direction != null) {
					if (direction.equals(Names.kNone)) {
						// legal wait
						move.move = false;
						moveWait = true;
						commandId.AddStatusComplete();
						continue;
					} else {
						move.moveDirection = Direction.parse(direction); 
						this.setFacing(move.moveDirection);
						commandId.AddStatusComplete();
						continue;
					}
				}
				
			} else if (commandName.equalsIgnoreCase(Names.kStopSimID)) {
				if (move.stopSim) {
					logger.warn(getName() + ": multiple stop commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.stopSim = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kPickUpID)) {
				if (move.pickup) {
					logger.warn(getName() + ": multiple " + Names.kPickUpID + " commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.pickup = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kPutDownID)) {
				if (move.putdown) {
					logger.warn(getName() + ": multiple " + Names.kPutDownID + " commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.putdown = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kFillUpID)) {
				if (move.fillup) {
					logger.warn(getName() + ": multiple " + Names.kFillUpID + " commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.fillup = true;
				commandId.AddStatusComplete();
				continue;
				
			} else {
				logger.warn("Unknown command: " + commandName);
				commandId.AddStatusError();
				continue;
			}
			
			logger.warn("Improperly formatted command: " + commandName);
			commandId.AddStatusError();
		}
		agent.ClearOutputLinkChanges();
		if (!agent.Commit()) {
			error(Names.Errors.commitFail + this.getName());
			Soar2D.control.stopSimulation();
		}
		return move;
	}
	
	public void reset() {
		super.reset();
		
		if (agent == null) {
			return;
		}
		
		agent.DestroyWME(self);
		agent.DestroyWME(view);
		agent.DestroyWME(cell);
		destination = null;
		
		metadata.destroy();
		metadata = null;

		this.initInputLink();

		if (!agent.Commit()) {
			error(Names.Errors.commitFail + this.getName());
			Soar2D.control.stopSimulation();
		}
	}

	public void shutdown() {
		assert agent != null;
		if (shutdownCommands != null) { 
			// execute the pre-shutdown commands
			for (String command : shutdownCommands) {
				String result = getName() + ": result: " + agent.ExecuteCommandLine(command, true);
				logger.info(getName() + ": shutdown command: " + command);
				if (agent.HadError()) {
					error(result);
				} else {
					logger.info(getName() + ": result: " + result);
				}
			}
		}
	}
}

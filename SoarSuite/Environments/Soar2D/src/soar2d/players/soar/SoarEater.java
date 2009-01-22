package soar2d.players.soar;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;
import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.players.Eater;
import soar2d.players.MoveInfo;
import soar2d.world.World;

/**
 * @author voigtjr
 *
 * the soar eater class is a human eater except controlled by soar instead of
 * human input.  someday may share code with eater so we're extending that for
 * now
 */
public class SoarEater extends Eater {
	private static Logger logger = Logger.getLogger(SoarEater.class);

	private SoarEaterIL input;
	
	/**
	 * the soar agent
	 */
	private Agent agent;
	
	/**
	 * soar commands to run before this agent is destroyed
	 */
	private String [] shutdownCommands;

	/**
	 *  Set to true when the eater collides with someone and is teleported (to possibly
	 *  the same location)
	 */
	boolean fragged = false;
	
	InputLinkMetadata metadata;

	/**
	 * @param agent a valid soar agent
	 * @param playerConfig the rest of the player config
	 */
	public SoarEater(Agent agent, String playerId) {
		super(playerId);

		this.agent = agent;
		agent.SetBlinkIfNoChange(false);
		
		this.shutdownCommands = playerConfig.shutdown_commands;
		
		input = new SoarEaterIL(agent, Soar2D.config.eatersConfig().vision);
		try {
			input.create(getName(), getPoints());
		} catch (CommitException e) {
			error(Names.Errors.commitFail + this.getName());
			Soar2D.control.stopSimulation();
		}
		
		metadata = InputLinkMetadata.load(agent);
		
		if (!agent.Commit()) {
			error(Names.Errors.commitFail + this.getName());
			Soar2D.control.stopSimulation();
		}
	}
	
	private void error(String message) {
		logger.error(message);
		Soar2D.control.errorPopUp(message);
	}
	
	public void update(int [] location) {
		World world = Soar2D.simulation.world;

		// check to see if we've moved
		super.update(location);
		
		// if we've been fragged, set move to true
		if (fragged) {
			moved = true;
		}
		
		try {
			input.update(moved, location, world.getMap(), getPoints());
		} catch (CommitException e) {
			error(Names.Errors.commitFail + this.getName());
			Soar2D.control.stopSimulation();
		}
		
		// commit everything
		if (!agent.Commit()) {
			error(Names.Errors.commitFail + this.getName());
			Soar2D.control.stopSimulation();
		}
		this.resetPointsChanged();

	}
	
	public MoveInfo getMove() {
		if (Soar2D.config.generalConfig().force_human) {
			return super.getMove();
		}

		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			logger.debug(getName() + " issued no command.");
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
					logger.warn(getName() + ": multiple move/jump commands detected (move)");
					continue;
				}
				move.move = true;
				move.jump = false;
				
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				if (direction != null) {
					if (direction.equals(Names.kNone)) {
						// legal wait
						move.move = false;
						moveWait = true;
						commandId.AddStatusComplete();
						continue;
					} else {
						move.moveDirection = Direction.getInt(direction); 
						this.setFacingInt(move.moveDirection);
						commandId.AddStatusComplete();
						continue;
					}
				}
				
			} else if (commandName.equalsIgnoreCase(Names.kJumpID)) {
				if (move.move) {
					logger.warn(getName() + ": multiple move/jump commands detected, ignoring (jump)");
					continue;
				}
				move.move = true;
				move.jump = true;
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				if (direction != null) {
					move.moveDirection = Direction.getInt(direction); 
					this.setFacingInt(move.moveDirection);
					commandId.AddStatusComplete();
					continue;
				}

			} else if (commandName.equalsIgnoreCase(Names.kStopSimID)) {
				if (move.stopSim) {
					logger.warn(getName() + ": multiple stop commands detected, ignoring");
					continue;
				}
				move.stopSim = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kOpenID)) {
				if (move.open) {
					logger.warn(getName() + ": multiple open commands detected, ignoring");
					continue;
				}
				move.open = true;
				commandId.AddStatusComplete();

				String openCode = commandId.GetParameterValue(Names.kOpenCodeID);
				if (openCode != null) {
					try {
						move.openCode = Integer.parseInt(openCode);
					} catch (NumberFormatException e) {	
						logger.warn(getName() + ": invalid open code");
						continue;
					}
					continue;
				}
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kDontEatID)) {
				if (move.dontEat) {
					logger.warn(getName() + ": multiple dont eat commands detected, ignoring");
					continue;
				}
				move.dontEat = true;
				commandId.AddStatusComplete();
				continue;
				
			} else {
				logger.warn("Unknown command: " + commandName);
				continue;
			}
			
			logger.warn("Improperly formatted command: " + commandName);
		}
		agent.ClearOutputLinkChanges();
		if (!agent.Commit()) {
			error(Names.Errors.commitFail + this.getName());
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
		
		try {
			input.destroy();
		} catch (CommitException e) {
			error(Names.Errors.commitFail + this.getName());
			Soar2D.control.stopSimulation();
		}

		metadata.destroy();
		metadata = null;
		metadata = InputLinkMetadata.load(agent);

		if (!agent.Commit()) {
		}

		agent.InitSoar();
			
		try {
			input.create(getName(), getPoints());
		} catch (CommitException e) {
			error(Names.Errors.commitFail + this.getName());
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

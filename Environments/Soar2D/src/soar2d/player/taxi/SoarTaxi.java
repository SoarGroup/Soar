package soar2d.player.taxi;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.logging.*;

import sml.*;
import soar2d.Direction;
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.player.MoveInfo;
import soar2d.player.Player;
import soar2d.player.PlayerConfig;
import soar2d.world.World;

/**
 * @author voigtjr
 *
 */
public class SoarTaxi extends Taxi {
	/**
	 * the soar agent
	 */
	private Agent agent;
	/**
	 * a random number, guaranteed to change every frame
	 */
	private float random;
	
	/**
	 * the wme for the random number
	 */
	private FloatElement randomWME;
	/**
	 * soar commands to run before this agent is destroyed
	 */
	private ArrayList<String> shutdownCommands;

	/**
	 * @param agent a valid soar agent
	 * @param playerConfig the rest of the player config
	 */
	public SoarTaxi(Agent agent, PlayerConfig playerConfig) {
		super(playerConfig);

		this.agent = agent;
		this.shutdownCommands = playerConfig.getShutdownCommands();
		
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
	
	public void update(java.awt.Point location) {
		World world = Soar2D.simulation.world;

		// check to see if we've moved
		super.update(location);
		
		// if we moved, update the location
		if (moved) {
			//agent.Update(xWME, location.x);
			//agent.Update(yWME, location.y);
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
		this.resetPointsChanged();
	}
	
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
		if (agent.GetNumberCommands() > 1) {
			logger.warning(getName() + ": " + agent.GetNumberCommands() 
					+ " commands detected, all but the first will be ignored");
		}
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();
			
			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (move.move || moveWait) {
					logger.warning(getName() + ": multiple move commands detected");
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
						move.moveDirection = Direction.getInt(direction); 
						this.setFacingInt(move.moveDirection);
						commandId.AddStatusComplete();
						continue;
					}
				}
				
			} else if (commandName.equalsIgnoreCase(Names.kStopSimID)) {
				if (move.stopSim) {
					logger.warning(getName() + ": multiple stop commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.stopSim = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kPickUpID)) {
				if (move.pickup) {
					logger.warning(getName() + ": multiple " + Names.kPickUpID + " commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.pickup = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kPutDownID)) {
				if (move.putdown) {
					logger.warning(getName() + ": multiple " + Names.kPutDownID + " commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.putdown = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kFillUpID)) {
				if (move.fillup) {
					logger.warning(getName() + ": multiple " + Names.kFillUpID + " commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.fillup = true;
				commandId.AddStatusComplete();
				continue;
				
			} else {
				logger.warning("Unknown command: " + commandName);
				commandId.AddStatusError();
				continue;
			}
			
			logger.warning("Improperly formatted command: " + commandName);
			commandId.AddStatusError();
		}
		agent.ClearOutputLinkChanges();
		if (!agent.Commit()) {
			Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
			Soar2D.control.stopSimulation();
		}
		return move;
	}
	
	public void reset() {
		super.reset();
		
		//if (agent == null) {
		//	return;
		//}

		//if (!agent.Commit()) {
		//	Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
		//	Soar2D.control.stopSimulation();
		//}
	}

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

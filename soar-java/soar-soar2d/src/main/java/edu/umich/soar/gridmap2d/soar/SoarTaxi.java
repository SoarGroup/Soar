package edu.umich.soar.gridmap2d.soar;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.map.TaxiMap;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Taxi;
import edu.umich.soar.gridmap2d.players.TaxiCommander;

import sml.Agent;
import sml.Identifier;

public class SoarTaxi implements TaxiCommander {
	private static Logger logger = Logger.getLogger(SoarTaxi.class);

	private Taxi player;
	private Agent agent;
	private String [] shutdownCommands;
	private SoarTaxiIL input;

	public SoarTaxi(Taxi taxi, Agent agent, String[] shutdown_commands) {
		this.player = taxi;
		this.agent = agent;
		this.shutdownCommands = shutdown_commands;
		
		agent.SetBlinkIfNoChange(false);
		
		input = new SoarTaxiIL(agent);
		input.create();

		if (!agent.Commit()) {
			Gridmap2D.control.errorPopUp(Names.Errors.commitFail + taxi.getName());
		}
	}

	public void update(TaxiMap taxiMap) {
		input.update(player.getMoved(), player.getLocation(), taxiMap, player.getPointsDelta(), player.getFuel());
		
		if (!agent.Commit()) {
			Gridmap2D.control.errorPopUp(Names.Errors.commitFail + player.getName());
			Gridmap2D.control.stopSimulation();
		}
	}

	public CommandInfo nextCommand() {
		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			if (logger.isDebugEnabled()) {
				logger.debug(player.getName() + " issued no command.");
			}
			return new CommandInfo();
		}

		// go through the commands
		// see move info for details
		CommandInfo move = new CommandInfo();
		boolean moveWait = false;
		if (agent.GetNumberCommands() > 1) {
			logger.warn(player.getName() + ": " + agent.GetNumberCommands() 
					+ " commands detected, all but the first will be ignored");
		}
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();
			
			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (move.move || moveWait) {
					logger.warn(player.getName() + ": multiple move commands detected");
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
						commandId.AddStatusComplete();
						continue;
					}
				}
				
			} else if (commandName.equalsIgnoreCase(Names.kStopSimID)) {
				if (move.stopSim) {
					logger.warn(player.getName() + ": multiple stop commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.stopSim = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kPickUpID)) {
				if (move.pickup) {
					logger.warn(player.getName() + ": multiple " + Names.kPickUpID + " commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.pickup = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kPutDownID)) {
				if (move.putdown) {
					logger.warn(player.getName() + ": multiple " + Names.kPutDownID + " commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.putdown = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kFillUpID)) {
				if (move.fillup) {
					logger.warn(player.getName() + ": multiple " + Names.kFillUpID + " commands detected, ignoring");
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
			Gridmap2D.control.errorPopUp(Names.Errors.commitFail + player.getName());
			Gridmap2D.control.stopSimulation();
		}
		
		return move;
	}

	public void reset() {
		if (agent == null) {
			return;
		}
		
		input.destroy();

		if (!agent.Commit()) {
			Gridmap2D.control.errorPopUp(Names.Errors.commitFail + player.getName());
			Gridmap2D.control.stopSimulation();
		}

		agent.InitSoar();
			
		input.create();
			 
		if (!agent.Commit()) {
			Gridmap2D.control.errorPopUp(Names.Errors.commitFail + player.getName());
			Gridmap2D.control.stopSimulation();
		}

	}

	public void shutdown() {
		assert agent != null;
		if (shutdownCommands != null) { 
			// execute the pre-shutdown commands
			for (String command : shutdownCommands) {
				String result = player.getName() + ": result: " + agent.ExecuteCommandLine(command, true);
				logger.info(player.getName() + ": shutdown command: " + command);
				if (agent.HadError()) {
					Gridmap2D.control.errorPopUp(result);
				} else {
					logger.info(player.getName() + ": result: " + result);
				}
			}
		}
	}
}

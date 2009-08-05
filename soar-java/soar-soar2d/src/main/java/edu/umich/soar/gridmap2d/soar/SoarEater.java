package edu.umich.soar.gridmap2d.soar;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.map.EatersMap;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Eater;
import edu.umich.soar.gridmap2d.players.EaterCommander;

import sml.Agent;
import sml.Identifier;

public final class SoarEater implements EaterCommander {
	private static Logger logger = Logger.getLogger(SoarEater.class);

	private Eater player;
	
	private SoarEaterIL input;
	private Agent agent;
	private String[] shutdownCommands;
	boolean fragged = false;
	
	public SoarEater(Eater player, Agent agent, int vision, String[] shutdownCommands) {
		this.player = player;
		this.agent = agent;
		agent.SetBlinkIfNoChange(false);
		
		this.shutdownCommands = shutdownCommands;
		
		input = new SoarEaterIL(agent, vision);
		input.create(player.getName(), player.getPoints());
		
		if (!agent.Commit()) {
			Gridmap2D.control.errorPopUp(Names.Errors.commitFail + player.getName());
		}
	}
	
	public void update(EatersMap eatersMap) {
		input.update(player.getMoved(), player.getLocation(), eatersMap, player.getPoints());
		
		// commit everything
		if (!agent.Commit()) {
			Gridmap2D.control.errorPopUp(Names.Errors.commitFail + player.getName());
			Gridmap2D.control.stopSimulation();
		}
	}
	
	public CommandInfo nextCommand() {
		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			logger.debug(player.getName() + " issued no command.");
			return new CommandInfo();
		}

		// go through the commands
		// see move info for details
		CommandInfo move = new CommandInfo();
		boolean moveWait = false;
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();
			
			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (move.move || moveWait) {
					logger.warn(player.getName() + ": multiple move/jump commands detected (move)");
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
						move.moveDirection = Direction.parse(direction); 
						commandId.AddStatusComplete();
						continue;
					}
				}
				
			} else if (commandName.equalsIgnoreCase(Names.kJumpID)) {
				if (move.move) {
					logger.warn(player.getName() + ": multiple move/jump commands detected, ignoring (jump)");
					continue;
				}
				move.move = true;
				move.jump = true;
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				if (direction != null) {
					move.moveDirection = Direction.parse(direction); 
					commandId.AddStatusComplete();
					continue;
				}

			} else if (commandName.equalsIgnoreCase(Names.kStopSimID)) {
				if (move.stopSim) {
					logger.warn(player.getName() + ": multiple stop commands detected, ignoring");
					continue;
				}
				move.stopSim = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kOpenID)) {
				if (move.open) {
					logger.warn(player.getName() + ": multiple open commands detected, ignoring");
					continue;
				}
				move.open = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kDontEatID)) {
				if (move.dontEat) {
					logger.warn(player.getName() + ": multiple dont eat commands detected, ignoring");
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
		}

		agent.InitSoar();
			
		input.create(player.getName(), player.getPoints());

		if (!agent.Commit()) {
			Gridmap2D.control.errorPopUp(Names.Errors.commitFail + player.getName());
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

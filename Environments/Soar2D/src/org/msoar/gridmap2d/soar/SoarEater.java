package org.msoar.gridmap2d.soar;

import java.io.File;

import org.apache.log4j.Logger;
import org.msoar.gridmap2d.Direction;
import org.msoar.gridmap2d.Names;
import org.msoar.gridmap2d.map.EatersMap;
import org.msoar.gridmap2d.players.CommandInfo;
import org.msoar.gridmap2d.players.Eater;
import org.msoar.gridmap2d.players.EaterCommander;

import sml.Agent;
import sml.Identifier;

public final class SoarEater implements EaterCommander {
	private static Logger logger = Logger.getLogger(SoarEater.class);

	private Eater eater;
	
	private SoarEaterIL input;
	private Agent agent;
	private String[] shutdownCommands;
	boolean fragged = false;
	private InputLinkMetadata metadata;
	private File commonMetadataFile;
	private File mapMetadataFile;
	
	public SoarEater(Eater eater, Agent agent, int vision, String[] shutdownCommands, File commonMetadataFile, File mapMetadataFile) throws Exception {
		this.eater = eater;
		this.agent = agent;
		this.commonMetadataFile = commonMetadataFile;
		this.mapMetadataFile = mapMetadataFile;
		agent.SetBlinkIfNoChange(false);
		
		this.shutdownCommands = shutdownCommands;
		
		input = new SoarEaterIL(agent, vision);
		try {
			input.create(eater.getName(), eater.getPoints());
		} catch (CommitException e) {
			throw new Exception(Names.Errors.commitFail + eater.getName());
		}
		
		metadata = InputLinkMetadata.load(agent, commonMetadataFile, mapMetadataFile);
		
		if (!agent.Commit()) {
			throw new Exception(Names.Errors.commitFail + eater.getName());
		}
	}
	
	public void update(EatersMap eatersMap) throws Exception {
		try {
			input.update(eater.getMoved(), eater.getLocation(), eatersMap, eater.getPoints());
		} catch (CommitException e) {
			throw new Exception(Names.Errors.commitFail + eater.getName());
		}
		
		// commit everything
		if (!agent.Commit()) {
			throw new Exception(Names.Errors.commitFail + eater.getName());
		}
	}
	
	public CommandInfo nextCommand() throws Exception {
		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			logger.debug(eater.getName() + " issued no command.");
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
					logger.warn(eater.getName() + ": multiple move/jump commands detected (move)");
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
					logger.warn(eater.getName() + ": multiple move/jump commands detected, ignoring (jump)");
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
					logger.warn(eater.getName() + ": multiple stop commands detected, ignoring");
					continue;
				}
				move.stopSim = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kOpenID)) {
				if (move.open) {
					logger.warn(eater.getName() + ": multiple open commands detected, ignoring");
					continue;
				}
				move.open = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kDontEatID)) {
				if (move.dontEat) {
					logger.warn(eater.getName() + ": multiple dont eat commands detected, ignoring");
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
			throw new Exception(Names.Errors.commitFail + eater.getName());
		}
		return move;
	}
	
	public void reset() throws Exception {
		if (agent == null) {
			return;
		}
		
		try {
			input.destroy();
		} catch (CommitException e) {
			throw new Exception(Names.Errors.commitFail + eater.getName());
		}

		metadata.destroy();
		metadata = null;
		metadata = InputLinkMetadata.load(agent, commonMetadataFile, mapMetadataFile);

		if (!agent.Commit()) {
			throw new Exception(Names.Errors.commitFail + eater.getName());
		}

		agent.InitSoar();
			
		try {
			input.create(eater.getName(), eater.getPoints());
		} catch (CommitException e) {
			throw new Exception(Names.Errors.commitFail + eater.getName());
		}
	}

	public void shutdown() throws Exception {
		assert agent != null;
		if (shutdownCommands != null) { 
			// execute the pre-shutdown commands
			for (String command : shutdownCommands) {
				String result = eater.getName() + ": result: " + agent.ExecuteCommandLine(command, true);
				logger.info(eater.getName() + ": shutdown command: " + command);
				if (agent.HadError()) {
					throw new Exception(result);
				} else {
					logger.info(eater.getName() + ": result: " + result);
				}
			}
		}
	}
}

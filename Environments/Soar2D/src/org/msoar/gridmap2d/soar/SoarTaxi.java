package org.msoar.gridmap2d.soar;

import java.io.File;

import org.apache.log4j.Logger;
import org.msoar.gridmap2d.Direction;
import org.msoar.gridmap2d.Names;
import org.msoar.gridmap2d.map.TaxiMap;
import org.msoar.gridmap2d.players.CommandInfo;
import org.msoar.gridmap2d.players.Taxi;
import org.msoar.gridmap2d.players.TaxiCommander;

import sml.Agent;
import sml.Identifier;

public class SoarTaxi implements TaxiCommander {
	private static Logger logger = Logger.getLogger(SoarTaxi.class);

	private Taxi taxi;
	private Agent agent;
	private String [] shutdownCommands;
	private InputLinkMetadata metadata;
	private File commonMetadataFile;
	private File mapMetadataFile;
	private SoarTaxiIL input;

	public SoarTaxi(Taxi taxi, Agent agent, String[] shutdown_commands, File commonMetadataFile, File mapMetadataFile) throws Exception {
		this.taxi = taxi;
		this.agent = agent;
		this.commonMetadataFile = commonMetadataFile;
		this.mapMetadataFile = mapMetadataFile;
		this.shutdownCommands = shutdown_commands;
		
		agent.SetBlinkIfNoChange(false);
		
		input = new SoarTaxiIL(agent);
		try {
			input.create();
		} catch (CommitException e) {
			throw new Exception(Names.Errors.commitFail + taxi.getName());
		}
		this.metadata = InputLinkMetadata.load(agent, commonMetadataFile, mapMetadataFile);
		
		if (!agent.Commit()) {
			throw new Exception(Names.Errors.commitFail + taxi.getName());
		}
	}

	public void update(TaxiMap taxiMap) throws Exception {
		try {
			input.update(taxi.getMoved(), taxi.getLocation(), taxiMap, taxi.getPointsDelta(), taxi.getFuel());
		} catch (CommitException e) {
			throw new Exception(Names.Errors.commitFail + taxi.getName());
		}
	}

	public CommandInfo nextCommand() throws Exception {
		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			if (logger.isDebugEnabled()) {
				logger.debug(taxi.getName() + " issued no command.");
			}
			return new CommandInfo();
		}

		// go through the commands
		// see move info for details
		CommandInfo move = new CommandInfo();
		boolean moveWait = false;
		if (agent.GetNumberCommands() > 1) {
			logger.warn(taxi.getName() + ": " + agent.GetNumberCommands() 
					+ " commands detected, all but the first will be ignored");
		}
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();
			
			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (move.move || moveWait) {
					logger.warn(taxi.getName() + ": multiple move commands detected");
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
					logger.warn(taxi.getName() + ": multiple stop commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.stopSim = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kPickUpID)) {
				if (move.pickup) {
					logger.warn(taxi.getName() + ": multiple " + Names.kPickUpID + " commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.pickup = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kPutDownID)) {
				if (move.putdown) {
					logger.warn(taxi.getName() + ": multiple " + Names.kPutDownID + " commands detected, ignoring");
					commandId.AddStatusError();
					continue;
				}
				move.putdown = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kFillUpID)) {
				if (move.fillup) {
					logger.warn(taxi.getName() + ": multiple " + Names.kFillUpID + " commands detected, ignoring");
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
			throw new Exception(Names.Errors.commitFail + taxi.getName());
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
			throw new Exception(Names.Errors.commitFail + taxi.getName());
		}

		metadata.destroy();
		metadata = null;
		metadata = InputLinkMetadata.load(agent, commonMetadataFile, mapMetadataFile);

		if (!agent.Commit()) {
			throw new Exception(Names.Errors.commitFail + taxi.getName());
		}

		agent.InitSoar();
			
		try {
			input.create();
		} catch (CommitException e) {
			throw new Exception(Names.Errors.commitFail + taxi.getName());
		}
	}

	public void shutdown() throws Exception {
		assert agent != null;
		if (shutdownCommands != null) { 
			// execute the pre-shutdown commands
			for (String command : shutdownCommands) {
				String result = taxi.getName() + ": result: " + agent.ExecuteCommandLine(command, true);
				logger.info(taxi.getName() + ": shutdown command: " + command);
				if (agent.HadError()) {
					throw new Exception(result);
				} else {
					logger.info(taxi.getName() + ": result: " + result);
				}
			}
		}
	}
}

package edu.umich.soar.sproom.command;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import lcmtypes.pose_t;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import sml.Agent;
import sml.Identifier;

class OutputLink {
	private static final Log logger = LogFactory.getLog(OutputLink.class);
	
	private final Agent agent;
	private final Map<Integer, OutputLinkCommand> seenCommands = new HashMap<Integer, OutputLinkCommand>();
	private DriveCommand driveCommand;
	
	static OutputLink newInstance(Agent agent) {
		return new OutputLink(agent);
	}
	
	private OutputLink(Agent agent) {
		this.agent = agent;
	}
	
	class OutputLinkActions {
		private DifferentialDriveCommand ddc;
		
		DifferentialDriveCommand getDDC() {
			return ddc;
		}
	}
	
	OutputLinkActions update(pose_t pose) {
		// TODO: synchronization

		List<Integer> currentTimeTags = new ArrayList<Integer>(agent.GetNumberCommands());
		List<DriveCommand> newDriveCommands = new ArrayList<DriveCommand>(agent.GetNumberCommands());
		
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandWme = agent.GetCommand(i);
			
			Integer tt = Integer.valueOf(commandWme.GetTimeTag());
			currentTimeTags.add(tt);
			
			if (!seenCommands.containsKey(tt)) {
				continue;
			}
			
			// haven't seen it, make it and store it
			OutputLinkCommand command = OutputLinkCommand.valueOf(commandWme);
			// valid commands are status-accepted at this point
			logger.debug("Processed: " + command);
			seenCommands.put(tt, command);
			
			if (command instanceof DriveCommand) {
				newDriveCommands.add((DriveCommand)command);
			}
		}
		
		// forget commands no longer on the input link
		seenCommands.keySet().retainAll(currentTimeTags);
		
		// remove drive command if that disappeared
		if (!seenCommands.containsKey(driveCommand.getTimeTag())) {
			driveCommand = null;
		}

		// update current commands
		OutputLinkActions actions = new OutputLinkActions();
		for (OutputLinkCommand command : seenCommands.values()) {
			command.update(pose);
			
			if (command instanceof DriveCommand) {
				// If there was no drive command or if it is a different drive command
				if (driveCommand == null || !driveCommand.equals(command)) {

					// interrupt the current drive command
					if (driveCommand != null) {
						driveCommand.interrupt();
					}
					
					// Set the new ddc
					driveCommand = (DriveCommand)command;
					actions.ddc = driveCommand.getDDC();
				}
			}
		}
		
		agent.ClearOutputLinkChanges();
		
		return actions;
	}
}

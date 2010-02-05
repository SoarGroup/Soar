package edu.umich.soar.sproom.soar;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.drive.DifferentialDriveCommand;
import edu.umich.soar.sproom.drive.DriveCommand;
import edu.umich.soar.sproom.soar.commands.OutputLinkCommand;

import sml.Agent;
import sml.Identifier;

class OutputLink {
	private static final Log logger = LogFactory.getLog(OutputLink.class);
	
	private final Agent agent;
	private final Map<Integer, OutputLinkCommand> seenCommands = new HashMap<Integer, OutputLinkCommand>();
	private DriveCommand driveCommand;
	private final Adaptable app;
	
	static OutputLink newInstance(Adaptable app) {
		return new OutputLink(app);
	}
	
	private OutputLink(Adaptable app) {
		this.app = app;
		this.agent = (Agent)app.getAdapter(Agent.class);
	}
	
	class OutputLinkActions {
		private DifferentialDriveCommand ddc;
		
		DifferentialDriveCommand getDDC() {
			return ddc;
		}
	}
	
	OutputLinkActions update() {
		// TODO: synchronization

		List<Integer> currentTimeTags = new ArrayList<Integer>(agent.GetNumberCommands());
		
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandWme = agent.GetCommand(i);
			
			Integer tt = Integer.valueOf(commandWme.GetTimeTag());
			currentTimeTags.add(tt);
			
			if (logger.isTraceEnabled()) {
				logger.trace(commandWme.GetAttribute() + ": " + tt);
			}
			
			if (seenCommands.containsKey(tt)) {
				logger.trace("seen");
				continue;
			}
			
			// haven't seen it, make it and store it
			OutputLinkCommand command = OutputLinkCommand.valueOf(commandWme);
			
			// valid commands are status-accepted at this point
			logger.debug("Processed: " + command);
			seenCommands.put(tt, command);
		}
		
		// forget commands no longer on the input link
		seenCommands.keySet().retainAll(currentTimeTags);
		
		// update current commands
		OutputLinkActions actions = new OutputLinkActions();
		for (OutputLinkCommand command : seenCommands.values()) {
			logger.trace("Updating " + command.getName());
			command.update(app);
			
			if (command instanceof DriveCommand) {
				// If there was no drive command or if it is a different drive command
				if (driveCommand == null || !driveCommand.equals(command)) {

					// interrupt the current drive command
					if (driveCommand != null) {
						logger.trace("Interrupting " + driveCommand.getTimeTag());
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
package org.msoar.sps.control;

import java.util.HashMap;

import lcmtypes.differential_drive_command_t;
import lcmtypes.pose_t;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 * Soar output-link management. Creates input for splinter and other parts of the system.
 */
public class OutputLinkManager {
	private static final Logger logger = Logger.getLogger(OutputLinkManager.class);

	private final Agent agent;
	private final SplinterInput input = new SplinterInput();
	final InputLinkInterface inputLink;
	boolean useFloatYawWmes = true;
	private HashMap<String, Command> commands = new HashMap<String, Command>();

	private Identifier runningCommandWme;
	private CommandStatus runningCommandStatus;
	private boolean runningCommandIsInterruptable = false;
	
	public OutputLinkManager(Agent agent, InputLinkInterface inputLink) {
		this.agent = agent;
		this.inputLink = inputLink;
		
		commands.put("motor", new MotorCommand());
		commands.put("move", new MoveCommand());
		commands.put("rotate", new RotateCommand());
		commands.put("rotate-to", new RotateToCommand());
		commands.put("stop", new StopCommand());
		commands.put("add-waypoint", new AddWaypointCommand());
		commands.put("remove-waypoint", new RemoveWaypointCommand());
		commands.put("enable-waypoint", new EnableWaypointCommand());
		commands.put("disable-waypoint", new DisableWaypointCommand());
		commands.put("broadcast-message", new BroadcastMessageCommand());
		commands.put("remove-message", new RemoveMessageCommand());
		commands.put("clear-messages", new ClearMessagesCommand());
		commands.put("configure", new ConfigureCommand());
	}
	
	public boolean getUseFloatYawWmes() {
		return useFloatYawWmes;
	}
	
	public boolean getDC(differential_drive_command_t dc, pose_t pose) {
		synchronized (input) {
			if (!input.hasInput()) {
				return false;
			}
			CommandStatus status = input.getDC(dc, pose);
			if (runningCommandWme != null) {
				if (status == CommandStatus.executing) {
					if (runningCommandStatus == CommandStatus.accepted) {
						runningCommandStatus = CommandStatus.executing;
						CommandStatus.executing.addStatus(agent, runningCommandWme);
					}
				} else if (status == CommandStatus.complete) {
					CommandStatus.complete.addStatus(agent, runningCommandWme);
					runningCommandWme = null;
				}
			}
		}
		return true;
	}
	
	public void update(pose_t pose, long lastUpdate) {
		// process output
		boolean producedInput = false;
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandWme = agent.GetCommand(i);
			if (runningCommandWme != null) {
				if (commandWme.GetTimeTag() == runningCommandWme.GetTimeTag()) {
					continue;
				}
			}
			
			String commandName = commandWme.GetAttribute();
			logger.trace(commandName + " " + commandWme.GetTimeTag());
			
			Command commandObject = commands.get(commandName);
			if (commandObject == null) {
				logger.warn("Unknown command: " + commandName);
				CommandStatus.error.addStatus(agent, commandWme);
				continue;
			}
			
			if (commandObject.modifiesInput() && producedInput) {
				logger.warn("Multiple input commands received, skipping " + commandName);
				continue;
			}
			
			CommandStatus status = commandObject.execute(commandWme, pose, this);
			if (status == CommandStatus.error) {
				CommandStatus.error.addStatus(agent, commandWme);
				continue;
			}

			if (status == CommandStatus.accepted) {
				CommandStatus.accepted.addStatus(agent, commandWme);
				
			} else if (status == CommandStatus.executing) {
				CommandStatus.accepted.addStatus(agent, commandWme);
				CommandStatus.executing.addStatus(agent, commandWme);
				
			} else if (status == CommandStatus.complete) {
				CommandStatus.accepted.addStatus(agent, commandWme);
				CommandStatus.complete.addStatus(agent, commandWme);
				
			} else {
				throw new IllegalStateException();
			}
			
			if (commandObject.modifiesInput()) {
				producedInput = true;
				synchronized (input) {
					if (runningCommandWme != null) {
						if (runningCommandIsInterruptable) {
							CommandStatus.interrupted.addStatus(agent, runningCommandWme);
						} else {
							CommandStatus.complete.addStatus(agent, runningCommandWme);
						}
						runningCommandWme = null;
					}
					
					if (status != CommandStatus.complete) {
						runningCommandWme = commandWme;
						runningCommandIsInterruptable = commandObject.isInterruptable();
						runningCommandStatus = status;
					}
					
					commandObject.updateInput(input);
					input.setUtime(lastUpdate);
				}
			}
		}
		
		if (!producedInput) {
			synchronized (input) {
				input.setUtime(lastUpdate);
			}
		}
	}	
}

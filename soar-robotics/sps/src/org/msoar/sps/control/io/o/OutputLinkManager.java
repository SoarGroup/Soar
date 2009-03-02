package org.msoar.sps.control.io.o;

import java.util.HashMap;

import lcmtypes.differential_drive_command_t;
import lcmtypes.pose_t;

import org.apache.log4j.Logger;
import org.msoar.sps.control.io.i.ReceivedMessagesIL;
import org.msoar.sps.control.io.i.WaypointsIL;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 * Soar output-link management. Creates input for splinter and other parts of the system.
 */
public class OutputLinkManager {
	private static Logger logger = Logger.getLogger(OutputLinkManager.class);

	private Agent agent;
	private SplinterInput command = null;
	WaypointsIL waypointsIL;
	ReceivedMessagesIL messagesIL;
	boolean useFloatYawWmes = true;
	HashMap<String, Command> commands = new HashMap<String, Command>();

	public OutputLinkManager(Agent agent, WaypointsIL waypoints, ReceivedMessagesIL messages) {
		this.agent = agent;
		this.waypointsIL = waypoints;
		this.messagesIL = messages;
		
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
	
	public boolean getDC(differential_drive_command_t dc, double currentYawRadians) {
		if (command == null) {
			return false;
		}
		command.getDC(dc, currentYawRadians);
		return true;
	}
	
	public void update(pose_t pose, long lastUpdate) {
		SplinterInput newSplinterInput = null;

		// process output
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandwme = agent.GetCommand(i);
			String commandName = commandwme.GetAttribute();
			Command command = commands.get(commandName);
			if (command == null) {
				logger.warn("Unknown command: " + commandName);
				commandwme.AddStatusError();
				continue;
			}
			
			newSplinterInput = command.execute(newSplinterInput, commandwme, pose, this);
		}

		if (newSplinterInput != null) {
			command = newSplinterInput;
		}
		if (command != null) {
			command.setUtime(lastUpdate);
		}
	}
}

package edu.umich.soar.robot;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;

import org.apache.log4j.Logger;


import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr Soar output-link management. Creates input for splinter and
 *         other parts of the system.
 */
final public class OutputLinkManager {
	private static final Logger logger = Logger.getLogger(OutputLinkManager.class);

	private final Agent agent;
	private final HashMap<String, Command> commands = new HashMap<String, Command>();
	private final Set<Integer> completedTimeTags = new HashSet<Integer>();
	
	private Command runningCommand;

	public OutputLinkManager(Agent agent, WaypointInterface waypoints, 
			MessagesInterface messages, ConfigureInterface configure,
			OffsetPose opose) {
		this.agent = agent;

		commands.put(MotorCommand.NAME, MotorCommand.newInstance());
		commands.put(SetVelocityCommand.NAME, SetVelocityCommand.newInstance());
		commands.put(SetHeadingCommand.NAME, SetHeadingCommand.newInstance(opose));
		commands.put(StopCommand.NAME, StopCommand.newInstance(opose));
		commands.put(EStopCommand.NAME, EStopCommand.newInstance());
		commands.put(AddWaypointCommand.NAME, AddWaypointCommand.newInstance(opose, waypoints, configure));
		commands.put(RemoveWaypointCommand.NAME, RemoveWaypointCommand.newInstance(waypoints));
		commands.put(EnableWaypointCommand.NAME, EnableWaypointCommand.newInstance(opose, waypoints));
		commands.put(DisableWaypointCommand.NAME, DisableWaypointCommand.newInstance(waypoints));
		commands.put(SendMessageCommand.NAME, SendMessageCommand.newInstance(messages));
		commands.put(RemoveMessageCommand.NAME, RemoveMessageCommand.newInstance(messages));
		commands.put(ClearMessagesCommand.NAME, ClearMessagesCommand.newInstance(messages));
		commands.put(ConfigureCommand.NAME, ConfigureCommand.newInstance(opose, configure));
	}

	public DifferentialDriveCommand update() {
		// TODO: update status of running command
		
		// process output
		DifferentialDriveCommand ddc = null;
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandWme = agent.GetCommand(i);

			// is it already complete?
			Integer timetag = Integer.valueOf(commandWme.GetTimeTag());
			if (completedTimeTags.contains(timetag)) {
				continue;
			}
			completedTimeTags.add(timetag);
			
			// is it already running?
			synchronized (this) {
				if (runningCommand != null && runningCommand.wme().GetTimeTag() == timetag) {
					continue;
				}
			}
			
			String commandName = commandWme.GetAttribute();
			logger.debug(commandName + " timetag:" + timetag);

			Command commandObject = commands.get(commandName);
			if (commandObject == null) {
				logger.warn("Unknown command: " + commandName);
				CommandStatus.error.addStatus(agent, commandWme);
				continue;
			}
			
			if (commandObject.createsDDC()) {
				if (ddc != null) {
					logger.warn("Ignoring command " + commandName + " because already have " + ddc);
					CommandStatus.error.addStatus(agent, commandWme);
					continue;
				}
				if (runningCommand != null) {
					runningCommand.interrupt();
					runningCommand = null;
				}
			}
			
			if (!commandObject.execute(agent, commandWme)) {
				if (commandObject.createsDDC()) {
					logger.warn("Error with new drive command, commanding estop.");
					ddc = DifferentialDriveCommand.newEStopCommand();
				}
				continue;
			}

			if (commandObject.createsDDC()) {
				ddc = commandObject.getDDC();
				runningCommand = commandObject;
				logger.debug(ddc);
			}
		}
		
		if (runningCommand != null && runningCommand.update()) {
			runningCommand = null;
		}
		
		return ddc;
	}
}

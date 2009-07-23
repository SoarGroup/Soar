package edu.umich.soar.sps.control;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;

import org.apache.log4j.Logger;

import edu.umich.soar.robot.AddWaypointCommand;
import edu.umich.soar.robot.ClearMessagesCommand;
import edu.umich.soar.robot.Command;
import edu.umich.soar.robot.CommandStatus;
import edu.umich.soar.robot.ConfigureCommand;
import edu.umich.soar.robot.DifferentialDriveCommand;
import edu.umich.soar.robot.DisableWaypointCommand;
import edu.umich.soar.robot.EStopCommand;
import edu.umich.soar.robot.EnableWaypointCommand;
import edu.umich.soar.robot.MotorCommand;
import edu.umich.soar.robot.OffsetPose;
import edu.umich.soar.robot.RemoveMessageCommand;
import edu.umich.soar.robot.RemoveWaypointCommand;
import edu.umich.soar.robot.SendMessageCommand;
import edu.umich.soar.robot.SetHeadingCommand;
import edu.umich.soar.robot.SetVelocityCommand;
import edu.umich.soar.robot.StopCommand;
import edu.umich.soar.robot.MessagesInterface;
import edu.umich.soar.robot.ConfigureInterface;
import edu.umich.soar.robot.WaypointInterface;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr Soar output-link management. Creates input for splinter and
 *         other parts of the system.
 */
final class OutputLinkManager {
	private static final Logger logger = Logger.getLogger(OutputLinkManager.class);

	private final OffsetPose opose;
	private final Agent agent;
	private final HashMap<String, Command> commands = new HashMap<String, Command>();
	private final Set<Integer> completedTimeTags = new HashSet<Integer>();
	
	private Command runningCommand;

	OutputLinkManager(Agent agent, WaypointInterface waypoints, 
			MessagesInterface messages, ConfigureInterface configure,
			OffsetPose opose) {
		this.opose = opose;
		this.agent = agent;

		commands.put(MotorCommand.NAME, MotorCommand.newInstance());
		commands.put(SetVelocityCommand.NAME, SetVelocityCommand.newInstance());
		commands.put(SetHeadingCommand.NAME, SetHeadingCommand.newInstance());
		commands.put(StopCommand.NAME, StopCommand.newInstance());
		commands.put(EStopCommand.NAME, EStopCommand.newInstance());
		commands.put(AddWaypointCommand.NAME, AddWaypointCommand.newInstance(waypoints, configure));
		commands.put(RemoveWaypointCommand.NAME, RemoveWaypointCommand.newInstance(waypoints));
		commands.put(EnableWaypointCommand.NAME, EnableWaypointCommand.newInstance(waypoints));
		commands.put(DisableWaypointCommand.NAME, DisableWaypointCommand.newInstance(waypoints));
		commands.put(SendMessageCommand.NAME, SendMessageCommand.newInstance(messages));
		commands.put(RemoveMessageCommand.NAME, RemoveMessageCommand.newInstance(messages));
		commands.put(ClearMessagesCommand.NAME, ClearMessagesCommand.newInstance(messages));
		commands.put(ConfigureCommand.NAME, ConfigureCommand.newInstance(configure));
	}

	DifferentialDriveCommand update() {
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
			
			if (!commandObject.execute(agent, commandWme, opose)) {
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
		
		if (runningCommand != null && runningCommand.update(opose)) {
			runningCommand = null;
		}
		
		return ddc;
	}
}

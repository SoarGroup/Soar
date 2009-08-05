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

	public OutputLinkManager(Agent agent) {
		this.agent = agent;
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
				CommandStatus.error.addStatus(commandWme, "Unknown command: " + commandName);
				continue;
			}
			
			if (commandObject.createsDDC()) {
				if (ddc != null) {
					if (commandObject instanceof EStopCommand) {
						CommandStatus.error.addStatus(commandWme, "Encountered estop command, overriding existing command " + ddc);
						ddc = DifferentialDriveCommand.newEStopCommand();
						continue;
					}
					CommandStatus.error.addStatus(commandWme, "Ignoring command " + commandName + " because already have " + ddc);
					continue;
				}
				if (runningCommand != null) {
					runningCommand.interrupt();
					runningCommand = null;
				}
			}
			
			if (!commandObject.execute(commandWme)) {
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
		
		agent.ClearOutputLinkChanges();

		return ddc;
	}

	public void destroy() {
		commands.clear();
		completedTimeTags.clear();
		runningCommand = null;
	}

	public void create(WaypointInterface waypoints, SendMessagesInterface msgSend,
			ReceiveMessagesInterface msgRecv, ConfigureInterface configure,
			OffsetPose opose, ObjectManipulationInterface manip) {

		commands.put(MotorCommand.NAME, MotorCommand.newInstance());
		commands.put(SetVelocityCommand.NAME, SetVelocityCommand.newInstance());
		commands.put(SetLinearVelocityCommand.NAME, SetLinearVelocityCommand.newInstance());
		commands.put(SetAngularVelocityCommand.NAME, SetAngularVelocityCommand.newInstance());
		commands.put(EStopCommand.NAME, EStopCommand.newInstance());

		if (opose != null) {
			commands.put(SetHeadingCommand.NAME, SetHeadingCommand.newInstance(opose));
			commands.put(SetHeadingLinearCommand.NAME, SetHeadingLinearCommand.newInstance(opose));
			commands.put(StopCommand.NAME, StopCommand.newInstance(opose));

			if (waypoints != null) {
				commands.put(RemoveWaypointCommand.NAME, RemoveWaypointCommand.newInstance(waypoints));
				commands.put(EnableWaypointCommand.NAME, EnableWaypointCommand.newInstance(waypoints));
				commands.put(DisableWaypointCommand.NAME, DisableWaypointCommand.newInstance(waypoints));
				commands.put(AddWaypointCommand.NAME, AddWaypointCommand.newInstance(opose, waypoints));
			} else {
				logger.debug("omitting waypoint commands");
			}
			
			if (configure != null) {
				commands.put(ConfigureCommand.NAME, ConfigureCommand.newInstance(opose, configure));
			} else {
				logger.debug("omitting configure commands");
			}
		} else {
			logger.debug("omitting pose commands");
		}
		
		if (msgSend != null && msgRecv != null) {
			commands.put(SendMessageCommand.NAME, SendMessageCommand.newInstance(msgSend, agent.GetAgentName()));
			commands.put(RemoveMessageCommand.NAME, RemoveMessageCommand.newInstance(msgRecv));
			commands.put(ClearMessagesCommand.NAME, ClearMessagesCommand.newInstance(msgRecv));
		} else {
			logger.debug("omitting message commands");
		}
		
		if (manip != null) {
			commands.put(GetObjectCommand.NAME, GetObjectCommand.newInstance(manip));
			commands.put(DropObjectCommand.NAME, DropObjectCommand.newInstance(manip));
		} else {
			logger.debug("omitting manipulation commands");
		}
	}
}

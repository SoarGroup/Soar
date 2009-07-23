/**
 * 
 */
package edu.umich.soar.sps.control;

import org.apache.log4j.Logger;

import edu.umich.soar.robot.OffsetPose;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Set target heading and speed. Note, does not "complete" like set-heading (without
 * linear velocity) does.
 */
final class SetHeadingLinearCommand extends DDCCommand implements Command {
	private static final Logger logger = Logger.getLogger(SetHeadingLinearCommand.class);
	private static final String YAW = "yaw";
	private static final String LINVEL = "linear-velocity";
	static final String NAME = "set-heading-linear";

	private double yaw;
	private double linearVelocity;
	
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newHeadingLinearVelocityCommand(yaw, linearVelocity);
	}

	public boolean execute(WaypointInterface waypoints, MessagesInterface messages,
			Agent agent, Identifier command,
			OffsetPose opose, OutputLinkManager outputLinkManager) {
		if (this.agent != null || this.command != null) {
			throw new IllegalStateException();
		}
		
		try {
			yaw = Double.parseDouble(command.GetParameterValue(YAW));
		} catch (NullPointerException ex) {
			logger.warn(NAME + ": No " + YAW + " on command");
			CommandStatus.error.addStatus(agent, command);
			return false;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse " + YAW + ": " + command.GetParameterValue(YAW));
			CommandStatus.error.addStatus(agent, command);
			return false;
		}
		yaw = Math.toRadians(yaw);

		try {
			linearVelocity = Double.parseDouble(command.GetParameterValue(LINVEL));
		} catch (NullPointerException ex) {
			logger.warn(NAME + ": No " + LINVEL + " on command");
			CommandStatus.error.addStatus(agent, command);
			return false;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse " + LINVEL + ": " + command.GetParameterValue(LINVEL));
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		logger.debug(String.format(NAME + ": y%3.1f l%1.3f", yaw, linearVelocity));
		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.executing.addStatus(agent, command);
		
		this.agent = agent;
		this.command = command;
		return true;
	}
}

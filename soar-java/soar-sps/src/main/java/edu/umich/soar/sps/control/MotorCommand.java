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
 * Command motors directly with throttles. 
 */
final class MotorCommand extends DDCCommand implements Command {
	private static final Logger logger = Logger.getLogger(MotorCommand.class);
	static final String NAME = "motor";

	private double left;
	private double right;
	
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newMotorCommand(left, right);
	}

	public boolean execute(WaypointInterface waypoints, MessagesInterface messages,
			Agent agent, Identifier command,
			OffsetPose opose, OutputLinkManager outputLinkManager) {
		if (this.agent != null || this.command != null) {
			throw new IllegalStateException();
		}
		
		try {
			left = Double.parseDouble(command.GetParameterValue("left"));
		} catch (NullPointerException ex) {
			logger.warn(NAME + ":No left on command");
			CommandStatus.error.addStatus(agent, command);
			return false;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse left: " + command.GetParameterValue("left"));
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		try {
			right = Double.parseDouble(command.GetParameterValue("right"));
		} catch (NullPointerException ex) {
			logger.warn(NAME + ":No right on command");
			CommandStatus.error.addStatus(agent, command);
			return false;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ":Unable to parse right: " + command.GetParameterValue("right"));
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		left = Math.max(left, -1.0);
		left = Math.min(left, 1.0);

		right = Math.max(right, -1.0);
		right = Math.min(right, 1.0);

		logger.debug(String.format(NAME + ": %10.3f %10.3f", left, right));
		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.executing.addStatus(agent, command);

		this.agent = agent;
		this.command = command;
		return true;
	}
}

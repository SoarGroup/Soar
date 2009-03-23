/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class MotorCommand implements Command {
	private static final Logger logger = Logger.getLogger(MotorCommand.class);
	static final String NAME = "motor";

	double left;
	double right;
	
	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		try {
			left = Double.parseDouble(command.GetParameterValue("left"));
		} catch (NullPointerException ex) {
			logger.warn(NAME + ":No left on command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse left: " + command.GetParameterValue("left"));
			return CommandStatus.error;
		}

		try {
			right = Double.parseDouble(command.GetParameterValue("right"));
		} catch (NullPointerException ex) {
			logger.warn(NAME + ":No right on command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ":Unable to parse right: " + command.GetParameterValue("right"));
			return CommandStatus.error;
		}

		left = Math.max(left, -1.0);
		left = Math.min(left, 1.0);

		right = Math.max(right, -1.0);
		right = Math.min(right, 1.0);

		logger.debug(String.format(NAME + ": %10.3f %10.3f", left, right));
		
		return CommandStatus.executing;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean createsDDC() {
		return true;
	}

	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newMotorCommand(left, right);
	}
}
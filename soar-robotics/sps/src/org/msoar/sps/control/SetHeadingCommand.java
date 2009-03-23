/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class SetHeadingCommand implements Command {
	private static final Logger logger = Logger.getLogger(SetHeadingCommand.class);
	private static final String YAW = "yaw";
	static final String NAME = "set-heading";

	double yaw;
	
	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		try {
			yaw = Double.parseDouble(command.GetParameterValue(YAW));
		} catch (NullPointerException ex) {
			logger.warn(NAME + ": No " + YAW + " on command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse " + YAW + ": " + command.GetParameterValue(YAW));
			return CommandStatus.error;
		}
		yaw = Math.toRadians(yaw);

		logger.debug(String.format(NAME + ": %10.3f", yaw));
		
		return CommandStatus.accepted;
	}
	
	public boolean isInterruptable() {
		return true;
	}

	public boolean createsDDC() {
		return true;
	}

	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newHeadingCommand(yaw);
	}
}
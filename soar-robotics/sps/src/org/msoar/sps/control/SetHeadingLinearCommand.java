/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class SetHeadingLinearCommand implements Command {
	private static final Logger logger = Logger.getLogger(SetHeadingLinearCommand.class);
	private static final String YAW = "yaw";
	private static final String LINVEL = "linear-velocity";
	static final String NAME = "set-heading-linear";

	double yaw;
	double linearVelocity;
	
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

		try {
			linearVelocity = Double.parseDouble(command.GetParameterValue(LINVEL));
		} catch (NullPointerException ex) {
			logger.warn(NAME + ": No " + LINVEL + " on command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse " + LINVEL + ": " + command.GetParameterValue(LINVEL));
			return CommandStatus.error;
		}

		logger.debug(String.format(NAME + ": y%3.1f l%1.3f", yaw, linearVelocity));
		
		return CommandStatus.accepted;
	}
	
	public boolean isInterruptable() {
		return true;
	}

	public boolean createsDDC() {
		return true;
	}

	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newHeadingLinearVelocityCommand(yaw, linearVelocity);
	}
}
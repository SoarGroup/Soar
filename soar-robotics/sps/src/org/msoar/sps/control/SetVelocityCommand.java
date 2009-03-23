/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

class SetVelocityCommand implements Command {
	private static final Logger logger = Logger.getLogger(SetVelocityCommand.class);
	private static final String LINVEL = "linear-velocity";
	private static final String ANGVEL = "angular-velocity";
	static final String NAME = "set-velocity";

	private double linearVelocity;
	private double angularVelocity;
	
	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		try {
			linearVelocity = Double.parseDouble(command.GetParameterValue(LINVEL));
		} catch (NullPointerException ex) {
			logger.warn(NAME + ": No " + LINVEL + " on command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse " + LINVEL + ": " + command.GetParameterValue(LINVEL));
			return CommandStatus.error;
		}

		try {
			angularVelocity = Double.parseDouble(command.GetParameterValue(ANGVEL));
		} catch (NullPointerException ex) {
			logger.warn(NAME + ": No " + ANGVEL + " on command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse " + ANGVEL + ": " + command.GetParameterValue(ANGVEL));
			return CommandStatus.error;
		}

		logger.debug(String.format(NAME + ": l%1.3f a%3.1f", linearVelocity, angularVelocity));

		return CommandStatus.executing;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean createsDDC() {
		return true;
	}

	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newLinearVelocityCommand(linearVelocity);
	}
}
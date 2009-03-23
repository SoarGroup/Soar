/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class ConfigureCommand implements Command {
	private static final Logger logger = Logger.getLogger(ConfigureCommand.class);
	static final String NAME = "configure";

	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		String yawFormat = command.GetParameterValue("yaw-format");
		if (yawFormat != null) {
			if (yawFormat.equals("float")) {
				outputLinkManager.useFloatYawWmes = true;
			} else if (yawFormat.equals("int")) {
				outputLinkManager.useFloatYawWmes = false;
			} else {
				logger.warn(NAME + ": Unknown format: " + yawFormat);
				return CommandStatus.error;
			}
			logger.info(NAME + ": set to " + yawFormat);
		}
		return CommandStatus.complete;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean createsDDC() {
		return false;
	}

	public DifferentialDriveCommand getDDC() {
		throw new AssertionError();
	}
}
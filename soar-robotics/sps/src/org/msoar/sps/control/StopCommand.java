/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class StopCommand implements Command {
	private static final Logger logger = Logger.getLogger(StopCommand.class);
	static final String NAME = "stop";

	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		logger.debug(NAME + ":");
		return CommandStatus.accepted;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean createsDDC() {
		return true;
	}

	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newVelocityCommand(0, 0);
	}
}
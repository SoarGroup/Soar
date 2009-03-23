/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class EStopCommand implements Command {
	private static final Logger logger = Logger.getLogger(EStopCommand.class);
	static final String NAME = "estop";

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
		return DifferentialDriveCommand.newEStopCommand();
	}
}
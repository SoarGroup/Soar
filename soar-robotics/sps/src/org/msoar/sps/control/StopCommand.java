/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

final class StopCommand implements Command {
	private static final Logger logger = Logger.getLogger(StopCommand.class);
	
	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
		logger.debug("stop:");
		return CommandStatus.complete;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean modifiesInput() {
		return true;
	}

	public void updateInput(SplinterInput input) {
		input.stop();
	}
}
/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class BroadcastMessageCommand implements Command {
	private static final Logger logger = Logger.getLogger(BroadcastMessageCommand.class);
	
	public CommandStatus execute(Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
		logger.warn("broadcast-message command not implemented, ignoring");
		return CommandStatus.error;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean modifiesInput() {
		return false;
	}

	public void updateInput(SplinterInput input) {
		assert false;
	}
}
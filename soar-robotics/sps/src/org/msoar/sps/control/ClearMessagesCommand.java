/**
 * 
 */
package org.msoar.sps.control;

import lcmtypes.pose_t;
import sml.Identifier;

final class ClearMessagesCommand implements Command {
	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
		inputLink.clearMessages();
		return CommandStatus.complete;
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
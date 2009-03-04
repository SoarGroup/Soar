/**
 * 
 */
package org.msoar.sps.control.o;

import lcmtypes.pose_t;
import sml.Identifier;

class ClearMessagesCommand implements Command {
	public CommandStatus execute(Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
		outputLinkManager.inputLink.clearMessages();
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
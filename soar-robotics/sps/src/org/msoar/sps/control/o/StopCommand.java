/**
 * 
 */
package org.msoar.sps.control.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class StopCommand implements Command {
	private static Logger logger = Logger.getLogger(StopCommand.class);
	
	public CommandStatus execute(Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
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
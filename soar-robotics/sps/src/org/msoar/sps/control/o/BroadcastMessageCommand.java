/**
 * 
 */
package org.msoar.sps.control.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class BroadcastMessageCommand implements Command {
	private static Logger logger = Logger.getLogger(BroadcastMessageCommand.class);
	
	public SplinterInput execute(SplinterInput input, Identifier commandwme, pose_t pose, OutputLinkManager outputLinkManager) {
		logger.warn("broadcast-message command not implemented, ignoring");
		commandwme.AddStatusError();
		return input;
	}
}
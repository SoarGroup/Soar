/**
 * 
 */
package org.msoar.sps.control.o;

import lcmtypes.pose_t;
import sml.Identifier;

class ClearMessagesCommand implements Command {
	public SplinterInput execute(SplinterInput input, Identifier commandwme, pose_t pose, OutputLinkManager outputLinkManager) {
		outputLinkManager.messagesIL.clear();
		commandwme.AddStatusComplete();
		return input;
	}
}
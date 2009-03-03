/**
 * 
 */
package org.msoar.sps.control.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class StopCommand implements Command {
	private static Logger logger = Logger.getLogger(StopCommand.class);
	
	public SplinterInput execute(SplinterInput input, Identifier commandwme, pose_t pose, OutputLinkManager outputLinkManager) {
		if (input != null) {
			// This is a warning
			logger.warn("Stop command received, possibly overriding previous orders");
		}

		logger.debug("stop:");

		input = new SplinterInput(0);

		commandwme.AddStatusComplete();
		return input;
	}
}
/**
 * 
 */
package org.msoar.sps.control.io.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class ConfigureCommand implements Command {
	private static Logger logger = Logger.getLogger(ConfigureCommand.class);
	
	public SplinterInput execute(SplinterInput input, Identifier commandwme, pose_t pose, OutputLinkManager outputLinkManager) {
		String yawFormat = commandwme.GetParameterValue("yaw-format");
		if (yawFormat != null) {
			if (yawFormat.equals("float")) {
				outputLinkManager.useFloatYawWmes = true;
			} else if (yawFormat.equals("int")) {
				outputLinkManager.useFloatYawWmes = false;
			} else {
				logger.warn("Unknown yaw-format: " + yawFormat);
				commandwme.AddStatusError();
				return input;
			}
			logger.info("yaw-format set to " + yawFormat);
		}
		commandwme.AddStatusComplete();
		return input;
	}
}
/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class ConfigureCommand implements Command {
	private static final Logger logger = Logger.getLogger(ConfigureCommand.class);
	
	public CommandStatus execute(Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
		String yawFormat = command.GetParameterValue("yaw-format");
		if (yawFormat != null) {
			if (yawFormat.equals("float")) {
				outputLinkManager.useFloatYawWmes = true;
			} else if (yawFormat.equals("int")) {
				outputLinkManager.useFloatYawWmes = false;
			} else {
				logger.warn("Unknown yaw-format: " + yawFormat);
				return CommandStatus.error;
			}
			logger.info("yaw-format set to " + yawFormat);
		}
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
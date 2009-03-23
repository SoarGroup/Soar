/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Configure robot.
 */
final class ConfigureCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(ConfigureCommand.class);
	static final String NAME = "configure";

	public boolean execute(InputLinkInterface inputLink, Agent agent,
			Identifier command, SplinterState splinter,
			OutputLinkManager outputLinkManager) {
		String yawFormat = command.GetParameterValue("yaw-format");
		if (yawFormat != null) {
			if (yawFormat.equals("float")) {
				outputLinkManager.useFloatYawWmes = true;
			} else if (yawFormat.equals("int")) {
				outputLinkManager.useFloatYawWmes = false;
			} else {
				logger.warn(NAME + ": Unknown format: " + yawFormat);
				CommandStatus.error.addStatus(agent, command);
				return false;
			}
			logger.info(NAME + ": set to " + yawFormat);
		}
		
		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}

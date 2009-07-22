/**
 * 
 */
package edu.umich.soar.sps.control;

import org.apache.log4j.Logger;

import edu.umich.soar.waypoints.OffsetPose;

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
			Identifier command, OffsetPose splinter,
			OutputLinkManager outputLinkManager) {
		if (splinter == null) {
			throw new AssertionError();
		}
		
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
			logger.info(NAME + ": yaw-format set to " + yawFormat);
		}
		
		String offsetX = command.GetParameterValue("offset-x");
		String offsetY = command.GetParameterValue("offset-y");
		if (offsetX != null || offsetY != null) {
			double[] offset = null;
			if (offsetX == null) {
				offsetX = "0";
			}
			if (offsetY == null) {
				offsetY = "0";
			}
			try {
				offset = new double[] {
						Double.parseDouble(offsetX),
						Double.parseDouble(offsetY),
				};
			} catch (NumberFormatException e) {
				logger.warn(NAME + ": Error parsing coordinates: " + offsetX + ", " + offsetY);
				CommandStatus.error.addStatus(agent, command);
				return false;
			}
			splinter.setOffset(offset);
			logger.debug(String.format("%s: offset set to x%10.3f y%10.3f", NAME, offset[0], offset[1]));
		}
		
		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}

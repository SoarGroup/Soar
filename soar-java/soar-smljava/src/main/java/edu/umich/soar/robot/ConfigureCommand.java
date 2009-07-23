/**
 * 
 */
package edu.umich.soar.robot;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Configure robot.
 */
final public class ConfigureCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(ConfigureCommand.class);
	public static final String NAME = "configure";

	public static Command newInstance(OffsetPose opose, ConfigureInterface configure) {
		return new ConfigureCommand(opose, configure);
	}
	
	public ConfigureCommand(OffsetPose opose, ConfigureInterface configure) {
		this.opose = opose;
		this.configure = configure;
	}

	private final OffsetPose opose;
	private final ConfigureInterface configure;

	@Override
	public boolean execute(Agent agent, Identifier command) {
		if (opose == null) {
			throw new AssertionError();
		}
		
		String yawFormat = command.GetParameterValue("yaw-format");
		if (yawFormat != null) {
			if (yawFormat.equals("float")) {
				configure.setFloatYawWmes(true);
			} else if (yawFormat.equals("int")) {
				configure.setFloatYawWmes(false);
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
			opose.setOffset(offset);
			logger.debug(String.format("%s: offset set to x%10.3f y%10.3f", NAME, offset[0], offset[1]));
		}
		
		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}

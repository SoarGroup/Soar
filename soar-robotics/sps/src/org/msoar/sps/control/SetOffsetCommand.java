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
 * Set location of origin.
 */
final class SetOffsetCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(SetOffsetCommand.class);
	static final String NAME = "set-offset";
	
	public boolean execute(InputLinkInterface inputLink, Agent agent,
			Identifier command, SplinterState splinter,
			OutputLinkManager outputLinkManager) {
		if (splinter == null) {
			throw new AssertionError();
		}

		double[] offset = null;
		try {
			offset = new double[] {
					Double.parseDouble(command.GetParameterValue("x")),
					Double.parseDouble(command.GetParameterValue("y")),
			};
		} catch (NullPointerException ignored) {
			logger.warn(NAME + ": Missing coordinates");
			CommandStatus.error.addStatus(agent, command);
			return false;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Error parsing coordinates");
			CommandStatus.error.addStatus(agent, command);
			return false;
		}
		
		logger.debug(String.format("%s: x%10.3f y%10.3f", NAME, offset[0], offset[1]));
		splinter.setOffset(offset);
		
		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}

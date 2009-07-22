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
 * Removes all messages from message list.
 */
final class ClearMessagesCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(ClearMessagesCommand.class);
	static final String NAME = "clear-messages";

	public boolean execute(InputLinkInterface inputLink, Agent agent,
			Identifier command, OffsetPose splinter,
			OutputLinkManager outputLinkManager) {
		inputLink.clearMessages();
		logger.info(NAME + ":");

		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}

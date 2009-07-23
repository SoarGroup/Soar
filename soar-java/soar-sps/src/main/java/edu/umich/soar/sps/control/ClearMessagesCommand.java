/**
 * 
 */
package edu.umich.soar.sps.control;

import org.apache.log4j.Logger;

import edu.umich.soar.robot.OffsetPose;

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

	public boolean execute(WaypointInterface waypoints, MessagesInterface messages,
			Agent agent, Identifier command,
			OffsetPose opose, OutputLinkManager outputLinkManager) {
		messages.clearMessages();
		logger.info(NAME + ":");

		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}

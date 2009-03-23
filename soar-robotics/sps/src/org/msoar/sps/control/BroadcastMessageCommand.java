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
 * Broadcasts a message to all listeners.
 */
final class BroadcastMessageCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(BroadcastMessageCommand.class);
	static final String NAME = "broadcast-message";

	public boolean execute(InputLinkInterface inputLink, Agent agent,
			Identifier command, SplinterState splinter,
			OutputLinkManager outputLinkManager) {
		
		logger.warn(NAME + ": command not implemented, ignoring");
		
		CommandStatus.error.addStatus(agent, command);
		return false;
	}
}

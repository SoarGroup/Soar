/**
 * 
 */
package edu.umich.soar.sps.control;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Removes a message from the received message list.
 */
final class RemoveMessageCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(RemoveMessageCommand.class);
	static final String NAME = "remove-message";

	public boolean execute(InputLinkInterface inputLink, Agent agent,
			Identifier command, SplinterState splinter,
			OutputLinkManager outputLinkManager) {

		int id = -1;
		try {
			id = Integer.parseInt(command.GetParameterValue("id"));
		} catch (NullPointerException ignored) {
			logger.warn(NAME + ": No id on command");
			CommandStatus.error.addStatus(agent, command);
			return false;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse id: " + command.GetParameterValue("id"));
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		logger.debug(String.format(NAME + ": %d", id));
		
		if (inputLink.removeMessage(id) == false) {
			logger.warn(NAME + ": Unable to remove message " + id + ", no such message");
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}

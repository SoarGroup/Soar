/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

final class RemoveMessageCommand implements Command {
	private static final Logger logger = Logger.getLogger(RemoveMessageCommand.class);
	
	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
		int id = -1;
		try {
			id = Integer.parseInt(command.GetParameterValue("id"));
		} catch (NullPointerException ignored) {
			logger.warn("No id on remove-message command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse id: " + command.GetParameterValue("id"));
			return CommandStatus.error;
		}

		logger.debug(String.format("remove-message: %d", id));
		
		if (inputLink.removeMessage(id) == false) {
			logger.warn("Unable to remove message " + id + ", no such message");
			return CommandStatus.error;
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
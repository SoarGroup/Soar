/**
 * 
 */
package org.msoar.sps.control.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class RemoveMessageCommand implements Command {
	private static Logger logger = Logger.getLogger(RemoveMessageCommand.class);
	
	public SplinterInput execute(SplinterInput input, Identifier commandwme, pose_t pose, OutputLinkManager outputLinkManager) {
		int id = -1;
		try {
			id = Integer.parseInt(commandwme.GetParameterValue("id"));
		} catch (NullPointerException ignored) {
			logger.warn("No id on remove-message command");
			commandwme.AddStatusError();
			return input;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse id: " + commandwme.GetParameterValue("id"));
			commandwme.AddStatusError();
			return input;
		}

		logger.debug(String.format("remove-message: %d", id));
		
		if (outputLinkManager.messagesIL.remove(id) == false) {
			logger.warn("Unable to remove message " + id + ", no such message");
			commandwme.AddStatusError();
			return input;
		}

		commandwme.AddStatusComplete();
		return input;
	}
}
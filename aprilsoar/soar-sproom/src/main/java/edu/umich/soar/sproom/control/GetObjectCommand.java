/**
 * 
 */
package edu.umich.soar.sproom.control;

import sml.Identifier;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * @author voigtjr
 *
 * Removes all messages from message list.
 */
final public class GetObjectCommand extends NoDDCAdapter implements Command {
	private static final Log logger = LogFactory.getLog(GetObjectCommand.class);
	static final String NAME = "get-object";

	static Command newInstance(ObjectManipulationInterface manip) {
		return new GetObjectCommand(manip);
	}
	
	private final ObjectManipulationInterface manip;
	
	private GetObjectCommand(ObjectManipulationInterface manip) {
		this.manip = manip;
	}

	@Override
	public boolean execute(Identifier command) {
		String idString = command.GetParameterValue("id");
		if (idString == null) {
			CommandStatus.error.addStatus(command, NAME + ": No id on command");
			return false;
		}
		
		int id = -1;
		try {
			id = Integer.parseInt(idString);
		} catch (NumberFormatException e) {
			CommandStatus.error.addStatus(command, NAME + ": error parsing id: " + idString);
			return false;
		}

		if (!manip.get(id)) {
			CommandStatus.error.addStatus(command, NAME + ": unable to get object id: " + idString + ", reason: " + manip.reason());
			return false;
		}
		
		logger.info(NAME + ": got id: " + idString);

		CommandStatus.accepted.addStatus(command);
		CommandStatus.complete.addStatus(command);
		return true;
	}
}

package edu.umich.soar.sproom.control;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import sml.Identifier;

public class DiffuseObjectCommand extends NoDDCAdapter implements Command {
	private static final Log logger = LogFactory.getLog(DiffuseObjectCommand.class);
	static final String NAME = "diffuse-object";

	static Command newInstance(ObjectManipulationInterface manip) {
		return new DiffuseObjectCommand(manip);
	}
	
	private final ObjectManipulationInterface manip;
	
	private DiffuseObjectCommand(ObjectManipulationInterface manip) {
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

		if (!manip.diffuse(id)) {
			CommandStatus.error.addStatus(command, NAME + ": unable to diffuse object id: " + idString + ", reason: " + manip.reason());
			return false;
		}
		
		logger.info(NAME + ": successful diffusal of id: " + idString);

		CommandStatus.accepted.addStatus(command);
		CommandStatus.complete.addStatus(command);
		return true;
	}
}

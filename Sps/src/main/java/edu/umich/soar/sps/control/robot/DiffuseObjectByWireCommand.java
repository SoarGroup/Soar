package edu.umich.soar.sps.control.robot;

import org.apache.log4j.Logger;

import sml.Identifier;

public class DiffuseObjectByWireCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(DiffuseObjectByWireCommand.class);
	static final String NAME = "diffuse-object-by-wire";

	static Command newInstance(ObjectManipulationInterface manip) {
		return new DiffuseObjectByWireCommand(manip);
	}
	
	private final ObjectManipulationInterface manip;
	
	private DiffuseObjectByWireCommand(ObjectManipulationInterface manip) {
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

		String colorString = command.GetParameterValue("color");
		if (colorString == null) {
			CommandStatus.error.addStatus(command, NAME + ": No color on command");
			return false;
		}
		
		if (!manip.diffuseByWire(id, colorString)) {
			CommandStatus.error.addStatus(command, NAME + ": unable to diffuse object id: " + idString 
					+ " by wire " + colorString + ", reason: " + manip.reason());
			return false;
		}
		
		logger.info(NAME + ": successful diffusal of id: " + idString + " by wire " + colorString);

		CommandStatus.accepted.addStatus(command);
		CommandStatus.complete.addStatus(command);
		return true;
	}
}

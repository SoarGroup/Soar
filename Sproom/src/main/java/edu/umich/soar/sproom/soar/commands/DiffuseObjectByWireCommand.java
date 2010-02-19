package edu.umich.soar.sproom.soar.commands;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;

import sml.Identifier;

public class DiffuseObjectByWireCommand extends OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(DiffuseObjectByWireCommand.class);
	static final String NAME = "diffuse-object-by-wire";

	private final Identifier wme;
	private int id;
	private String color;
	
	public DiffuseObjectByWireCommand(Identifier wme) {
		super(wme);
		this.wme = wme;
	}

	@Override
	protected boolean accept() {
		String idString = wme.GetParameterValue(SharedNames.ID);
		try {
			id = Integer.parseInt(idString);
		} catch (NullPointerException e) {
			addStatusError("No " + SharedNames.ID);
			return false;
		} catch (NumberFormatException e) {
			addStatusError("Error parsing " + SharedNames.ID + ": " + idString);
			return false;
		}
		
		color = wme.GetParameterValue(SharedNames.COLOR);
		if (color == null) {
			addStatusError("No " + SharedNames.COLOR);
			return false;
		}
		
		logger.debug(id + " " + color);
		addStatus(CommandStatus.ACCEPTED);
		return true;
	}

	@Override
	public void update(Adaptable app) {
		// TODO: get object manipulation interface, perform diffusal
		addStatusError("Not implemented.");
	}
	
}

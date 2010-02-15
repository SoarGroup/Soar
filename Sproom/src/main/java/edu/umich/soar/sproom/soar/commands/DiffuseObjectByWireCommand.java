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
	protected OutputLinkCommand accept() {
		String idString = wme.GetParameterValue(SharedNames.ID);
		try {
			id = Integer.parseInt(idString);
		} catch (NullPointerException e) {
			return new InvalidCommand(wme, "No " + SharedNames.ID);
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Error parsing " + SharedNames.ID + ": " + idString);
		}
		
		color = wme.GetParameterValue(SharedNames.COLOR);
		if (color == null) {
			return new InvalidCommand(wme, "No " + SharedNames.COLOR);
		}
		
		logger.debug(id + " " + color);
		addStatus(CommandStatus.ACCEPTED);
		return this;
	}

	@Override
	public void update(Adaptable app) {
		// TODO: get object manipulation interface, perform diffusal
		addStatus(CommandStatus.ERROR, "Not implemented.");
	}
	
}

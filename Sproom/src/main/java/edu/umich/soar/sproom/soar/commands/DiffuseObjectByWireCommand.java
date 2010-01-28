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
	private boolean complete = false;
	
	DiffuseObjectByWireCommand(Identifier wme) {
		super(Integer.valueOf(wme.GetTimeTag()));
		this.wme = wme;
	}

	@Override
	public String getName() {
		return NAME;
	}

	@Override
	public OutputLinkCommand accept() {
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
		CommandStatus.accepted.addStatus(wme);
		return this;
	}

	@Override
	public void update(Adaptable app) {
		if (!complete) {
			// TODO: get object manipulation interface, perform diffusal
			CommandStatus.error.addStatus(wme, "Not implemented.");
			complete = true;
		}
	}
	
}

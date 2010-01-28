package edu.umich.soar.sproom.soar.commands;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;

import sml.Identifier;

public class DiffuseObjectCommand extends OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(DiffuseObjectCommand.class);
	static final String NAME = "diffuse-object";

	private final Identifier wme;
	private int id;
	private boolean complete = false;
	
	DiffuseObjectCommand(Identifier wme) {
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
		
		logger.debug(id);
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

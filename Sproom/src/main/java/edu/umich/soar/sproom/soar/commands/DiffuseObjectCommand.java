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
	
	public DiffuseObjectCommand(Identifier wme) {
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
		
		logger.debug(id);
		addStatus(CommandStatus.ACCEPTED);
		return this;
	}

	@Override
	public void update(Adaptable app) {
		// TODO: get object manipulation interface, perform diffusal
		addStatus(CommandStatus.ERROR, "Not implemented.");
	}

}

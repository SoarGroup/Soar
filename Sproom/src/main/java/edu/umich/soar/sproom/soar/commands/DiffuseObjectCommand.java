package edu.umich.soar.sproom.soar.commands;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;

import sml.Identifier;

/**
 * Domain specific command to diffuse an object.
 *
 * @author voigtjr@gmail.com
 */
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
		
		logger.debug(id);
		addStatus(CommandStatus.ACCEPTED);
		return true;
	}

	@Override
	public void update(Adaptable app) {
		// TODO: get object manipulation interface, perform diffusal
		addStatusError("Not implemented.");
	}

}

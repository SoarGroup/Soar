package edu.umich.soar.sproom.soar.commands;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.Pose;
import edu.umich.soar.sproom.metamap.VirtualObject;
import edu.umich.soar.sproom.metamap.VirtualObjects;

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
		VirtualObjects vobjs = (VirtualObjects)app.getAdapter(VirtualObjects.class);
		
		VirtualObject object = vobjs.getObject(id);
		if (object == null) {
			addStatusError("No such object.");
			return;
		}
		
		Pose pose = (Pose)app.getAdapter(Pose.class);
		if (!object.isInRange(pose)) {
			addStatusError("Object too far.");
			return;
		}

		object.diffuse();
		return;
	}

}

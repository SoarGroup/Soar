/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import jmat.LinAlg;
import sml.Identifier;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.CommandConfig;
import edu.umich.soar.sproom.command.Pose;
import edu.umich.soar.sproom.command.VirtualObject;
import edu.umich.soar.sproom.command.VirtualObjects;
import edu.umich.soar.sproom.soar.Cargo;

/**
 * @author voigtjr
 *
 * Removes all messages from message list.
 */
public class GetObjectCommand extends OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(GetObjectCommand.class);
	static final String NAME = "get-object";

	private final Identifier wme;
	private int id;
	
	public GetObjectCommand(Identifier wme) {
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
		Cargo cargo = (Cargo)app.getAdapter(Cargo.class);
		if (cargo.isCarrying()) {
			addStatusError("Already carrying an object.");
			return;
		}
		
		VirtualObjects vobjs = (VirtualObjects)app.getAdapter(VirtualObjects.class);
		
		VirtualObject object = vobjs.getObject(id);
		if (object == null) {
			addStatusError("No such object.");
			return;
		}
		
		Pose pose = (Pose)app.getAdapter(Pose.class);
		double distance = LinAlg.distance(object.getPos(), pose.getPose().pos);
		CommandConfig c = CommandConfig.CONFIG;
		double manipDist = c.getManipulationDistance();
		manipDist += object.getSize()[0] / 2.0;
		if (distance > c.getManipulationDistance()) {
			addStatusError("Object too far.");
			return;
		}
		
		cargo.setCarriedObject(vobjs.removeObject(id));
		addStatus(CommandStatus.COMPLETE);
		return;
	}

}

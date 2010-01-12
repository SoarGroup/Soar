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
	private boolean complete = false;
	
	GetObjectCommand(Identifier wme) {
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
			complete = true;
			
			Cargo cargo = (Cargo)app.getAdapter(Cargo.class);
			if (cargo.isCarrying()) {
				CommandStatus.error.addStatus(wme, "Already carrying an object.");
				return;
			}
			
			VirtualObjects vobjs = (VirtualObjects)app.getAdapter(VirtualObjects.class);
			
			VirtualObject object = vobjs.getObject(id);
			if (object == null) {
				CommandStatus.error.addStatus(wme, "No such object.");
				return;
			}
			
			Pose pose = (Pose)app.getAdapter(Pose.class);
			double distance = LinAlg.distance(object.getPos(), pose.getPose().pos);
			CommandConfig c = CommandConfig.CONFIG;
			if (distance > c.getGetDistance()) {
				CommandStatus.error.addStatus(wme, "Object too far.");
				return;
			}
			
			cargo.setCarriedObject(vobjs.removeObject(id));
			CommandStatus.complete.addStatus(wme);
			return;
		}
	}

}

/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import jmat.LinAlg;
import lcmtypes.pose_t;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.CommandConfig;
import edu.umich.soar.sproom.command.Pose;
import edu.umich.soar.sproom.command.VirtualObject;
import edu.umich.soar.sproom.command.VirtualObjects;
import edu.umich.soar.sproom.soar.Cargo;

import sml.Identifier;

/**
 * Removes all messages from message list.
 *
 * @author voigtjr@gmail.com
 */
public class DropObjectCommand extends OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(DropObjectCommand.class);
	static final String NAME = "drop-object";

	private final Identifier wme;
	private int id;
	
	public DropObjectCommand(Identifier wme) {
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
		if (!cargo.isCarrying()) {
			addStatusError("Not carrying an object.");
			return;
		}

		VirtualObject object = cargo.getCarriedObject();
		Pose pose = (Pose)app.getAdapter(Pose.class);
		VirtualObjects vobjs = (VirtualObjects)app.getAdapter(VirtualObjects.class);

		pose_t botPose = pose.getPose();
		double dropDistance = CommandConfig.CONFIG.getManipulationDistance();
		dropDistance += object.getSize()[0] / 2.0;

		// Move the block dropDistance in front of us
		double[] dpos = LinAlg.quatRotate(botPose.orientation, new double[] { dropDistance, 0, 0 });
		double[] objPose = LinAlg.add(botPose.pos, dpos);
		
		object.setPos(objPose);
		vobjs.addObject(object);
		cargo.setCarriedObject(null);
		addStatus(CommandStatus.COMPLETE);
		return;
	}

}

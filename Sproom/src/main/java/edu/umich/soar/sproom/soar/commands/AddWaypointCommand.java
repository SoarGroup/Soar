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
import edu.umich.soar.sproom.command.Waypoints;

import sml.Identifier;
import sml.WMElement;

/**
 * @author voigtjr
 *
 * Add a waypoint to the waypoint system.
 */
public class AddWaypointCommand extends OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(AddWaypointCommand.class);
	static final String NAME = "add-waypoint";

	private final Identifier wme;
	private String type;
	private String id;
	private Double x;
	private Double y;
	private Double z;
	private Double yaw;

	public AddWaypointCommand(Identifier wme) {
		super(wme);
		this.wme = wme;
	}

	@Override
	protected OutputLinkCommand accept() {
		WMElement idwme = wme.FindByAttribute(SharedNames.ID, 0);
		if (idwme == null) {
			return new InvalidCommand(wme, "No " + SharedNames.ID);
		}
		
		type = idwme.GetValueType();
		id = idwme.GetValueAsString();
		
		try {
			x = Double.valueOf(wme.GetParameterValue(SharedNames.X));
			x = CommandConfig.CONFIG.lengthFromView(x);
		} catch (NullPointerException ignored) {
			// use current
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + SharedNames.X + ": " + wme.GetParameterValue(SharedNames.X));
		}

		try {
			y = Double.valueOf(wme.GetParameterValue(SharedNames.Y));
			y = CommandConfig.CONFIG.lengthFromView(y);
		} catch (NullPointerException ignored) {
			// use current
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + SharedNames.Y + ": " + wme.GetParameterValue(SharedNames.Y));
		}

		try {
			z = Double.valueOf(wme.GetParameterValue(SharedNames.Z));
			z = CommandConfig.CONFIG.lengthFromView(z);
		} catch (NullPointerException ignored) {
			// use current
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + SharedNames.Z + ": " + wme.GetParameterValue(SharedNames.Z));
		}

		try {
			yaw = Double.valueOf(wme.GetParameterValue(SharedNames.YAW));
			yaw = CommandConfig.CONFIG.angleFromView(yaw);
		} catch (NullPointerException ignored) {
			// use current
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + SharedNames.YAW + ": " + wme.GetParameterValue(SharedNames.YAW));
		}

		logger.debug(String.format("%16s %10.3f %10.3f %10.3f", id, x, y, z, yaw));
		addStatus(CommandStatus.ACCEPTED);
		return this;
	}
	
	@Override
	public void update(Adaptable app) {
		Waypoints waypoints = (Waypoints)app.getAdapter(Waypoints.class);
		Pose pose = (Pose)app.getAdapter(Pose.class);
		pose_t waypointPose = pose.getPose();
		if (x != null) {
			waypointPose.pos[0] = x;
		}
		if (y != null) {
			waypointPose.pos[1] = y;
		}
		if (z != null) {
			waypointPose.pos[2] = z;
		}
		if (yaw != null) {
			waypointPose.orientation = LinAlg.rollPitchYawToQuat(new double[] { 0, 0, yaw });
		}

		waypoints.createWaypoint(id, type, waypointPose);
		addStatus(CommandStatus.COMPLETE);
	}

}

/**
 * 
 */
package edu.umich.soar.sproom.command;

import jmat.LinAlg;

import lcmtypes.pose_t;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.wp.Waypoints;

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

	private static final String ID = "id";
	private static final String X = "x";
	private static final String Y = "y";
	private static final String Z = "z";
	private static final String YAW = "yaw";
	
	private final Identifier wme;
	private String type;
	private String id;
	private Double x;
	private Double y;
	private Double z;
	private Double yaw;
	private boolean complete = false;

	AddWaypointCommand(Identifier wme) {
		super(Integer.valueOf(wme.GetTimeTag()));
		this.wme = wme;
	}

	@Override
	public OutputLinkCommand accept() {
		WMElement idwme = wme.FindByAttribute(ID, 0);
		if (idwme == null) {
			return new InvalidCommand(wme, "No " + ID);
		}
		
		type = idwme.GetValueType();
		id = idwme.GetValueAsString();
		
		try {
			x = Double.valueOf(wme.GetParameterValue(X));
			x = CommandConfig.CONFIG.lengthFromView(x);
		} catch (NullPointerException ignored) {
			// use current
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + X + ": " + wme.GetParameterValue(X));
		}

		try {
			y = Double.valueOf(wme.GetParameterValue(Y));
			y = CommandConfig.CONFIG.lengthFromView(y);
		} catch (NullPointerException ignored) {
			// use current
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + Y + ": " + wme.GetParameterValue(Y));
		}

		try {
			z = Double.valueOf(wme.GetParameterValue(Z));
			z = CommandConfig.CONFIG.lengthFromView(z);
		} catch (NullPointerException ignored) {
			// use current
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + Z + ": " + wme.GetParameterValue(Z));
		}

		try {
			yaw = Double.valueOf(wme.GetParameterValue(YAW));
			yaw = CommandConfig.CONFIG.angleFromView(yaw);
		} catch (NullPointerException ignored) {
			// use current
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + YAW + ": " + wme.GetParameterValue(YAW));
		}

		logger.debug(String.format("%16s %10.3f %10.3f %10.3f", id, x, y, z, yaw));
		CommandStatus.accepted.addStatus(wme);
		return this;
	}
	
	@Override
	public void update(pose_t pose, Adaptable app) {
		if (!complete) {
			Waypoints waypoints = (Waypoints)app.getAdapter(Waypoints.class);
			pose_t waypointPose = pose.copy();
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
			
			waypoints.createWaypoint(id, type, pose);
			CommandStatus.complete.addStatus(wme);
			complete = true;
		}
	}

	@Override
	public String getName() {
		return NAME;
	}
}

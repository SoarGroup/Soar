/**
 * 
 */
package edu.umich.soar.sproom.command;

import lcmtypes.pose_t;
import sml.Identifier;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.wp.Waypoints;

/**
 * @author voigtjr
 *
 * Enable waypoint.
 */
public class EnableWaypointCommand extends OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(EnableWaypointCommand.class);
	static final String NAME = "enable-waypoint";

	private static final String ID = "id";

	private final Identifier wme;
	private String id;
	private boolean complete;

	EnableWaypointCommand(Identifier wme) {
		super(Integer.valueOf(wme.GetTimeTag()));
		this.wme = wme;
	}

	@Override
	public OutputLinkCommand accept() {
		String id = wme.GetParameterValue(ID);
		if (id == null) {
			return new InvalidCommand(wme, "No " + ID);
		}
		
		logger.debug(id);
		CommandStatus.accepted.addStatus(wme);
		return this;
	}
	
	@Override
	public void update(pose_t pose, Adaptable app) {
		if (!complete) {
			Waypoints waypoints = (Waypoints)app.getAdapter(Waypoints.class);
			waypoints.enableWaypoint(id);
			CommandStatus.complete.addStatus(wme);
			complete = true;
		}
	}

	@Override
	public String getName() {
		return NAME;
	}
}

/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.command.Waypoints;

import sml.Identifier;

/**
 * Removes a waypoint from the waypoint system.
 *
 * @author voigtjr@gmail.com
 */
public class RemoveWaypointCommand extends OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(RemoveWaypointCommand.class);
	static final String NAME = "remove-waypoint";

	private static final String ID = "id";

	private final Identifier wme;
	private String id;

	public RemoveWaypointCommand(Identifier wme) {
		super(wme);
		this.wme = wme;
	}

	@Override
	protected boolean accept() {
		String id = wme.GetParameterValue(ID);
		if (id == null) {
			addStatusError("No " + ID);
			return false;
		}
		
		logger.debug(id);
		addStatus(CommandStatus.ACCEPTED);
		return true;
	}
	
	@Override
	public void update(Adaptable app) {
		Waypoints waypoints = (Waypoints)app.getAdapter(Waypoints.class);
		waypoints.removeWaypoint(id);
		addStatus(CommandStatus.COMPLETE);
	}

}

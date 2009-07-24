package edu.umich.soar.gridmap2d.soar;

import jmat.LinAlg;
import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import edu.umich.soar.gridmap2d.players.RoomPlayer;
import edu.umich.soar.robot.OffsetPose;
import edu.umich.soar.robot.ConfigureInterface;
import edu.umich.soar.robot.WaypointsIL;
import edu.umich.soar.robot.MessagesIL;

public class SoarRobotSelfIL {

	private final Identifier self;
	private final WaypointsIL waypointsIL;
	private final MessagesIL messagesIL;
	private final IntElement area;
	private final FloatElement x;
	private final FloatElement y;
	private final FloatElement z;
	private final StringElement cx;
	private final StringElement cy;
	private FloatElement fYaw;
	private IntElement iYaw;
	private final ConfigureInterface configure;
	private final Identifier pose;
	private final OffsetPose opose;
	
	public SoarRobotSelfIL(Agent agent, Identifier self, 
			OffsetPose opose, ConfigureInterface configure) {
		this.self = self;
		this.configure = configure;
		this.opose = opose;
		
		Identifier waypoints = self.CreateIdWME("waypoints");
		waypointsIL = new WaypointsIL(waypoints, opose, configure);
		
		Identifier messages = self.CreateIdWME("received-messages");
		messagesIL = new MessagesIL(messages);
		
		self.CreateStringWME("name", agent.GetAgentName());
		area = self.CreateIntWME("area", -1);
		
		pose = self.CreateIdWME("pose");
		x = pose.CreateFloatWME("x", opose.getPose().pos[0]);
		y = pose.CreateFloatWME("y", opose.getPose().pos[1]);
		z = pose.CreateFloatWME("z", opose.getPose().pos[2]);
		
		Identifier collision = self.CreateIdWME("collision");
		cx = collision.CreateStringWME("x", "false");
		cy = collision.CreateStringWME("y", "false");
		
		setYaw(LinAlg.quatToRollPitchYaw(opose.getPose().orientation)[2]);
	}
	
	private void setYaw(double radians) {
		double degrees = Math.toDegrees(radians);
		if (configure.isFloatYawWmes()) {
			if (fYaw == null) {
				fYaw = pose.CreateFloatWME("yaw", degrees);
			}
			fYaw.Update(degrees);
			if (iYaw != null) {
				iYaw.DestroyWME();
				iYaw = null;
			}
		} else {
			if (iYaw == null) {
				iYaw = pose.CreateIntWME("yaw", (int)degrees);
			}
			iYaw.Update((int)degrees);
			if (fYaw != null) {
				fYaw.DestroyWME();
				fYaw = null;
			}
		}
	}
	
	public void destroy() {
		self.DestroyWME();
	}

	public void update(RoomPlayer player) {
		x.Update(opose.getPose().pos[0]);
		y.Update(opose.getPose().pos[1]);
		z.Update(opose.getPose().pos[2]);
		setYaw(LinAlg.quatToRollPitchYaw(opose.getPose().orientation)[2]);
		area.Update(player.getState().getLocationId());
		cx.Update(player.getState().isCollisionX() ? "true" : "false");
		cy.Update(player.getState().isCollisionY() ? "true" : "false");
		waypointsIL.update();
		messagesIL.update();
	}

	public WaypointsIL getWaypointsIL() {
		return waypointsIL;
	}

	public MessagesIL getMessagesIL() {
		return messagesIL;
	}


}

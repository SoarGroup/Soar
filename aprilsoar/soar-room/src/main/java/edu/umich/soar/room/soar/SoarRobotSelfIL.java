package edu.umich.soar.room.soar;

import jmat.LinAlg;
import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import edu.umich.soar.robot.OffsetPose;
import edu.umich.soar.robot.ConfigureInterface;
import edu.umich.soar.robot.WaypointsIL;
import edu.umich.soar.robot.MessagesIL;
import edu.umich.soar.room.map.CarryInterface;
import edu.umich.soar.room.map.Robot;

public class SoarRobotSelfIL {

	private final Identifier self;
	private final WaypointsIL waypointsIL;
	private final MessagesIL messagesIL;
	private final IntElement area;
	private final FloatElement x;
	private final FloatElement y;
	private final FloatElement z;
	private final FloatElement xvel;
	private final FloatElement yvel;
	private final FloatElement zvel;
	private final FloatElement yawvel;
	private final StringElement cx;
	private final StringElement cy;
	private FloatElement fYaw;
	private IntElement iYaw;
	private final ConfigureInterface configure;
	private final Identifier pose;
	private final OffsetPose opose;
	private Identifier carry;
	private IntElement carryid;
	private final CarryInterface ci;
	private final StringElement malfunction;
	
	public SoarRobotSelfIL(Agent agent, Identifier self, 
			OffsetPose opose, ConfigureInterface configure, CarryInterface ci) {
		this.self = self;
		this.configure = configure;
		this.opose = opose;
		this.ci = ci;
		
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
		xvel = pose.CreateFloatWME("x-velocity", opose.getPose().vel[0]);
		yvel = pose.CreateFloatWME("y-velocity", opose.getPose().vel[1]);
		zvel = pose.CreateFloatWME("z-velocity", opose.getPose().vel[2]);
		yawvel = pose.CreateFloatWME("yaw-velocity", Math.toDegrees(opose.getPose().rotation_rate[2]));
		
		Identifier collision = self.CreateIdWME("collision");
		cx = collision.CreateStringWME("x", "false");
		cy = collision.CreateStringWME("y", "false");
		
		setYaw(LinAlg.quatToRollPitchYaw(opose.getPose().orientation)[2]);

		if (ci.hasObject()) {
			carry = self.CreateIdWME("carry");
			carryid = carry.CreateIntWME("id", ci.getRoomObject().getId());
			carry.CreateStringWME("type", ci.getRoomObject().getCellObject().getProperty("name"));
		}
		
		malfunction = self.CreateStringWME("malfunction", ci.isMalfunctioning() ? Boolean.TRUE.toString() : Boolean.FALSE.toString());
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

	public void update(Robot player) {
		x.Update(opose.getPose().pos[0]);
		y.Update(opose.getPose().pos[1]);
		z.Update(opose.getPose().pos[2]);
		xvel.Update(opose.getPose().vel[0]);
		yvel.Update(opose.getPose().vel[1]);
		zvel.Update(opose.getPose().vel[2]);
		yawvel.Update(Math.toDegrees(opose.getPose().rotation_rate[2]));
		setYaw(LinAlg.quatToRollPitchYaw(opose.getPose().orientation)[2]);
		area.Update(player.getState().getLocationId());
		cx.Update(player.getState().isCollisionX() ? Boolean.TRUE.toString() : Boolean.FALSE.toString());
		cy.Update(player.getState().isCollisionY() ? Boolean.TRUE.toString() : Boolean.FALSE.toString());
		waypointsIL.update();
		messagesIL.update();
		malfunction.Update(ci.isMalfunctioning() ? Boolean.TRUE.toString() : Boolean.FALSE.toString());

		if (ci.hasObject()) {
			int objectId = ci.getRoomObject().getId();
			if (carry != null) {
				if (carryid.GetValue() != objectId) {
					carry.DestroyWME();
					carry = null;
				}
			}
			
			if (carry == null) {
				carry = self.CreateIdWME("carry");
				carryid = carry.CreateIntWME("id", objectId);
				carry.CreateStringWME("type", ci.getRoomObject().getCellObject().getProperty("name"));
			}
		} else {
			if (carry != null) {
				carry.DestroyWME();
				carry = null;
			}
		}
	}

	public WaypointsIL getWaypointsIL() {
		return waypointsIL;
	}

	public MessagesIL getMessagesIL() {
		return messagesIL;
	}
}

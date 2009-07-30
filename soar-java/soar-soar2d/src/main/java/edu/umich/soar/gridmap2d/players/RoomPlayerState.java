package edu.umich.soar.gridmap2d.players;


import java.util.Arrays;

import edu.umich.soar.gridmap2d.map.CellObject;

import jmat.LinAlg;
import jmat.MathUtil;

import lcmtypes.pose_t;

public class RoomPlayerState implements CarryInterface {

	private pose_t pose;
	
	private boolean collisionX;
	private boolean collisionY;
	private int locationId;
	private boolean hasDestYaw;
	private double destYaw;
	private double destYawSpeed;
	private CellObject carry;

	public void reset() {
		pose = new pose_t();
		carry = null;
		
		setCollisionX(false);
		setCollisionY(false);
		setLocationId(-1);
		resetDestYaw();
	}
	
	public void setCollisionX(boolean collisionX) {
		this.collisionX = collisionX;
	}

	public boolean isCollisionX() {
		return collisionX;
	}

	public void setCollisionY(boolean collisionY) {
		this.collisionY = collisionY;
	}

	public boolean isCollisionY() {
		return collisionY;
	}

	public void setLocationId(int locationId) {
		this.locationId = locationId;
	}

	public int getLocationId() {
		return locationId;
	}
	
	public void stop() {
		Arrays.fill(pose.vel, 0);
		Arrays.fill(pose.rotation_rate, 0);
	}
	
	public double getYaw() {
		return MathUtil.mod2pi(LinAlg.quatToRollPitchYaw(pose.orientation)[2]);
	}
	
	public void update(double elapsed) {
		// rotate
		double[] rpy = LinAlg.quatToRollPitchYaw(pose.orientation);
		rpy[2] = MathUtil.mod2pi(rpy[2]);
		double togo = 0;
		if (hasDestYaw) {
			togo = destYaw - rpy[2];
			if (togo < 0) {
				pose.rotation_rate[2] = destYawSpeed * -1;
			} else {
				pose.rotation_rate[2] = destYawSpeed;
			}
		}
		double change = pose.rotation_rate[2] * elapsed;
		if (hasDestYaw) {
			if (Math.abs(change) > Math.abs(togo)) {
				change = togo;
			}
		}
		rpy[2] += change;
		rpy[2] = MathUtil.mod2pi(rpy[2]);
		double [] newVel = Arrays.copyOf(pose.vel, pose.vel.length);
		newVel[0] = Math.cos(change) * pose.vel[0] - Math.sin(change) * pose.vel[1];
		newVel[1] = Math.sin(change) * pose.vel[0] + Math.cos(change) * pose.vel[1];
		pose.vel = newVel;
		pose.orientation = LinAlg.rollPitchYawToQuat(rpy);

		// translate
		LinAlg.add(pose.pos, LinAlg.scale(pose.vel, elapsed), pose.pos);
	}
	
	public void setAngularVelocity(double angvel) {
		pose.rotation_rate[2] = angvel;
	}

	public void setLinearVelocity(double linvel) {
		double yaw = getYaw();
		pose.vel[0] = Math.cos(yaw) * linvel;
		pose.vel[1] = Math.sin(yaw) * linvel;
	}
	
	public pose_t getPose() {
		return pose.copy();
	}

	public void setPos(double [] pos) {
		pose.pos[0] = pos[0];
		pose.pos[1] = pos[1];
	}
	
	public void resetDestYaw() {
		hasDestYaw = false;
	}
	
	public void setDestYaw(double yaw, double speed) {
		hasDestYaw = true;
		destYaw = MathUtil.mod2pi(yaw);
		destYawSpeed = speed;
	}

	@Override
	public boolean hasObject() {
		return carry != null;
	}

	@Override
	public CellObject getObject() {
		return carry;
	}

	public void pickUp(CellObject object) {
		assert carry == null;
		carry = object;
	}

	public void drop() {
		assert carry != null;
		carry = null;
	}
}

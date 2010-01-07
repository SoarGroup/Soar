package edu.umich.soar.sproom.soar;

import jmat.LinAlg;
import jmat.MathUtil;
import lcmtypes.pose_t;
import edu.umich.soar.FloatWme;
import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.Pose;
import sml.Identifier;

public class PointDataIL implements InputLinkElement {

	private final Identifier root;
	private final double[] pos;
	private final FloatWme distance;
	private final YawWme yaw;
	private final YawWme relYaw;
	private final YawWme absRelYaw;
	private boolean destroyed = false;

	PointDataIL(Identifier root, double[] pos) {
		this.root = root;
		this.pos = new double[pos.length];
		System.arraycopy(pos, 0, this.pos, 0, pos.length);

		FloatWme.newInstance(root, SharedNames.X, this.pos[0]);
		FloatWme.newInstance(root, SharedNames.Y, this.pos[0]);
		FloatWme.newInstance(root, SharedNames.Z, this.pos[2]);
		
		distance = FloatWme.newInstance(root, SharedNames.DISTANCE);
		yaw = YawWme.newInstance(root, SharedNames.YAW);
		relYaw = YawWme.newInstance(root, SharedNames.RELATIVE_BEARING);
		absRelYaw = YawWme.newInstance(root, SharedNames.ABS_RELATIVE_BEARING);
	}

	@Override
	public void update(Adaptable app) {
		if (destroyed) {
			throw new IllegalStateException();
		}
		
		Pose poseClass = (Pose)app.getAdapter(Pose.class);
		pose_t pose = poseClass.getPose();
		
		distance.update(LinAlg.distance(pose.pos, pos));

		double [] delta = LinAlg.subtract(pos, pose.pos);
		double yawVal = Math.atan2(delta[1], delta[0]);
		yaw.update(yawVal);
		
		double relYawVal = yawVal - LinAlg.quatToRollPitchYaw(pose.orientation)[2];
		relYawVal = MathUtil.mod2pi(relYawVal);
		relYaw.update(relYawVal);
		
		absRelYaw.update(Math.abs(relYawVal));
	}
	
	void destroy() {
		root.DestroyWME();
		destroyed = true;
	}
}

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

	private final pose_t point;
	private final FloatWme distance;
	private final YawWme yaw;
	private final YawWme relYaw;
	private final YawWme absRelYaw;

	PointDataIL(Identifier root, Adaptable app, pose_t point) {
		this.point = point.copy();

		FloatWme.newInstance(root, SharedNames.X, this.point.pos[0]);
		FloatWme.newInstance(root, SharedNames.Y, this.point.pos[0]);
		FloatWme.newInstance(root, SharedNames.Z, this.point.pos[2]);
		
		distance = FloatWme.newInstance(root, SharedNames.DISTANCE);
		yaw = YawWme.newInstance(root, SharedNames.YAW);
		relYaw = YawWme.newInstance(root, SharedNames.RELATIVE_BEARING);
		absRelYaw = YawWme.newInstance(root, SharedNames.ABS_RELATIVE_BEARING);
	}

	@Override
	public void update(Adaptable app) {
		Pose poseClass = (Pose)app.getAdapter(Pose.class);
		pose_t pose = poseClass.getPose();
		
		distance.update(LinAlg.distance(pose.pos, point.pos));

		double [] delta = LinAlg.subtract(point.pos, pose.pos);
		double yawVal = Math.atan2(delta[1], delta[0]);
		yaw.update(yawVal);
		
		double relYawVal = yawVal - LinAlg.quatToRollPitchYaw(pose.orientation)[2];
		relYawVal = MathUtil.mod2pi(relYawVal);
		relYaw.update(relYawVal);
		
		absRelYaw.update(Math.abs(relYawVal));
	}
}

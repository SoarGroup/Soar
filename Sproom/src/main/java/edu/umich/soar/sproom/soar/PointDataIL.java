package edu.umich.soar.sproom.soar;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.Pose;
import edu.umich.soar.sproom.command.RelativePointData;
import sml.Identifier;

/**
 * Input link management of point data, a collection of distances and angles relative
 * to the current agent's position.
 *
 * @author voigtjr@gmail.com
 */
public class PointDataIL implements InputLinkElement {

	private final Identifier root;
	private final double[] pos;
	private final DistanceWme distance;
	private final YawWme yaw;
	private final YawWme relYaw;
	private final YawWme absRelYaw;
	private boolean destroyed = false;
	private RelativePointData rpd;

	PointDataIL(Identifier root, double[] pos) {
		this.root = root;
		this.pos = new double[pos.length];
		System.arraycopy(pos, 0, this.pos, 0, pos.length);

		DistanceWme.newInstance(root, SharedNames.X, this.pos[0]);
		DistanceWme.newInstance(root, SharedNames.Y, this.pos[1]);
		DistanceWme.newInstance(root, SharedNames.Z, this.pos[2]);
		
		distance = DistanceWme.newInstance(root, SharedNames.DISTANCE);
		yaw = YawWme.newInstance(root, SharedNames.YAW);
		relYaw = YawWme.newInstance(root, SharedNames.RELATIVE_BEARING);
		absRelYaw = YawWme.newInstance(root, SharedNames.ABS_RELATIVE_BEARING);
	}

	@Override
	public void update(Adaptable app) {
		if (destroyed) {
			throw new IllegalStateException();
		}
		
		Pose pose = (Pose)app.getAdapter(Pose.class);
		rpd = Pose.getRelativePointData(pose.getPose(), pos);

		distance.update(rpd.distance);
		yaw.update(rpd.yaw);
		relYaw.update(rpd.relativeYaw);
		absRelYaw.update(Math.abs(rpd.relativeYaw));
	}
	
	RelativePointData getRelativePointData() {
		return rpd;
	}
	
	@Override
	public void destroy() {
		this.root.DestroyWME();
		destroyed = true;
	}
}

package edu.umich.soar.sproom.soar;

import lcmtypes.pose_t;
import sml.Agent;
import sml.Identifier;
import edu.umich.soar.IntWme;
import edu.umich.soar.StringWme;
import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.Pose;

public class SelfIL implements InputLinkElement {
	private static final String NAME = "name";
	private static final String AREA = "area";

	private IntWme area;
	private DistanceWme[] xyz = new DistanceWme[3];
	private SpeedWme[] xyzVelocity = new SpeedWme[3];
	private YawWme yaw;
	private YawVelocityWme yawVelocity;
	
	public SelfIL(Identifier root, Adaptable app) {
		Agent agent = (Agent)app.getAdapter(Agent.class);
		
		StringWme.newInstance(root, NAME, agent.GetAgentName());
		area = IntWme.newInstance(root, AREA);
		
		Identifier pose = root.CreateIdWME(SharedNames.POSE);
		xyz[0] = DistanceWme.newInstance(pose, SharedNames.X);
		xyz[1] = DistanceWme.newInstance(pose, SharedNames.Y);
		xyz[2] = DistanceWme.newInstance(pose, SharedNames.Z);
		yaw = YawWme.newInstance(pose, SharedNames.YAW);
		xyzVelocity[0] = SpeedWme.newInstance(pose, SharedNames.X_VELOCITY);
		xyzVelocity[1] = SpeedWme.newInstance(pose, SharedNames.Y_VELOCITY);
		xyzVelocity[2] = SpeedWme.newInstance(pose, SharedNames.Z_VELOCITY);
		yawVelocity = YawVelocityWme.newInstance(pose, SharedNames.YAW_VELOCITY);
		
		update(app);
	}

	@Override
	public void update(Adaptable app) {
		area.update(0); // FIXME: temp

		Pose poseClass = (Pose)app.getAdapter(Pose.class);
		pose_t pose = poseClass.getPose();
		
		for (int i = 0; i < xyz.length; ++i) {
			xyz[i].update(pose.pos[i]);
			xyzVelocity[i].update(pose.vel[i]);
		}
		yaw.update(poseClass.getYaw());
		yawVelocity.update(pose.rotation_rate[2]);
	}
}

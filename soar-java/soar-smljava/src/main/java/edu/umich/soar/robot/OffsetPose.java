package edu.umich.soar.robot;

import lcmtypes.pose_t;

public interface OffsetPose {
	pose_t getPose();
	void setOffset(pose_t offset);
	pose_t getOffset();
}

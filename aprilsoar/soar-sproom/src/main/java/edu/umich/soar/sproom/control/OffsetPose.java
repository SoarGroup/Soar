package edu.umich.soar.sproom.control;

import lcmtypes.pose_t;

public interface OffsetPose {
	pose_t getPose();
	void setOffset(pose_t offset);
	pose_t getOffset();
}

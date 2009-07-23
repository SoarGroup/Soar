package edu.umich.soar.sps.control;

import edu.umich.soar.robot.OffsetPose;
import sml.Identifier;

abstract class NoDDCAdapter implements Command {

	public boolean createsDDC() {
		return false;
	}

	public DifferentialDriveCommand getDDC() {
		throw new AssertionError();
	}

	public void interrupt() {
	}

	public Identifier wme() {
		throw new AssertionError();
	}

	public boolean update(OffsetPose opose) {
		throw new AssertionError();
	}
}

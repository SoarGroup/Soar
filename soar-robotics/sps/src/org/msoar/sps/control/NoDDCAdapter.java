package org.msoar.sps.control;

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

	public boolean update(SplinterState splinter) {
		throw new AssertionError();
	}
}

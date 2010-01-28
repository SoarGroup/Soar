package edu.umich.soar.sps.control.robot;

import sml.Identifier;

abstract class NoDDCAdapter implements Command {

	@Override
	public boolean createsDDC() {
		return false;
	}

	@Override
	public DifferentialDriveCommand getDDC() {
		throw new AssertionError();
	}

	@Override
	public void interrupt() {
	}

	@Override
	public Identifier wme() {
		throw new AssertionError();
	}

	@Override
	public boolean update() {
		throw new AssertionError();
	}
}

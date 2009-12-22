package edu.umich.soar.sproom.command;

interface DriveCommand {
	DifferentialDriveCommand getDDC();
	void interrupt();
	Integer getTimeTag();
}

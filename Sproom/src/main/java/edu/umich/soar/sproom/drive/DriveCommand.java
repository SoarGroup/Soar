package edu.umich.soar.sproom.drive;

public interface DriveCommand {
	public DifferentialDriveCommand getDDC();
	public void interrupt();
	public Integer getTimeTag();
}

package edu.umich.soar.sproom.drive;

/**
 * Interface implemented by things listening to drive commands.
 *
 * @author voigtjr@gmail.com
 */
public interface DriveListener {
	public void handleDriveEvent(DifferentialDriveCommand d);
}

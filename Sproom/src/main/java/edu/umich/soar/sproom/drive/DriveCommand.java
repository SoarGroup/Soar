package edu.umich.soar.sproom.drive;

/**
 * Interface implemented by things issuing drive commands.
 *
 * @author voigtjr@gmail.com
 */
public interface DriveCommand {
	public DifferentialDriveCommand getDDC();
	public void interrupt();
	public Integer getTimeTag();
}

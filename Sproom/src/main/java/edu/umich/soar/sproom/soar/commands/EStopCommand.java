/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import sml.Identifier;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.drive.DifferentialDriveCommand;
import edu.umich.soar.sproom.drive.DriveCommand;

/**
 * @author voigtjr
 *
 * Emergency stop.
 */
public class EStopCommand extends OutputLinkCommand implements DriveCommand {
	static final String NAME = "estop";

	public EStopCommand(Identifier wme) {
		super(wme);
	}
	
	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newEStopCommand();
	}
	
	@Override
	protected boolean accept() {
		addStatus(CommandStatus.ACCEPTED);
		return true;
	}
	
	@Override
	public void update(Adaptable app) {
		addStatus(CommandStatus.COMPLETE);
	}

	@Override
	public void interrupt() {
	}
}

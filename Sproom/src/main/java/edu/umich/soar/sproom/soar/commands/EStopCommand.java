/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import sml.Identifier;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.drive.DifferentialDriveCommand;
import edu.umich.soar.sproom.drive.DriveCommand;

/**
 * @author voigtjr
 *
 * Emergency stop.
 */
public class EStopCommand extends OutputLinkCommand implements DriveCommand {
	private static final Log logger = LogFactory.getLog(EStopCommand.class);
	static final String NAME = "estop";

	private final Identifier wme;
	private boolean complete = false;

	EStopCommand(Identifier wme) {
		super(Integer.valueOf(wme.GetTimeTag()));
		this.wme = wme;
	}
	
	@Override
	public String getName() {
		return NAME;
	}
	
	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newEStopCommand();
	}
	
	@Override
	public OutputLinkCommand accept() {
		logger.debug(NAME + ":");
		CommandStatus.accepted.addStatus(wme);
		return this;
	}
	
	@Override
	public void update(Adaptable app) {
		if (!complete) {
			CommandStatus.complete.addStatus(wme);
			complete = true;
		}
	}

	@Override
	public void interrupt() {
	}
}

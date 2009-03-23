/**
 * 
 */
package org.msoar.sps.control;

import sml.Identifier;

interface Command {
	CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager);
	boolean createsDDC();
	boolean isInterruptable();
	DifferentialDriveCommand getDDC();
}
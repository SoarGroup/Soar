/**
 * 
 */
package edu.umich.soar.robot;

import sml.Identifier;

interface Command {
	/**
	 * @param command The command wme to store.
	 * @param outputLinkManager The calling instance.
	 * @return true on success
	 */
	boolean execute(Identifier command);
	/**
	 * @return true if command is done executing.
	 */
	boolean update();
	/**
	 * Command is interrupted.
	 */
	void interrupt();
	/**
	 * @return Returns command wme.
	 */
	Identifier wme();
	/**
	 * @return true if the command generates a DDC.
	 */
	boolean createsDDC();
	/**
	 * @return Generated DDC.
	 */
	DifferentialDriveCommand getDDC();
}
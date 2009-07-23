/**
 * 
 */
package edu.umich.soar.robot;

import sml.Agent;
import sml.Identifier;

public interface Command {
	/**
	 * @param agent Soar agent.
	 * @param command The command wme to store.
	 * @param opose Current state.
	 * @param outputLinkManager The calling instance.
	 * @return true on success
	 */
	boolean execute(Agent agent, Identifier command, OffsetPose opose);
	/**
	 * @param opose Current state.
	 * @return true if command is done executing.
	 */
	boolean update(OffsetPose opose);
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
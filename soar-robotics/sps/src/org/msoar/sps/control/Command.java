/**
 * 
 */
package org.msoar.sps.control;


import lcmtypes.pose_t;
import sml.Identifier;

interface Command {
	public CommandStatus execute(Identifier command, pose_t pose, OutputLinkManager outputLinkManager);
	public boolean modifiesInput();
	public boolean isInterruptable();
	public void updateInput(SplinterInput input);
}
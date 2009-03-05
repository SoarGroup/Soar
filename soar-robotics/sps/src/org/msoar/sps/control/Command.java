/**
 * 
 */
package org.msoar.sps.control;

import lcmtypes.pose_t;
import sml.Identifier;

interface Command {
	CommandStatus execute(InputLinkInterface inputLink, Identifier command, pose_t pose, OutputLinkManager outputLinkManager);
	boolean modifiesInput();
	boolean isInterruptable();
	void updateInput(SplinterInput input);
}
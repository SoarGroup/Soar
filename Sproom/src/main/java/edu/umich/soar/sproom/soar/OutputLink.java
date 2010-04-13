package edu.umich.soar.sproom.soar;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.drive.DifferentialDriveCommand;
import edu.umich.soar.sproom.drive.DriveCommand;
import edu.umich.soar.sproom.soar.commands.OutputLinkCommand;

import sml.Agent;
import sml.WMElement;
import sml.Agent.OutputNotificationInterface;

/**
 * Output link management, responsible for reading agent commands and producing drive and other commands.
 *
 * @author voigtjr@gmail.com
 */
class OutputLink {
	private static final Log logger = LogFactory.getLog(OutputLink.class);
	
	private final Agent agent;
	private final Map<Integer, OutputLinkCommand> commands = new HashMap<Integer, OutputLinkCommand>();
	private DriveCommand driveCommand;
	private final Adaptable app;
	
	static OutputLink newInstance(Adaptable app) {
		return new OutputLink(app);
	}
	
	private OutputLink(Adaptable app) {
		this.app = app;
		this.agent = (Agent)app.getAdapter(Agent.class);
		
		agent.RegisterForOutputNotification(new OutputNotificationInterface() {
			@Override
			public void outputNotificationHandler(Object data, Agent agent) {

				// invalidate any removals
				for (int i = 0; i < agent.GetNumberOutputLinkChanges(); ++i) {
					if (agent.IsOutputLinkChangeAdd(i) == false) {
						WMElement wme = agent.GetOutputLinkChange(i);
						OutputLinkCommand command = commands.get(wme.GetTimeTag());
						if (command != null) {
							logger.debug("Invalidating " + command);
							command.invalidateWme();
						}
					}
				}
				
				// add new commands
				for (int i = 0; i < agent.GetNumberCommands(); ++i) {
					OutputLinkCommand command = OutputLinkCommand.valueOf(agent.GetCommand(i));
					if (command == null) {
						continue;
					}

					commands.put(command.getTimeTag(), command);
					
					if (logger.isDebugEnabled())
						logger.debug("Accepted: " + command);
				}

				// done with current change list
				agent.ClearOutputLinkChanges();
				
				// update current commands
				for (OutputLinkCommand command : commands.values()) {
					// check if is terminated, can be terminated out of sequence
					// by interrupt() below.
					if (command.isTerminated()) {
						continue;
					}
					
					if (logger.isTraceEnabled()) {
						logger.trace("Updating " + command);
					}
					
					command.update(OutputLink.this.app);
					
					if (command instanceof DriveCommand) {

						if (!command.equals(driveCommand)) {
							if (driveCommand != null) {
								if (logger.isDebugEnabled())
									logger.debug(String.format("Interrupting %s with %s", driveCommand, command));
								driveCommand.interrupt();
							}
							
							driveCommand = (DriveCommand)command;
						}
					}
				}
				
				// remove terminated commands
				Iterator<Map.Entry<Integer, OutputLinkCommand>> iter = commands.entrySet().iterator();
				while (iter.hasNext()) {
					Map.Entry<Integer, OutputLinkCommand> entry = iter.next();
					if (entry.getValue().isTerminated()) {
						if (entry.getValue().equals(driveCommand)) {
							driveCommand = null;
						}
						if (logger.isDebugEnabled()) {
							logger.debug("Removing " + entry.getValue());
						}
						iter.remove();
					}
				}
			}
		}, null);
	}
	
	DifferentialDriveCommand getDDC() {
		return driveCommand != null ? driveCommand.getDDC() : null;
	}
	
}

package edu.umich.soar.sproom.soar.commands;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;

import sml.Identifier;
import sml.StringElement;

/**
 * Parent abstract class for output link commands, contains many utility functions like converting
 * an identifier to its command handler.
 *
 * @author voigtjr@gmail.com
 */
public abstract class OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(OutputLinkCommand.class);
	private final static Map<String, Class<? extends OutputLinkCommand>> commands = new HashMap<String, Class<? extends OutputLinkCommand>>();
	
	static {
		commands.put(EStopCommand.NAME, EStopCommand.class);
		commands.put(StopCommand.NAME, StopCommand.class);
		commands.put(MotorCommand.NAME, MotorCommand.class);
		commands.put(SetLinearVelocityCommand.NAME, SetLinearVelocityCommand.class);
		commands.put(SetAngularVelocityCommand.NAME, SetAngularVelocityCommand.class);
		commands.put(SetVelocityCommand.NAME, SetVelocityCommand.class);
		commands.put(SetHeadingCommand.NAME, SetHeadingCommand.class);
		commands.put(SetHeadingLinearCommand.NAME, SetHeadingLinearCommand.class);
		
		commands.put(ConfigureCommand.NAME, ConfigureCommand.class);

		commands.put(AddWaypointCommand.NAME, AddWaypointCommand.class);
		commands.put(EnableWaypointCommand.NAME, EnableWaypointCommand.class);
		commands.put(DisableWaypointCommand.NAME, DisableWaypointCommand.class);
		commands.put(RemoveWaypointCommand.NAME, RemoveWaypointCommand.class);

		commands.put(SendMessageCommand.NAME, SendMessageCommand.class);
		commands.put(RemoveMessageCommand.NAME, RemoveMessageCommand.class);
		commands.put(ClearMessagesCommand.NAME, ClearMessagesCommand.class);
		
		commands.put(GetObjectCommand.NAME, GetObjectCommand.class);
		commands.put(DropObjectCommand.NAME, DropObjectCommand.class);
	}
	
	public static OutputLinkCommand valueOf(Identifier wme) {
		String name = wme.GetAttribute();
		
		Class<? extends OutputLinkCommand> klass = commands.get(name);
		if (klass != null) {
			try {
				Constructor<? extends OutputLinkCommand> ctor = klass.getConstructor(new Class<?>[] { Identifier.class });
				OutputLinkCommand command = ctor.newInstance(wme);
				return command.accept() ? command : null;
				
			} catch (InvocationTargetException e) {
				logger.error(e.getMessage());
				return null;
			} catch (NoSuchMethodException e) {
				logger.error(e.getMessage());
				return null;
			} catch (IllegalAccessException e) {
				logger.error(e.getMessage());
				return null;
			} catch (InstantiationException e) {
				logger.error(e.getMessage());
				return null;
			}
		}
		
		logger.warn("No such command: " + name);
		return null;
	}

	protected enum CommandStatus {
		ACCEPTED,
		EXECUTING,
		COMPLETE,
		ERROR,
		INTERRUPTED;
		
		private static final Log logger = LogFactory.getLog(CommandStatus.class);
		private static final String STATUS = "status";
		
		private StringElement addStatus(Identifier command) {
			return command.CreateStringWME(STATUS, this.toString().toLowerCase());
		}
		
		private StringElement addStatus(Identifier command, String message) {
			if (this.equals(ERROR)) {
				logger.warn(command.GetAttribute() + " message: " + message);
			} else {
				logger.info(command.GetAttribute() + " message: " + message);
			}
			command.CreateStringWME("message", message);
			return command.CreateStringWME(STATUS, this.toString().toLowerCase());
		}
		
		private boolean isTerminated() {
			return this.equals(COMPLETE) || this.equals(INTERRUPTED) || this.equals(ERROR);
		}
	}

	private Identifier wme;
	private final String name;
	private final int tt;
	private CommandStatus status;

	protected OutputLinkCommand(Identifier commandwme) {
		this.wme = commandwme;
		
		// cache these so they are accessible after the wme is invalid
		this.name = commandwme.GetAttribute();
		this.tt = commandwme.GetTimeTag();
	}
	
	public void invalidateWme() {
		wme = null;
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder("(");
		sb.append(name);
		sb.append(": ");
		sb.append(tt);
		sb.append(")");
		return sb.toString();
	}
	
	public Integer getTimeTag() {
		return tt;
	}
	
	@Override
	public boolean equals(Object obj) {
		if (obj instanceof OutputLinkCommand) {
			OutputLinkCommand olc = (OutputLinkCommand)obj;
			return tt == olc.tt;
		}
		return super.equals(obj);
	}

	@Override
	public int hashCode() {
		return Integer.valueOf(tt).hashCode();
	}

	protected void addStatusError(String message) {
		addStatus(CommandStatus.ERROR, message);
	}
	
	protected void addStatus(CommandStatus status) {
		addStatus(status, null);
	}
	
	private void addStatus(CommandStatus status, String message) {
		if (this.status != status) {
			this.status = status;
			
			if (wme != null) {
				if (message == null) {
					status.addStatus(wme);
				} else {
					status.addStatus(wme, message);
				}

				if (isTerminated()) {
					wme = null;
				}
			}
			
			if (logger.isDebugEnabled() || status == CommandStatus.ERROR) {
				String msg;
				if (message == null) {
					msg = String.format("%s (%d): %s", name, tt, status.toString());
				} else {
					msg = String.format("%s (%d): %s: %s", name, tt, status.toString(), message);
				}

				if (status != CommandStatus.ERROR) {
					logger.debug(msg);
				} else {
					logger.warn(msg);
				}
			}
		}
	}
	
	protected boolean isComplete() {
		return status == CommandStatus.COMPLETE;
	}
	
	public boolean isTerminated() {
		return status == null || status.isTerminated();
	}

	/**
	 * Parse the command and mark it status accepted or status error (some kind
	 * of parsing or syntax error). Do not mark any other statuses at this
	 * point.
	 * 
	 * @return True if status accepted, false if status error.
	 */
	protected abstract boolean accept();
	public abstract void update(Adaptable app);
	
}

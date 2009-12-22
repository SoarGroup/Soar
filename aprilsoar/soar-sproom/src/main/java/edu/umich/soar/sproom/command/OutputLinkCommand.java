package edu.umich.soar.sproom.command;

import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;

import lcmtypes.pose_t;

import sml.Identifier;

abstract class OutputLinkCommand {
	private final static Map<String, Class<? extends OutputLinkCommand>> commands = new HashMap<String, Class<? extends OutputLinkCommand>>();
	
	static {
		// TODO: use reflection to load commands
		commands.put(EStopCommand.NAME, EStopCommand.class);
		commands.put(StopCommand.NAME, StopCommand.class);
		commands.put(MotorCommand.NAME, MotorCommand.class);
		commands.put(SetLinearVelocityCommand.NAME, SetLinearVelocityCommand.class);
		commands.put(SetAngularVelocityCommand.NAME, SetAngularVelocityCommand.class);
		commands.put(SetVelocityCommand.NAME, SetVelocityCommand.class);
		commands.put(SetHeadingCommand.NAME, SetHeadingCommand.class);
		commands.put(SetHeadingLinearCommand.NAME, SetHeadingLinearCommand.class);
	}
	
	static OutputLinkCommand valueOf(Identifier wme) {
		String name = wme.GetAttribute();
		
		// TODO: profile: a prototype pattern might be a better option
		Class<? extends OutputLinkCommand> klass = commands.get(name);
		if (klass != null) {
			try {
				OutputLinkCommand command = klass.getConstructor(Identifier.class).newInstance(wme);
				return command.accept();
				
			} catch (InvocationTargetException e) {
				e.printStackTrace();
				return new InvalidCommand(wme, e.getClass().toString());
			} catch (NoSuchMethodException e) {
				e.printStackTrace();
				return new InvalidCommand(wme, e.getClass().toString());
			} catch (IllegalAccessException e) {
				e.printStackTrace();
				return new InvalidCommand(wme, e.getClass().toString());
			} catch (InstantiationException e) {
				e.printStackTrace();
				return new InvalidCommand(wme, e.getClass().toString());
			}
		}
		
		return new InvalidCommand(wme, "No such command.");
	}

	private final Integer tt;

	protected OutputLinkCommand(Integer tt) {
		this.tt = tt;
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder("(");
		sb.append(getName());
		sb.append(": ");
		sb.append(tt);
		sb.append(")");
		return sb.toString();
	}
	
	public abstract String getName();
	public abstract OutputLinkCommand accept();
	public abstract boolean update(pose_t pose);
	
	public Integer getTimeTag() {
		return tt;
	}
	
	@Override
	public boolean equals(Object obj) {
		if (obj instanceof OutputLinkCommand) {
			OutputLinkCommand olc = (OutputLinkCommand)obj;
			return tt.equals(olc.tt);
		}
		return super.equals(obj);
	}

	@Override
	public int hashCode() {
		return tt.hashCode();
	}
}

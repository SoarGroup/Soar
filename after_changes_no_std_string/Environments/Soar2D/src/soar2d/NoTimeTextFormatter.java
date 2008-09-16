package soar2d;

import java.util.logging.*;

public class NoTimeTextFormatter extends Formatter {
	public NoTimeTextFormatter() {
		super();
	}

	public String format(LogRecord record) {
		String output = Integer.toString(Soar2D.simulation.world.getWorldCount());
		output += " ";
		output += record.getLevel().getName();
		output += " ";
		output += record.getMessage();
		output += java.lang.System.getProperty("line.separator");
		return output;
	}

}

package soar2d;

import java.text.*;
import java.util.*;
import java.util.logging.Formatter;
import java.util.logging.*;

public class TextFormatter extends Formatter {
	SimpleDateFormat format = new SimpleDateFormat("HH:mm:ss");
	
	public TextFormatter() {
		super();
	}

	public String format(LogRecord record) {
		Date d = new Date(record.getMillis());
		String output = format.format(d);
		output += " ";
		output += record.getLevel().getName();
		output += " ";
		output += record.getMessage();
		output += java.lang.System.getProperty("line.separator");
		return output;
	}

}

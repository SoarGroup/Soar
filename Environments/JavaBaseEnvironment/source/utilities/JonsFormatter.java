package utilities;

import java.text.DateFormat;
import java.util.Date;
import java.util.logging.Formatter;
import java.util.logging.LogRecord;

public class JonsFormatter extends Formatter {

	public JonsFormatter() {
		super();
	}

	public String format(LogRecord record) {
		Date d = new Date(record.getMillis());
		String output = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.LONG).format(d);
		output += " ";
		output += record.getLoggerName();
		output += " ";
		output += record.getLevel().getName();
		output += " ";
		output += record.getMessage();
		output += "\n";
		return output;
	}

}

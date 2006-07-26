package utilities;

import java.text.*;
import java.util.Date;
import java.util.logging.Formatter;
import java.util.logging.LogRecord;

public class JonsFormatter extends Formatter {
	SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss,SSS");
	
	public JonsFormatter() {
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

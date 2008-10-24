package splintersoar;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.logging.Formatter;
import java.util.logging.LogRecord;

public class TextFormatter extends Formatter {
	SimpleDateFormat format = new SimpleDateFormat("HH:mm:ss");

	public TextFormatter() {
		super();
	}

	@Override
	public String format(LogRecord record) {
		Date d = new Date(record.getMillis());
		StringBuilder output = new StringBuilder();
		output.append(format.format(d));
		output.append(String.format(" %s (%s): %s%n", record.getLoggerName(), record.getLevel().getName(), record.getMessage()));
		return output.toString();
	}
}

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

	public String format(LogRecord record) {
		Date d = new Date(record.getMillis());
		StringBuilder output = new StringBuilder();
		output.append( format.format( d ) );
		output.append( " " );
		output.append( record.getLevel().getName() );
		output.append( " " );
		output.append( record.getMessage() );
		output.append( java.lang.System.getProperty("line.separator") );
		return output.toString();
	}

}


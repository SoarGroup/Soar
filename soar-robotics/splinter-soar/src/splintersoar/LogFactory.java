package splintersoar;

import java.util.logging.ConsoleHandler;
import java.util.logging.Level;
import java.util.logging.Logger;

public class LogFactory {

	private static Logger logger;
	public static Logger simpleLogger()
	{
		if ( logger != null )
			return logger;
		
		logger = Logger.getLogger(Logger.GLOBAL_LOGGER_NAME);
		
		Level level = Level.ALL;
		ConsoleHandler handler = new ConsoleHandler();
		handler.setFormatter(new TextFormatter());
		handler.setLevel( level );
		logger.setLevel( level );
		logger.addHandler(handler);
		logger.setUseParentHandlers( false );

		return logger;
	}
}

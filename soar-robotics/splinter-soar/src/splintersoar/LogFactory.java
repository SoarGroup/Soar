package splintersoar;

import java.util.logging.ConsoleHandler;
import java.util.logging.Level;
import java.util.logging.Logger;

public class LogFactory {

	public static Logger createSimpleLogger( Level level )
	{
		Logger logger = Logger.getLogger(Logger.GLOBAL_LOGGER_NAME);
		
		ConsoleHandler handler = new ConsoleHandler();
		handler.setFormatter(new TextFormatter());
		handler.setLevel( level );
		logger.setLevel( level );
		logger.addHandler(handler);
		logger.setUseParentHandlers( false );

		return logger;
	}
}

package splintersoar;

import java.util.logging.ConsoleHandler;
import java.util.logging.Level;
import java.util.logging.Logger;

public class LogFactory {

	public static Logger createSimpleLogger(String component, Level level) {
		Logger logger = Logger.getLogger(component);

		ConsoleHandler handler = new ConsoleHandler();
		handler.setFormatter(new TextFormatter());
		handler.setLevel(level);
		logger.setLevel(level);
		logger.addHandler(handler);
		logger.setUseParentHandlers(false);

		return logger;
	}

	public static void main(String[] args) {
		Logger logger1 = createSimpleLogger("logger1", Level.ALL);
		Logger logger2 = createSimpleLogger("logger2", Level.INFO);
		Logger logger1copy = Logger.getLogger("logger1");

		logger1.info("info test 1");
		logger2.info("info test 2");
		logger1copy.info("info test 3");
		logger1.fine("fine test 1");
		logger2.fine("fine test 2");
		logger1copy.fine("fine test 3");

	}
}

/////////////////////////////////////////////////////////////////
// smem command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com,
//         Nate Derbinsky, nlderbin@umich.edu
// Date  : 2009
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "agent.h"
#include "misc.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace cli;
using namespace sml;

namespace Logging
{
	class Logger
	{
	protected:
		sml::agent* theAgent;
		
	public:
		virtual void trace(std::string output) = 0;
		virtual void debug(std::string output) = 0;
		virtual void info(std::string output) = 0;
		virtual void warn(std::string output) = 0;
		virtual void error(std::string output) = 0;
	};
	
	class DefaultLogger : public Logger
	{
		DefaultLogger(sml::agent* theAgent)
		: this->::Logger.theAgent(theAgent)
		{}
	public:
		virtual void trace(std::string output)
		{
			
		}
		
		virtual void debug(std::string output)
		{
			
		}
		
		virtual void info(std::string output)
		{
			
		}
		
		virtual void warn(std::string output)
		{
			
		}
		
		virtual void error(std::string output)
		{
			
		}
		
	private:
		void print(std::string output)
		{
			xml_object(thisAgent, kTagRHS_write, kRHS_String, output.c_str());
		}
	};
	
	class LogManager
	{
	public:
#pragma mark - Class-Types
		
#pragma mark - Casting
		template <typename E>
		constexpr typename std::underlying_type<E>::type to_underlying(E e) {
			return static_cast<typename std::underlying_type<E>::type>(e);
		}
		
#pragma mark - EchoMode
		enum class EchoMode : char
		{
			off,
			simple,
			on,
			unknown
		}
		
		std::string echoModeToString(EchoMode e)
		{
			switch (e)
			{
				case off: return "OFF";
				case simple: return "SIMPLE";
				case on: return "ON";
				default: return "UNKNOWN";
			}
		}
		
		EchoMode stringToEchoMode(std::string echoMode)
		{
			if (strncasecmp(echoMode.c_str(), "OFF", 3) == 0)
				return EchoMode.off;
			else if (strncasecmp(echoMode.c_str(), "SIMPLE", 6) == 0)
				return EchoMode.simple;
			else if (strncasecmp(echoMode.c_str(), "ON", 2) == 0)
				return EchoMode.on;
			
			return EchoMode.unknown;
		}
		
#pragma mark - SourceLocationMethod
		enum class SourceLocationMethod : char
		{
			none,
			disk,
			stack,
			unknown
		}
		
		std::string sourceLocationMethodToString(SourceLocationMethod slm)
		{
			switch (slm)
			{
				case none: return "NONE";
				case disk: return "DISK";
				case stack: return "STACK";
				default: return "UNKNOWN";
			}
		}
		
		SourceLocationMethod stringToSourceLocationMethod(std::string slm)
		{
			if (strncasecmp(slm.c_str(), "NONE", 4) == 0)
				return SourceLocationMethod.none;
			else if (strncasecmp(slm.c_str(), "DISK", 4) == 0)
				return SourceLocationMethod.disk;
			else if (strncasecmp(slm.c_str(), "STACK", 5) == 0)
				return SourceLocationMethod.stack;
			
			return SourceLocationMethod.unknown;
		}
		
#pragma mark - LogLevel
		enum class LogLevel : char
		{
			unknown = -1,
			trace = 1,
			debug = 2,
			info = 3,
			warn = 4,
			error = 5
		}
		
		std::string logLevelToString(LogLevel ll)
		{
			switch (ll)
			{
				case trace: return "TRACE";
				case debug: return "DEBUG";
				case info: return "INFO";
				case warn: return "WARN";
				case error: return "ERROR";
				default: return "UNKNOWN";
			}
		}
		
		LogLevel stringToLogLevel(std::string ll)
		{
			if (strncasecmp(ll.c_str(), "TRACE", 5) == 0)
				return LogLevel.trace;
			else if (strncasecmp(slm.c_str(), "DEBUG", 5) == 0)
				return LogLevel.debug;
			else if (strncasecmp(slm.c_str(), "INFO", 4) == 0)
				return LogLevel.info;
			else if (strncasecmp(slm.c_str(), "WARN", 4) == 0)
				return LogLevel.warn;
			else if (strncasecmp(slm.c_str(), "ERROR", 5) == 0)
				return LogLevel.error;
		}
		
		// Variables
#pragma mark - Variables
		EchoMode echoMode = EchoMode.on;
		boolean active = true;
//		boolean strict = false;
		boolean abbreviate = true;
		SourceLocationMethod sourceLocationMethod = SourceLocationMethod.disk;
		LogLevel currentLogLevel = LogLevel.info;
		std::unordered_map<std::string, Logger*> loggers;
		std::unordered_set<std::string> disabledLoggers;
		
		static const std::string format = "%F %T";
		
		// Exceptions? Soar doesn't use exceptions...
//		public class LoggerException extends Exception
//		{
//			private static final long serialVersionUID = 1L;
//			public LoggerException(String message)
//			{
//				super(message);
//			}
//		}
		
#pragma mark - Constructors
		LogManager()
		{
			loggers.insert("default", new DefaultLogger(nullptr));
		}
		
#pragma mark - Getters
		Logger* getLogger(std::string loggerName)
		{
//			Logger logger = loggers.get(loggerName);
//			if (logger == null)
//			{
//				if (strict)
//					throw new LoggerException("Logger [" + loggerName + "] does not exists (strict mode enabled).");
//				logger = LoggerFactory.getLogger(loggerName);
//				loggers.put(loggerName, logger);
//			}
//			return logger;
			
			return loggers[loggerName];
		}
		
//		public Set<String> getLoggerNames()
//		{
//			return new HashSet<String>(loggers.keySet());
//		}
		
		size_t getLoggerCount()
		{
			return loggers.size();
		}
		
//		public Logger addLogger(String loggerName) throws LoggerException
//		{
//			Logger logger = loggers.get(loggerName);
//			if (logger != null)
//			{
//				if (strict)
//					throw new LoggerException("Logger [" + loggerName + "] already exists (strict mode enabled).");
//    	}
//			else
//			{
//				logger = LoggerFactory.getLogger(loggerName);
//				loggers.put(loggerName, logger);
//			}
//			return logger;
//		}
		
		void addLogger(std::string loggerName, Logger* logger)
		{
			logger[loggerName] = logger;
		}
		
		bool hasLogger(std::string loggerName)
		{
			return logger.find(loggerName) != logger.end();
		}
		
		std::string getLoggerStatus()
		{
			std::string result
					= "      Log Settings     \n";
			result += "=======================\n";
			result += "logging:           " + (isActive() ? "on" : "off") + "\n";
			result += "strict:            " + (isStrict() ? "on" : "off") + "\n";
			result += "echo mode:         " + echoModeToString(getEchoMode()) + "\n";
			result += "log level:         " + logLevelToString(getLogLevel()).toLowerCase() + "\n";
			result += "source location:   " + sourceLocationToString(getSourceLocationMethod()).toLowerCase() + "\n";
			result += "abbreviate:        " + (getAbbreviate() ? "yes" : "no") + "\n";
			result += "number of loggers: " + loggers.size() + "\n";
			result += "------- Loggers -------\n";
			
			for (auto loggerName : loggers)
			{
				result += (disabledLoggers.find(loggerName) == disabledLoggers.end() ? "*" : " ") + " " + loggerName.first + "\n";
			}
			
			return result;
		}
		
#pragma mark - Logging Methods
		void log(std::string loggerName, LogLevel logLevel, std::vector<std::string> args, bool collapse)
		{
			if (!isActive())
				return;
			
			Logger* logger = getLogger(loggerName);
			
			String result = formatArguments(args, collapse);
			
			if (logLevel == LogLevel.debug)
				logger.debug(result);
			else if (logLevel == LogLevel.info)
				logger.info(result);
			else if (logLevel == LogLevel.warn)
				logger.warn(result);
			else if (logLevel == LogLevel.trace)
				logger.trace(result);
			else
				logger.error(result);
			
			if (echoMode != EchoMode.off &&
				to_underlying(currentLogLevel) >= to_underlying(logLevel) &&
				disabledLoggers.find(loggerName) != disabledLoggers.end())
			{
				agent.getPrinter().startNewLine();
				
				if (echoMode == EchoMode.simple)
					xml_object(thisAgent, kTagRHS_write, kRHS_String, result.c_str());
				else
				{
					string output = "[" + logLevel.toString() + " " + getTimestamp() + "] " + loggerName + ": " + result;
					
					xml_object(thisAgent, kTagRHS_write, kRHS_String, output.c_str());
				}
			}
		}
		
		std::string formatArguments(std::vector<std::string> args, bool collapse)
		{
			if (args.size() > 1)
			{
				String formatString = args.get(0);
				
				if (formatString.find("{}") != std::string::npos)
				{
					size_t pos = 0;
					
					auto it = ++(args.begin());
					
					while ((pos = formatString.find("{}", pos)) != std::string::npos)
					{
						std::string replace = *(it++);
						
						formatString.replace(pos, 2, replace);
						pos += replace.length();
						
						if (it == args.end())
							break;
					}
				}
				
				return formatString;
			}
			
			if (args.size() == 1)
				return args[0];
			else
				return "";
		}
		
		static std::string getTimestamp()
		{
			char buffer[100];
			
			time_t rawtime;
			time(&rawtime);
			
			struct tm* timeInfo = localtime(&rawtime);
			
			strftime(buffer, 100, format.c_str(), timeInfo);
			
			return std::string(buffer);
		}
		
		bool isActive() { return active; }
		bool isStrict() { return strict; }
		bool getAbbreviate() { return abbreviate; }
		EchoMode getEchoMode() { return echoMode; }
		LogLevel getLogLevel() { return currentLogLevel; }
		SourceLocationMethod getSourceLocationMethod() { return sourceLocationMethod; }
		
		void setStrict(bool strict) { this->strict = strict; }
		void setEchoMode(EchoMode echoMode) { this->echoMode = echoMode; }
		void setLogLevel(LogLevel logLevel) { this->logLevel = logLevel; }
		void setSourceLocationMethod(SourceLocationMethod sourceLocationMethod) { this->sourceLocationMethod = sourceLocationMethod; }
		void setAbbreviate(bool abbreviate) { this->abbreviate = abbreviate; }
		
		void enableLogger(std::string name)
		{
			if (disabledLoggers.find(name) != disabledLoggers.end())
				disabledLoggers.erase(disabledLoggers.find(name));
		}
		
		void disableLogger(std::string name)
		{
			if (loggers.find(name) != loggers.end())
				disabledLoggers.insert(name);
		}
	}
	
	LogManager logManager;
}

bool CommandLineInterface::DoLog(const char pOp, const std::string* pAttr, const std::string* pVal)
{
	agent* thisAgent = m_pAgentSML->GetSoarAgent();
	std::ostringstream tempString;
	
	if (!pOp)
	{
		// Print Log Settings
		PrintCLIMessage(Logging::logManager.getLoggerStatus());
		
		return true;
	}
	else if (pOp == 'e' || pOp == 'y')
	{
		if (pAttr->length() == 0)
		{
			if (Logging::logManager.isActive())
				return "Logger already enabled.";
			else
			{
				Logging::logManager.setActive(true);
				return "Logging enabled.";
			}
		}
		else
		{
			Logging::logManager.enableLogger(*pAttr);
			
			return "Logger [" + pAttr + "] enabled.";
		}
		
		return result;
	}
	else if (pOp == 'd' || pOp == 'n')
	{
		if (pAttr->length() == 0)
		{
			if (!Logging::logManager.isActive())
				return "Logger already disabled.";
			else
			{
				Logging::logManager.setActive(false);
				return "Logging disabled.";
			}
		}
		else
		{
			Logging::logManager.enableLogger(*pAttr);
			
			return "Logger [" + pAttr + "] enabled.";
		}
		
		return result;
	}
	else if (pOp == 'l')
	{
		Logging::logManager.setLogLevel(*stringToLogLevel(pAttr));
		
		return "Logger level set to: " + pAttr;
	}
	else
	{
		
	}
	
	return SetError("Unknown option.");
}

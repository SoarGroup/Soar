/////////////////////////////////////////////////////////////////
// Sample logging application for SML
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Feb 2006
//
// This application shows an example of how to build a simple
// logging tool.  To use it you would run a Soar kernel somewhere
// (in an environment or inside the debugger for example) and then
// run this logger.  The logger listens for certain events and
// then creates a log file.
//
// The idea is that you could take this simple app and modify it
// to log what you need for your specific application, outputing the
// data in whatever format you want.
//
// This sample is broken into two parts:
// LoggerWinC.cpp contains all of the Windows specific code for putting up a little
//                window to control the logging.
// Logger.cpp contains the SML code for actually doing the logging.
//
// This way if you're not using Windows or wish to add logging to an existing app
// the part you're interested in is Logger.cpp
//
/////////////////////////////////////////////////////////////////

#include <string>
extern bool StartLogging(char const* pFilename, bool append, std::string* pErrorMsg) ;
extern bool StopLogging() ;
extern bool IsLogging() ;



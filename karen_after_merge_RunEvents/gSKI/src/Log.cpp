#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
	created:	28:9:2001   16:26
	filename:	chaoslog
	file ext:	cpp
	file path:	d:\projects\countrychaos\code\chaosserver
	author:		jcrossman@soartech.com
	
	purpose:	
*********************************************************************/


#include "Log.h"
#include "gSKI_Error.h"

#ifdef WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif

// For error checking on directory creation
#include <errno.h>

namespace gSKI {

   /*
   ==================================
   
   ==================================
   */
   int Log::Mkdir(std::string &dirName)
   {
#ifdef WIN32
      return _mkdir(dirName.c_str());
#else
      return mkdir(dirName.c_str(),S_IRWXU);
#endif
   }


   /**
    * Constructor
    *
    * Opens up all the logs that need opening.
    *
    * @param szDirectory Directory in under which the log files should be stored
    */
   Log::Log(const char* szDirectory, unsigned char nFlags, Kernel *kernel) : 
                                                      m_nFlags(nFlags),
                                                      m_listenerManager(0),
                                                      m_kernel(kernel)
   {

      // Hold our directory location
      std::string strDirectory;
      if(szDirectory && szDirectory[0] != '\0')
      {
         strDirectory  = szDirectory;
#ifdef WIN32
         strDirectory += "\\";
#else
         strDirectory += "/";
#endif
      }

      // Add the Logs subdirectory
      //strDirectory = strDirectory + std::string("Logs");

      // Make sure the directory exists
      int errorcode;
      errorcode = Mkdir(strDirectory);

      // TODO: Handle log directory creation error appropriately (Consider
      // moving this to some sort of cross platform class or function)
      if ( errorcode == -1 && errno != EEXIST) {
         // Error has occured (check errno values)
         std::string errormsg;
         switch (errno) {
         case EEXIST:
            // Windows and Linux Error: Name of existing file,dir or device
            errormsg = "Name of existing file directory or device!";
            break;
         case ENOENT:
            // Windows and Linux Error: Path not found
            errormsg = "Path not found!";
            break;
#ifndef WIN32
         case EPERM:
            // Linux only Error: Filesystem doesn't support directories
            errormsg = "Filesystem doesn't support directories!";
            break;
         case EFAULT:
            // Linux only Error: pathname outside accessible address space
            errormsg = "Pathname outside of accessible address space!";
            break;
         case EACCES:
            // Linux only Error: parent directory doesn't allow write 
            // or a directory in the pathname did not allow execute 
            errormsg = "Access denied!";
            break;
         case ENAMETOOLONG:
            // Linux only Error: Directory name too long
            errormsg = "Directory name too long!";
            break;
         case ENOTDIR:
            // Linux only Error: a component of the path is not a directory
            errormsg = "A component of the path is not a directory!";
            break;
         case ENOMEM:
            // Linux only Error: insufficient kernel memory was available
            errormsg = "Insufficient kernel memory available!";
            break;
         case EROFS:
            // Linux only Error: read only file system
            errormsg = "Readonly file system!";
            break;
         case ELOOP:
            // Linux only Error: too many symbolic links encountered in path
            errormsg = "Too many symbolic links in path!";
            break;
         case ENOSPC:
            // Linux only Error: no space available for directory
            errormsg = "No space on device available for directory!";
            break;
#endif
         }
      } else {  // No Error has occured
         //
         // Now create the log files
         m_fSysLog.open((strDirectory + std::string("system.log")).c_str());
         m_fErrLog.open((strDirectory + std::string("error.log")).c_str());
         m_fWarnLog.open((strDirectory + std::string("warn.log")).c_str());
         m_fInfoLog.open((strDirectory + std::string("info.log")).c_str());
#ifdef _DEBUG
         m_fDebugLog.open((strDirectory + std::string("debug.log")).c_str());
#endif
      }
   }


   /**
    * Helper method that actually does the logging.
    *
    *  @param strMsg Complete message to be logged
    */
   void Log::logMsg(const std::string& strMsg, std::ofstream &outStream, egSKIPrintEventId EventID)
   {
      //
      // Write output to file
      outStream << strMsg.c_str() << std::endl;

      //
      // Process callback if needed.
      if(m_listenerManager != 0){
         Kernel::tLogListenerManager::tNotifier notification(m_kernel, strMsg.c_str());
         m_listenerManager->Notify(EventID, notification);
      }
   }

   /** 
      Logs an error to all indicated logs

      An error is a serious problem in the system.  In debug mode,
         logging an error will cause execution of the program to
         halt and a MegaAssert to occur.  In release, only the
         message is printed.

      @param szMsg   Message that should be written to the log.
   */
   void Log::Error(const char* szMsg)
   {
     if(!(m_nFlags & ERR_LOG)) return;

     std::string strTmp  = "**** Error: " + std::string(szMsg);

      logMsg(strTmp, m_fErrLog, gSKIEVENT_LOG_ERROR);

      //
      // Because this is an error, we want to flush the log.
      m_fErrLog.flush();
  }

   /** 
      Logs a warning to all indicated logs

      A warning is a problem in the system, but a problem that probably
         won't cause the system to crash.  The problem, however, may result
         in unexpected behavior.  This method does not interrupt 
         the program in any way.

      @param szMsg   Message that should be written to the log.
   */
   void Log::Warning(const char* szMsg)
   {
     if(!(m_nFlags & WARN_LOG)) return;
      std::string strTmp  = "***  Warning: ";
                  strTmp += szMsg;
      logMsg(strTmp, m_fWarnLog, gSKIEVENT_LOG_WARNING);
   }

   /**
      Logs an information message to all indicated logs

      An information message is a message to show that something has occured
         or will be occuring, but isn't necessarily a problem.

      @param szMsg   Message that should be written to the log.
   */
   void Log::Info(const char* szMsg)
   {
      if(!(m_nFlags & INFO_LOG)) return;
      std::string strTmp  = "*    Info: ";
                  strTmp += szMsg;
      logMsg(strTmp, m_fInfoLog, gSKIEVENT_LOG_INFO);
   }

   /**
      Logs a debug message to all indicated logs.

      A debug message is a message you only want output during debugging.
         The function does nothing when not in debug mode.

      @param szMsg   Message that should be written to the log.
   */
   void Log::Debug(const char* szMsg)
   {
   #ifdef _DEBUG
      if(!(m_nFlags & DEBUG_LOG)) return;
      std::string strTmp  = "**   DBG: ";
      strTmp += szMsg;
      logMsg(strTmp, m_fDebugLog, gSKIEVENT_LOG_DEBUG);
   #endif
   }

   /*
   ==================================
   
   ==================================
   */
   void Log::SetCallback(Kernel::tLogListenerManager* listenerManager)
   {
      m_listenerManager = listenerManager;
   }

}

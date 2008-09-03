/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
	created:	28:9:2001   16:08
	filename:	chaoslog
	file ext:	h
	file path:	d:\projects\countrychaos\code\chaosserver
	author:		jcrossman@soartech.com
	
	purpose:	
*********************************************************************/


#ifndef __CHAOS_LOG_H
#define __CHAOS_LOG_H

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "gSKI_Kernel.h"

#include <string>
#include <fstream>

#include "ILog.h"

namespace gSKI {

   /**
   */
   class Log: public ILog
   {
   private:
      
      /**
       * @note This should not be used!
       */
      Log(Log &l);

      /**
       * @note This should not be used!
       */
      void operator=(const Log&);

   public:
      
      /**
         Constructor

         Opens up all the logs that need opening.

         This is the list of available modes.       
         @li CONSOLE  
         @li SYS_LOG  
         @li ERR_LOG  
         @li INFO_LOG 
         @li WARN_LOG 
         @li DEBUG_LOG
         @li ALL      
         @li DEF_ERR  
         @li DEF_WARN 
         @li DEF_INFO 
         @li DEF_DEBUG

         @param szDirectory Directory in under which the log files should be stored
         @param nFlags The set of flags that indicate what kind of logging should
                       be done.
      */
      Log(const char* szDirectory, unsigned char nFlags, Kernel *kernel);


      /** Virtual destructor (usual reasons) */
      virtual ~Log() {}

      /** 
         Logs an error to all indicated logs

         An error is a serious problem in the system.  In debug mode,
            logging an error will cause execution of the program to
            halt and a MegaAssert to occur.  In release, only the
            message is printed.

         @param szMsg   Message that should be written to the log.
       */
      virtual void Error(const char* szMsg);

      /** 
         Logs a warning to all indicated logs

         A warning is a problem in the system, but a problem that probably
            won't cause the system to crash.  The problem, however, may result
            in unexpected behavior.  This method does not interrupt 
            the program in any way.

         @param szMsg   Message that should be written to the log.
      */
      virtual void Warning(const char* szMsg);

      /**
         Logs an information message to all indicated logs

         An information message is a message to show that something has occured
            or will be occuring, but isn't necessarily a problem.

         @param szMsg   Message that should be written to the log.
      */
      virtual void Info(const char* szMsg);

      /**
         Logs a debug message to all indicated logs.

         A debug message is a message you only want output during debugging.
            The function does nothing when not in debug mode.

         @param szMsg   Message that should be written to the log.
      */
      virtual void Debug(const char* szMsg);

      /**
       * @brief: 
       */
      void SetCallback(Kernel::tLogListenerManager* listenerManager);

   private:

      /**
         Helper method that actually does the logging.

         @param strMsg Complete message to be logged
      */
      void logMsg(const std::string& strMsg, std::ofstream &outStream, egSKIPrintEventId EventID);

      int Mkdir(std::string &dirName);

      
      /**
         List of files to which we log.
      */
      //{
      std::ofstream        m_fSysLog;
      std::ofstream        m_fErrLog;
      std::ofstream        m_fWarnLog;
      std::ofstream        m_fInfoLog;
   #ifdef _DEBUG
      std::ofstream        m_fDebugLog;
   #endif
      //}

      unsigned char        m_nFlags;

      /**
       * @brief: This allows us to trigger an event when a log message occurs.
       */
      Kernel::tLogListenerManager* m_listenerManager;

      Kernel*                      m_kernel;

// Console logging is triggered with an event????  Or we hold a pointer to the console,...
   };
}

#endif

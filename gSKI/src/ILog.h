/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
	created:	28:9:2001   15:38
	filename:	ilog
	file ext:	h
	file path:	d:\projects\countrychaos\code\shared
	author:		jcrossman@soartech.com
	
	purpose:	
*********************************************************************/


#ifndef __ILOG_H
#define __ILOG_H

namespace gSKI {

   /**
   */
   class ILog
   {
   public:

      /** Define what logs are available */
      enum AVAILABLE_LOGS {
         NONE        = 0x0,
         CONSOLE     = 0x1,
         SYS_LOG     = 0x2,
         ERR_LOG     = 0x4,
         INFO_LOG    = 0x8,
         WARN_LOG    = 0x10,
         DEBUG_LOG   = 0x20,
         ALL         = 0xFF,
         DEF_ERR     = CONSOLE | SYS_LOG | ERR_LOG,
         DEF_WARN    = CONSOLE | SYS_LOG | WARN_LOG,
         DEF_INFO    = CONSOLE | SYS_LOG | INFO_LOG,
         DEF_DEBUG   = CONSOLE | DEBUG_LOG
      };

   public:
      
      /** Virtual destructor (usual reasons) */
      virtual ~ILog() {}

      /** 
         Logs an error to all indicated logs

         An error is a serious problem in the system.  In debug mode,
            logging an error will cause execution of the program to
            halt and a MegaAssert to occur.  In release, only the
            message is printed.

         @param szMsg   Message that should be written to the log.
      */
      virtual void Error(const char* szMsg)   = 0;

      /** 
         Logs a warning to all indicated logs

         A warning is a problem in the system, but a problem that probably
            won't cause the system to crash.  The problem, however, may result
            in unexpected behavior.  This method does not interrupt 
            the program in any way.

         @param szMsg   Message that should be written to the log.
      */
      virtual void Warning(const char* szMsg) = 0;

      /**
         Logs an information message to all indicated logs

         An information message is a message to show that something has occured
            or will be occuring, but isn't necessarily a problem.

         @param szMsg   Message that should be written to the log.
      */
      virtual void Info(const char* szMsg)    = 0;

      /**
         Logs a debug message to all indicated logs.

         A debug message is a message you only want output during debugging.
            The function does nothing when not in debug mode.

         @param szMsg   Message that should be written to the log.
      */
      virtual void Debug(const char* szMsg)  = 0;
   };

}

#endif

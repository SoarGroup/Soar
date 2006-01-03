/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

#ifndef GSKI_STRUCTURES_H
#define GSKI_STRUCTURES_H

#ifdef __cplusplus
namespace gSKI{
#endif

enum { gSKI_EXTENDED_ERROR_MESSAGE_LENGTH = 256 };
typedef unsigned long tgSKIErrId;


struct Version{
   unsigned short major;
   unsigned short minor;
   unsigned short micro;
};

   /** IMPORTANT NOTE
    *
    * If you add an error number here, you  must add an error
    *  message to the end of the list of error messages in gSKI_Error.h
    *  The system depends on these lists being in the same order.  Always
    *  add to the end (just before gSKIERR_MAXERRNO)
    */
   enum {
      gSKIERR_NONE,
      gSKIERR_NO_ERROR_RETURNED,
      gSKIERR_FILE_NOT_FOUND,
      gSKIERR_BAD_FILE_FORMAT,
      gSKIERR_PRODUCTION_ALREADY_EXISTS,
      gSKIERR_PRODUCTION_DOES_NOT_EXISTS,
      gSKIERR_DISK_FULL,
      gSKIERR_BAD_FILE_PERMISSIONS,
      gSKIERR_INVALID_PTR,
      gSKIERR_INVALID_PATTERN,
      gSKIERR_AGENT_DOES_NOT_EXIST,
      gSKIERR_ELEMENT_NOT_FUNCTION,
      gSKIERR_ELEMENT_NOT_SYMBOL,
      gSKIERR_SYMBOL_NOT_STRING,
      gSKIERR_SYMBOL_NOT_INT,
      gSKIERR_SYMBOL_NOT_DOUBLE,
      gSKIERR_SYMBOL_NOT_OBJECT,
      gSKIERR_PREFERENCE_NOT_BINARY,
      gSKIERR_AGENT_RUNNING,
      gSKIERR_NOT_IMPLEMENTED,
      gSKIERR_AGENT_STOPPED,
      gSKIERR_AGENT_HALTED,
      gSKIERR_NO_AGENTS_TO_RUN,
      gSKIERR_UNABLE_TO_ADD_PRODUCTION,
      gSKIERR_RHS_FUNCTION_ALREADY_EXISTS,
      gSKIERROR_NO_SUCH_RHS_FUNCTION,
      gSKIERR_MAXERRNO // gSKIERR_MAXERRNO must __ALWAYS__ be __LAST__!!!
   };

/**
 * @brief Error structure
 *
 * This stucture holds the relevant informaation about an error condition.
 * This class would typically be returned from a function call.
 */

struct Error {
   tgSKIErrId     Id;               /**< Unique error identifier */
   const char*    Text;             /**< Test message describing the error. */
   char           ExtendedMsg[gSKI_EXTENDED_ERROR_MESSAGE_LENGTH]; 
                                    /**< Extra text messages relating to the error */
};


#ifdef __cplusplus 
}
typedef struct gSKI::Error gSKI_Error;
typedef struct gSKI::Version gSKI_Version;
#else
   typedef struct Error gSKI_Error;
   typedef struct Version gSKI_Version;
#endif


#endif



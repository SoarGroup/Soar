/********************************************************************
* @file gski_errorids.h 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/19/2002   11:38
*
* purpose: 
*********************************************************************/
#ifndef GSKI_ERRORIDS_H
#define GSKI_ERRORIDS_H

#include "gSKI_Structures.h"

namespace gSKI {

//   enum { gSKI_EXTENDED_ERROR_MESSAGE_LENGTH = 256 };

   /**
    * Used to descript error identifiers.
    */
//   typedef unsigned long tgSKIErrId;

   /**
    * @brief Error structure
    *
    * This stucture holds the relevant informaation about an error condition.
    * This class would typically be returned from a function call.
    */
//   struct Error {
//   public:
//      tgSKIErrId     Id;               /**< Unique error identifier */
//      const char*    Text;             /**< Test message describing the error. */
//      char           ExtendedMsg[gSKI_EXTENDED_ERROR_MESSAGE_LENGTH]; 
//                                       /**< Extra text messages relating to the error */
//   };

   /**
    * @brief the Error handling class
    */
   class ErrorClass : private gSKI::Error
   {
   public:
      tgSKIErrId   GetgSKIErrorId() const { return Id; }
      const char*  GetErrorMsg()    const { return Text; }
      const char*  GetExtendedMsg() const { return ExtendedMsg; }
   };

}
#endif

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_symbol.h 
*********************************************************************
* created:	   6/14/2002   14:54
*
* purpose: 
********************************************************************/

#ifndef IGSKI_SYMBOL_H
#define IGSKI_SYMBOL_H

#include "gSKI_Enumerations.h"
#include "IgSKI_Release.h"

#ifdef _WIN32
#undef GetObject
#endif

namespace gSKI {

   // Forward declarations
   struct Error;
   class IWMObject;

   /**
    * @brief Interface used to represent the id, attribute and value of a WME.
    *
    * This interface is used essentially as a variant to represent the 
    * identifiers, attributes and values exposed by the IWme interface.
    * The interface contains methods for retrieving both numerical and
    * string representations of the various types stored in the symbol.
    * The current DOUBLE, INT, STRING and ID types can be stored in a
    * symbol (@see egSKI_SYMBOL_TYPE).
    *
    * Pointers to ISymbols can be obtained in the following ways:
    *   @li Using the GetAttribute(), or GetValue methods of IWme
    *   @li From an ISymbolFactory instance
    */
   class ISymbol: public IRelease {

   public:
      
      /**
       * @brief Virtual Destructor
       *
       * Including a virtual destructor for the usual safety reasons. 
       */
      virtual ~ISymbol() {}

      /** 
       * @brief Returns the symbol type. 
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return the symbol type (@see gSKI_Enumerations.h)
       */
      virtual egSKISymbolType GetType(Error* err = 0) const = 0;

      /**
       * @brief Returns a string representation of the symbol value.
       *
       * This method returns the string representation of a symbol. This
       * method is valid for all symbol types.
       *
       * Variables will return a string representation of the variable
       *  (e.g. <var>). The same goes for numerical symbols (int and double).
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return a string representation of the symbol value
       */
      virtual const char* GetString(Error* err = 0) const = 0;
          
      /**
       * @brief Returns the symbol value as an integer.
       *
       * This method returns a symbol value as an integer. This method is
       *  valid for the numerical symbol types (DOUBLE, INT). If the
       *  type is STRING or ID, the return value will be 0 and error
       *  information will be noted in the err parameter.
       *
       * Possible Errors:
       *   @li gSKIERR_SYMBOL_NOT_INT If the symbol is not an integer type
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return The integer value of the Symbol
       */
      virtual int GetInt(Error* err = 0) const = 0;

      /**
       * @brief Returns the symbol value as a floating point number.
       *
       * This method returns the floating point number representation of a 
       * Symbol. This method is valid for the INT and FLOAT symbol types but
       * will return 0.0 and an error for ID and STRING symbols.
       * 
       * Possible Errors:
       *   @li gSKIERR_SYMBOL_NOT_DOUBLE If the symbol is not a floating point
       *                       number type
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return the float value of the Symbol
       */
      virtual double GetDouble(Error* pErr = 0) const = 0;

      /**
       * @brief Returns a pointer to the object represented by an ID Symbol
       *
       * This method returns a pointer to the WMObject that corresponds to
       * this ID type symbol. For string, int and double symbols this function
       * returns a NULL pointer.
       *
       * Possible Errors:
       *   @li gSKIERR_SYMBOL_NOT_OBJECT If the symbol is not an object type
       *
       * @return A pointer to the WMObject, or NULL if the symbol is not an
       *          ID type symbol or the WMObject no longer exists.
       */
      virtual IWMObject* GetObject(Error* err = 0) const = 0;

      /**
       * @brief Compares two symbols to determine if they are equal.
       *
       * This method compares two Symbols to see if they are equal. Two 
       * symbols are equal if they have the same type and internal value (i.e.
       * two FLOAT symbols are equal if s1.GetAsDouble() == s2.GetAsDouble()).
       * Two symbols of different types are never equal.
       * 
       * @param  pSymbol The symbol to be compared with.
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return true if symbols are equal, false if they are not
       */
      virtual bool IsEqual(const ISymbol* pSymbol,
                           Error* pErr = 0) const = 0;

   };
   
}

#endif 

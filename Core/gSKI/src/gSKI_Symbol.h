/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_symbol.h 
*********************************************************************
* created:	   6/14/2002   14:54
*
* purpose: 
********************************************************************/

#ifndef GSKI_SYMBOL_H
#define GSKI_SYMBOL_H

#include "IgSKI_Symbol.h"
#include "symtab.h"
#include "gSKI_ReleaseImpl.h"

#include <string>

namespace gSKI {

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
    * @note IMPORTANT: gSymbol is a wrapper around the soar symbol.
    *        There are two ways the wrapper could work.  It could
    *        take over ownership of the symbol, or it could share
    *        ownership.  We have implemented the latter.  This means
    *        that if you pass in a Soar Symbol pointer to the constructor
    *        or to SetValue, gSymbol will add a reference to it.  If you
    *        had added a reference, you will need to release it yourself.
    *        The place where this is the biggest issue is when you pass
    *        in a Soar symbol that you create using make_XXXX_symbol or
    *        get_new_io...  The Soar functions for creating new symbols
    *        initialize the reference count to 1.  You will need to release
    *        your reference count after passing the Soar symbol into a 
    *        gSymbol object.  However, if you just grab a raw Symbol* 
    *        from some Soar structure, you do not have to release or add ref
    *        as long as you put it in a gSymbol and manipulate it through
    *        the gSymbol.
    *
    * Pointers to ISymbols can be obtained in the following ways:
    *   @li Using the GetAttribute(), or GetValue methods of IWme
    *   @li From an ISymbolFactory instance
    */
   class gSymbol: public RefCountedReleaseImpl<ISymbol, false> {

   public:

      /**
       * @brief: The Symbolic Constant constructor Version of gSymbol
       *
       * @note See notes about passing in soar Symbol* in the class comments
       *            for gSKI::gSymbol
       *
       * @param sym Pointer to the soar symbol that this object wraps
       */
      //{
      gSymbol(agent* a, Symbol* sym, IWMObject* obj, bool releaseObj);        // from a soar symbol
      gSymbol(agent* a, const char* value);
      gSymbol(agent* a, int value);
      gSymbol(agent* a, double value);
      gSymbol(const gSymbol& rhs);
      //}

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
      egSKISymbolType GetType(Error* err = 0) const;

      /**
       * @brief Returns a string representation of the symbol value.
       *
       * This method returns the string representation of a symbol. This
       * method is valid only for identifier and string symbol types. 
       * Integer and float symbols return a null pointer and an error message. 
       * Identifier (ID) symbols will return a string of the form
       * (IDLetterIDNumber).
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return a string representation of the symbol value
       */
      const char* GetString(Error* err = 0) const;
          
      /**
       * @brief Returns the symbol value as an integer.
       *
       * This method returns a symbol value as an integer. This method is
       *  valid for the numerical symbol types (DOUBLE, INT). If the
       *  type is STRING or ID, the return value will be 0 and error
       *  information will be noted in the err parameter.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return The integer value of the gSymbol
       */
      int GetInt(Error* err = 0) const;

      /**
       * @brief Returns the symbol value as a floating point number.
       *
       * This method returns the floating point number representation of a 
       * gSymbol. This method is valid for the INT and FLOAT symbol types but
       * will return 0.0 and an error for ID and STRING symbols.
       * 
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return the float value of the gSymbol
       */
      double GetDouble(Error* err = 0) const;
      // TODO: Check that int and double gSymbols work with base soar symbol.
      // it may need to be long and float or long and double.

      /**
       * @brief Returns a pointer to the object represented by an ID gSymbol
       *
       * This method returns a pointer to the WMObject that corresponds to
       * this ID type symbol. For string, int and double symbols this function
       * returns a NULL pointer.
       *
       * @return A pointer to the WMObject, or NULL if the symbol is not an
       *          ID type symbol or the WMObject no longer exists.
       */
      IWMObject* GetObject(Error* err = 0) const;

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
      bool IsEqual(const ISymbol* pSymbol, Error* err = 0) const;
      
      /**
       * @brief Static helper function for converting raw soar symbols to strings
       *
       * 
       */
      static std::string ConvertSymbolToString(Symbol *sym);
      static std::string ConvertSymbolToString(ISymbol *sym);
     
      /**
       * @brief Static helper function for casting ISymbols to gSymbols
       */
      static gSymbol* ConvertSymbol(const ISymbol* sym);

      /**
       * @brief Allowing access to the raw soar symbol
       */
      Symbol* GetSoarSymbol() { return m_sym; }

   private:

      /** Hold a pointer to the soar symbol type */
      Symbol*             m_sym;

      /** Pointer to the agent which owns this symbol */
      agent*              m_agent;

      /** Pointer to the object associated with this symbol */
      IWMObject*          m_obj;
      bool                m_releaseObj;

      /** Holds a string representation of the symbol */
      std::string         m_symstring;

      gSymbol();
      gSymbol& operator=(const gSymbol& rhs);

   protected:
      /**
         Destructor. Private to ensure that no one tries to explicitly
         call delete. They must use Release().
		 2/23/05: changed to protected to eliminate gcc warning
       */
      virtual ~gSymbol();
   };
   
}

#endif 


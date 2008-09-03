#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_symbol.cpp
*********************************************************************
* created:	   7/22/2002   13:52
*
* purpose: 
*********************************************************************/

#include "gSKI_Symbol.h"
#include "gSKI_Error.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_WMObject.h"
#include "MegaAssert.h"

#include "gSKI_EnumRemapping.h"

#include "stdio.h"
#include <vector>

//
// Explicit Export for this file.
//#include "MegaUnitTest.h"
//
//DEF_EXPOSE(gSKI_Symbol)

#ifdef _WIN32
#define safeSprintf _snprintf
#else
#define safeSprintf snprintf
#endif


namespace gSKI
{

   gSymbol::gSymbol(agent* a, Symbol* sym, IWMObject* obj, bool releaseObj): 
      m_sym(sym), m_agent(a), m_obj(obj), m_releaseObj(releaseObj)
   {
      MegaAssert(a != 0, "Cannot create a symbol with a 0 agent pointer.");
      MegaAssert(sym != 0, "Cannot use a null symbol to make a gSymbol.");

      symbol_add_ref(m_sym);

      // Setting the string representation of the symbol
      m_symstring = m_sym ? ConvertSymbolToString(m_sym) : "";
   }

   gSymbol::gSymbol(agent* a, const char* value): 
      m_sym(0), m_agent(a), m_obj(0), m_releaseObj(false)
   {
      MegaAssert( a != 0, "Cannot create a symbol with a 0 agent pointer.");
      MegaAssert( value != 0, "Cannot create a symbol for a null string.");

      std::vector<char> vchar;
      vchar.resize(strlen(value)+1);
      strcpy(&vchar[0],value);

      m_sym = make_sym_constant(a, &vchar[0]);
      MegaAssert(m_sym != 0,"Couldn't find or create symbol!");

      // Setting the string representation of the symbol
      m_symstring = m_sym ? ConvertSymbolToString(m_sym) : "";
   }

   gSymbol::gSymbol(agent* a, double value): 
      m_sym(0), m_agent(a), m_obj(0), m_releaseObj(false)
   {
      MegaAssert( a != 0, "Cannot create a symbol with a 0 agent pointer.");

      m_sym = make_float_constant(a, static_cast<float>(value));
      MegaAssert( m_sym != 0,"Couldn't find or create symbol!");

      // Setting the string representation of the symbol
      m_symstring = m_sym ? ConvertSymbolToString(m_sym) : "";
   }

   gSymbol::gSymbol(agent* a, int value): 
      m_sym(0), m_agent(a), m_obj(0), m_releaseObj(false)
   {
      MegaAssert( a != 0, "Cannot create a symbol with a 0 agent pointer.");

      m_sym = make_int_constant(a, value);
      MegaAssert( m_sym != 0, "Cannot find or create symbol!");

      // Setting the string representation of the symbol
      m_symstring = m_sym ? ConvertSymbolToString(m_sym) : "";
   }

   gSymbol::gSymbol(const gSymbol& rhs) :
      m_sym(rhs.m_sym), m_agent(rhs.m_agent), m_obj(rhs.m_obj),
      m_releaseObj(false), m_symstring(rhs.m_symstring)
   {
      if(m_sym)
      {
         symbol_add_ref(m_sym);
      }
   }

   gSymbol::~gSymbol() 
   {
      if(m_sym)
      {
         symbol_remove_ref(m_agent, m_sym);
      }
      if(m_releaseObj && m_obj)
      {
         m_obj->Release();
      }
   }

   egSKISymbolType gSymbol::GetType(Error* err) const
   {
      MegaAssert(m_sym != 0, "Internal symbol pointer is 0, should never happen.");

      ClearError(err);
      return EnumRemappings::ReMapSymbolType(m_sym->sc.common_symbol_info.symbol_type);
   }

   const char* gSymbol::GetString(Error* err) const
   {
      MegaAssert(m_sym != 0, "Symbol pointer is 0.  Should not happen.");
      ClearError(err);
      
      return m_symstring.c_str();
   }

   int gSymbol::GetInt(Error* err) const
   {
      MegaAssert(m_sym != 0, "gSymbol pointer is 0.  Should not happen.");
      MegaAssert(GetType() == gSKI_INT, "gSymbol is not a string. Cannot get it.");

      ClearError(err);
      
      if(GetType() == gSKI_INT)
         return m_sym->ic.value;
      else
      {
         SetError(err, gSKIERR_SYMBOL_NOT_INT);
         return 0;
      }
   }

   double gSymbol::GetDouble(Error* err) const
   {
      MegaAssert(m_sym != 0, "gSymbol pointer is 0.  Should not happen.");
      MegaAssert((GetType() == gSKI_INT)||(GetType() == gSKI_DOUBLE), "gSymbol is not a string. Cannot get it.");

      ClearError(err);
      
      switch (GetType())
      {
      case gSKI_DOUBLE:
         return m_sym->fc.value;
      case gSKI_INT:
         return m_sym->ic.value;
      default:
         SetError(err, gSKIERR_SYMBOL_NOT_DOUBLE);
         return 0;
      }
   }

   IWMObject* gSymbol::GetObject(Error* err) const
   {
      MegaAssert(m_obj != 0, "An object type symbol should not point to a null object!");

      ClearError(err);
      if (GetType() != gSKI_OBJECT)
      {
         SetError(err, gSKIERR_SYMBOL_NOT_OBJECT);
         return 0;
      }
      return m_obj;
   }

   /*
     ===============================
     
     ===============================
   */
   
   bool gSymbol::IsEqual(const ISymbol* sym,
                         Error*         err) const
   {
      MegaAssert(m_sym != 0, "Symbol pointer is 0.  Should not happen.");
      MegaAssert(sym != 0, "Symbol pointer parameter is 0 in IsEqual().");

      // This could use a better design.  Problem is, this is the classic double
      //  dispatch problem that is difficult to solve in oo.  Take a look later
      //  and see if this can be made cleaner and easier to maintain.
      ClearError(err);
      if(sym)
      {
         // Store the type, cause we are gonna have to test it in many places
         egSKISymbolType st = sym->GetType();
         switch (GetType())
         {
         case gSKI_DOUBLE:
            if((st == gSKI_DOUBLE) || (st == gSKI_INT))
               return ((m_sym->fc.value == sym->GetDouble())? true: false);
            break;
         case gSKI_INT:
            if(st == gSKI_DOUBLE)
               return ((m_sym->ic.value == sym->GetInt())? true: false);
            break;
         case gSKI_STRING:
            if(st == gSKI_STRING)
               return ((strcmp(m_sym->sc.name, sym->GetString()) == 0)? true: false);
            break;
         case gSKI_VARIABLE:
            if(st == gSKI_VARIABLE)
               return ((strcmp(m_sym->var.name, sym->GetString()) == 0)? true: false);
            break;
         case gSKI_OBJECT:
            if(st == gSKI_OBJECT)
               return GetObject()->IsEqual(sym->GetObject());
            break;
         default:
            MegaAssert(false, "gSymbol type is invalid. Should not happen.");
            break;
         }

         // The symbol passed in as a parameter is a different type (so not equal)
         // We will return false at the end of the file
      }
      else
      {
         SetError(err, gSKIERR_INVALID_PTR);
      }

      // Not equal because of some error condition or because they are wrong types
      return false;
   }

   std::string gSymbol::ConvertSymbolToString(Symbol* sym)    
   {
      if ( sym == 0 ) return "";

      // No buffer overrun problems but resulting string may be 
      // truncated.
      char temp[128];

      switch (sym->common.symbol_type) {
      case VARIABLE_SYMBOL_TYPE:
         return (char*) (sym->var.name);
         break;
      case IDENTIFIER_SYMBOL_TYPE:
         safeSprintf(temp,128,"%c%lu",sym->id.name_letter,sym->id.name_number);
         return std::string(temp);
         break;
      case SYM_CONSTANT_SYMBOL_TYPE:
         return (char *) (sym->sc.name);
         break;
      case INT_CONSTANT_SYMBOL_TYPE:
         safeSprintf(temp,128, "%ld",sym->ic.value);
         return std::string(temp);
         break;
      case FLOAT_CONSTANT_SYMBOL_TYPE:
         safeSprintf(temp, 128, "%g",sym->fc.value);
         return std::string(temp);
         break;
      default:
         MegaAssert( false, "This symbol type not supported!");
         return "";
         break;
      }
   }

   std::string gSymbol::ConvertSymbolToString(ISymbol* sym)
   {
      // Downcasting to a gSymbol object and using the 
      // raw symbol conversion function
      gSymbol* gsym = (gSymbol*)(sym);
      MegaAssert( gsym != 0, "Specified symbol null or not a gSymbol!");
      return ConvertSymbolToString(gsym->m_sym);
   }

  gSymbol* gSymbol::ConvertSymbol(const ISymbol* sym)
  {
    MegaAssert( sym != 0, "ISymbol cannot be null!");
    ISymbol* tempsym = const_cast<ISymbol*>(sym);
    //gSymbol* gsym = (gSymbol*)(tempsym);
    gSymbol* gsym = dynamic_cast<gSymbol*>(tempsym);
    MegaAssert( gsym != 0, "Dynamic cast of ISymbol failed!");
    return gsym;
  }

}

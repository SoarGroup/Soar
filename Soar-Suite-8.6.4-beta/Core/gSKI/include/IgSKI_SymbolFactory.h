/********************************************************************
* @file igski_symbolfactory.h 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   7/11/2002   14:54
*
* purpose: 
********************************************************************/

#ifndef IGSKI_SYMBOLFACTORY_H
#define IGSKI_SYMBOLFACTORY_H

#include "IgSKI_Release.h"

namespace gSKI {

  // Forward declarations
  struct Error;
  class ISymbol;

  /**
   * @brief Interface used to create various symbol types.
   *
   * This interface is used to generate new symbols for various uses 
   * including creation of new Wme's etc. A pointer to this interface
   * can be obtained from the IWorkingMemory interface. Symbols created
   * using this interface must be released. This interface provides factory
   * methods for creating INT, DOUBLE and STRING symbol types. ID Symbol 
   * types are essentially encapsulated by IWMObjects with creation of
   * these objects handled by through the IWorkingMemory interface.
   */
  class ISymbolFactory {
  
  public:
    /**
     * @brief Virtual Destructor
     *
     * Including a virtual destructor for the usual safety reasons.
     */
    virtual ~ISymbolFactory() {}

    /**
     * @brief Creates a new integer symbol
     *
     * This method returns a pointer to a new symbol of type INT that has
     * the specified integer value. The Release method (of the IRelease
     * interface) should be called when symbols created using this factory
     * method are no longer needed.
     *
     * @param ivalue The integer value for the new symbol.
     * @param err Pointer to client-owned error structure.  If the pointer
     *               is not 0 this structure is filled with extended error
     *               information.  If it is 0 (the default) extended error
     *               information is not returned.
     *
     * @return a pointer to a new symbol of type INT with value ivalue
     */
    virtual ISymbol* CreateIntSymbol(int ivalue, Error * err = 0) const = 0;

    /**
     * @brief Creates a new double symbol
     *
     * This method returns a pointer to a new symbol of type DOUBLE that has
     * the specified double value. The Release method (of the IRelease
     * interface) should be called when symbols created using this factory
     * method are no longer needed.
     *
     * @param dvalue The double value for the new symbol.
     * @param err Pointer to client-owned error structure.  If the pointer
     *               is not 0 this structure is filled with extended error
     *               information.  If it is 0 (the default) extended error
     *               information is not returned.
     *
     * @return a pointer to a new symbol of type DOUBLE with value dvalue.
     */
    virtual ISymbol* CreateDoubleSymbol(double dvalue, 
                                        Error * err = 0) const = 0;

    /**
     * @brief Creates a new string symbol
     *
     * This method returns a pointer to a new symbol of type STRING that has
     * the specified string  value. The Release method (of the IRelease
     * interface) should be called when symbols created using this factory
     * method are no longer needed. If a NULL pointer is passed as a value
     * then a NULL ISymbol pointer is returned.
     *
     * @param svalue The string value for the new symbol.
     * @param err Pointer to client-owned error structure.  If the pointer
     *               is not 0 this structure is filled with extended error
     *               information.  If it is 0 (the default) extended error
     *               information is not returned.
     *
     * @return a pointer to a new symbol of type STRING with value svalue
     *          or NULL if svalue is NULL
     */
    virtual ISymbol* CreateStringSymbol(const char* svalue,
                                        Error * err = 0) const = 0;
  };

}
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/********************************************************************
* @file gski_inputlink.cpp
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   7/22/2002   12:53
*
* purpose: 
*********************************************************************/

#include "gSKI_SymbolFactory.h"
#include "gSKI_Error.h"
#include "MegaAssert.h"
#include "gSKI_Symbol.h"

//
// Explicit Export for this file.
//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_SymbolFactory);

namespace gSKI
{

  /*
    ===============================

    ===============================
  */
  SymbolFactory::SymbolFactory(agent* agent):
    m_agent(agent)
  {    
  }

  /*
    ===============================

    ===============================
  */
  SymbolFactory::~SymbolFactory() {}

  /*
    ===============================

    ===============================
  */
  void SymbolFactory::SetAgentPtr(agent* a) 
   {
      MegaAssert(a != 0, "Cannot set the agent pointer to 0 in the SymbolFactory.");
      m_agent = a;
   }

  /*
    ===============================

    ===============================
  */

  ISymbol* SymbolFactory::CreateIntSymbol(int ivalue, Error * err) const
  {
    ClearError(err);
    MegaAssert(m_agent != 0, "Cannot initialize a symbol with no agent structure available");
    return new gSymbol(m_agent, ivalue);
  }

  /*
    ===============================

    ===============================
  */

  ISymbol* SymbolFactory::CreateDoubleSymbol(double dvalue, 
                                                    Error * err) const
  {
    ClearError(err);
    MegaAssert(m_agent != 0, "Cannot initialize a symbol with no agent structure available");
    return new gSymbol(m_agent, dvalue);
  }

  /*
    ===============================

    ===============================
  */

  ISymbol* SymbolFactory::CreateStringSymbol(const char* svalue,
                                                    Error * err) const
  {
    ClearError(err);
    MegaAssert(m_agent != 0, "Cannot initialize a symbol with no agent structure available");
    return new gSymbol(m_agent, svalue);
  }

}
                  
  

/********************************************************************
* @file gskitestapp.cpp 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/10/2002   15:02
*
* purpose: 
*********************************************************************/


#ifndef __TEST_RHS_FUNCTIONS_H
#define __TEST_RHS_FUNCTIONS_H

#include "IgSKI_RhsFunction.h"
#include "IgSKI_SymbolFactory.h"
#include "IgSKI_Symbol.h"
#include "IgSKI_Agent.h"


#include "MegaAssert.h"

#include <string>
#include <stdio.h>
//#include <sstream>

#ifdef WIN32
#define SNPRINTF _snprintf
#else
#define SNPRINTF snprintf
#endif

class CreateValue: public gSKI::IRhsFunction
{
public:

   const char*  GetName() const                    { return "create-value";  }
   int          GetNumExpectedParameters() const   { return gSKI_PARAM_NUM_VARIABLE; }
   bool         IsValueReturned() const            { return true; }

   gSKI::ISymbol*     Execute(gSKI::tISymbolIterator* pArguments, gSKI::ISymbolFactory* pSymbolFactory)
   {
      std::string         name;
      std::vector<double> nums;
      gSKI::ISymbol*      pSym;

      if(pArguments->IsValid())
      {
         pSym = pArguments->GetVal();
         MegaAssert(pSym != 0, "Argument passes is null");

         MegaAssert(pSym->GetType() == gSKI_STRING, "Wrong first argument to create-value, should be string");
         name = pSym->GetString();

         for(pArguments->Next(); pArguments->IsValid(); pArguments->Next())
         {
            pSym = pArguments->GetVal();
            MegaAssert(pSym != 0, "Retrieved a 0 symbol.");
            MegaAssert(pSym->GetType() == gSKI_DOUBLE || pSym->GetType() == gSKI_INT, "Expected a number as an argument.");

            nums.push_back(pSym->GetDouble());
         }
      }

      double result = 0;
      if(name == std::string("add"))
      {
         for(std::vector<double>::iterator it = nums.begin(); it != nums.end(); ++it)
            result += *it;
      }
      else
      {
         for(std::vector<double>::iterator it = nums.begin(); it != nums.end(); ++it)
            result *= *it;
      }

      return pSymbolFactory->CreateDoubleSymbol(result);
   }
};

class ConcatNumbers: public gSKI::IRhsFunction
{
public:

   const char*  GetName() const                    { return "concat-numbers";  }
   int          GetNumExpectedParameters() const   { return (gSKI_PARAM_NUM_VARIABLE); }
   bool         IsValueReturned() const            { return true; }

   gSKI::ISymbol*     Execute(gSKI::tISymbolIterator* pArguments, gSKI::ISymbolFactory* pSymbolFactory)
   {
      std::string         result;
      gSKI::ISymbol*      pSym;

     for(;pArguments->IsValid(); pArguments->Next())
     {
         pSym = pArguments->GetVal();
         MegaAssert(pSym != 0, "Retrieved a 0 symbol.");
         MegaAssert(pSym->GetType() == gSKI_DOUBLE || pSym->GetType() == gSKI_INT, "Expected a number as an argument.");

         char temp[128];
         SNPRINTF(temp, 128, "%f", pSym->GetDouble());

         result += std::string(temp);
      }

      return pSymbolFactory->CreateStringSymbol(result.c_str());
   }
};

class VerifyResult: public gSKI::IRhsFunction
{
public:

   const char*  GetName() const                    { return "verify-result";  }
   int          GetNumExpectedParameters() const   { return 1; }
   bool         IsValueReturned() const            { return false; }

   gSKI::ISymbol*     Execute(gSKI::tISymbolIterator* pArguments, gSKI::ISymbolFactory* pSymbolFactory)
   {
      // Get rid of warning
      //pSymbolFactory;

      gSKI::ISymbol*      pSym;

      pSym = pArguments->GetVal();
      MegaAssert(pSym != 0, "Retrieved a 0 symbol.");
      MegaAssert(pSym->GetType() == gSKI_STRING, "Expected a string as an argument.");

      return 0;
   }
};

class DoHalt: public gSKI::IRhsFunction
{
private:

   gSKI::IAgent* m_agent;

public:

   DoHalt(gSKI::IAgent* a): m_agent(a) {}

   const char*  GetName() const                    { return "do-halt";  }
   int          GetNumExpectedParameters() const   { return 0; }
   bool         IsValueReturned() const            { return false; }

   gSKI::ISymbol*     Execute(gSKI::tISymbolIterator* pArguments, gSKI::ISymbolFactory* pSymbolFactory)
   {
      // Get rid of warnings
      //pArguments;
      //pSymbolFactory;

      // This will stop the agent
      m_agent->Halt();
      return 0;
   }
};

void testRhsFunctions(const std::string& prodFileName);

#endif



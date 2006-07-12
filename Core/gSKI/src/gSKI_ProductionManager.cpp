#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_productionmanager.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/


//
// gSKI Specific Headers
#include "gSKI_ProductionManager.h"
#include "gSKI_Error.h"
#include "gSKI_Enumerations.h"
#include "gSKI_SetActiveAgent.h"
#include "gSKI_Production.h"
#include "IgSKI_Iterator.h"
#include "gSKI_EnumRemapping.h"
#include "gSKI_Agent.h"

//
// Soar Kernel Specific Headers
#include "parser.h"
#include "agent.h"
#include "production.h"
#include "symtab.h"
#include "gski_event_system_functions.h"
#include "instantiations.h"
#include "rete.h"

#include "pcreposix.h"

//
// Utility Headers
#include "MegaAssert.h"
#include "Log.h"

//
// std Headers
#include <fstream>
#include <string>
#include <iostream>
#include <cstdio>

//
// Explicit Export for this file.
//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_ProductionManager);

#ifdef WIN32
#include <direct.h>
#define GetCwd _getcwd
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#define GetCwd getcwd
#endif

#ifdef _WIN32
#define safeSprintf _snprintf
#else
#define safeSprintf snprintf
#endif




using namespace std;

namespace gSKI {

   /*
   ==================================
                          ____       _      _
 _ __ ___  __ _  _____  _|  _ \  ___| | ___| |_ ___
| '__/ _ \/ _` |/ _ \ \/ / | | |/ _ \ |/ _ \ __/ _ \
| | |  __/ (_| |  __/>  <| |_| |  __/ |  __/ ||  __/
|_|  \___|\__, |\___/_/\_\____/ \___|_|\___|\__\___|
__        |___/
\ \      / / __ __ _ _ __  _ __   ___ _ __
 \ \ /\ / / '__/ _` | '_ \| '_ \ / _ \ '__|
  \ V  V /| | | (_| | |_) | |_) |  __/ |
   \_/\_/ |_|  \__,_| .__/| .__/ \___|_|
                    |_|   |_|
   ==================================
   */
   void regexDeleteWrapper(std::string& inputFile, 
                      const char* regularexpression)
   {
      // TODO: Handle errors appropriately in the following code
      // Compiling the regular expression
      regex_t compiled_pattern;
      int error_code = regcomp( &compiled_pattern,
                                regularexpression,
                                REG_EXTENDED|REG_NEWLINE);

      MegaAssert( error_code == 0, "Could not compile regular expression!");

      // Applying the regular expressions until all matches have been found
      regmatch_t match;
      
      while ( regexec(&compiled_pattern,
                      inputFile.c_str(),
                      1,
                      &match,
                      0) == 0 ) {
        inputFile.erase(match.rm_so, match.rm_eo-match.rm_so);      
      }  

      // Freeing the C API stuff
      regfree( &compiled_pattern );
   }


   /*
   ==================================
 ____                _            _   _
|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
| |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
|  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
|_|  _|_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
|  \/  | __ _ _ __   __ _  __ _  ___ _ __
| |\/| |/ _` | '_ \ / _` |/ _` |/ _ \ '__|
| |  | | (_| | | | | (_| | (_| |  __/ |
|_|  |_|\__,_|_| |_|\__,_|\__, |\___|_|
                          |___/
   ==================================
   */
   ProductionManager::ProductionManager(Agent *agent_)  : m_agent(agent_) 
   {

   }


   /*
   ===============================
 /\/|___                _            _   _
|/\/  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
   | |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
   |  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
 __|_|_  |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
|  \/  | __ _ _ __   __ _  __ _  ___ _ __
| |\/| |/ _` | '_ \ / _` |/ _` |/ _ \ '__|
| |  | | (_| | | | | (_| | (_| |  __/ |
|_|  |_|\__,_|_| |_|\__,_|\__, |\___|_|
                          |___/
   ===============================
   */
   ProductionManager::~ProductionManager() 
   {
   
   }


   /*
   ==================================
 _                 _ _____ _ _
| | ___   __ _  __| |  ___(_) | ___
| |/ _ \ / _` |/ _` | |_  | | |/ _ \
| | (_) | (_| | (_| |  _| | | |  __/
|_|\___/ \__,_|\__,_|_|   |_|_|\___|
   ==================================
   */
   string ProductionManager::loadFile(std::istream& is)
   {
      std::string soarFile;
      soarFile.erase();
      soarFile.reserve(is.rdbuf()->in_avail());
      char c;
      while(is.get(c))
      {
         if(soarFile.capacity() == soarFile.size())
            soarFile.reserve(soarFile.capacity() * 2);
         soarFile.append(1, c);
      }

      return soarFile;
   }


   /*
   ==================================
            _    ____                          _
  __ _  ___| |_ / ___|   _ _ __ _ __ ___ _ __ | |_
 / _` |/ _ \ __| |  | | | | '__| '__/ _ \ '_ \| __|
| (_| |  __/ |_| |__| |_| | |  | | |  __/ | | | |_
 \__, |\___|\__|\____\__,_|_|  |_|  \___|_| |_|\__|
_|___/    __         _    _             ____  _               _
\ \      / /__  _ __| | _(_)_ __   __ _|  _ \(_)_ __ ___  ___| |_ ___  _ __ _   _
 \ \ /\ / / _ \| '__| |/ / | '_ \ / _` | | | | | '__/ _ \/ __| __/ _ \| '__| | | |
  \ V  V / (_) | |  |   <| | | | | (_| | |_| | | | |  __/ (__| || (_) | |  | |_| |
   \_/\_/ \___/|_|  |_|\_\_|_| |_|\__, |____/|_|_|  \___|\___|\__\___/|_|   \__, |
                                  |___/                                     |___/
   ==================================
   */
   std::string ProductionManager::getCurrentWorkingDirectory()
   {
      int pathLength = 32;
      char *pathName = new char[pathLength];

      char *c;
      while((c = GetCwd(pathName, pathLength))  == 0)
      {
         pathLength *= 2;
         delete[] pathName;
         pathName = new char[pathLength];
      }

      std::string tmp(pathName);
      delete[] pathName;
      return tmp;
   }

   /*
   ==================================
                           ____
 _ __   __ _ _ __ ___  ___/ ___|  ___  _   _ _ __ ___ ___ ___
| '_ \ / _` | '__/ __|/ _ \___ \ / _ \| | | | '__/ __/ _ Y __|
| |_) | (_| | |  \__ \  __/___) | (_) | |_| | | | (_|  __|__ \
| .__/ \__,_|_|  |___/\___|____/ \___/ \__,_|_|  \___\___|___/
|_|
   ==================================
   */
   bool ProductionManager::parseSources(tStringSet& sourcedFiles, std::string &file,
                                        std::string &path,
                                        Error *err)
   {
      ClearError(err);
      const char* regularexpression("[sS]ource\\s+\\\"(.*?)\\\"");
      
      unsigned int startChar = 0;
      regex_t compiled_pattern;
      int error_code = regcomp( &compiled_pattern,
                                regularexpression,
                                REG_EXTENDED|REG_NEWLINE);
      MegaAssert( error_code == 0, "Could not compile regular expression!");

      // Applying the regular expressions until all matches have been found
      regmatch_t* match = new regmatch_t[2];
      while ( regexec(&compiled_pattern,
                      file.c_str() + startChar,
                      2,
                      match,
                      0) == 0 ) {
	std::string fname = file.substr(startChar + match[1].rm_so,  match[1].rm_eo - match[1].rm_so);

   // cout <<"Trying to load a file named "<<"\""<<fname<<"\""<<endl;
         std::string fullPath = path;
         fullPath += fname;

         Error e;
         loadSoarFile(sourcedFiles, fullPath.c_str(), &e);
         if(err != 0 && isError(e) ) {
            memcpy(err, &e, sizeof(Error));
            return false;
         }
         //
         // update search position:
	      startChar+= match[0].rm_eo;
      }
      return true;
   }


   /*
   ===============================
 _                    _ ____                   _____ _ _
| |    ___   __ _  __| / ___|  ___   __ _ _ __|  ___(_) | ___
| |   / _ \ / _` |/ _` \___ \ / _ \ / _` | '__| |_  | | |/ _ \
| |__| (_) | (_| | (_| |___) | (_) | (_| | |  |  _| | | |  __/
|_____\___/ \__,_|\__,_|____/ \___/ \__,_|_|  |_|   |_|_|\___|
   ===============================
   */
   bool ProductionManager::LoadSoarFile(const char *fileName, Error *err)
   {
      tStringSet sourcedFiles;
      return loadSoarFile(sourcedFiles, fileName, err);
   }


   /*
   ===============================
 _                 _ ____                   _____ _ _
| | ___   __ _  __| / ___|  ___   __ _ _ __|  ___(_) | ___
| |/ _ \ / _` |/ _` \___ \ / _ \ / _` | '__| |_  | | |/ _ \
| | (_) | (_| | (_| |___) | (_) | (_| | |  |  _| | | |  __/
|_|\___/ \__,_|\__,_|____/ \___/ \__,_|_|  |_|   |_|_|\___|
   ===============================
   */

   bool ProductionManager::loadSoarFile(tStringSet& sourcedFiles, const char *fileName, Error *err)
   {
      ClearError(err);

      bool isAbsolutePath = false;

#ifdef WIN32
      std::string separator = "\\";
      if(fileName[1] == ':' && (fileName[2] == '\\' || fileName[2] == '/')) 
         isAbsolutePath = true;
#else
      std::string separator = "/";
      if(fileName[0] == '/')
         isAbsolutePath = true;
#endif
      std::string fullPath;
      if(!isAbsolutePath)
      {
         //
         // First get get the current working directory.
         fullPath = getCurrentWorkingDirectory() + separator ;
         fullPath += fileName;;
      } else {
         fullPath = fileName;
      }
  
      //
      // Now we change back slashes to forward slashes
      std::string::size_type s;
      while((s = fullPath.find_first_of("\\")) != std::string::npos)
      {
         fullPath[s] = '/';
      }
	  string prev;
	  do{
		  prev = fullPath;
		  regexDeleteWrapper(fullPath, "[^/]+/\\.\\./");     
	  }while(prev != fullPath);
	
      //
      // At this point we have the fully qualified absolute path
      // of the file being sourced.  
      std::string pname; /**< Name of the path */

      pname = fullPath.substr(0, fullPath.find_last_of("/") + 1);
      //fname = fullPath.substr(fullPath.find_last_of("/") + 1);

      ifstream soarFile;
      if(sourcedFiles.find(fullPath) == sourcedFiles.end()){
         soarFile.open(fullPath.c_str());
         if(!soarFile.is_open()) {
            char msg[gSKI_EXTENDED_ERROR_MESSAGE_LENGTH];
            safeSprintf(msg, gSKI_EXTENDED_ERROR_MESSAGE_LENGTH, "Could not find %s", fileName);
            SetErrorExtended(err, gSKIERR_FILE_NOT_FOUND, msg);
            return false;
         }
	 // cout << "Sourcing: \""<<fullPath<<"\""<<endl;
         sourcedFiles.insert(fullPath);
	 
	 //Do some error checking to make sure we didn't get in an infinite loop
         assert(sourcedFiles.size() < 1000000 );
	      if(sourcedFiles.size() >= 1000000 ) {
	          return false;
         }
         std::string file = loadFile(soarFile);

         //cout << "====================\n" << file << "====================\n";
         removeComments(file);
         //cout << "====================\n" << file << "====================\n";
         parseProductions(file);
         //cout << "====================\n" << file << "====================\n";
         bool success = parseSources(sourcedFiles, file, pname, err);

		 // DJP - BUGBUG: This function returns true based on whether "parseSources" succeeds, which is in turn based
		 // on whether it finds other "source" commands in the file.  If there are none, we always get "true" back.
		 // Also, we're not getting extended error information
		 // about "parseProductions" failing etc.  This is a key area to get good error information back as it's
		 // very likely we'll have errors inside sourced soar files and right now this doesn't support that.
         return success;
      } else { //if it already has been opened, then we don't have to worry about
         //loading it
         return true;
      }
   }

   /*
   ===============================
    _       _     _ ____                _            _   _
   / \   __| | __| |  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
  / _ \ / _` |/ _` | |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
 / ___ \ (_| | (_| |  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
/_/   \_\__,_|\__,_|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
   ===============================
   */
   bool ProductionManager::AddProduction(const IProduction *newProd, Error *pErr)
   {
      ClearError(pErr);

      return true;
   }

   /*
   ===============================
 ____                               ____                _            _   _
|  _ \ ___ _ __ ___   _____   _____|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \ |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
|  _ <  __/ | | | | | (_) \ V /  __/  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
|_| \_\___|_| |_| |_|\___/ \_/ \___|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
   ===============================
   */
   bool ProductionManager::RemoveProduction(IProduction* prod, Error* err) const
   {
      ClearError(err);

      prod->Excise();

      delete this;
      return true;
   }

   bool ProductionManager::RemoveAllProductions(int& exciseCount, Error* err ) const
   {
      agent * a;
	  ClearError(err);
	  a = m_agent->GetSoarAgent();
	  exciseCount = exciseCount + a->num_productions_of_type[USER_PRODUCTION_TYPE] +
                                  a->num_productions_of_type[CHUNK_PRODUCTION_TYPE] +
								  a->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE] +
								  a->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];
      excise_all_productions(m_agent->GetSoarAgent(), false);
	  return true;
   }

   /*
   ===============================
 ____                               ____                _            _   _
|  _ \ ___ _ __ ___   _____   _____|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \ |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
|  _ <  __/ | | | | | (_) \ V /  __/  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
|_|_\_\___|_| |_| |_|\___/ \_/ \___|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
/ ___|  ___| |_
\___ \ / _ \ __|
 ___) |  __/ |_
|____/ \___|\__|
   ===============================
   */
   bool ProductionManager::RemoveProductionSet(tIProductionIterator* prodSet, Error* err)
   {
      ClearError(err);
   
      return true;
   }

   bool ProductionManager::RemoveAllRLProductions(int& exciseCount, Error* err) const
   {
     agent * a;
     ClearError(err);
     a = m_agent->GetSoarAgent();
     
     for (production* prod=a->all_productions_of_type[DEFAULT_PRODUCTION_TYPE]; prod != NIL; prod = prod->next)
       {
	 if (prod->RL){
	   exciseCount++;
	   excise_production (a, prod, a->sysparams[TRACE_LOADING_SYSPARAM]);
	 }
       }
     for (production* prod=a->all_productions_of_type[USER_PRODUCTION_TYPE]; prod != NIL; prod = prod->next)
       {
	 if (prod->RL){
	   exciseCount++;
	   excise_production (a, prod,a->sysparams[TRACE_LOADING_SYSPARAM]);
	 }
       }
     for (production* prod=a->all_productions_of_type[CHUNK_PRODUCTION_TYPE];	prod != NIL; prod = prod->next)
       {
	 if (prod->RL){
	   exciseCount++;
	   excise_production (a, prod,a->sysparams[TRACE_LOADING_SYSPARAM]);
	 }
       }
     
     a->RL_count = 1;
     return true;
   }

   bool ProductionManager::RemoveAllChunks(int& exciseCount, Error* err) const
   {
      agent * a;
	  ClearError(err);
	  a = m_agent->GetSoarAgent();
	  exciseCount = exciseCount + a->num_productions_of_type[CHUNK_PRODUCTION_TYPE] +
								  a->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE];
      excise_all_productions_of_type(m_agent->GetSoarAgent(), CHUNK_PRODUCTION_TYPE, false);
      excise_all_productions_of_type(m_agent->GetSoarAgent(), JUSTIFICATION_PRODUCTION_TYPE, false);
	  return true;
   }

   bool ProductionManager::RemoveAllUserProductions(int& exciseCount, Error* err) const
   {
      agent * a;
	  ClearError(err);
	  a = m_agent->GetSoarAgent();
	  exciseCount = exciseCount + a->num_productions_of_type[USER_PRODUCTION_TYPE];
      excise_all_productions_of_type(m_agent->GetSoarAgent(), USER_PRODUCTION_TYPE, false);
 	  return true;
   }

   bool ProductionManager::RemoveAllDefaultProductions(int& exciseCount, Error* err) const
   {
      agent * a;
	  ClearError(err);
	  a = m_agent->GetSoarAgent();
	  exciseCount = exciseCount + a->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];
      excise_all_productions_of_type(m_agent->GetSoarAgent(), DEFAULT_PRODUCTION_TYPE, false);
 	  return true;
   }

   /*
   ===============================
 _                    _ ____      _
| |    ___   __ _  __| |  _ \ ___| |_ ___
| |   / _ \ / _` |/ _` | |_) / _ \ __/ _ \
| |__| (_) | (_| | (_| |  _ <  __/ ||  __/
|_____\___/ \__,_|\__,_|_| \_\___|\__\___|
   ===============================
   */
   bool ProductionManager::LoadRete(const char* fn, Error* err)
   {
      ClearError(err);

      FILE* f = fopen(fn,"rb");

      if(f == 0)
      {
         char errMsg[gSKI_EXTENDED_ERROR_MESSAGE_LENGTH];
         safeSprintf(errMsg, gSKI_EXTENDED_ERROR_MESSAGE_LENGTH, "Could not open file %s", fn);
         SetErrorExtended(err, gSKIERR_FILE_NOT_FOUND, errMsg);
         return false;
      }

      load_rete_net(m_agent->GetKernel()->GetSoarKernel(), m_agent->GetSoarAgent(), f);

      fclose(f);
   
      return true;
   }

   /*
   ===============================
 ____                  ____      _
/ ___|  __ ___   _____|  _ \ ___| |_ ___
\___ \ / _` \ \ / / _ \ |_) / _ \ __/ _ \
 ___) | (_| |\ V /  __/  _ <  __/ ||  __/
|____/ \__,_| \_/ \___|_| \_\___|\__\___|
   ===============================
   */
   bool ProductionManager::SaveRete(const char *fn, Error *err) const
   {
      ClearError(err);

      FILE* f = fopen(fn,"wb");

      if(f == 0)
      {
         char errMsg[gSKI_EXTENDED_ERROR_MESSAGE_LENGTH];
         safeSprintf(errMsg, gSKI_EXTENDED_ERROR_MESSAGE_LENGTH, "Could not open file %s", fn);
         SetErrorExtended(err, gSKIERR_FILE_NOT_FOUND, errMsg);
         return false;
      }

      save_rete_net(m_agent->GetSoarAgent(), f);

      fclose(f);
   
      return true;
   }

   /*
   ===============================
  ____      _   __  __       _       _              _
 / ___| ___| |_|  \/  | __ _| |_ ___| |__   ___  __| |
| |  _ / _ \ __| |\/| |/ _` | __/ __| '_ \ / _ \/ _` |
| |_| |  __/ |_| |  | | (_| | || (__| | | |  __/ (_| |
 \____|\___|\__|_|  |_|\__,_|\__\___|_| |_|\___|\__,_|
|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __  ___
| |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \/ __|
|  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | \__ \
|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|___/
   ===============================
   */
   tIProductionMatchIterator *ProductionManager::GetMatchedProductions(Error* err) const
   {
   
      return 0;
   }

   /*
   ===============================
  ____      _   __  __       _       _     ____       _
 / ___| ___| |_|  \/  | __ _| |_ ___| |__ / ___|  ___| |_ ___
| |  _ / _ \ __| |\/| |/ _` | __/ __| '_ \\___ \ / _ \ __/ __|
| |_| |  __/ |_| |  | | (_| | || (__| | | |___) |  __/ |_\__ \
 \____|\___|\__|_|  |_|\__,_|\__\___|_| |_|____/ \___|\__|___/
   ===============================
   */
   IMatchSet* ProductionManager::GetMatchSets(const IProduction* prod, Error* err) const
   {
   
      return 0;
   }

   /*
   ===============================
  ____      _    ____                _ _ _   _
 / ___| ___| |_ / ___|___  _ __   __| (_) |_(_) ___  _ __
| |  _ / _ \ __| |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \
| |_| |  __/ |_| |__| (_) | | | | (_| | | |_| | (_) | | | |
 \____|\___|\__|\____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|
|  \/  | __ _| |_ ___| |__   ___ ___ ___
| |\/| |/ _` | __/ __| '_ \ / _ Y __/ __|
| |  | | (_| | || (__| | | |  __|__ \__ \
|_|  |_|\__,_|\__\___|_| |_|\___|___/___/
   ===============================
   */
   tIWmeIterator* ProductionManager::GetConditionMatches(const ICondition* condition, Error* err) const
   {
   
      return 0;
   }


   /*
   ===============================
  ____      _   ____                _            _   _
 / ___| ___| |_|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
| |  _ / _ \ __| |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
| |_| |  __/ |_|  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
 \____|\___|\__|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
   ===============================
   */

   /*
WARNING!!!  All of the Get*Production(s) methods appear to leak symbol ref counts in
the kernel under certain circumstances.  See Bug 536.  Use at your own risk.
DJP: I believe we've isolated this so that the leaks are only a risk when "includeConditions" is true and in practice
there has not yet been a case where we've ever needed to set includeConditions to true (i.e. you want to examine the conditions not just work
with the production names).
   */

   tIProductionIterator* ProductionManager::GetProduction(const char* pattern, bool includeConditions, Error* err) const
   {
      //
      // Fetch the array of linked lists that hold all of the productions.
      agent*                      a = m_agent->GetSoarAgent();
      production*                 currentProdList;
      std::vector<IProduction *>  userProds;
      for(unsigned int i = 0; i < NUM_PRODUCTION_TYPES; ++i)
      {
         currentProdList = a->all_productions_of_type[i];

         for(; currentProdList != 0; currentProdList = currentProdList->next)
         {
			char const* pName = currentProdList->name->sc.name ;
            if(strcmp(pattern, pName) == 0)
            {
               userProds.push_back(new Production(currentProdList, includeConditions, a));
            }
         }
      }
      return new tProductionIter(userProds);
   }

   /*
   ===============================
  ____      _      _    _ _ ____                _            _   _
 / ___| ___| |_   / \  | | |  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __  ___
| |  _ / _ \ __| / _ \ | | | |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \/ __|
| |_| |  __/ |_ / ___ \| | |  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | \__ \
 \____|\___|\__/_/   \_\_|_|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|___/
   ===============================
   */
   tIProductionIterator* ProductionManager::GetAllProductions(bool includeConditions, Error* err) const
   {
      prodVec UserProdVec;

      for(unsigned char type = 0; type < NUM_PRODUCTION_TYPES; ++type)
      {
         GetProductions(UserProdVec, includeConditions, type);
      }

      return new tProductionIter(UserProdVec);
   }

   /*
   ==================================
  ____      _   ____                _            _   _
 / ___| ___| |_|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __  ___
| |  _ / _ \ __| |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \/ __|
| |_| |  __/ |_|  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | \__ \
 \____|\___|\__|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|___/
   ==================================
   */
   void ProductionManager::GetProductions(prodVec& prodVec, bool includeConditions, unsigned char prodType) const
   {
      agent*              a = m_agent->GetSoarAgent();
      production*         prods;
      prods = a->all_productions_of_type[prodType];

      //
      // While there are still user productions in the list.
      for(; prods != 0; prods = prods->next)
      {
         Production *p = new Production(prods, includeConditions, a);
         prodVec.push_back(p);
      }
   }

   /*
   ===============================
  ____      _   _   _
 / ___| ___| |_| | | |___  ___ _ __
| |  _ / _ \ __| | | / __|/ _ \ '__|
| |_| |  __/ |_| |_| \__ \  __/ |
 \____|\___|\__|\___/|___/\___|_| _   _
|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __  ___
| |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \/ __|
|  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | \__ \
|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|___/
   ===============================
   */
   tIProductionIterator* ProductionManager::GetUserProductions(Error* err) const
   {
      prodVec UserProdVec;

      GetProductions(UserProdVec, false, USER_PRODUCTION_TYPE);

      return new tProductionIter(UserProdVec);
   }

   /*
   ===============================
  ____      _    ____ _                 _
 / ___| ___| |_ / ___| |__  _   _ _ __ | | _____
| |  _ / _ \ __| |   | '_ \| | | | '_ \| |/ / __|
| |_| |  __/ |_| |___| | | | |_| | | | |   <\__ \
 \____|\___|\__|\____|_| |_|\__,_|_| |_|_|\_\___/
   ===============================
   */
   tIProductionIterator* ProductionManager::GetChunks(Error* err) const
   {
      prodVec ChunkProdVec;

      GetProductions(ChunkProdVec, false, CHUNK_PRODUCTION_TYPE);

      return new tProductionIter(ChunkProdVec);
   }

   /*
   ===============================
  ____      _      _           _   _  __ _           _   _
 / ___| ___| |_   | |_   _ ___| |_(_)/ _(_) ___ __ _| |_(_) ___  _ __  ___
| |  _ / _ \ __|  | | | | / __| __| | |_| |/ __/ _` | __| |/ _ \| '_ \/ __|
| |_| |  __/ || |_| | |_| \__ \ |_| |  _| | (_| (_| | |_| | (_) | | | \__ \
 \____|\___|\__\___/ \__,_|___/\__|_|_| |_|\___\__,_|\__|_|\___/|_| |_|___/
   ===============================
   */
   tIProductionIterator* ProductionManager::GetJustifications(Error* err) const
   {
      prodVec JustificationProdVec;

      GetProductions(JustificationProdVec, false, JUSTIFICATION_PRODUCTION_TYPE);

      return new tProductionIter(JustificationProdVec);
   }

   /*
   ===============================
  ____      _   ____        __             _ _
 / ___| ___| |_|  _ \  ___ / _| __ _ _   _| | |_
| |  _ / _ \ __| | | |/ _ \ |_ / _` | | | | | __|
| |_| |  __/ |_| |_| |  __/  _| (_| | |_| | | |_
 \____|\___|\__|____/_\___|_|  \__,_|\__,_|_|\__|
|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __  ___
| |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \/ __|
|  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | \__ \
|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|___/
   ===============================
   */
   tIProductionIterator* ProductionManager::GetDefaultProductions(Error* err) const
   {
      prodVec DefaultProdVec;

      GetProductions(DefaultProdVec, false, DEFAULT_PRODUCTION_TYPE);

      return new tProductionIter(DefaultProdVec);
   }
  
   /*
   ==================================  ____                                     _
    _ __ ___ _ __ ___   _____   _____ / ___|___  _ __ ___  _ __ ___   ___ _ __ | |_ ___
   | '__/ _ \ '_ ` _ \ / _ \ \ / / _ \ |   / _ \| '_ ` _ \| '_ ` _ \ / _ \ '_ \| __/ __|
   | | |  __/ | | | | | (_) \ V /  __/ |__| (_) | | | | | | | | | | |  __/ | | | |_\__ \
   |_|  \___|_| |_| |_|\___/ \_/ \___|\____\___/|_| |_| |_|_| |_| |_|\___|_| |_|\__|___/
   ==================================
   */
   void ProductionManager::removeComments(std::string & inputFile)
   {
      //
      // (^)    Search for anything that starts at the beginning of the line
      // (\\s*) is followed by any number of spaces 
      // (#)    followed by a pound sign
      // (.*?)  followed by the shortest sequence of any characters 
      // ($)    ending at the end of the line.
      //
      //regexDeleteWrapper(inputFile, "sp");
      //regexDeleteWrapper(inputFile, "\n[:space:]*?#.*?\n");
      regexDeleteWrapper(inputFile, "^\\s*?#.*?$");

      //
      // While I am at it, I will remove all blank lines.
      //regexDeleteWrapper(inputFile, "^[:space:]*$");
   }


   /*
   ==================================
                       _    _ _                        _
 ___  ___   __ _ _ __ / \  | | |_ ___ _ __ _ __   __ _| |_ ___
/ __|/ _ \ / _` | '__/ _ \ | | __/ _ \ '__| '_ \ / _` | __/ _ \
\__ \ (_) | (_| | | / ___ \| | ||  __/ |  | | | | (_| | ||  __/
|___/\___/ \__,_|_|/_/ _ \_\_|\__\___|_|  |_| |_|\__,_|\__\___|
|_ _|_ __  _ __  _   _| |_
 | || '_ \| '_ \| | | | __|
 | || | | | |_) | |_| | |_
|___|_| |_| .__/ \__,_|\__|
          |_|
   ==================================
   */
   void ProductionManager::soarAlternateInput(agent *ai_agent, char  *ai_string, 
                                              char  *ai_suffix, Bool   ai_exit   )
   {
   // Side effects:
   //	The soar agents alternate input values are updated and its
   //      current character is reset to a whitespace value.
   ai_agent->alternate_input_string = ai_string;
   ai_agent->alternate_input_suffix = ai_suffix;
   ai_agent->current_char = ' ';
   ai_agent->alternate_input_exit = ai_exit;
   return;
   }


   /*
   ==================================
    _       _     _ ____                _            _   _
   / \   __| | __| |  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
  / _ \ / _` |/ _` | |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
 / ___ \ (_| | (_| |  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
/_/   \_\__,_|\__,_|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
   ==================================
   */
   void ProductionManager::AddProduction(char *productionText, Error *err)
   {  
      ClearError(err);
      agent* a = m_agent->GetSoarAgent();

      // TODO: This should not be needed, FIX!
      soarAlternateInput(a, productionText, ") ", true); 
      set_lexer_allow_ids (a, false);
      get_lexeme(a);
      
      production* p;
      p = parse_production(a);

	  set_lexer_allow_ids (a, true);
      soarAlternateInput( a, 0, 0, true); 

	  if (!p) { // added by voigtjr
		  SetError(err, gSKIERR_UNABLE_TO_ADD_PRODUCTION);
		  return;
	  }
  }
   

   /*
     ==================================
                                ____                _            _   _
      _ __   __ _ _ __ ___  ___|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __  ___
     | '_ \ / _` | '__/ __|/ _ \ |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \/ __|
     | |_) | (_| | |  \__ \  __/  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | \__ \
     | .__/ \__,_|_|  |___/\___|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|___/
     |_|
     ==================================
   */
   void ProductionManager::parseProductions(string &inputFile)
   {

////
////      //
////      // This regex is designed to find soar productions.  
////      // It is looking for:
////      // (^)      the beginning of the line followed by:
////      // (\\s*?)  a minimal number of spaces followed by:
////      // (sp)     the literal string "sp" followed by:
////      // (.*?)    the minimal number of characters followed by:
////      // (-->)    the literal string "-->" followed by:
////      // (.*?)    the minimal number of characters followed by:
////      // (\\})    the literal character "}" followed by:
////      // (\\s*)   the minimal number of spaces followed by:
////      // ($|;)    the end of the line or a semicolon.
////      //
////      // To summarize, we are looking for an "sp" preceded optionally
////      //  by spaces, the "-->" separater, and the trailing "}".  The
////      //  reason we go to the trouble of searching for the "-->" is
////      //  because it is not uncommon to have a "}" before that, and 
////      //  extremely uncommon to have one after.
////      //
////

      // TODO: Handle errors appropriately in the following code
      // Compiling the regular expression
//         "^\\s*?sp\\s*?\\{(.*?-->.*?)\\}\\s*(?:$|;)",
      /////
      /////  FIXME.  I Modified the behavior of regcomp to let '.' match
      /////          a newline.  There is no way to get the posix compliant
      /////          'regcomp' to do that.  These calls should be changed
      /////          to the standard pcre calls.
      /////
      regex_t compiled_pattern;
      int error_code = regcomp( &compiled_pattern,
         "^\\s*?sp\\s*?\\{(.*?-->.*?)\\}",
                                REG_EXTENDED|REG_NEWLINE);

      MegaAssert( error_code == 0, "Could not compile regular expression!");

      // Applying the regular expressions until all matches have been found
      regmatch_t match[3];
      
      while ( regexec(&compiled_pattern,
                      inputFile.c_str(),
                      3,
                      match,
                      0) == 0 ) {

         // Saving the match as a string
         string prod = inputFile.substr(match[1].rm_so, match[1].rm_eo-match[1].rm_so );

         // TODO: Remove this or send it to the Debug log (when there is one)
		 //voigtjr: removed next three lines
         //cout << "========================" << endl;
         //cout << "PRODUCTION = " << prod << endl;
         //cout << "========================" << endl;

         // TODO: This should not be needed, FIX!
//         soarAlternateInput(a, const_cast<char *>(prod.c_str()), ") ", true); 
//         set_lexer_allow_ids (a, false);
//         get_lexeme(a);
//         production* p;
//         p = parse_production(a);

         AddProduction(const_cast<char *>(prod.c_str()));

         // Removing the largest match from the inputFile string
         inputFile.erase(match[0].rm_so, match[0].rm_eo-match[0].rm_so);      
      }  

      // Freeing the C API stuff
      regfree( &compiled_pattern );     

      // All the production matches are removed as part of the search so no
      // call to regexDeleteWrapper is needed here
   }
   /*
   ==================================
    _       _     _ ____                _            _   _
   / \   __| | __| |  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
  / _ \ / _` |/ _` | |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
 / ___ \ (_| | (_| |  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
/_/   \_\__,_|\__,_|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
| |   (_)___| |_ ___ _ __   ___ _ __
| |   | / __| __/ _ \ '_ \ / _ \ '__|
| |___| \__ \ ||  __/ | | |  __/ |
|_____|_|___/\__\___|_| |_|\___|_|
   ==================================
   */
   void ProductionManager::AddProductionListener(egSKIProductionEventId eventId, 
                                                 IProductionListener* listener, 
                                                 bool                 allowAsynch ,
                                                 Error*               err)
   {
      MegaAssert(listener, "Cannot add a 0 listener pointer.");
      if(!listener)
      {
         SetError(err, gSKIERR_INVALID_PTR);
         return;
      }

      ClearError(err);
      m_productionListeners.AddListener(eventId, listener);

      // If we have added our first listener, we tell the kernel
      //  we want to recieve these events.
      if(m_productionListeners.GetNumListeners(eventId) == 1)
      {
         // This is a kernel call (not part of gSKI)
         gSKI_SetAgentCallback(m_agent->GetSoarAgent(), 
                               EnumRemappings::RemapProductionEventType(eventId),
                               static_cast<void*>(this),
                               HandleKernelProductionCallbacks);
      }   
   }

   /*
   ==================================
 ____                               ____                _            _   _
|  _ \ ___ _ __ ___   _____   _____|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \ |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
|  _ <  __/ | | | | | (_) \ V /  __/  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
|_| \_\___|_|_|_| |_|\___/ \_/ \___|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
| |   (_)___| |_ ___ _ __   ___ _ __
| |   | / __| __/ _ \ '_ \ / _ \ '__|
| |___| \__ \ ||  __/ | | |  __/ |
|_____|_|___/\__\___|_| |_|\___|_|
   ==================================
   */
   void ProductionManager::RemoveProductionListener(egSKIProductionEventId eventId,
                                                    IProductionListener* listener,
                                                    Error*               err)
   {
      MegaAssert(listener, "Cannot remove a 0 listener pointer.");
      if(!listener)
      {
         SetError(err, gSKIERR_INVALID_PTR);
         return;
      }

      ClearError(err);
      m_productionListeners.RemoveListener(eventId, listener);

      // If we have no more listeners, stop asking kernel to
      //  notify us
      if(m_productionListeners.GetNumListeners(eventId) == 0)
      {
         // This is a kernel call (not part of gSKI)
         // Setting the callback to 0 causes the kernel
         //   not to fire the event
         // This is a kernel call (not part of gSKI)
         gSKI_SetAgentCallback(m_agent->GetSoarAgent(), 
                               EnumRemappings::RemapProductionEventType(eventId),
                               0, 0);
      }
   }

   /*
   ==================================
 _   _                 _ _      _  __                    _
| | | | __ _ _ __   __| | | ___| |/ /___ _ __ _ __   ___| |
| |_| |/ _` | '_ \ / _` | |/ _ \ ' // _ \ '__| '_ \ / _ \ |
|  _  | (_| | | | | (_| | |  __/ . \  __/ |  | | | |  __/ |
|_|_|_|\__,_|_| |_|\__,_|_|\___|_|\_\___|_|  |_| |_|\___|_|
|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
| |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
|  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
|_|___|_|  \___/_\__,_|\__,_|\___|\__|_|\___/|_| |_|
 / ___|__ _| | | |__   __ _  ___| | _____
| |   / _` | | | '_ \ / _` |/ __| |/ / __|
| |__| (_| | | | |_) | (_| | (__|   <\__ \
 \____\__,_|_|_|_.__/ \__,_|\___|_|\_\___/
   ==================================
   */
   void ProductionManager::HandleKernelProductionCallbacks(unsigned long         eventId, 
                                                           unsigned char         eventOccured,
                                                           void*                 object, 
                                                           agent*                soarAgent, 
                                                           void*                 data)
   {
      ProductionManager*   pm = static_cast<ProductionManager*>(object);
      Production*           p;
      IProductionInstance* pi;

	  // The gSKI events are
	  // BEFORE_PRODUCTION_REMOVED and
	  // AFTER_PRODUCTION_ADDED
	  // AFTER_PRODUCTION_FIRED
	  // BEFORE_PRODUCTION_RETRACTED
	  // so check that the timing of the before/after logic is correct from the kernel.
	  if ((eventOccured && eventId == gSKI_K_EVENT_PRODUCTION_REMOVED) ||
		  (!eventOccured && eventId == gSKI_K_EVENT_PRODUCTION_ADDED) ||
		  (!eventOccured && eventId == gSKI_K_EVENT_PRODUCTION_FIRED) ||
		  (eventOccured && eventId == gSKI_K_EVENT_PRODUCTION_RETRACTED))
	  {
		  // Why is the kernel issuing a callback which we can't pass on to gSKI.  One or the other should change.
		  return ;
	  }

         if((eventId == gSKI_K_EVENT_PRODUCTION_ADDED) || 
            (eventId == gSKI_K_EVENT_PRODUCTION_REMOVED))
         {
            p  = new Production(static_cast<production*>(data), false, pm->m_agent->GetSoarAgent());
            pi = 0;
         }
         else
         {
            // The data passed in is a production instance
            instantiation* soarPI = static_cast<instantiation*>(data);

            // Get the production from the instantiation
            p =  new Production(soarPI->prod, false, pm->m_agent->GetSoarAgent());

            // Dont have an instantiation yet
            pi = 0;
         }

         // We have to change the the event id from a kernel id to a gSKI id
         ProductionNotifier pn(pm->m_agent, p, pi);
         pm->m_productionListeners.Notify(EnumRemappings::Map_Kernel_to_gSKI_ProdEventId(eventId, eventOccured), pn);

         // Clean up the new data members
         if(p)
            p->Release();
         
//         if(pi)
//            pi->Release();   
	}
}


//MUT stuff

bool makeDir(const char* name){
#ifdef WIN32
	return !(_mkdir(name));
#else
	return !(mkdir(name, S_IRWXU | S_IRWXG | S_IRWXO));
	return false;
#endif
}

bool removeDir(const char* name){
#ifdef WIN32
	return !(_rmdir(name));
#else
	return !(rmdir(name));
#endif
}


#include "gSKI_Agent.h"
#include "gSKI_AgentManager.h"
#include "gSKI_Kernel.h"
#include "gSKI_KernelFactory.h"
#include "gSKI_Stub.h"

using namespace gSKI;
#ifndef NO_MEGA_UNIT_TESTS

void prodTest();

DEF_TEST_INSUITE(ProductionFileLoadTesting, Start)
{
   prodTest();
}

#endif

inline string makeDummyProd(const string& name){
  string dummyProduction = "sp{ "+ name + "\n"
  			    "(state <s> ^type state \n"
   			    "^io.output-link <ol> )\n"
			    "-->\n"
			    "(<ol> ^test <t1>)\n"
			    "(<t1> ^value " + name + ")\n"
			    "}\n";
   return dummyProduction;
}

inline bool isValidProdName(const string& name){
   return name == "test1" || name == "test2" || name == "test3" || name == "test4";
}

#ifndef NO_MEGA_UNIT_TESTS

void prodTest(){
   bool madeDir = false;
   bool madeDir2 = false;
   ofstream out;
   out.open("test1.soar");
   VALIDATE(!out.fail());
   out <<"source \"MUT/test2.soar\""<<endl;
   out <<"source \"test1.soar\""<<endl;
   out<<makeDummyProd("test1")<<endl;
   out.close();
   out.open("test3.soar");
   VALIDATE(!out.fail());
   out << "source \"test1.soar\""<<endl;
   out << "source \"MUT/test2.soar\""<<endl;
   out << "source \"MUT/innerDir/test4.soar\"" <<endl;
   out<<makeDummyProd("test3")<<endl;
   out.close();
   out.open("MUT/test2.soar");
   if(out.fail()){
      //most likely directory doesn't exist
	  madeDir = true;
	  VALIDATE(makeDir("MUT"));
	  out.clear();
      out.open("MUT/test2.soar");
	  VALIDATE(!out.fail());
   }
   out<<"source \"../test3.soar\""<<endl;
   out<<makeDummyProd("test2")<<endl;
   out.close();
   out.open("MUT/innerDir/test4.soar");
   if(out.fail()){
      //most likely directory doesn't exist
	  madeDir2 = true;
	  VALIDATE(makeDir("MUT/innerDir"));
	  out.clear();
      out.open("MUT/innerDir/test4.soar");
	  VALIDATE(!out.fail());
   }
   out << "source \"../../test1.soar\""<<endl;
   out << "source \"../test2.soar\""<<endl;
   out << "source \"../../test3.soar\""<<endl;
   out<<makeDummyProd("test4")<<endl;
   out.close();
   IKernelFactory*      kF    = gSKI_CreateKernelFactory();
   VALIDATE(kF != 0);

   IKernel*             k     = kF->Create();
   VALIDATE(k != 0);

   IAgentManager*       IAM   = k->GetAgentManager();
   VALIDATE(IAM != 0);

   IAgent*              agent = IAM->AddAgent("ProductionTestAgent");
   VALIDATE(agent != 0);

   IProductionManager*  IPM   = agent->GetProductionManager();
   VALIDATE(IPM != 0);

   VALIDATE(IPM->LoadSoarFile("test1.soar"));
   tIProductionIterator* itr= IPM->GetAllProductions();
   VALIDATE(itr->IsValid());
   VALIDATE(isValidProdName(itr->GetVal()->GetName()));
   itr->Next();
   VALIDATE(itr->IsValid());
   VALIDATE(isValidProdName(itr->GetVal()->GetName()));
   itr->Next();
   VALIDATE(itr->IsValid());
   VALIDATE(isValidProdName(itr->GetVal()->GetName()));
   itr->Next();
   VALIDATE(itr->IsValid());
   VALIDATE(isValidProdName(itr->GetVal()->GetName()));
   itr->Next();
   VALIDATE(!itr->IsValid());
   
   itr->Release();
   IAM->RemoveAgentByName(agent->GetName());
//   k->Release();  
   kF->Release();
      
   VALIDATE(!remove("MUT/innerDir/test4.soar"));
   if(madeDir2){
	   VALIDATE(removeDir("MUT/innerDir"));
   }
   VALIDATE(!remove("MUT/test2.soar"));
   VALIDATE(!remove("test1.soar"));
   VALIDATE(!remove("test3.soar"));
   if(madeDir){
      VALIDATE(removeDir("MUT"));
   }

}

#endif




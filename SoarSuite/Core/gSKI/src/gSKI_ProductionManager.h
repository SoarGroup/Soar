/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_ProductionManager.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef GSKI_PRODUCTIONMANAGER_H
#define GSKI_PRODUCTIONMANAGER_H

#include "IgSKI_Iterator.h"
#include "IgSKI_ProductionManager.h"
#include "IterUtils.h"
#include "gSKI_Iterator.h"

#include "EventManagementTemplates.h"

#include <string>
#include <set>
#include <map>


typedef struct agent_struct agent;
typedef std::set<std::string> tStringSet;

namespace gSKI {
   
   class IProduction;
   class IMatchSet;
   class IProductionMatch;
   class IProductionInstance;
   class IWME;
   class ICondition;
   class Production;
   class Agent;
   struct Error;

   /**
    * @brief This is the abstract base-class for the Class that manages
    *        productions. 
    *
    *        This interface is used to do the following:
    *        @li To create new productions
    *        @li In cross/production methods
    *        @li Accessing production information for a particular
    *            time.  (Since time is not part of the Production
    *            structure, we need to deal with it at a higher level.
    *            Particularly when we are dealing with matches, whether
    *            or not a production matches is determined not only by
    *            the production conditions, but by the state of memory.)
    *
    *    These methods are not specific to a particular production at every
    *    point in time.
    */
   class ProductionManager : public IProductionManager
   {
   public:

      /**
       * @brief Destructor for the ProductionManager
       *
       * This function insures that the destructor in the most derived
       * class is called when it is destroyed.  This will always be 
       * neccesary because this is a pure-virtual base-class.
       */
     ~ProductionManager();

     /**
      * @brief: Constructor for the ProductionManager
      *
      * @param agent_ The agent associated with this Production Manager.
      */
     ProductionManager(Agent *agent_);

      /**
       * @brief This loads a standard Soar File
       *
       * The file that gets loaded is a text file containing the productions
       * and certain other minimal information relevant to those productions.
       * This is <b>NOT</b> a Tcl file.  It will not parse Tcl files.  If you
       * need to parse a Tcl File, you will need to include the Tcl Parsing
       * library.  The name of the file can be specified as either the full
       * path, or the relative path. 
       *
       * @note When is this valid to call?
       *
       * <p>Errors Returned:
       *    @li @c gSKIERR_FILE_NOT_FOUND
       *    @li @c gSKIERR_BAD_FILE_FORMAT
       *
       * @param fileName The name of the file to be loaded.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       */
     bool LoadSoarFile(const char *fileName, Error *err = 0);

      /**
       * @brief Adds a production
       *
       * <p>Errors Returned:
       *    @li @c gSKIERR_PRODUCTION_ALREADY_EXISTS
       *
       * @param newProd The production to be added.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       * 
       * @returns Success or failure of the operation.
       */
      // TODO: What is the intermediate form used to add a production?
     bool AddProduction(const IProduction *newProd, Error *pErr = 0);

      /**
       * @brief Removed the specified Production
       *
       * <p>Errors Returned:
       *    @li @c gSKIERR_PRODUCTION_DOES_NOT_EXISTS
       *
       * @param p The production to be removed.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns Success or failure of the operation.
       */
     bool RemoveProduction(IProduction* p, Error* err = 0) const;

	  /**
       * @brief Remove all Productions
       *
       * <p>Errors Returned:
       *    @li @c gSKIERR_PRODUCTION_DOES_NOT_EXISTS
       *
       * @param  i   Counter for number of productions being removed.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns Success or failure of the operation.
       */
       bool RemoveAllProductions(int& i, Error *pErr ) const;

	  /**
       * @brief Remove all UserProductions
       *
       * <p>Errors Returned:
       *    @li @c gSKIERR_PRODUCTION_DOES_NOT_EXISTS
       *
       * @param  i   Counter for number of productions being removed.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns Success or failure of the operation.
       */
       bool RemoveAllUserProductions(int& i, Error *pErr ) const;

	  /**
       * @brief Remove all Chunks and Justifications
       *
       * <p>Errors Returned:
       *    @li @c gSKIERR_PRODUCTION_DOES_NOT_EXISTS
       *
       * @param  i   Counter for number of productions being removed.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns Success or failure of the operation.
       */
       bool RemoveAllChunks (int& i, Error *pErr ) const;

	  /**
       * @brief Remove all DefaultProductions
       *
       * <p>Errors Returned:
       *    @li @c gSKIERR_PRODUCTION_DOES_NOT_EXISTS
       *
       * @param  i   Counter for number of productions being removed.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns Success or failure of the operation.
       */
       bool RemoveAllDefaultProductions(int& i, Error *pErr ) const;

      /**
       * @brief Deletes a set of Productions
       *
       * This takes an iterator to a set of WMEs that are used
       * to remove Productions.  This could, for instance, be used
       * to remove all chunk productions by doing something like this:
       *
       * <p>Errors Returned:
       *    @li @c gSKIERR_PRODUCTION_DOES_NOT_EXISTS
       * 
       * @code
       *    ProdMgr.RemoveProductionSet(ProdMgr.GetChunks());
       * @endcode
       *
       * @param prodSet The set of productions to be removed.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns Success or failure of the operation.
       */
     bool RemoveProductionSet(tIProductionIterator* prodSet, Error* err = 0);

      /**
       * @brief Loads a RETE From a file
       *
       * <p>Errors Returned:
       *    @li @c gSKIERR_FILE_NOT_FOUND
       *
       * @param fn The name of the file to load the RETE from.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns Success or failure of the operation.
       */
     bool LoadRete(const char* fn, Error* err = 0);

      /**
       * @brief Saves all of the production to a file is RETE format.
       *
       * <p>Errors Returned:
       *    @li @c gSKIERR_DISK_FULL
       *    @li @c gSKIERR_BAD_FILE_PERMISSIONS
       *
       * @param fn The name of the file to save the RETE to.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns Success or failure of the operation.
       */
     bool SaveRete(const char *fn, Error *err = 0) const;


/////////////////////////////////////////////////////////////////////////////

      /**
       * @brief Gets the productions that are currently matched.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns An iterator to the set of currently matched Productions.
       */
     tIProductionMatchIterator *GetMatchedProductions(Error* err = 0) const;

      /**
       * @brief Gets match sets for the given production.
       *
       * This function is in the Production Manager instead of the production
       * because matches have a temporal aspect.  The match set for a production
       * will probably change from decision cycle to decision cycle.
       *
       * It is also worth noting that the value returned from this call is 
       * always a MatchSet, even if there are no negated conditions at all.
       * What this means is that the MatchSet returned will be a non-negated
       * set.  The only one you will find in the list.
       *
       * <p>Errors Returned:
       *    @li @c gSKI_INVALID_PTR
       *
       * @param prod This is the production we are looking for the
       *             matches for.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The set of matches to the given production.
       */
     IMatchSet* GetMatchSets(const IProduction* prod, Error* err = 0) const;


      /**
       *	@brief Get wmes that match a single condition
       *
       * Given a single condition, we find all of the WMEs that match it
       * at the current point in time.  It is important to note that if 
       * the condition is negated, it returns the list of wmes that match
       * the un-negated condition.  This can be used to determine why the
       * condition is not matching as exptected.
       *
       * <p>Errors Returned:
       *    @li @c gSKI_INVALID_PTR
       *
       * @param condition The condition we are seeking the matches for.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       * 
       * @returns A set of WMEs that match the given Condition
       */
     tIWmeIterator* GetConditionMatches(const ICondition* condition, Error* err = 0) const;

/////////////////////////////////////////////////////////////////////////////

      // TODO: Determine and describe what a pattern is.
      /**
       * @brief Returns the list of all Productions that match the given
       *        pattern.
       *
       * <p>Errors Returned:
       *    @li @c gSKIERR_INVALID_PATTERN
       *
       * @todo Specify what a pattern is exactly.
       *
       * @param pattern The Pattern we are matching against.
	   * @param includeConditions	If true, the productions include all gSKI condition objects.
	   *			This process involves copying the underlying list of conditions in kernel production, so
	   *			you should only do this if you're going to work with the conditions (which is not common).
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The list of productions that match the pattern.
       */
     tIProductionIterator* GetProduction(const char* pattern, bool includeConditions = false, Error* err = 0) const;

      /**
       * @brief Gets all of the productions.
       *
	   * @param includeConditions	If true, the productions include all gSKI condition objects.
	   *			This process involves copying the underlying list of conditions in kernel production, so
	   *			you should only do this if you're going to work with the conditions (which is not common).
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns An iterator to a list of all of the productions for
       *          this agent.
       */
     tIProductionIterator* GetAllProductions(bool includeConditions = false, Error* err = 0) const;
 
      /**
       * @brief Gets user productions
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns An iterator to a list of all user productions
       *          for this agent.
       */
    tIProductionIterator* GetUserProductions(Error* err = 0) const;

      /**
       * @brief Gets all of the chunk productions.
       *
       * @returns An iterator to a list of all of the Productions 
       *          created as chunks for this agent.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       */
     tIProductionIterator* GetChunks(Error* err = 0) const;

      /**
       * @brief Get all justification productions.
       *          productions.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns An iterator to a list of justification
       */
      // TODO: This should take an argument.  What does this do?
     tIProductionIterator* GetJustifications(Error* err = 0) const;

      /**
       * @brief Get all default productions.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns An iterator to a list of all default productions
       *          for this agent.
       */
     tIProductionIterator* GetDefaultProductions(Error* err = 0) const;

   private:
      /**
       * @brief: Helper function to load in a Soar File without repeats
       *
       * @param sourcedFiles the set of filenames with full paths that have been loaded in
       * @param fileName the filename for the file we are attempting to load
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       * @returns if it was successful or not
       */
      bool loadSoarFile(tStringSet& sourcedFiles, const char *fileName, Error *err);
      /*************************** Listeners *********************************/

      /**
      *  @brief Adds a listener for production events
      *
      *  Call this method to register a listener to recieve production events.
      *  Agent production events are:
      *     @li gSKIEVENT_AFTER_PRODUCTION_ADDED,
      *     @li gSKIEVENT_BEFORE_PRODUCTION_REMOVED,
      *     @li gSKIEVENT_AFTER_PRODUCTION_FIRED,
      *     @li gSKIEVENT_BEFORE_PRODUCTION_RETRACTED,
      *
      *  If this listener has already been added for the given event, nothing happens
      *
      *  Possible Errors:
      *     @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener A pointer to a client owned listener that will be called back when
      *                      an event occurs.  Because the listener is client owned, it is not
      *                      cleaned up by the kernel when it shuts down.  The same listener
      *                      can be registered to recieve multiple events.  If this listener
      *                      is 0, no listener is added and an error is recored in err.
      *  @param allowAsynch A flag indicating whether or not it is ok for the listener to be
      *                         notified asynchonously of system operation.  If you specify "true"
      *                         the system may not callback the listener until some time after
      *                         the event occurs. This flag is only a hint, the system may callback
      *                         your listener synchronously.  The main purpose of this flag is to
      *                         allow for efficient out-of-process implementation of event callbacks
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      void AddProductionListener(egSKIProductionEventId eventId, 
                                 IProductionListener* listener, 
                                 bool                 allowAsynch = false,
                                 Error*               err         = 0);

     /**
      *  @brief Removes an production event listener
      *
      *  Call this method to remove a previously added event listener.
      *  The system will automatically remove all listeners when the kernel shutsdown;
      *   however, since all listeners are client owned, the client is responsible for
      *   cleaning up memory used by listeners.
      *
      *  If the given listener is not registered to recieve the given event, this
      *     function will do nothing (but a warning is logged).
      *
      *  Agent production events are:
      *     @li gSKIEVENT_AFTER_PRODUCTION_ADDED,
      *     @li gSKIEVENT_BEFORE_PRODUCTION_REMOVED,
      *     @li gSKIEVENT_AFTER_PRODUCTION_FIRED,
      *     @li gSKIEVENT_BEFORE_PRODUCTION_RETRACTED,
      *
      *  Possible Errors:
      *     @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener A pointer to the listener you would like to remove.  Passing a 0
      *                     pointer causes nothing to happen except an error being recorded
      *                     to err.
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      void RemoveProductionListener(egSKIProductionEventId eventId,
                                     IProductionListener* listener,
                                     Error*               err = 0);

   private:

      ///////////////// LISTENER MANAGEMENT STUFF ////////////////////////////
      /** 
       * @brief Event notifier for run events
       */
      class ProductionNotifier {
      public:
         /**
          * @brief 
          */
         ProductionNotifier(IAgent* a, IProduction* p, IProductionInstance* m) : 
                                                                    m_agent(a), 
                                                                    m_prod(p), 
                                                                    m_match(m) 
         {
         
         }

         /**
          * @brief 
          */
         void operator()(egSKIProductionEventId eventId, IProductionListener* listener)
         {
            listener->HandleEvent(eventId, m_agent, m_prod, m_match);
         }
      private:
         IAgent*               m_agent;
         IProduction*          m_prod;
         IProductionInstance*  m_match;
      };
      
      /** 
       * @brief Listener manager definitions 
       */
      //{
      typedef ListenerManager<egSKIProductionEventId, IProductionListener, ProductionNotifier>   tProductionListenerManager;
      //}

      /** 
       * @brief Static function to handle callbacks for the production listeners.
       *
       * @param eventId  Id of the kernel level event that occured.
       * @param eventOccured true if the event happened already, false if
       *                      the event is about to happen
       * @param object   Pointer to an Agent object (casted to void)
       * @param soarAgent Pointer to the kernel level agent structure associated
       *                    with this callback
       * @param data      Callback data (in this case a const char*)
       */
      static void HandleKernelProductionCallbacks(unsigned long         eventId, 
                                                  unsigned char         eventOccured,
                                                  void*                 object, 
                                                  agent*                soarAgent, 
                                                  void*                 data);

      ////////////////////////////////////////////////////////////////////////

   private:
      /**
       * @brief: Helper function load load a file in to a string.
       *
       * @param is The stream to be copied to the string.
       */
     std::string loadFile(std::istream& is);

     /**
      * @brief: Remove the comments from the soar file.
      *
      * @param inputFile The string holding the input file we want to
      *                  remove comments from.
      */
     void removeComments(std::string & inputFile);

     /**
      * @brief: Parses out the Soar Productions and loads them in
      *         to the kernel.
      */
     void parseProductions(std::string &inputFile);

     /**
      * @brief Parses all of the file for sources and attempts to source them
      *
      * @param sourcedFiles the Set of strings containing the files that have already
      *              been sourced
      * @param file The input string we are search for the "source"
      *              command in.
      * @param path The path to the soar file that we are parsing. 
      * @param  err Pointer to client-owned error structure.  If the pointer
      *              is not NULL this structure is filled with extended error
      *              information.  If it is NULL (the default) extended error
      *              information is not returned.
      */
	 bool parseSources(tStringSet& sourcedFiles, std::string &file, std::string &path, Error *err);

     /**
      * @brief Gets the current working directory.
      *
      * @returns a string holding the path of the cwd.
      */
     std::string getCurrentWorkingDirectory();

  
   private:
        /**
         * @brief: A vector of productions.
         */
        typedef std::vector<IProduction *> prodVec;

        /**
         * @brief: Returns all productions of given type in a std::vector
         *         of productions.
         *
         * @param prodVec the vector the productions will be placed in.
		 * @param includeConditions	If true, the productions include all gSKI condition objects.
		 *			This process involves copying the underlying list of conditions in kernel production, so
		 *			you should only do this if you're going to work with the conditions (which is not common).
         * @param prodType The type of productions that will be added to the vector.
         */
        void GetProductions(prodVec& prodVec, bool includeConditions, unsigned char prodType) const;

        /**
         * @brief: Selects an alternate input source for soar productions.
         *
         * @prod ai_agent
         * @prod ai_string
         * @prod ai_suffix
         * @prod ai_exit
         *
         */
        void soarAlternateInput(agent *ai_agent, char  *ai_string, 
                                char  *ai_suffix, char  ai_exit   );

        /**
         * @brief This takes a const char * representation of a production,
         *        already fully parsed and adds it to the agent.
         *
         * @param thisAgent The agent that the production will be added to.
         * @param productionText The production being added to the agent.
         *
         */
        void AddProduction(char* productionText, Error* err = 0);

 
   private:

         
        typedef FwdContainerType< std::vector<IProduction * > >  tProductionVec;
        typedef IteratorWithRelease<tProductionVec::V, tProductionVec::t>  tProductionIter;

        Agent*                         m_agent; /**< The agent that holds these productions. */

        tProductionListenerManager     m_productionListeners;
 
   };
}
#endif

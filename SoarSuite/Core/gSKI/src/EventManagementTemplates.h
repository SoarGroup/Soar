/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file eventmanagementtemplates.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef EVENT_MANAGEMENT_TEMPLATES_H
#define EVENT_MANAGEMENT_TEMPLATES_H

#include "MegaAssert.h"
#include "gSKI_Events.h"
#include "gSKI_Error.h"

#include <vector>
#include <map>
#include <algorithm>

namespace gSKI
{

   /**  
    * @brief Helper class to manage listeners
    *
    * This class does all the basics of managing listeners.
    *  It holds a list of them, adds and removes from them
    *  and handles thread pauses when required.
    */
   template<typename EventType, typename ListenerType, typename NotifierType>
   class ListenerManager
   {
   public:
      
      typedef ListenerType tListener;
      typedef NotifierType tNotifier;
      typedef unsigned int tKey;

      /** 
       * Defines a list of listeners
       */
      //{
      typedef std::vector<tListener*>         tListenerVector;
      typedef typename tListenerVector::iterator  tListenerVectorIt;

      typedef std::map<tKey, tListenerVector> tListenerMap;
      typedef typename tListenerMap::iterator     tListenerMapIt;
      //}

   public:

      /** 
       * @brief Adds a listener to the listener list
       *
       * @param eventId  Id of the event to add a listener for
       * @param listener Pointer to the listener to add to list
	   * @return true if the listener was added (i.e. not a duplicate)
       */
      bool AddListener(EventType eventId, tListener* listener)
      {
         // Get the correct list of listeners
         tListenerVector& listenerVec = m_listeners[static_cast<tKey>(eventId)];

         // Only add if it is not in the list already
         tListenerVectorIt it = std::find(listenerVec.begin(), listenerVec.end(), listener);
         if(it == listenerVec.end())
		 {
            listenerVec.push_back(listener);
			return true ;
		 }
		 return false ;
      }

      /** 
       * @brief Removes a listener from the listener list
       * 
       * A removed listener will no longer recieve event notifications.
       *
       * @param eventId  Id of the event to remove the listener for
       * @param listener Pointer to the listener to remove.
	   * @return true if listener was removed (i.e. it was listening previously)
       */
      bool RemoveListener(EventType eventId, tListener* listener)
      {
         tListenerMapIt itMap = m_listeners.find(static_cast<tKey>(eventId));
         if(itMap != m_listeners.end())
         {
            // Just for convenience, we get a reference to the list
            tListenerVector& listenerVec = (*itMap).second;

            // Only remove if it is in the list now
            tListenerVectorIt itVec = std::find(listenerVec.begin(), listenerVec.end(), listener);
            if(itVec != listenerVec.end())
			{
               listenerVec.erase(itVec);
			   return true ;
			}
         }
		 return false ;
      }

      /** 
       * @brief Notifies listeners of an event
       *
       * This template cannnot know what data is passed to the listener;
       *  therefore, it calls a notifier object to notify the listener
       *  of the event. 
       *
       * This method also stops the execution of the agent thread
       *  if the callback stops the system.
       *
       * @param eventId  Event of which to notify the listener
       * @param notifier Reference to a functor object that overloads
       *          operator() to take a pointer to a listener.  This
       *          method should callback the listener's event method
       *          with appropriate data.
       */
      void Notify(EventType eventId, tNotifier& notifier)
      {
         tListenerMapIt itMap = m_listeners.find(static_cast<tKey>(eventId));
         if(itMap != m_listeners.end())
         {
            // Just for convenience, we get a reference to the list
            tListenerVector& listenerVec = (*itMap).second;

            tListenerVectorIt itVec;
            tListenerVectorIt itEND = listenerVec.end();
            for(itVec= listenerVec.begin(); itVec != itEND; ++itVec)
            {
               notifier(eventId, *itVec);
               
               // Do a check for stop here!

            }
         }

         // Do a stop after all callbacks here!
      }

	  // Same as "Notify" above, but returns true if a notifier returns true
	  // and stops notification at that point.
	  bool NotifyGetResult(EventType eventId, tNotifier& notifier)
	  {
	     bool result = false ;

         tListenerMapIt itMap = m_listeners.find(static_cast<tKey>(eventId));
         if(itMap != m_listeners.end())
         {
            // Just for convenience, we get a reference to the list
            tListenerVector& listenerVec = (*itMap).second;

            tListenerVectorIt itVec;
            tListenerVectorIt itEND = listenerVec.end();
            for(itVec= listenerVec.begin(); itVec != itEND; ++itVec)
            {
               result = notifier(eventId, *itVec);
               
			   if (result)
				   return result ;
            }
         }

		 return result ;
	  }

      /** 
       * @brief Get the number of listeners this manager is holding for
       *           the given event.
       *
       * @param eventId Id for which to retrieve the listeners
       * @return Number of listeners for the given event
       */
      unsigned int GetNumListeners(EventType eventId)
      {
         tListenerMapIt itMap = m_listeners.find(static_cast<tKey>(eventId));
         return (itMap != m_listeners.end())? static_cast<unsigned int>(((*itMap).second).size()): 0;
      }

      /** 
       * @brief Begin and End access for listeners (allows iteration)
       */
      //{
      tListenerVectorIt    Begin(EventType eventId)              
      { 
         return m_listeners[static_cast<tKey>(eventId)].begin(); 
      }
      tListenerVectorIt    End(EventType eventId)
      { 
         return m_listeners[static_cast<tKey>(eventId)].end();   
      }
      //}

   private:

      /** List of event listeners */
      tListenerMap   m_listeners;
   };

   /** 
    * @brief Simple helper to do the tasks for registering listeners
    *
    * @param manager  Listener manager you wish to add to
    * @param eventId  Id of the event for which to register
    * @param listener Pointer to the listener to add to the manager
    * @param err      gSKI's error structure for reporting errors
    */
   template<typename Manager, typename EventType, typename Listener>
   inline void AddListenerToManager(Manager& manager, EventType eventId,
                                    Listener* listener, Error* err)
   {
      MegaAssert(listener, "Cannot add a 0 listener pointer.");
      if(!listener)
      {
         SetError(err, gSKIERR_INVALID_PTR);
         return;
      }

      ClearError(err);
      manager.AddListener(eventId, listener);
   }

   /** 
    * @brief Simple helper to do the tasks for unregistering listeners
    *
    * @param manager  Listener manager you wish to add to
    * @param eventId  Id of the event for which to unregister
    * @param listener Pointer to the listener to add to the manager
    * @param err      gSKI's error structure for reporting errors
    */
   template<typename Manager, typename EventType, typename Listener>
   inline void RemoveListenerFromManager(Manager& manager, EventType eventId,
                                         Listener* listener, Error* err)
   {
      MegaAssert(listener, "Cannot add a 0 listener pointer.");
      if(!listener)
      {
         SetError(err, gSKIERR_INVALID_PTR);
         return;
      }

      ClearError(err);
      manager.RemoveListener(eventId, listener);
   }
}

#endif

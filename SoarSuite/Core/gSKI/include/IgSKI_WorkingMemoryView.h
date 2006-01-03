/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_workingmemoryview.h 
*********************************************************************
* created:	   7/11/2002   14:54
*
* purpose: 
*********************************************************************/

#ifndef IGSKI_WORKINGMEMORYVIEW_H
#define IGSKI_WORKINGMEMORYVIEW_H

#include "IgSKI_Iterator.h"

namespace gSKI {

   // Forward declarations
   class IWMObject;
   class IWme;
   class IWMStaticView;
   struct Error;

   /**
    * @brief This interface represents a collection of working memory data
    *         and provides methods for searching and creating sub views
    *
    * This interface is used to manaage a  collection of working memory data 
    * (both WMObjects and Wmes) and provides methods for searching over
    * memory data elements and creating static snap shots of protions of 
    * working memory (using the CreateSubView() method).
    */
   class IWorkingMemoryView {

   public:
      /**
       * @brief Virtual Destructor
       *
       * Including a virtual destructor for the usual C++ safety reasons.
       */
      virtual ~IWorkingMemoryView() {}

      /**
       * @brief Returns the WMObject corresponding to the specified id
       *
       * This method returns the WMObject that corresponds to the specified ID
       * string. If no WMObject exists with the specified ID string or the 
       * string itself is NULL. then a NULL WMObject pointer is returned.
       *
       * @param idstring the ID string of the desired WMObject
       * @param object a pointer to the desired object pointer (holds
       *             the return value). Returning the actual pointer
       *             would make it too easy to create memory leaks
       *             since this object has to be released.
       * @param err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return A pointer to the WMObject that corresponds to the specified
       *          id string, NULL if a WMObject with that ID does not exist
       *          or if the idstring itself is NULL.
       */
      virtual void GetObjectById(const char* idstring,
                                       IWMObject** object,
                                       Error* err = 0) const = 0;

      /** 
       * @brief Returns an iterator to all WMObjects that exist in the view
       *
       * This method returns an iterator to all the WMObjects that exist in
       * the view. The WMObjects will be released when the iterator is
       * released. To hold onto a WMObject pointer from an iterator use 
       * the GetValAndDetach() method. These WMObjects represent the state 
       * of the agent's working memory when this function is called but, 
       * since the Soar agent may modify its own working memory, may no longer
       * be accurate in future decision cycles.
       *
       * @param err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return an iterator to all the WMObjects currently in the agent's
       *          working memory
       */
      virtual tIWMObjectIterator* GetAllObjects(Error* err=0) const = 0;

      /**
       * @brief Returns an iterator to all Wmes that exist in the view
       *
       * This method returns an iterator to all the Wmes that exist in
       * the view. The Wme managed by this iterator will be released when the
       * iterator is released. To hold onto a Wme pointer from an iterator
       * use the GetValAndDetach() method. These Wmes represent the state of
       * the agent's working memory when this function is called, but, since
       * the Soar agent may modify its own working memory, may no longer be
       * accurate in future decision cycles.
       * 
       * @param err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return an iterator to all the Wmes currently in the agent's
       *          working memory
       */
      virtual tIWmeIterator* GetAllWmes(Error* err=0) const = 0;

      /**
       * @brief Returns an iterator to all the WMObjects that match the 
       *         specified criteria
       *
       * This method returns an iterator to all the WMObjects that match the
       * specified criteria.
       *
       * @param err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return an iterator to all the WMObjects that match the specified
       *          criteria
       */
      virtual tIWMObjectIterator* FindObjectsByCriteria(Error* err=0) const =0;

      /**
       * @brief Returns an iterator to all Wmes that match the specified 
       *         criteria
       *
       * This method returns an iterator to all the Wmes that match the 
       * specified criteria.
       *
       * @param err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return an iterator to all the WMObjects that match the specified
       *          criteria
       */
      // TODO: Need a better definition of what the criteria is. For speed
      // purposes it may make sense to provide fixed interfaces for some
      // kind of searches and a more extensible interface for other types
      // of searches. Possibly an interface with a single method that returns
      // true if a Wme or WMObject matches a criteria (and false otherwise).
      // This wouldn't be necessary if the "user" could create tWMeIterators
      // and tWMObjectIterators. (Convienience and standard for these patterns)
      virtual tIWmeIterator* FindWmesByCriteria(Error* err=0) const = 0;

      /**
       * @brief Creates a static subview of the working memory view
       *
       * This method creates a static subview of the current working memory
       * view using the specified WMObject as the root of the view. This
       * static subview consists of all the WMObjects referenced by the
       * specified WMObject, all the WMObjects referenced by these objects
       * and so on. None of the objects that reference the root object are
       * guaranteed to be included in the subview but due to circular 
       * references they may be included.
       *
       * @param rootobject A pointer to the root WMObject of the new subview
       * @param err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return a pointer to the subview created from rootobject
       */
      virtual IWMStaticView* CreateSubView(const IWMObject* rootobject,
                                           Error* err = 0) const = 0;

   };
}

#endif


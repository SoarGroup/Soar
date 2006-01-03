/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_outputwme.h 
*********************************************************************
* created:	   6/17/2002   13:16
*
* purpose: 
*********************************************************************/

#ifndef GSKI_OUTPUTWME_H
#define GSKI_OUTPUTWME_H

#include "IgSKI_Wme.h"
#include "gSKI_OutputWMObject.h"
#include "gSKI_ReleaseImpl.h"

#include "symtab.h"
#include "wmem.h"

namespace gSKI {

   class OutputWorkingMemory;
   class gSymbol;

   /**
    * @brief Interface for Soar Working Memory Elements
    *
    * This interface is used to externally expose the working memory of 
    * Soar agents. A WME consists of an ID, attribute and a value which are
    * all encapsulated using the ISymbol interface.
    */
   // TODO: Is it appropriate to keep WME preferences inside the WME interface
   // or is this only needed when adding WME's.
  class OutputWme: public RefCountedReleaseImpl<IWme, false> {
  public:

     /**
      * @brief For constructing OutputWme objects from io_wme objects
      *
      */
     OutputWme(OutputWorkingMemory* manager, wme* wme, IWMObject* valobj = 0);

     /**
      * @brief For constructing OutputWme objects from io_wme objects
      *
      */
     OutputWme(OutputWorkingMemory* manager, OutputWMObject* wmo, gSymbol* attr, gSymbol* val);


      /**
       * @brief Returns the object that owns this WME
       *
       * This method returns the WMObject that owns this WME. The owning 
       * WMObject is essentially the object that has the same ID as the ID
       * of the WME. For old school soar programmers this can be considered
       * a replacement for the GetID method that might be expected in the
       * WME interface. If the owning object no longer exists then this 
       * function returns NULL.
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return the object whose ID corresponds to the ID of the WME, NULL
       *          if the objetect no longer exists in the agent's current working memory.
       */
     IWMObject* GetOwningObject(Error* err = 0) const;

      /**
       * @brief Returns the attribute of the WME.
       *
       * This method returns the attribute of the WME (the attribute of the
       * ID, Attribute, Value symbol triplet). This attribute symbol pointer
       * is owned by the IWme. To obtain your own copy, use the Clone() method
       * of the ISymbol interface.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       * 
       * return the attribte symbol of the WME's (ID, Attribute, Value) symbol 
       *        triplet
       */
     const ISymbol* GetAttribute(Error* err = 0) const;

      /**
       * @brief Returns the value of the WME.
       *
       * This method returns the value of the WME (the value of the ID, 
       * attribute, value symbol triplet). This value symbol pointer is 
       * owned by the IWme. To obtain your own copy use the Clone() method of
       * the ISymbol interface.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       * 
       * return the value symbol of the WME's (ID, Attribute, Value) symbol 
       *        triplet
       */
     const ISymbol* GetValue(Error* err = 0) const;

      /**
       * @brief Returns the time tag of the WME.
       *
       * This method returns the time tag of the WME which is a unique id 
       * for each instance of the Soar kernel. Only one WME per kernel instance
       * (regardless of the number of agents) will have a given time tag.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       * 
       * return the time tag of the WME
       */
     long GetTimeTag(Error* err = 0) const;

      /**
       * @brief Returns the type of support that the WME has
       *
       * This method returns the type of support that the WME has. The possible
       * support types are described in IgSKI_Enumberations.h. The support 
       * type and supporting WME's ( see the GetSupportProductions() method )
       * determine the lifetime of a WME.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return the support type of the WME
       */
     egSKISupportType GetSupportType(Error * err = 0) const;
      
      /**
       * @brief Returns an iterator to the productions that created/support
       * the WME
       *
       * This method returns an iterator to the productions that 
       * created/support the WME. For WME's manually added by the user or
       * created externally on the input link, this iterator will be empty.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return an iterator to the productions that support the WME
       */
      /* TODO: This method should probably be eliminated */
     tIProductionIterator* 
         GetSupportProductions(Error* err = 0) const;

      /**
       * @brief Returns true if the WME has been removed from the agent's working memory
       *
       * This method returns true if WME has been removed from the agent's working memory.
       *  This function is necessary due to the dynamic nature of Soar's working
       *  memory. A WME obtained at one point in the decision cycle may no longer
       *  still exist in the agent's working memory at a later time.
       *
       * @note This method applies to the memory unit that owns this WME.
       *        If this object is part of main working memory, this method will
       *        tell you if the WMObject still exists in main working memory.
       *        If it is part of the input link or the ouput link, it tells you
       *        whether or not it exists in the input or output link.  Nothing
       *        can be removed from a static view, so this method will always
       *        return true for WMObjects owned by static views.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return true if the WME has been removed from working memory, false if
       *          it still exists in the agent's current working memory.
       */
     bool HasBeenRemoved(Error* err = 0) const;

     /**
       * @brief Compares two WMEs for equality.
       * 
       * This method compares two WMEs for equality. WME's are equal if they have
       *   equal ID's, Attributes, and Values.
       *
       * @param WME The WME with which to compare this WME
       * @param err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return true if the WME's are equal and false if they are not
       */
     bool IsEqual(IWme* wme, Error* err = 0) const;

     // TODO: We've removed the ability to get support information from 
     // the WME's themselves. Does this functionality need to be added 
     // or is it more appropriate in IWorkingMemory or IProductionManager?

     /**
      * @brief
      */
     void SetOwningObject(OutputWMObject *);
     bool AttributeEquals(const std::string& attr);

  private:
     OutputWorkingMemory* m_manager;
     OutputWMObject* m_owningobject;

     gSymbol* m_gattr;
     gSymbol* m_gvalue;

     unsigned long m_timetag;

  protected:
     /**
        Private destructor to ensure that client only deletes objects with Release.
		2/23/05: changed to protected to eliminate gcc warning
     */
     virtual ~OutputWme();
   };

}

#endif

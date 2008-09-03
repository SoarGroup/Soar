/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_iterator.h 
*********************************************************************
* created:	   6/13/2002   15:32
*
* purpose: 
*********************************************************************/

#ifndef IGSKI_ITERATOR_H
#define IGSKI_ITERATOR_H

#include <vector>

#include "IgSKI_Release.h"
#include "gSKI_Enumerations.h"

namespace gSKI {

   /** Forward declarations */
   struct Error;

  /**
   * @brief Iterator interface template
   *
   * This template defines the interface for iterators to any type in the
   *  gSKI system.  Iterators are pointer-like objects that know how to
   *  traverse a collection of items.  In the gSKI system an iterator 
   *  is the only public access given collections of elements.
   *
   * When returned by a gSKI function, iterators are owned by the client, therefore
   *  the client must release the iterator (using the Release() method).  If the
   *  iterator is passed into a callback, it is owned by the calling method and
   *  does not need to be destroyed.
   *
   * When iterators are created in gSKI, they start out pointing at the first element
   *  in the collection they point at.
   *
   * If an iterator points at an empty collection, it will be invalid immediately (IsValid will
   *  return false).
   *
   * If you need to make a copy of an iterator, use the clone method.  Clone will create a copy
   *  of the iterator pointing at exactly the same location in the same collection.
   *
   * If you are interfacing to gSKI using an advanced C++ compiler, use the iterator wrapper
   *  class to wrapp the iterator pointers returned by the gSKI API.
   */
   template<typename T>
   class IIterator: public IRelease {
   public:
      
      /** Typedef of the template parameter */
      typedef T tReturnType;

      /** Typedef of the type returned from a clone */
      typedef IIterator<tReturnType>    tCloneType;

   public:
      /**
      * @brief 
      */
      virtual ~IIterator() {}

      /**
      * @brief Go to the next element in the list this iterator references
      *
      * If the iterator is invalid, calling Next has no effect (but the
      *  error information object will contain an error).
      *
      * Possible Errors:
      *   @li gSKIERR_ITERATOR_INVALID
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      virtual void           Next(Error* err = 0) = 0;

      /**
      * @brief The validity of an iterator
      *
      * If IsValid returns true, it is safe to dereference the iterator.
      *
      * An iterator is considered valid if dereferencing it through GetVal
      *  will return a valid value.  For this to happen, the iterator must
      *  be currently pointing at a valid value in a collection.
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return true if the iterator is pointing at a valid element in the
      *                 underlying collection.  false otherwise.
      */
      virtual bool           IsValid(Error* err = 0) const = 0;

      /**
      *  @brief Gets the number of elements in the collection the iterator points at
      *
      *  This is not a classic iterator method, but it is useful.  Use this method
      *   when you want to preallocate space to hold elements pointed at by an iterator.
      *  
      *  If an iterator points to an empty collection, this method returns 0.
      *
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      *  @return The number of elements stored in the collection underlying this iterator
      */ 
      virtual unsigned long  GetNumElements(Error* err = 0) const = 0;
      
      /**
      * @brief Gets the value the iterator is currently pointing at
      *
      * If the iterator is currently not valid (i.e. IsValid() == false), the
      *  behavior of GetVal is undefined.  If tReturnType is a pointer,
      *  GetVal should return 0 if the iterator is not valid.
      *
      * Possible Errors:
      *   @li gSKIERR_ITERATOR_INVALID
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return The value currently pointed at by the iterator
      */
      virtual tReturnType    GetVal(Error* err = 0) = 0;

   };

   /* Forward declarations for particular instantiations of the iterator */
   class IProductionMatch;
   class IConditionSet;
   class IInstanceInfo;
   class IProduction;
   class ICondition;
   class IMatchSet;
   class IRhsAction;
   class IMatch;
   class IProductionMatch;
   class IWme;
   class IWMObject;
   class IInstanceInfo;
   class ISymbol;
   class IAction;
   class IAgent;
   class IAgentThreadGroup;
   class IActionElement;
   class IRhsFunctionAction;
   class ITestSet;
   class ITest;
   class IMultiAttribute;

   /** Typedefs for iterator instantiations */
   typedef IIterator<IRhsFunctionAction*>        tIRhsFunctionActionIterator;
   typedef IIterator<IAgentThreadGroup*>         tIAgentThreadGroupIterator;
   typedef IIterator<IProductionMatch *>         tIProductionMatchIterator;
   typedef IIterator<IActionElement*>            tIActionElementIterator;
   typedef IIterator<egSKIPreferenceType>        tPreferenceTypeIterator;
   typedef IIterator<IInstanceInfo*>	         tIInstanceInfoIterator;
   typedef IIterator<IConditionSet *>            tIConditionSetIterator;
   typedef IIterator<IProduction *>              tIProductionIterator;
   typedef IIterator<ICondition *>               tIConditionIterator;
   typedef IIterator<IRhsAction *>               tIRhsActionIterator;
   typedef IIterator<IWMObject *>                tIWMObjectIterator;
   typedef IIterator<IMatchSet *>                tIMatchSetIterator;
   typedef IIterator<ITestSet *>                 tITestSetIterator;
   typedef IIterator<ISymbol*>                   tISymbolIterator;
   typedef IIterator<IMatch *>                   tIMatchIterator;
   typedef IIterator<IAgent*>                    tIAgentIterator;
   typedef IIterator<ITest *>                    tITestIterator;
   typedef IIterator<IWme *>                     tIWmeIterator;
   typedef IIterator<IMultiAttribute *>          tIMultiAttributeIterator;
}

#endif

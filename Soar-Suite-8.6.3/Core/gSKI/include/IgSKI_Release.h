/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_destroy.h 
*********************************************************************
* created:	   6/13/2002   15:00
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_RELEASE_H
#define IGSKI_RELEASE_H

namespace gSKI {

   struct Error;

   /**
   *  Interface for releasing client-owned objects.
   *
   *  This interface is used by all client-owned objects
   *   to allow them to destroy their instance of that object.
   *
   *  The IsClientOwned method is provided for cases where you
   *   are not sure whether you should release an object or cases
   *   where you are using the same code to process client-owned
   *   and system owned objects.
   *
   */
   class IRelease {
   public:

      /**
       *  @brief Virtual destructor
       *
       *  An object may be destroyed using only a pointer to the IDestroy base class,
       *   therefore the virtual destructor is important.
       */
      virtual ~IRelease() {}

      /**
      * @brief Destroys this object
      * 
      * Call this method to remove this object from memory.  It is not safe to
      *  use this object after Release is called.
      *
      * You cannot release an object that is system owned.   If you attempt to
      *  do so, release will have no effect and err will be filled with error
      *  information (if an error structure is passed in).
      *
      * Possible Errors:
      *   @li gSKIERR_NOT_CLIENT_OWNED
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
	  *
	  * @return true if the object was deleted.
      *
      */
      virtual bool Release(Error* err = 0) = 0;
   };

}

#endif

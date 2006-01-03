/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_ReleaseImpl.h 
*********************************************************************
* created:	   4/21/2004
*
* purpose: 
*********************************************************************/
#ifndef GSKI_RELEASEIMPL_H
#define GSKI_RELEASEIMPL_H

#include <assert.h>

#include "gSKI_Error.h"
#include "IgSKI_Release.h"

namespace gSKI
{
#ifdef GSKI_DEBUG_RELEASE_IMPL
   void ReleaseImplDebugAddObject(IRelease* pRelease);
   void ReleaseImplDebugRemoveObject(IRelease* pRelease);
#endif

   template <typename T, bool clientOwned>
   class SimpleReleaseImpl : public T
   {
   public:
      typedef T tReleaseImplParent;

      SimpleReleaseImpl()
      {
#ifdef GSKI_DEBUG_RELEASE_IMPL
         ReleaseImplDebugAddObject(this);
#endif
      }

      SimpleReleaseImpl(SimpleReleaseImpl& rhs)
      {
#ifdef GSKI_DEBUG_RELEASE_IMPL
         ReleaseImplDebugAddObject(this);
#endif
      }

      /** Destructor */
      virtual ~SimpleReleaseImpl() 
      { 
#ifdef GSKI_DEBUG_RELEASE_IMPL
         ReleaseImplDebugRemoveObject(this);
#endif
      }

      /** Implement gSKI::IRelease */
      //{
      virtual bool Release(Error* err = 0)
      {
         ClearError(err);
         delete this;
		 return true ;
      }

      virtual bool IsClientOwned(Error* err = 0) const 
      { 
         ClearError(err);
         return clientOwned; 
      }
      //}

   private:
   }; // class SimpleReleaseImpl

   class IRefCountedReleaseImpl
   {
   public:
      virtual ~IRefCountedReleaseImpl() {}
      virtual unsigned long AddRef() = 0;
      virtual unsigned long GetRefCount() const = 0;
   };

   template <typename T, bool clientOwned>
   class RefCountedReleaseImpl : 
      public T, 
      public IRefCountedReleaseImpl
   {
   public:
      typedef T tReleaseImplParent;

      /** 
         Constructor 

         Initializes count to 1
      */
      RefCountedReleaseImpl() : m_count(1) 
      {
#ifdef GSKI_DEBUG_RELEASE_IMPL
         ReleaseImplDebugAddObject(this);
#endif
      }

      /**
         Copy constructor

         Initializes new ref count to 1
      */
      RefCountedReleaseImpl(const RefCountedReleaseImpl& rhs) : T(rhs), m_count(1) 
      {
#ifdef GSKI_DEBUG_RELEASE_IMPL
         ReleaseImplDebugAddObject(this);
#endif
      }

      /** Destructor */
      virtual ~RefCountedReleaseImpl() 
      {
         assert(m_count == 0);
#ifdef GSKI_DEBUG_RELEASE_IMPL
         ReleaseImplDebugRemoveObject(this);
#endif
      }

      RefCountedReleaseImpl& operator=(const RefCountedReleaseImpl& rhs)
      {
         if(this != &rhs)
         {
            m_count = 1;
         }
         return *this;
      }

      /**
         Add a reference to the object

         @returns The new reference count
      */
      virtual unsigned long AddRef()
      {
         ++m_count;
         return m_count;
      }

      /** Returns the current reference count */
      virtual unsigned long GetRefCount() const { return m_count; }

      /** Implement gSKI::IRelease */
      //{
      virtual bool Release(Error* err = 0)
      {
         assert(m_count > 0);
         ClearError(err);
         if(--m_count == 0)
         {
            delete this;
			return true ;
         }
		 return false ;
      }

      virtual bool IsClientOwned(Error* err = 0) const 
      { 
         ClearError(err);
         return clientOwned; 
      }
      //}

   private:
      unsigned long m_count;  /// The reference count
   }; // class RefCountedReleaseImpl
} // namespace gSKI

#endif // GSKI_RELEASEIMPL_H


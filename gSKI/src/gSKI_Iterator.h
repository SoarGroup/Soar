/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_iterator.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef GSKI_ITERATOR_H
#define GSKI_ITERATOR_H

#include "IgSKI_Iterator.h"

#include "gSKI_Error.h"
#include "gSKI_ReleaseImpl.h"
#include "MegaAssert.h"

namespace gSKI
{
   /**
    * @brief This is the iterator type.  It is designed to allow users to access
    *        std STL containers safely.  It is currently limited to only iterating
    *        in the forward direction, and the user cannot return to the beginning.
    */
   template<typename T, typename Tcon>
   class Iterator : public SimpleReleaseImpl<IIterator<T>, true>
   {
   public:
      
   protected:

      typename Tcon::iterator    m_it; /**< Container holding the values */
      Tcon                      m_con;/**< Iterator in to above container. */

      /**
       * @brief Copy Constructor
       */
      Iterator(const Iterator &it) : m_it(it.m_it), m_con(it.m_con)
      {
      }

      /**
       * @brief Helper function to insure that all of the elements of this iterator are valid.
       */
      void isIteratorGood() const
      {
         MegaAssert(m_it <= m_con.end() ,"Bad iterator: outside valid range!");
         MegaAssert(m_it >= m_con.begin(),"Bad iterator: outside valid range!");
      }
   
   public: ///////////////////////////////////////////////////////////////////////////////
      

      /**
       * @brief Virtual destructor
       */
      virtual ~Iterator() {  } 
       
      /**
       * @brief Constructor
       *
       * This constructor creates an iterator from a container.
       *
       * @param con The container we are constructing the iterator from.
       */
      Iterator(const Tcon &con)
      {
         std::copy(con.begin(), con.end(), std::back_inserter(m_con));
         m_it = m_con.begin();
      }

      /**
       * @brief Return the value held by this iterator.
       */
      virtual T GetVal(Error *err)
      {
         ClearError(err);
         isIteratorGood();

         MegaAssert( m_it != m_con.end(), 
                     "Trying to GetVal when iterator is at end!");

         // Note that returning 0 only works for pointer types
         if ( m_it != m_con.end() ) 
         {
            return *m_it; 
         } 
         else 
         {
            return 0;
         }
      }

      /**
       * @brief Checks to see if the iterator is valid.  
       *
       * This will most commonly be used to test for the
       * end condition of an iterator.
       */
      virtual bool IsValid(Error *err) const
      { 
         ClearError(err);
         isIteratorGood();
         return (m_it != m_con.end());
      }

      /**
       * @brief Retrieve the next iterator in the container.
       */
      virtual void Next(Error *err)
      {
         ClearError(err);
         isIteratorGood();

         // Checking for next calls when iterator is already at its end
         MegaAssert( m_it != m_con.end(), 
                     "Trying to advance iterator past end!");

         // Only advancing if not at the end
         if ( m_it != m_con.end() ) 
         {
            ++m_it;
         }
      }

      /**
       * @brief Fetches the number of elements in the container.
       * 
       * This value is not dependent on where the iterator is within
       * the list.
       */
      virtual unsigned long GetNumElements(Error *err) const
      {
         ClearError(err);
         isIteratorGood();
         return static_cast<unsigned long>(m_con.size());
      }

      /**
       * @brief Returns the number of elements remaining
       *        in the container.  This is inclusive of this
       *        iterator.
       */
      virtual unsigned int GetRemainingElements(Error *err)
      {
         isIteratorGood();
         return (unsigned int)(m_con.end() - m_it);
      }
   };

   template<typename T, typename Tcon>
   class IteratorWithRelease : public Iterator<T, Tcon>
   {
      typedef Iterator<T, Tcon> tBase;
   public:
      virtual ~IteratorWithRelease() 
      {
         // If the current value has been accessed, then it's the client's
         // responsibility to release it. Skip it.
         if(m_gotValue && tBase::m_it != tBase::m_con.end())
         {
            ++tBase::m_it;
         }
         // Release the remaining, unaccessed elements.
         for(typename Tcon::iterator it = tBase::m_it; it != tBase::m_con.end(); ++it)
         {
            (*it)->Release();
         }
      } 
       
      IteratorWithRelease(const Tcon &con) : tBase(con), m_gotValue(false) 
      {
      }
      virtual T GetVal(Error *err)
      {
         m_gotValue = true;
         return tBase::GetVal(err);
      }
      virtual void Next(Error *err)
      {
         // If they didn't call GetVal() on this element, then release it before
         // moving on to the next one.
         if(!m_gotValue && tBase::m_it != tBase::m_con.end())
         {
            (*tBase::m_it)->Release(err);
         }
         m_gotValue = false;
         tBase::Next(err);
      }
   private:
      bool m_gotValue;
   };
}
#endif

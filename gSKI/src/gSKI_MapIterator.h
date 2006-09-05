/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_mapiterator.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef GSKI_MAPITERATOR_H
#define GSKI_MAPITERATOR_H

#include "IgSKI_Iterator.h"

#include "gSKI_Iterator.h"
#include "gSKI_ReleaseImpl.h"
#include "gSKI_Error.h"
#include "MegaAssert.h"

namespace gSKI
{
    /**
    * @brief This a special version of the iterator type for STL maps
    *        It is a copy and paste version of Iterator with a single
    *        change to the GetVal method.
    */
   template<typename T, typename Tcon>
   class MapIterator : public SimpleReleaseImpl< IIterator<T> >
   {
   public:
      /**
       * @brief This is the type of the container. 
       */
      typedef T tValueType;
      typedef typename Tcon::iterator tContainerIteratorType;
      
   private:

      tContainerIteratorType    m_it;  /**< Container holding the values */
      Tcon                      m_con; /**< Iterator in to above container. */

      /**
       * @brief Copy Constructor
       */
      MapIterator(const MapIterator& it): m_con(it.m_con)
      {
         m_it  = it.m_it;
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
      virtual ~MapIterator() {  } 
       
      /**
       * @brief Constructor
       *
       * This constructor creates an iterator from a container.
       *
       * @param con The container we are constructing the iterator from.
       */
      MapIterator(const Tcon &con)
      {
         std::copy(con.begin(), con.end(), std::back_inserter(m_con));
         m_it = m_con.begin();
      }

      /**
       * @brief Return the value held by this iterator.
       */
      tValueType GetVal(Error *err)
      {
         ClearError(err);
         isIteratorGood();

         MegaAssert( m_it != m_con.end(), 
                     "Trying to GetVal when iterator is at end!");

         // Note that returning 0 only works for pointer types
         if ( m_it != m_con.end() ) 
         {
            return ((*m_it).second); 
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
      bool IsValid(Error *err) const
      { 
         ClearError(err);
         isIteratorGood();
         return (m_it != m_con.end());
      }

      /**
       * @brief Retrieve the next iterator in the container.
       */
      void Next(Error *err)
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
      unsigned long GetNumElements(Error *err) const
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
      unsigned int GetRemainingElements(Error *err)
      {
         isIteratorGood();
         return( m_con->end() - m_it);
      }
   };
}
#endif

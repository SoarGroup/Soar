/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file iterutils.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef ITERUTILS_H
#define ITERUTILS_H

#include "gSKI_Error.h"

/**
   A "traits" template that defines the typenames of a container type's
   iterators, etc with shorter names.
*/
template <typename Container> struct FwdContainerType
{
   typedef Container t;                      /// The container type
   typedef typename t::value_type V;         /// The value_type
   typedef typename t::const_iterator CIt;   /// The const_iterator type
   typedef typename t::iterator It;          /// The iterator type
};
   
template <typename Container> struct BiContainerType : 
public FwdContainerType < Container >
{
   typedef typename Container::const_reverse_iterator CRIt;   /// The const_reverse_iterator type
   typedef typename Container::reverse_iterator RIt;          /// The iterator type
};

namespace gSKI {
   struct Error;
}

/*
  ==================================
   
  ==================================
*/
template<typename T>
inline bool IsInvalidPtr(T p, gSKI::Error *err)
{
   if(p == 0) {
      gSKI::SetError(err, gSKI::gSKIERR_INVALID_PTR);
      return true; 
   }
   return false;
}

#endif

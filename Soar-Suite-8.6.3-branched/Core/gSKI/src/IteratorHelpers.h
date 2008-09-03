/********************************************************************
* @file iteratorhelpers.h 
****************************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
**************************************************************************** 
* created:	   5/30/2002   15:45
*
* purpose: Provides helper functions for using iterator_adaptors.
*********************************************************************/
#ifndef ITERATOR_HELPERS_H
#define ITERATOR_HELPERS_H

#include <boost/iterator_adaptors.hpp> // needed for default_iterator stuff.
#include <functional>
#include <string>

namespace gSKI {

/**
 * @class transform_iterator_generator_const
 *
 * This is a templatized typedef class that is used to define the
 * iterator_adaptor type we are using to wrap an adapter holding
 * a value to an adaptor holding a const value.
 *
 * @param AdaptableUnaryFunction The unary function the instantiation that
 *                               will be used in the instantiation of the 
 *                               policy class.
 * @param Iterator The container iterator we are adapting.
 */
template <class AdaptableUnaryFunction, class Iterator>
class TransformIteratorGeneratorConst
{
    typedef const typename AdaptableUnaryFunction::result_type  value_type;
public:
   typedef boost::iterator_adaptor<Iterator,
                                   boost::transform_iterator_policies<AdaptableUnaryFunction>,
                                   value_type, 
                                   const value_type, 
                                   const value_type*, 
                                   std::input_iterator_tag>  type;
};

/**
 * @class transform_iterator_generator_ref
 *
 * This is a templatized typedef class that is used to define the
 * iterator_adaptor type we are using to wrap an adapter and have
 * the dereference (operator*() ).
 *
 * @param AdaptableUnaryFunction The unary function the instantiation that
 *                               will be used in the instantiation of the 
 *                               policy class.
 * @param Iterator The container iterator we are adapting.
 */
template <class AdaptableUnaryFunction, class Iterator>
class TransformIteratorGeneratorRef
{
    typedef typename AdaptableUnaryFunction::result_type value_type;
public:
   typedef boost::iterator_adaptor<Iterator,
           boost::transform_iterator_policies<AdaptableUnaryFunction>,
           value_type, value_type, value_type, std::input_iterator_tag>
      type;
};

/**
 * @class MakeConst
 *
 * This is a functor that is used to process the dereferenced value
 * from the iterator.  This one returns a const'ed version of the
 * contents of the competitor.
 *
 * @param Iterator This is the iterator that we will be adapting.
 * @param RetType  This will be the const version of what is in
 *                 the container.  This can't be automated by the
 *                 template because if you use const typename T,
 *                 it converts it to T const, or when T is a 
 *                 pointer, it becomes "type * const", not
 *                 "type const *".  That is why this parameter is
 *                 needed.
 */
template<typename Iterator, typename RetType>
class MakeConst : public std::unary_function<Iterator, RetType>
{
public:
   RetType operator()(Iterator::value_type &_Left) const
   {
      return (_Left);
   }
};

/**
 * @class getPCC
 *
 * This is a functor that is used to process the dereferenced value
 * from the iterator.  This one returns the const char * version of
 * a std::string.
 */
class getPCC : public std::unary_function<const std::string &, const char *>
{
public:
   const char *operator()(const std::string &_Left) const
   {
      return (_Left.c_str());
   }
};


/**
 * @class Get2nd
 *
 */
template<class Iterator>
class Get2nd : public std::unary_function<Iterator, Iterator::second_type &>
{
public:
   Iterator::second_type &operator()(Iterator &_Left) const
   {
      return (_Left.second);
   }
};

}

#endif

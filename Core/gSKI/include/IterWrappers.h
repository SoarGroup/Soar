#ifndef ITER_WRAPPERS_H
#define ITER_WRAPPERS_H

#ifndef _FUNCTIONAL_
#include <functional> // needed for unary_function
#endif

#ifndef _STRING_
#include <string>
#endif

#ifndef BOOST_ITERATOR_ADAPTOR_DWA053000_HPP_
#include <boost/iterator_adaptors.hpp> // needed for default_iterator stuff.
#endif

#ifdef _DEBUG
#define TYPE(str,var)    std::string str(typeid(var).name())
#else
#define TYPE(str,val) 
#endif

/*
* transform_iterator_policies takes a unary function as an argument.	
*
* Hold and store a functor and apply it upon any dereference, returning
* the result.
*
*  
*/
template <class AdaptableUnaryFunction>
struct transform_iterator_policies : public boost::default_iterator_policies
{
   /**
   * Default Constructor
   */ 
   transform_iterator_policies() { }

   /*
   * Store the functor locally.
   */
   transform_iterator_policies(const AdaptableUnaryFunction& f)
      : m_f(f) { }

      /**
      * When dereferencing, apply the filter first.
      */
      template <class IteratorAdaptor>
       IteratorAdaptor::reference dereference(const IteratorAdaptor& iter) const
      { 
         TYPE(s_5, IteratorAdaptor::reference);
         return m_f(*iter.base()); 
      }

      AdaptableUnaryFunction m_f; /**< Functor to apply at dereference */
};

/**
 */
template <class AdaptableUnaryFunction>
struct transform_iterator_policies_ref : public boost::default_iterator_policies
{
   /**
   * Default Constructor
   */ 
   transform_iterator_policies_ref() { }

   /*
   * Store the functor locally.
   */
   transform_iterator_policies_ref(const AdaptableUnaryFunction& f)
      : m_f(f) { }

      /**
      * When dereferencing, apply the filter first.
      */
      template <class IteratorAdaptor>
       const IteratorAdaptor::reference &dereference(const IteratorAdaptor& iter) const
      { 
         TYPE(s_5, IteratorAdaptor::reference);
         return m_f(iter.base()); 
      }

      AdaptableUnaryFunction m_f;   /**< Functor to apply at dereference */
};



/*****************************************************************
 * These three structs are used to wrap a specialized iterator
 * adaptor that is used to convert a std::iterator to a std::iterator
 * of const values.  This allows passing iterators without so much
 * concern over the value held by the iterator (particularly if it
 * is a pointer) being changed.  If you have an interator whose
 * value_type is Stuff*, the adapted iterator will return 
 * const Stuff *, which is safer.
 */

/*
=====================
This is the functor that actually does the modification
to const on the iterator member.
=====================
*/
template<class _Ty>
struct constize : public std::unary_function<_Ty, const _Ty>
{
   typedef const _Ty retType;
   template<typename T>
   retType operator()(T _Left) const
   {
      TYPE(s_1, retType);
      TYPE(s_2, const _Ty);
      TYPE(s_3,operator());
      TYPE(s_4, _Left);
      TYPE(s_5,  int* const);
      return (_Left);
   }
};

/*
=====================
This class is derived from the boost::iterator_adaptor as an additional
layer of abstraction.  The reason for this is to allow it's constructor
to offer the value of the functor to the iterator_adaptor automatically.
This functor will not change, so the end user should not have to type it
in every time.  This class feels like an un-needed layer of abstraction.
It seems like ConstIterWrapper and ConstIter should be merged, but I could
not find a way that I could do that easily.
=====================
*/
template<typename BaseType, typename Policies, typename Value, typename Reference>
struct ConstIterWrapper : public boost::iterator_adaptor<BaseType, Policies, Value, Reference>
{
   ConstIterWrapper(BaseType v) : boost::iterator_adaptor<BaseType, Policies, Value, Reference>
                  (v, transform_iterator_policies<constize<Value> >() ){}
};


/**
 * This is, more or less, a templated typedef.  If templated typedefs were
 * supported, this would not be needed. 8|.  This defines an iterator_adaptor
 * on any std::iterator<type> that converts it to a std::iterator<const type>.
 * This is only used for pointer members.
 *
 * This is used to create iterator adaptors that are easy to use.
 *
 * @example
 *    typedef ConstIter< std::set<sss *, const sss *>::iterator >::type ConstIterPsss;
 *
 *    ConstPsssSet beginIter(sssSetValue.begin());
 */
template<typename iterator, typename returnType>
struct ConstIterPtr {
   typedef    ConstIterWrapper<iterator, 
                               transform_iterator_policies<constize<iterator::value_type> >,
                               iterator::value_type,
                               returnType>  type;
};

/**
 * This is, more or less, a templated typedef.  If templated typedefs were
 * supported, this would not be needed. 8|.  This defines an iterator_adaptor
 * on any std::iterator<type> that converts it to a std::iterator<const type>.
 *
 * This is used to create iterator adaptors that are easy to use.
 *
 * @example
 *    typedef ConstIter< std::set<sss *>::iterator >::type ConstIterPsss;
 *
 *    ConstPsssSet beginIter(sssSetValue.begin());
 */
template<typename iterator>
struct ConstIter {
   typedef    ConstIterWrapper<iterator, 
                               transform_iterator_policies<constize<iterator::value_type> >,
                               iterator::value_type,
                               const iterator::value_type>  type;
};


/**
 *===============================================================================
 */
struct toConstCharStar : public std::unary_function<std::string, const char *>
{
   const char *operator()(std::string &_Left) const
   {
      return (_Left.c_str());
   }
};

/**
 *
 */
template<typename BaseType, typename Policies, typename Value, typename Reference>
struct CCharPtrWrapper : public boost::iterator_adaptor<BaseType, Policies, Value, Reference>
{
   CCharPtrWrapper(BaseType v) : boost::iterator_adaptor<BaseType, Policies, Value, Reference>
                  (v, transform_iterator_policies<toConstCharStar >() ){}
};

/**
 *
 */
template<typename iterator>
struct CCharPtrIter {
   typedef    CCharPtrWrapper<iterator, 
                              transform_iterator_policies<toConstCharStar>,
                              char *,
                              const char *>  type;
};



/*****************************************************************
 * These three structs are used to wrap a specialized iterator
 * adaptor that is used to convert a std::pair<x,y> to a std::iterator
 * of y values.  This allows passing maps without so much
 * concern over acessing the second value in the pair.  If you have 
 * an interator whose value_type is pair<x,y *>, the adapted iterator 
 * will return y *, which is easier to work with.
 */

/**
 * This is the functor that actually does the modification
 * to retrieve the second part of the pair..
 */
template<class _Ty>
struct getPairSecond : public std::unary_function<_Ty, _Ty>
{
   template<class Iterator>
      _Ty operator()(Iterator &_Left) const
   {
      return (_Left.second);
   }
};


/**
 * This class is derived from the boost::iterator_adaptor as an additional
 * layer of abstraction.  The reason for this is to allow it's constructor
 * to offer the value of the functor to the iterator_adaptor automatically.
 * This functor will not change, so the end user should not have to type it
 * in every time.  This class feels like an un-needed layer of abstraction.
 * It seems like ConstIterWrapper and ConstIter should be merged, but I could
 * not find a way that I could do that easily.
 */
template<typename BaseType, typename Policies, typename Value, typename Reference>
class PairIterWrapperPtr : public   boost::iterator_adaptor<BaseType, Policies, Value, const Reference, const Value &>
{
public:
   PairIterWrapperPtr(BaseType v) : boost::iterator_adaptor<BaseType, Policies, Value, const Reference, const Value &>
                                    (v, transform_iterator_policies<getPairSecond<Value> >() )
   {    }
};


/*
 * This is, more or less, a templated typedef.  If templated typedefs were
 * supported, this would not be needed. 8|.  This defines an iterator_adaptor
 * on any std::iterator<type> that converts it to a std::iterator<const type>.
 *
 * This is used to create iterator adaptors that are easy to use.
 *
 * @example
 *    typedef Pair2ndIter< std::map<int, int>::iterator >::type SecondIterIntInt;
 *
 *    SecondIterIntInt beginIter(IntIntMapValue.begin());
 */
template<typename iterator>
class Pair2ndIterPtr {
private:
   typedef iterator::value_type::second_type Value;
public:
   typedef PairIterWrapperPtr<iterator,  
                              transform_iterator_policies<getPairSecond<Value> >,
                              Value, 
                              const Value> type;

};





template<class _Ty>
struct getPairSecondRef : public std::unary_function<_Ty, _Ty>
{
   template<class Iterator>
      _Ty operator()(Iterator &_Left) const
   {
      return ((*_Left).second);
   }
};

/**
 *
 */
template<typename BaseType, typename Policies, typename Value, typename Reference>
class PairIterWrapper : public   boost::iterator_adaptor<BaseType, Policies, Value, Reference>
{
public:
   PairIterWrapper(BaseType v) : boost::iterator_adaptor<BaseType, Policies, Value, Reference>
                                     (v, Policies() )
   {    }
};

/**
 *
 */
template<typename iterator>
class Pair2ndIter {
private:
   typedef iterator::value_type::second_type Value;
public:
   typedef PairIterWrapper<iterator,  
                           transform_iterator_policies_ref<getPairSecondRef<Value> >,
                           Value, 
                           const Value &> type;

};

#endif
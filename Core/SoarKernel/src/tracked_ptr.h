/** tracked_ptr.h -- Mitchell Keith Bloch -- bazald@gmail.com -- 20130712
 *
 *  tracked_ptr provides a pointer type along the lines of shared_ptr,
 *  weak_ptr, and unique_ptr. The key objective of tracked_ptr is to
 *  allow multiple pointers to the same address, but to provide error
 *  checking at deletion time.
 *
 *  The following errors are checked:
 *  1. Other tracked_ptr instances point to the address at the time of
 *     deletion.
 *  2. No other tracked_ptr instances point to the address at the time
 *     it is overwritten or zeroed, but not deleted.
 *  3. Any tracked_ptr instances exist at exit.
 **/

#ifndef TRACKED_PTR_H
#define TRACKED_PTR_H

#include <cassert>
#include <memory>

template <typename T>
class tracked_ptr_deleter {
public:
  void operator()(T * const t) {
    delete t;
  }
};
template <typename T>
class tracked_ptr_array_deleter {
public:
  void operator()(T * const t) {
    delete [] t;
  }
};
template <typename T>
class tracked_ptr_null_deleter {
public:
  void operator()(T * const) {
  }
};

template <typename T, typename Deleter = tracked_ptr_deleter<T> >
class tracked_ptr {
public:
  typedef T element_type;
  typedef Deleter deleter_type;
  typedef element_type * pointer;
  
  tracked_ptr(const pointer ptr_ = 0);
  
  ~tracked_ptr() {
#ifndef NDEBUG
    zero();
#endif
  }

  tracked_ptr(const tracked_ptr<T, Deleter> &rhs);
  tracked_ptr<T, Deleter> & operator=(const tracked_ptr<T, Deleter> &rhs);

  /** Delete the pointer and zero out the pointer. **/
  void delete_and_zero();
  /** Zero out the pointer without deleting it. **/
  void zero();

  void swap(tracked_ptr<T, Deleter> &rhs) {
    tracked_ptr<T, Deleter> temp(rhs);
    rhs = *this;
    *this = temp;
  }

  pointer get() const {return ptr;}
  deleter_type get_deleter() const {return deleter_type();}
  operator bool() const {return ptr;}
  
  element_type & operator*() const {return *ptr;}
  element_type & operator->() const {return *ptr;}

  template <typename INT>
  element_type & operator[](const INT &index) const {
    return ptr[index];
  }
  
  bool operator==(const tracked_ptr<T, Deleter> &rhs) const {return ptr == rhs.ptr;}
  bool operator!=(const tracked_ptr<T, Deleter> &rhs) const {return ptr != rhs.ptr;}
  bool operator<(const tracked_ptr<T, Deleter> &rhs) const {return ptr < rhs.ptr;}
  bool operator<=(const tracked_ptr<T, Deleter> &rhs) const {return ptr <= rhs.ptr;}
  bool operator>(const tracked_ptr<T, Deleter> &rhs) const {return ptr > rhs.ptr;}
  bool operator>=(const tracked_ptr<T, Deleter> &rhs) const {return ptr >= rhs.ptr;}
  bool operator==(const T * const rhs) const {return ptr == rhs;}
  bool operator!=(const T * const rhs) const {return ptr != rhs;}
  bool operator<(const T * const rhs) const {return ptr < rhs;}
  bool operator<=(const T * const rhs) const {return ptr <= rhs;}
  bool operator>(const T * const rhs) const {return ptr > rhs;}
  bool operator>=(const T * const rhs) const {return ptr >= rhs;}

private:
  pointer ptr;
};

template <typename T, typename Deleter>
inline bool operator==(const T * const lhs, const tracked_ptr<T, Deleter> &rhs) {return lhs == rhs.get();}
template <typename T, typename Deleter>
inline bool operator!=(const T * const lhs, const tracked_ptr<T, Deleter> &rhs) {return lhs != rhs.get();}
template <typename T, typename Deleter>
inline bool operator<(const T * const lhs, const tracked_ptr<T, Deleter> &rhs) {return lhs < rhs.get();}
template <typename T, typename Deleter>
inline bool operator<=(const T * const lhs, const tracked_ptr<T, Deleter> &rhs) {return lhs <= rhs.get();}
template <typename T, typename Deleter>
inline bool operator>(const T * const lhs, const tracked_ptr<T, Deleter> &rhs) {return lhs > rhs.get();}
template <typename T, typename Deleter>
inline bool operator>=(const T * const lhs, const tracked_ptr<T, Deleter> &rhs) {return lhs >= rhs.get();}

class pointer_tracker {
  template <typename T, typename Deleter>
  friend class tracked_ptr;

#ifdef NDEBUG
  inline static void set_pointer(const void * to, const void * from) {}
  inline static void clear_pointer(const void * to, const void * from) {}
  inline static unsigned int count(const void * to) {return 0;}
#else
  static void set_pointer(const void * to, const void * from);
  static void clear_pointer(const void * to, const void * from);
  static unsigned int count(const void * to);
#endif
};

template <typename T, typename Deleter>
tracked_ptr<T, Deleter>::tracked_ptr(const pointer ptr_)
  : ptr(ptr_)
{
  pointer_tracker::set_pointer(ptr, this);
}

template <typename T, typename Deleter>
tracked_ptr<T, Deleter>::tracked_ptr(const tracked_ptr<T, Deleter> &rhs)
  : ptr(rhs.ptr)
{
  pointer_tracker::set_pointer(ptr, this);
}

template <typename T, typename Deleter>
tracked_ptr<T, Deleter> & tracked_ptr<T, Deleter>::operator=(const tracked_ptr<T, Deleter> &rhs) {
#ifndef NDEBUG
  if(ptr != rhs.ptr)
#endif
  {
    pointer_tracker::clear_pointer(ptr, this);
    assert(pointer_tracker::count(ptr) != 0);
    ptr = rhs.ptr;
    pointer_tracker::set_pointer(ptr, this);
  }
}

template <typename T, typename Deleter>
void tracked_ptr<T, Deleter>::delete_and_zero() {
#ifndef NDEBUG
  if(ptr)
#endif
  {
    assert(pointer_tracker::count(ptr) == 1);
    deleter_type()(ptr);
    pointer_tracker::clear_pointer(ptr, this);
    ptr = 0;
  }
}

template <typename T, typename Deleter>
void tracked_ptr<T, Deleter>::zero() {
#ifndef NDEBUG
  if(ptr)
#endif
  {
    pointer_tracker::clear_pointer(ptr, this);
    assert(pointer_tracker::count(ptr) != 0);
    ptr = 0;
  }
}

#endif

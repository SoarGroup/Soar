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

//#include "../linkage.h"
#define UTILITY_LINKAGE

template <typename T>
struct null_delete {
  void operator()(const T * const &) {}
};

enum tracked_ptr_mode {
  TPM_DEFAULT = 0x0,
  TPM_UNDELETEABLE = 0x1,
  TPM_UNTRACKED = 0x2,
  TPM_UNDEL_UNTRA = 0x3
};

template <typename T, typename Deleter = std::default_delete<T>, tracked_ptr_mode TPM = TPM_DEFAULT>
class tracked_ptr {
public:
  typedef T element_type;
  typedef Deleter deleter_type;
  typedef element_type * pointer;
  enum {deleteable = !(TPM & TPM_UNDELETEABLE)};
  enum {tracked = !(TPM & TPM_UNTRACKED)};

  tracked_ptr(const pointer ptr_ = nullptr);

  ~tracked_ptr() {
#ifndef NDEBUG
    zero();
#endif
  }

  tracked_ptr(const tracked_ptr<T, Deleter, TPM> &rhs);
  tracked_ptr<T, Deleter, TPM> & operator=(const tracked_ptr<T, Deleter, TPM> &rhs);
//  explicit tracked_ptr(SymbolType) { };

  /** Delete the pointer and zero out the pointer. **/
  void delete_and_zero();
  /** Zero out the pointer without deleting it. **/
  void zero();

  void swap(tracked_ptr<T, Deleter, TPM> &rhs) {
    tracked_ptr<T, Deleter, TPM> temp(rhs);
    rhs = *this;
    *this = temp;
  }

  pointer get() const {return ptr;}
  deleter_type get_deleter() const {return deleter_type();}
  operator bool() const {return ptr != nullptr;}

  element_type & operator*() const {return *ptr;}
  element_type * operator->() const {return ptr;}

  template <typename INT>
  element_type & operator[](const INT &index) const {
    return ptr[index];
  }

  bool operator==(const tracked_ptr<T, Deleter, TPM> &rhs) const {return ptr == rhs.ptr;}
  bool operator!=(const tracked_ptr<T, Deleter, TPM> &rhs) const {return ptr != rhs.ptr;}
  bool operator<(const tracked_ptr<T, Deleter, TPM> &rhs) const {return ptr < rhs.ptr;}
  bool operator<=(const tracked_ptr<T, Deleter, TPM> &rhs) const {return ptr <= rhs.ptr;}
  bool operator>(const tracked_ptr<T, Deleter, TPM> &rhs) const {return ptr > rhs.ptr;}
  bool operator>=(const tracked_ptr<T, Deleter, TPM> &rhs) const {return ptr >= rhs.ptr;}
  bool operator==(const T * const rhs) const {return ptr == rhs;}
  bool operator!=(const T * const rhs) const {return ptr != rhs;}
  bool operator<(const T * const rhs) const {return ptr < rhs;}
  bool operator<=(const T * const rhs) const {return ptr <= rhs;}
  bool operator>(const T * const rhs) const {return ptr > rhs;}
  bool operator>=(const T * const rhs) const {return ptr >= rhs;}


private:
  pointer ptr;
};

template <typename T, typename Deleter, tracked_ptr_mode TPM>
inline bool operator==(const T * const lhs, const tracked_ptr<T, Deleter, TPM> &rhs) {return lhs == rhs.get();}
template <typename T, typename Deleter, tracked_ptr_mode TPM>
inline bool operator!=(const T * const lhs, const tracked_ptr<T, Deleter, TPM> &rhs) {return lhs != rhs.get();}
template <typename T, typename Deleter, tracked_ptr_mode TPM>
inline bool operator<(const T * const lhs, const tracked_ptr<T, Deleter, TPM> &rhs) {return lhs < rhs.get();}
template <typename T, typename Deleter, tracked_ptr_mode TPM>
inline bool operator<=(const T * const lhs, const tracked_ptr<T, Deleter, TPM> &rhs) {return lhs <= rhs.get();}
template <typename T, typename Deleter, tracked_ptr_mode TPM>
inline bool operator>(const T * const lhs, const tracked_ptr<T, Deleter, TPM> &rhs) {return lhs > rhs.get();}
template <typename T, typename Deleter, tracked_ptr_mode TPM>
inline bool operator>=(const T * const lhs, const tracked_ptr<T, Deleter, TPM> &rhs) {return lhs >= rhs.get();}

namespace std {
  template <typename T, typename Deleter, tracked_ptr_mode TPM>
  struct hash<tracked_ptr<T, Deleter, TPM>> {
    size_t operator()(const tracked_ptr<T, Deleter, TPM> &tptr) const {
      return hash<T *>()(tptr.get());
    }
  };
}

template <bool Tracked>
class UTILITY_LINKAGE pointer_tracker {};

template <>
class UTILITY_LINKAGE pointer_tracker<false> {
  template <typename T, typename Deleter, tracked_ptr_mode TPM>
  friend class tracked_ptr;

  inline static void set_pointer(const void *, const void *) {}
  inline static void clear_pointer(const void *, const void *, const bool & = true) {}
public:
  inline static size_t count(const void *) {return 0;}
  inline static void print(const void *) {}

  inline static void break_on(const size_t &) {}
};

template <>
class UTILITY_LINKAGE pointer_tracker<true> {
  template <typename T, typename Deleter, tracked_ptr_mode TPM>
  friend class tracked_ptr;

#ifdef NDEBUG
  inline static void set_pointer(const void *, const void *) {}
  inline static void clear_pointer(const void *, const void *, const bool & = true) {}
public:
  inline static size_t count(const void *) {return 0;}
  inline static void print(const void *) {}
#else
  static void set_pointer(const void * to, const void * from);
  static void clear_pointer(const void * to, const void * from, const bool &deleteable);
public:
  static size_t count(const void * to);
  static void print(const void * to);
#endif

  static void break_on(const size_t &count);
};

template <typename T, typename Deleter, tracked_ptr_mode TPM>
tracked_ptr<T, Deleter, TPM>::tracked_ptr(const pointer ptr_)
  : ptr(ptr_)
{
  pointer_tracker<tracked>::set_pointer(ptr, this);
}

template <typename T, typename Deleter, tracked_ptr_mode TPM>
tracked_ptr<T, Deleter, TPM>::tracked_ptr(const tracked_ptr<T, Deleter, TPM> &rhs)
  : ptr(rhs.ptr)
{
  pointer_tracker<tracked>::set_pointer(ptr, this);
}

template <typename T, typename Deleter, tracked_ptr_mode TPM>
tracked_ptr<T, Deleter, TPM> & tracked_ptr<T, Deleter, TPM>::operator=(const tracked_ptr<T, Deleter, TPM> &rhs) {
#ifndef NDEBUG
  if(ptr != rhs.ptr)
#endif
  {
    pointer_tracker<tracked>::clear_pointer(ptr, this, deleteable);
    assert(!deleteable || !ptr || pointer_tracker<tracked>::count(ptr) != 0);
    ptr = rhs.ptr;
    pointer_tracker<tracked>::set_pointer(ptr, this);
  }

  return *this;
}

template <typename T, typename Deleter, tracked_ptr_mode TPM>
void tracked_ptr<T, Deleter, TPM>::delete_and_zero() {
#ifndef NDEBUG
  assert(deleteable);
  if(ptr)
#endif
  {
#ifndef NDEBUG
    if(pointer_tracker<tracked>::count(ptr) != 1) {
      pointer_tracker<tracked>::clear_pointer(ptr, this, deleteable);
      pointer_tracker<tracked>::print(ptr);
      assert(pointer_tracker<tracked>::count(ptr) == 1);
    }
#endif
    deleter_type()(ptr);
    pointer_tracker<tracked>::clear_pointer(ptr, this, deleteable);
    ptr = nullptr;
  }
}

template <typename T, typename Deleter, tracked_ptr_mode TPM>
void tracked_ptr<T, Deleter, TPM>::zero() {
#ifndef NDEBUG
  if(ptr)
#endif
  {
    pointer_tracker<tracked>::clear_pointer(ptr, this, deleteable);
    assert(!deleteable || pointer_tracker<tracked>::count(ptr) != 0);
    ptr = nullptr;
  }
}

template <typename T, typename Deleter, tracked_ptr_mode TPM>
std::ostream & operator<<(std::ostream &os, const tracked_ptr<T, Deleter> &ptr) {
  return os << ptr.get();
}

#endif

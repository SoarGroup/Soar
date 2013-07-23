/** tracked_ptr.cpp -- Mitchell Keith Bloch -- bazald@gmail.com -- 20130712 **/

#include "tracked_ptr.h"

#include <algorithm>
#include <map>

#ifndef NDEBUG
class pointer_tracker_impl {
  pointer_tracker_impl(const pointer_tracker_impl &);
  pointer_tracker_impl & operator=(const pointer_tracker_impl &);

  friend class pointer_tracker;

  pointer_tracker_impl() {}

  ~pointer_tracker_impl() {
    assert(address_to_pointer.empty());
  }

  class Clear_Check {
  public:
    Clear_Check(const void * from_) : from(from_) {}

    bool operator()(const std::pair<const void *, const void *> &atp) {
      return atp.second == from;
    }

  private:
    const void * from;
  };

  class Count {
  public:
    Count() : count(0u) {}

    void operator()(const std::pair<const void *, const void *> &) {
      ++count;
    }

    unsigned int count;
  };

//public:
  static pointer_tracker_impl & get() {
    static pointer_tracker_impl pt;
    return pt;
  }

  void set_pointer(const void * to, const void * from) {
    if(to)
      address_to_pointer.insert(std::make_pair(to, from));
  }

  void clear_pointer(const void * to, const void * from) {
    if(to) {
      std::pair<std::multimap<const void *, const void *>::iterator, std::multimap<const void *, const void *>::iterator> range = address_to_pointer.equal_range(to);
      std::multimap<const void *, const void *>::iterator atp = std::find_if(range.first, range.second, Clear_Check(from));
      assert(atp != address_to_pointer.end());
      address_to_pointer.erase(atp);
    }
  }

  unsigned int count(const void * to) {
    std::pair<std::multimap<const void *, const void *>::iterator, std::multimap<const void *, const void *>::iterator> range = address_to_pointer.equal_range(to);
    Count count = std::for_each(range.first, range.second, Count());
    return count.count;
  }

//private:
  std::multimap<const void *, const void *> address_to_pointer;
};

void pointer_tracker::set_pointer(const void * to, const void * from) {
  return pointer_tracker_impl::get().set_pointer(to, from);
}

void pointer_tracker::clear_pointer(const void * to, const void * from) {
  return pointer_tracker_impl::get().clear_pointer(to, from);
}

unsigned int pointer_tracker::count(const void * to) {
  return pointer_tracker_impl::get().count(to);
}
#endif

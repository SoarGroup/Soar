/** tracked_ptr.cpp -- Mitchell Keith Bloch -- bazald@gmail.com -- 20130712 **/

#include "tracked_ptr.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <unordered_map>

#include "Export.h"

static size_t g_tracked_ptr_break = 0;
static size_t g_tracked_ptr_count = 0;

class UTILITY_LINKAGE pointer_tracker_impl {
  pointer_tracker_impl(const pointer_tracker_impl &);
  pointer_tracker_impl & operator=(const pointer_tracker_impl &);

  friend class pointer_tracker<true>;

  pointer_tracker_impl() {}

  ~pointer_tracker_impl() {
    assert(address_to_pointer.empty());
  }

//public:
  static pointer_tracker_impl & get() {
    static pointer_tracker_impl pt;
    return pt;
  }

  void set_pointer(const void * to, const void * from) {
    if(to) {
      if(++g_tracked_ptr_count == g_tracked_ptr_break)
        assert(g_tracked_ptr_count != g_tracked_ptr_break);
      address_to_pointer[to][from] = g_tracked_ptr_count;
    }
  }

  void clear_pointer(const void * to, const void * from, const bool &
#ifndef NDEBUG
                                                                     deleteable
#endif
                                                                               ) {
    if(to) {
      if(++g_tracked_ptr_count == g_tracked_ptr_break)
        assert(g_tracked_ptr_count != g_tracked_ptr_break);
      auto found_to = address_to_pointer.find(to);
      if(found_to != address_to_pointer.end()) {
        auto found_from = found_to->second.find(from);
        assert(found_from != found_to->second.end());
        found_to->second.erase(found_from);
        if(found_to->second.empty())
          address_to_pointer.erase(found_to);
      }
      else
        assert(!deleteable || found_to != address_to_pointer.end());
    }
  }

  size_t count(const void * to) {
    return address_to_pointer.count(to);
  }

  void print(const void * to) {
    if(to) {
      if(++g_tracked_ptr_count == g_tracked_ptr_break)
        assert(g_tracked_ptr_count != g_tracked_ptr_break);
      auto found_to = address_to_pointer.find(to);
      assert(found_to != address_to_pointer.end());
      std::for_each(found_to->second.begin(), found_to->second.end(), [to](const std::pair<const void *, size_t> &from) {
        std::cerr << "tracked_ptr: " << to << " from " << from.first << " at " << from.second << std::endl;
      });
    }
  }

//private:
  std::unordered_map<const void *, std::map<const void *, size_t>> address_to_pointer;
};

#ifndef NDEBUG
void pointer_tracker<true>::set_pointer(const void * to, const void * from) {
  return pointer_tracker_impl::get().set_pointer(to, from);
}

void pointer_tracker<true>::clear_pointer(const void * to, const void * from, const bool &deleteable) {
  return pointer_tracker_impl::get().clear_pointer(to, from, deleteable);
}

size_t pointer_tracker<true>::count(const void * to) {
  return pointer_tracker_impl::get().count(to);
}

void pointer_tracker<true>::print(const void * to) {
  return pointer_tracker_impl::get().print(to);
}
#endif

void pointer_tracker<true>::break_on(const size_t &count) {
  g_tracked_ptr_break = count;
}

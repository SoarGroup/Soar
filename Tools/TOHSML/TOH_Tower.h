#ifndef TOH_TOWER_H
#define TOH_TOWER_H

#include "TOH_Disk.h"

#include <string>
#include <vector>

class Soar_Agent;

class TOH_Tower {
  TOH_Tower(const TOH_Tower &);
  TOH_Tower & operator=(const TOH_Tower &);

public:
  inline TOH_Tower(sml::Agent &agent, const char &name_index_);
  inline ~TOH_Tower();

  const char & get_index() const {return m_name;}
  const char * get_name() const  {return &m_name;}
  size_t get_height() const      {return m_impl.size();}
  bool is_empty() const          {return m_impl.empty();}

  inline void push_TOH_Disk(TOH_Disk * const &disk);
  inline TOH_Disk * top_TOH_Disk() const;
  inline void pop_TOH_Disk();

  inline void move_disk_to(TOH_Tower &receiver);

private:
  char m_name;
  const char m_null; //< This *MUST* follow m_name; This allows you to refer to the address of m_name as a c_str;

  std::vector<TOH_Disk *> m_impl;

  sml::Agent * const m_agent_ptr;
  sml::StringElement * m_peg_wme;
};

#endif

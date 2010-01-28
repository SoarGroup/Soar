/* TOH_Tower.h
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * The Tower class for C++ Towers of Hanoi SML.
 */

#ifndef TOH_TOWER_H
#define TOH_TOWER_H

#include "TOH_Disk.h"

#include <vector>

/** This class represents a Tower in Towers of Hanoi.
  *
  * It is responsible for sending and retracting WMEs from an 
  * agent's working memory.
  *
  * Additionally, it is responsible for tellings the Disks
  * when to send and retract WMEs from an agent's working 
  * memory.
  */
class TOH_Tower {
  /// Disabled (No Implementation)
  TOH_Tower(const TOH_Tower &);
  TOH_Tower & operator=(const TOH_Tower &);

public:
  inline TOH_Tower(sml::Agent &agent, const char &name_index_);
  inline ~TOH_Tower();

  const char & get_index() const {return m_name;}
  const char * get_name() const  {return &m_name;}
  size_t get_height() const      {return m_impl.size();}
  bool is_empty() const          {return m_impl.empty();}
  // Returns the sizes of the disks on the Tower in descending order
  inline std::vector<int> get_stack() const;

  inline void push_TOH_Disk(TOH_Disk * const &disk);
  inline TOH_Disk * top_TOH_Disk() const;
  inline void pop_TOH_Disk();

  inline void move_disk_to(TOH_Tower &receiver);

private:
  char m_name;
  /* m_null *MUST* follow m_name.  This allows you to refer to the 
   * address of m_name as a c_str.  Alternatively, you could make 
   * m_name a char[2] with the second char being '\0'.  That would 
   * require more code changes and an overall decrease in clarity.
   */
  const char m_null;

  std::vector<TOH_Disk *> m_impl;

  sml::Agent * const m_agent_ptr;
  sml::StringElement * m_peg_wme;
};

#endif

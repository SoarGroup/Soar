/* TOH_Disk.inl
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * Implementation of TOH_Disk.h
 */

#ifndef TOH_TOWER_INL
#define TOH_TOWER_INL

#include "TOH_Tower.h"

#include "TOH_Game.h"

TOH_Tower::TOH_Tower(sml::Agent &agent, const char &name_)
: m_name(name_),
m_null('\0'),
m_agent_ptr(&agent),
m_peg_wme(0)
{
  m_peg_wme = agent.CreateStringWME(agent.GetInputLink(), "peg", &m_name);
}

TOH_Tower::~TOH_Tower() {
  for(std::vector<TOH_Disk *>::iterator it = m_impl.begin(); it != m_impl.end(); ++it)
    delete *it;
  
  m_agent_ptr->DestroyWME(m_peg_wme);
}

std::vector<int> TOH_Tower::get_stack() const {
  std::vector<int> disk_sizes;
  disk_sizes.reserve(m_impl.size());
  for(std::vector<TOH_Disk *>::const_iterator it = m_impl.begin(); it != m_impl.end(); ++it)
    disk_sizes.push_back((*it)->get_size());
  return disk_sizes;
}

void TOH_Tower::push_TOH_Disk(TOH_Disk * const &disk) {
  disk->send_disk_WME_to(*this);
  m_impl.push_back(disk);
}

TOH_Disk * TOH_Tower::top_TOH_Disk() const {
  assert(!m_impl.empty());

  return *m_impl.rbegin();
}

void TOH_Tower::pop_TOH_Disk() {
  assert(!m_impl.empty());

  (*m_impl.rbegin())->retract_disk_WME();

  m_impl.pop_back();
}

void TOH_Tower::move_disk_to(TOH_Tower &receiver) {
  TOH_Disk * disk = top_TOH_Disk();
  pop_TOH_Disk();
  receiver.push_TOH_Disk(disk);
}

#endif

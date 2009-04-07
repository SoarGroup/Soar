/* TOH_Disk.inl
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * Implementation of TOH_Disk.h
 */

#ifndef TOH_DISK_INL
#define TOH_DISK_INL

#include "TOH_Disk.h"

#include "TOH_Game.h"
#include "TOH_Tower.h"

TOH_Disk::TOH_Disk(sml::Agent &agent, const int &size_)
: m_size(size_),
m_agent_ptr(&agent),
m_holds(0),
m_disk_size_1(0),
m_disk_size_2(0),
m_peg_name(0),
m_supporting_disk(0),
m_null_supporting_disk(0)
{
  m_holds = agent.CreateIdWME(agent.GetInputLink(), "holds");
  // Redundant information sending to follow
  // Both versions seem to be required by the current Soar rules
  m_disk_size_1 = agent.CreateIntWME(agent.GetInputLink(), "disk", m_size);
  m_disk_size_2 = agent.CreateIntWME(m_holds, "disk", m_size);
}

TOH_Disk::~TOH_Disk() {
  m_agent_ptr->DestroyWME(m_null_supporting_disk);
  m_agent_ptr->DestroyWME(m_supporting_disk);
  m_agent_ptr->DestroyWME(m_peg_name);
  m_agent_ptr->DestroyWME(m_disk_size_2);
  m_agent_ptr->DestroyWME(m_disk_size_1);
  m_agent_ptr->DestroyWME(m_holds);
}

void TOH_Disk::send_disk_WME_to(const TOH_Tower &receiver) {
  assert(!m_peg_name);
  assert(!m_supporting_disk);
  assert(!m_null_supporting_disk);

  m_peg_name = m_agent_ptr->CreateStringWME(m_holds, "on", receiver.get_name());

  if(!receiver.is_empty())
    m_supporting_disk = m_agent_ptr->CreateIntWME(m_holds, "above", receiver.top_TOH_Disk()->get_size());
  else
    m_null_supporting_disk = m_agent_ptr->CreateStringWME(m_holds, "above", "none");
}

void TOH_Disk::retract_disk_WME() {
  assert(m_peg_name);
  assert(!m_supporting_disk ^ !m_null_supporting_disk);

  if(m_peg_name) {
    m_agent_ptr->DestroyWME(m_peg_name);
    m_peg_name = 0;
  }
  if(m_supporting_disk) {
    m_agent_ptr->DestroyWME(m_supporting_disk);
    m_supporting_disk = 0;
  }
  if(m_null_supporting_disk) {
    m_agent_ptr->DestroyWME(m_null_supporting_disk);
    m_null_supporting_disk = 0;
  }
}

#endif

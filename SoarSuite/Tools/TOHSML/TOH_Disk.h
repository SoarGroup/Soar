#ifndef TOH_DISK_H
#define TOH_DISK_H

#include "sml_Client.h"
#include <string>

class TOH_Tower;

class TOH_Disk {
  TOH_Disk(const TOH_Disk &);
  TOH_Disk & operator=(const TOH_Disk &);

public:
  inline TOH_Disk(sml::Agent &agent, const int &size_);
  inline ~TOH_Disk();

  const int & get_size() const {return m_size;}

  inline void send_disk_WME_to(const TOH_Tower &receiver);
  inline void retract_disk_WME_from(const TOH_Tower &sender);

private:
  int m_size;

  sml::Agent * const m_agent_ptr;
  sml::Identifier * m_holds;
  sml::IntElement * m_disk_size_1;
  sml::IntElement * m_disk_size_2;
  sml::StringElement * m_peg_name;
  sml::IntElement * m_supporting_disk;
  sml::StringElement * m_null_supporting_disk;
};

#endif

/* TOH_Disk.h
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * The Disk class for C++ Towers of Hanoi SML.
 */

#ifndef TOH_DISK_H
#define TOH_DISK_H

#include "sml_Client.h"

class TOH_Tower;

/** This class represents a Disk in Towers of Hanoi.
  *
  * It is responsible for sending and retracting WMEs from an 
  * agent's working memory, provided it is told to do so by
  * the Towers sending and receiving it.
  */
class TOH_Disk {
  /// Disabled (No Implementation)
  TOH_Disk(const TOH_Disk &);
  TOH_Disk & operator=(const TOH_Disk &);

public:
  /// Create a TOH_Disk of a given size in an Agent's working memory
  inline TOH_Disk(sml::Agent &agent, const int &size_);
  inline ~TOH_Disk();

  const int & get_size() const {return m_size;}

  /// Create the Agent's WMEs for this TOH_Disk
  inline void send_disk_WME_to(const TOH_Tower &receiver);
  /// Retract the Agent's WMEs for this TOH_Disk
  inline void retract_disk_WME();

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

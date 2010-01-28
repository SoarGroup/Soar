/* TOH_Game.h
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * The actual Towers of Hanoi game object that runs the show.
 */

#ifndef TOH_GAME_H
#define TOH_GAME_H

#include "Soar_Kernel.h"
#include "Soar_Agent.h"
#include <vector>
#include <string>

class TOH_Tower;
class Stats_Tracker;

#ifdef WIN32
#define TOH_AGENT_PRODUCTIONS "\\share\\soar\\Demos\\towers-of-hanoi-SML.soar"
#else
#define TOH_AGENT_PRODUCTIONS "/share/soar/Demos/towers-of-hanoi-SML.soar"
#endif

/* The TOH_Game object is a complete instance of Towers of Hanoi.
 * It is responsible for creating the Kernel, Agent, Towers, Disks, ...
 *
 * Tell it to run, and you're off.
 */
class TOH_Game {
  /// Disabled (No Implementation)
  TOH_Game(const TOH_Game &);
  TOH_Game & operator=(const TOH_Game &);

  friend inline void toh_update_event_handler(sml::smlUpdateEventId, void *user_data_ptr, sml::Kernel* kernel_ptr, sml::smlRunFlags);

public:
  /* TOH_Game will create the default 'Soar_Kernel()' if a kernel is not provided.
   * TOH_Game will take care of the deletion of the given kernel if one is provided.
   */
  inline TOH_Game(const std::string &agent_productions = TOH_AGENT_PRODUCTIONS,
                  sml::Kernel * const kernel = 0);
  inline ~TOH_Game();

  static inline void run_trials(const int &num_trials);
  static inline void remote_trials(const int &num_trials,
                                   const std::string &ip_address, const int &port);

  // Returns the "disk size" stacks for all towers
  inline std::vector<std::vector<int> > get_tower_stacks() const;

  inline void run();
  inline void step();
  inline bool is_finished() const;

private:
  inline void update(sml::Kernel &kernel);

  inline void move_disk(const char &source_peg_index, const char &destination_peg_index);

  Soar_Kernel m_kernel;
  Soar_Agent m_agent;

  std::vector<TOH_Tower *> m_towers;
};

#endif

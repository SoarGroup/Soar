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

//#define TOH_COUNT_STEPS

/* The TOH_Game object is a complete instance of Towers of Hanoi.
 * It is responsible for creating the Kernel, Agent, Towers, Disks, ...
 *
 * Tell it to run, and you're off.
 *
 * Alternatively, set TOH_COUNT_STEPS and it will know when it is
 * done.  You can then run it one step at a time.
 */
class TOH_Game {
  /// Disabled (No Implementation)
  TOH_Game(const TOH_Game &);
  TOH_Game & operator=(const TOH_Game &);

  friend inline void toh_update_event_handler(sml::smlUpdateEventId, void *user_data_ptr, sml::Kernel* kernel_ptr, sml::smlRunFlags);

public:
  inline TOH_Game(const std::string &agent_productions = "../../Environments/JavaTOH/towers-of-hanoi-SML.soar");
  inline ~TOH_Game();

  static inline void run_trials(const int &num_trials);

  // Returns the "disk size" stacks for all towers
  inline std::vector<std::vector<int> > get_tower_stacks() const;

  inline void run();
  inline void step();
#ifdef TOH_COUNT_STEPS
  bool is_finished() const {return m_command_count == 2047; /* 2^11 - 1 (for 11 disks) */}
#endif

private:
  inline void update(sml::Kernel &kernel);

  inline void move_disk(const char &source_peg_index, const char &destination_peg_index);

  Soar_Kernel m_kernel;
  Soar_Agent m_agent;

  std::vector<TOH_Tower *> m_towers;

#ifdef TOH_COUNT_STEPS
  int m_command_count;
#endif
};

#endif

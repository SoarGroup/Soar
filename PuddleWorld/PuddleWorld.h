/* PuddleWorld.h
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 *
 * The actual Towers of Hanoi game object that runs the show.
 */

#ifndef CARTPOLE_H
#define CARTPOLE_H

#include "Soar_Kernel.h"
#include "Soar_Agent.h"
#include <vector>
#include <string>

class Stats_Tracker;

#define PUDDLEWORLD_AGENT_PRODUCTIONS "test_agents/puddle-world.soar"

class PuddleWorld {
  /// Disabled (No Implementation)
  PuddleWorld(const PuddleWorld &);
  PuddleWorld & operator=(const PuddleWorld &);

  friend inline void toh_update_event_handler(sml::smlUpdateEventId, void *user_data_ptr, sml::Kernel* kernel_ptr, sml::smlRunFlags);

public:
  /* PuddleWorld will create the default 'Soar_Kernel()' if a kernel is not provided.
   * PuddleWorld will take care of the deletion of the given kernel if one is provided.
   */
  inline PuddleWorld(const float &initial_min_x,
                     const float &initial_min_y,
                     const float &initial_max_x,
                     const float &initial_max_y,
                     const std::string &agent_productions = PUDDLEWORLD_AGENT_PRODUCTIONS,
                     const bool &remote = false,
                     const int &port = sml::Kernel::kDefaultSMLPort,
                     sml::Kernel * const kernel = 0);
  inline ~PuddleWorld();

  static inline void run_trials(const int &num_trials,
                                const std::string &agent_productions = PUDDLEWORLD_AGENT_PRODUCTIONS);
  static inline void remote_trials(const int &num_trials,
                                   const std::string &ip_address,
                                   const int &port,
                                   const std::string &agent_productions = PUDDLEWORLD_AGENT_PRODUCTIONS);

  inline void run();
  inline void step();

  bool is_finished() const;
  bool is_success() const;

  void do_sp(const float &x_div, const float &y_div);

  void reinit(const bool &init_soar, const int &after_episode);
  void srand(const int &seed);
  void ExecuteCommandLine(const std::string &command);

  bool SpawnDebugger();
  bool debugging();
  bool is_running();
  bool StopSelf();
  
private:
  void update();

  Soar_Kernel m_kernel;
  Soar_Agent m_agent;
  int m_reward_counter;

  float m_reward_total;
  float m_initial_min_x;
  float m_initial_min_y;
  float m_initial_max_x;
  float m_initial_max_y;

  sml::StringElement * m_state;
  sml::IntElement * m_step;
  sml::FloatElement * m_reward;
  sml::FloatElement * m_x;
  sml::FloatElement * m_y;
};

#endif

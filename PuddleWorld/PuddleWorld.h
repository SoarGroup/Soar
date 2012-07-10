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
  inline PuddleWorld(const std::string &agent_productions = PUDDLEWORLD_AGENT_PRODUCTIONS,
                  const bool &remote = false,
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

  void set_sp(const int &episode, const float &x_div, const float &y_div);
  void do_sp(const int &episode);
    
  void reinit(const bool &init_soar, const int &after_episode);
  void srand(const int &seed);
  void ExecuteCommandLine(const std::string &command);

  bool SpawnDebugger();
  bool debugging();
  
private:
  void update();

  Soar_Kernel m_kernel;
  Soar_Agent m_agent;

  sml::StringElement * m_state;
  sml::IntElement * m_step;
  sml::FloatElement * m_reward;
  sml::FloatElement * m_x;
  sml::FloatElement * m_y;
  
  int m_sp_episode;
  float m_sp_x_div;
  float m_sp_y_div;
};

#endif

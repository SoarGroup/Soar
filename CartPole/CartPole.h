/* CartPole.h
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
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

#define CARTPOLE_AGENT_PRODUCTIONS "test_agents/cartpole-random-SML.soar"

/* The CartPole object is a complete instance of Towers of Hanoi.
 * It is responsible for creating the Kernel, Agent, Towers, Disks, ...
 *
 * Tell it to run, and you're off.
 */
class CartPole {
  /// Disabled (No Implementation)
  CartPole(const CartPole &);
  CartPole & operator=(const CartPole &);

  friend inline void toh_update_event_handler(sml::smlUpdateEventId, void *user_data_ptr, sml::Kernel* kernel_ptr, sml::smlRunFlags);

public:
  /* CartPole will create the default 'Soar_Kernel()' if a kernel is not provided.
   * CartPole will take care of the deletion of the given kernel if one is provided.
   */
  inline CartPole(const std::string &agent_productions = CARTPOLE_AGENT_PRODUCTIONS,
                  sml::Kernel * const kernel = 0);
  inline ~CartPole();

  static inline void run_trials(const int &num_trials,
                                const std::string &agent_productions = CARTPOLE_AGENT_PRODUCTIONS);
  static inline void remote_trials(const int &num_trials,
                                   const std::string &ip_address,
                                   const int &port,
                                   const std::string &agent_productions = CARTPOLE_AGENT_PRODUCTIONS);

  inline void run();
  inline void step();

  bool is_finished() const;
  bool is_success() const;

  void reinit(const bool &init_soar);
  bool SpawnDebugger();

private:
  void update();

  Soar_Kernel m_kernel;
  Soar_Agent m_agent;

  sml::StringElement * m_state;
  sml::IntElement * m_step;
  sml::FloatElement * m_x;
  sml::FloatElement * m_x_dot;
  sml::FloatElement * m_theta;
  sml::FloatElement * m_theta_dot;
};

#endif

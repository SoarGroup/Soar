/* CartPole.inl
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * Implementation of CartPole.h
 */

#ifndef CARTPOLE_INL
#define CARTPOLE_INL

#include "CartPole.h"

#include "Soar_Kernel.h"
#include "Stats_Tracker.inl"
#include <cstring>

void toh_update_event_handler(sml::smlUpdateEventId /*id*/, void *user_data_ptr, sml::Kernel* /*kernel_ptr*/, sml::smlRunFlags /*run_flags*/) {
  assert(user_data_ptr);
  static_cast<CartPole *>(user_data_ptr)->update();
}

CartPole::CartPole(const std::string &agent_productions,
                   sml::Kernel * const kernel)
: m_kernel(kernel ? kernel :
           sml::Kernel::CreateKernelInNewThread(true)),
  m_agent(m_kernel, kernel ? "" : "CartPole"),
  m_state(0),
  m_step(0),
  m_x(0),
  m_x_dot(0),
  m_theta(0),
  m_theta_dot(0)
{
  m_agent.LoadProductions(agent_productions);

  m_kernel->RegisterForUpdateEvent(sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, toh_update_event_handler, this);
  m_agent->ExecuteCommandLine("watch 0");

  m_state = m_agent->CreateStringWME(m_agent->GetInputLink(), "state", "non-terminal");
  m_step = m_agent->CreateIntWME(m_agent->GetInputLink(), "step", 0);
  m_x = m_agent->CreateFloatWME(m_agent->GetInputLink(), "x", 0.0f);
  m_x_dot = m_agent->CreateFloatWME(m_agent->GetInputLink(), "x-dot", 0.0f);
  m_theta = m_agent->CreateFloatWME(m_agent->GetInputLink(), "theta", 0.0f);
  m_theta_dot = m_agent->CreateFloatWME(m_agent->GetInputLink(), "theta-dot", 0.0f);

  if(!m_agent->Commit())
    abort();
}

CartPole::~CartPole() {
  m_agent->DestroyWME(m_theta_dot);
  m_agent->DestroyWME(m_theta);
  m_agent->DestroyWME(m_x_dot);
  m_agent->DestroyWME(m_x);
  m_agent->DestroyWME(m_step);
  m_agent->DestroyWME(m_state);

  if(!m_agent->Commit())
    abort();
}

void CartPole::run_trials(const int &num_trials,
                          const std::string &agent_productions) {
  std::cout << "Running the C++ CartPole SML (Local)" << std::endl;

  Stats_Tracker stats_tracker;
  for(int i = 0; i < num_trials; ++i) {
    CartPole game(agent_productions);
    stats_tracker.time_run(game.m_agent, i, num_trials);
  }
}

void CartPole::remote_trials(const int &num_trials,
                             const std::string &ip_address,
                             const int &port,
                             const std::string &agent_productions) {
  std::cout << "Running the C++ CartPole SML (Remote)" << std::endl;

  Stats_Tracker stats_tracker;
  for(int i = 0; i < num_trials; ++i) {
    CartPole game(agent_productions,
                  sml::Kernel::CreateRemoteConnection(true,
                                                      ip_address.empty() ? 0 : ip_address.c_str(),
                                                      port,
                                                      false));
    stats_tracker.time_run(game.m_agent, i, num_trials);
  }
}

void CartPole::run() {
  //// Version 1
  const std::string result = m_agent->RunSelfForever();
  std::cout << result << std::endl;

  //// Version 2
  //const std::string result = m_agent->ExecuteCommandLine("time run");
  //std::cout << result << std::endl;

  //// Version 3
  //stats_tracker.time_run(m_agent);
}

void CartPole::step() {
  if(is_finished())
    return;

  m_agent->RunSelf(1u);
}

#endif

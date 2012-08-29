/* PuddleWorld.inl
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 *
 * Implementation of PuddleWorld.h
 */

#ifndef CARTPOLE_INL
#define CARTPOLE_INL

#include "PuddleWorld.h"

#include "Soar_Kernel.h"
#include "Stats_Tracker.inl"
#include <cstring>
#include <cmath>

void toh_update_event_handler(sml::smlUpdateEventId /*id*/, void *user_data_ptr, sml::Kernel* /*kernel_ptr*/, sml::smlRunFlags /*run_flags*/) {
  assert(user_data_ptr);
  static_cast<PuddleWorld *>(user_data_ptr)->update();
}

PuddleWorld::PuddleWorld(const float &initial_min_x,
                         const float &initial_min_y,
                         const float &initial_max_x,
                         const float &initial_max_y,
                         const std::string &agent_productions,
                         const bool &remote,
                         const int &port,
                         sml::Kernel * const kernel)
: m_kernel(kernel ? kernel :
#ifdef NDEBUG
           remote ? sml::Kernel::CreateKernelInNewThread(port)
                  : sml::Kernel::CreateKernelInCurrentThread(true, port)
#else
                    sml::Kernel::CreateKernelInNewThread(port)
#endif
          ),
  m_agent(m_kernel, kernel ? "" : "PuddleWorld"),
  m_episode_count(1),
  m_step_count(0),
  m_reward_counter(0),
  m_reward_total(0.0f),
  m_initial_min_x(initial_min_x),
  m_initial_min_y(initial_min_y),
  m_initial_max_x(initial_max_x),
  m_initial_max_y(initial_max_y),
  m_terminal_reward(0.0f),
  m_state(0),
  m_step(0),
  m_reward(0),
  m_terminal(0),
  m_x(0),
  m_y(0)
{
  m_agent.LoadProductions(agent_productions);

  m_kernel->RegisterForUpdateEvent(sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, toh_update_event_handler, this);
  m_agent->ExecuteCommandLine("watch 0");

  m_state = m_agent->CreateStringWME(m_agent->GetInputLink(), "state", "non-terminal");
  m_step = m_agent->CreateIntWME(m_agent->GetInputLink(), "step", 0);
  m_reward = m_agent->CreateFloatWME(m_agent->GetInputLink(), "reward", 0.0f);

  float x, y;
  do {
    /// Normal
    x = m_initial_min_x + float(rand()) / RAND_MAX * (m_initial_max_x - m_initial_min_x);
    y = m_initial_min_y + float(rand()) / RAND_MAX * (m_initial_max_y - m_initial_min_y);

//     /// Simplified, Deterministic
//     x = int(float(rand()) / RAND_MAX * 20) * 0.05f;
//     y = int(float(rand()) / RAND_MAX * 20) * 0.05f;
  } while(x >= 0.95f && y <= 0.05f);
  m_x = m_agent->CreateFloatWME(m_agent->GetInputLink(), "x", x);
  m_y = m_agent->CreateFloatWME(m_agent->GetInputLink(), "y", y);

  if(!m_agent->Commit())
    abort();
}

PuddleWorld::~PuddleWorld() {
  m_agent->DestroyWME(m_y);
  m_agent->DestroyWME(m_x);
  if(m_terminal)
    m_agent->DestroyWME(m_terminal);
  m_agent->DestroyWME(m_reward);
  m_agent->DestroyWME(m_step);
  m_agent->DestroyWME(m_state);

  if(!m_agent->Commit())
    abort();
}

void PuddleWorld::run_trials(const int &num_trials,
                          const std::string &agent_productions) {
  std::cout << "Running the C++ PuddleWorld SML (Local)" << std::endl;

  Stats_Tracker stats_tracker;
  for(int i = 0; i < num_trials; ++i) {
    PuddleWorld game(0.0f, 0.0f, 1.0f, 1.0f, agent_productions);
    stats_tracker.time_run(game.m_agent, i, num_trials);
  }
}

void PuddleWorld::remote_trials(const int &num_trials,
                             const std::string &ip_address,
                             const int &port,
                             const std::string &agent_productions) {
  std::cout << "Running the C++ PuddleWorld SML (Remote)" << std::endl;

  Stats_Tracker stats_tracker;
  for(int i = 0; i < num_trials; ++i) {
    PuddleWorld game(0.0f, 0.0f, 1.0f, 1.0f, agent_productions,
                  sml::Kernel::CreateRemoteConnection(true,
                                                      ip_address.empty() ? 0 : ip_address.c_str(),
                                                      port,
                                                      false));
    stats_tracker.time_run(game.m_agent, i, num_trials);
  }
}

void PuddleWorld::run(const int &step_count_) {
  //// Version 1
  const std::string result = step_count_ < 1 ? m_agent->RunSelfForever()
                                             : m_agent->RunSelf(step_count_);
//   if(result != "DirectRun completed" &&
//      result != "\nAn agent halted during the run.")
//   {
//     std::cerr << result << std::endl;
//   }

  //// Version 2
  //const std::string result = m_agent->ExecuteCommandLine("time run");
  //std::cerr << result << std::endl;

  //// Version 3
  //stats_tracker.time_run(m_agent);
}

void PuddleWorld::step() {
  if(is_finished())
    return;

  m_agent->RunSelf(1u);
}

#endif

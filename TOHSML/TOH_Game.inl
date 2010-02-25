/* TOH_Game.inl
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * Implementation of TOH_Game.h
 */

#ifndef TOH_GAME_INL
#define TOH_GAME_INL

#include "TOH_Game.h"

#include "Soar_Kernel.h"
#include "TOH_Disk.inl"
#include "TOH_Tower.inl"
#include "Stats_Tracker.inl"
#include <cstring>

void toh_update_event_handler(sml::smlUpdateEventId /*id*/, void *user_data_ptr, sml::Kernel* kernel_ptr, sml::smlRunFlags /*run_flags*/) {
  assert(user_data_ptr);
  static_cast<TOH_Game *>(user_data_ptr)->update(*kernel_ptr);
}

TOH_Game::TOH_Game(const std::string &agent_productions,
                   sml::Kernel * const kernel)
: m_kernel(kernel ? kernel :
           sml::Kernel::CreateKernelInCurrentThread(sml::Kernel::kDefaultLibraryName, true)),
  m_agent(m_kernel, "TOH")
{
  const int num_towers = 3;
  const int num_disks = 11;

  //std::cout << m_kernel->GetLibraryLocation() + agent_productions << std::endl;
  m_agent.LoadProductions(m_kernel->GetLibraryLocation() + agent_productions);

  m_kernel->RegisterForUpdateEvent(sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, toh_update_event_handler, this);
  m_agent->ExecuteCommandLine("watch 0");

  m_towers.reserve(num_towers);
  for(char c = 'A', end = char(c + num_towers); c != end; ++c)
    m_towers.push_back(new TOH_Tower(m_agent, c));

  TOH_Tower &t = *m_towers[0];
  for(int i = num_disks; i > 0; --i)
    t.push_TOH_Disk(new TOH_Disk(m_agent, i));

  if(!m_agent->Commit())
    abort();
}

TOH_Game::~TOH_Game() {
  for(std::vector<TOH_Tower *>::iterator it = m_towers.begin(); it != m_towers.end(); ++it)
    delete *it;
}

std::vector<std::vector<int> > TOH_Game::get_tower_stacks() const {
  std::vector<std::vector<int> > tower_stacks;
  tower_stacks.reserve(m_towers.size());
  for(std::vector<TOH_Tower *>::const_iterator it = m_towers.begin(); it != m_towers.end(); ++it)
    tower_stacks.push_back((*it)->get_stack());
  return tower_stacks;
}

void TOH_Game::run_trials(const int &num_trials) {
  std::cout << "Running the C++ Towers of Hanoi SML Demo (Local)" << std::endl;

  Stats_Tracker stats_tracker;
  for(int i = 0; i < num_trials; ++i) {
    TOH_Game game;
    stats_tracker.time_run(game.m_agent, i, num_trials);
  }
}

void TOH_Game::remote_trials(const int &num_trials,
                             const std::string &ip_address,
                             const int &port) {
  std::cout << "Running the C++ Towers of Hanoi SML Demo (Remote)" << std::endl;

  Stats_Tracker stats_tracker;
  for(int i = 0; i < num_trials; ++i) {
    TOH_Game game(TOH_AGENT_PRODUCTIONS,
                  sml::Kernel::CreateRemoteConnection(true,
                                                      ip_address.empty() ? 0 : ip_address.c_str(),
                                                      port,
                                                      false));
    stats_tracker.time_run(game.m_agent, i, num_trials);
  }
}

void TOH_Game::run() {
  //// Version 1
  const std::string result = m_agent->RunSelfForever();
  std::cout << result << std::endl;

  //// Version 2
  //const std::string result = m_agent->ExecuteCommandLine("time run");
  //std::cout << result << std::endl;

  //// Version 3
  //stats_tracker.time_run(m_agent);
}

void TOH_Game::step() {
  if(is_finished())
    return;

  m_agent->RunSelf(1u);
}

bool TOH_Game::is_finished() const {
  size_t other_towers = 0;
  size_t goal_tower = 0;
  for(std::vector<TOH_Tower *>::const_iterator it = m_towers.begin(); it != m_towers.end(); ++it) {
    other_towers += goal_tower;
    goal_tower += (*it)->get_height();
  }
  return !other_towers;
}

void TOH_Game::update(sml::Kernel &/*kernel*/) {
  // Go through all the commands we've received (if any) since we last ran Soar.
  const int num_commands = m_agent->GetNumberCommands();

  for(int i = 0; i < num_commands; ++i) {
    sml::Identifier * const command_ptr = m_agent->GetCommand(i);

    if(!strcmp("move-disk", command_ptr->GetCommandName())) {
      const char * source_peg_name = command_ptr->GetParameterValue("source-peg");
      const char * destination_peg_name = command_ptr->GetParameterValue("destination-peg");

      if(!source_peg_name || strlen(source_peg_name) != 1 ||
         !destination_peg_name || strlen(destination_peg_name) != 1)
        abort();
    
      // Change the state of the world and generate new input
      move_disk(*source_peg_name - 'A', *destination_peg_name - 'A');
    }
    else
      abort();

    // Update environment here to reflect agent's command
    // Then mark the command as completed
    command_ptr->AddStatusComplete();
  }

  if(!m_agent->Commit())
    abort();
}

void TOH_Game::move_disk(const char &source_peg_index, const char &destination_peg_index) {
  const int end = int(m_towers.size());

  if(-1 < source_peg_index && source_peg_index < end &&
     -1 < destination_peg_index && destination_peg_index < end)
    m_towers[source_peg_index]->move_disk_to(*m_towers[destination_peg_index]);
  else
    abort();
}

#endif

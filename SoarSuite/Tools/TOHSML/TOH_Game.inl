#ifndef TOH_GAME_INL
#define TOH_GAME_INL

#include "TOH_Game.h"

#include "TOH_Disk.inl"
#include "TOH_Tower.inl"

std::vector<std::vector<int> > TOH_Game::get_tower_stacks() const {
  std::vector<std::vector<int> > tower_stacks;
  tower_stacks.reserve(m_towers.size());
  for(std::vector<TOH_Tower *>::const_iterator it = m_towers.begin(); it != m_towers.end(); ++it)
    tower_stacks.push_back((*it)->get_stack());
  return tower_stacks;
}

void TOH_Game::run() {
#ifdef TOH_COUNT_STEPS
  if(is_finished())
    return;
#endif

//#ifdef _DEBUG
//  {
//    std::string rubbish;
//    std::getline(std::cin, rubbish);
//  }
//#endif

  //const std::string result = m_agent.RunSelfForever();
  const std::string result = m_agent->ExecuteCommandLine("time run");
  std::cout << result << std::endl;
}

void TOH_Game::step() {
#ifdef TOH_COUNT_STEPS
  if(is_finished())
    return;
#endif

  m_agent->RunSelf(1u);
}

void TOH_Game::update(sml::Kernel &/*kernel*/) {
  // Go through all the commands we've received (if any) since we last ran Soar.
  const int num_commands = m_agent->GetNumberCommands();

#ifdef TOH_COUNT_STEPS
  m_command_count += num_commands;
#endif

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

  m_agent->ClearOutputLinkChanges();
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

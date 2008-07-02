#ifndef TOH_GAME_INL
#define TOH_GAME_INL

#include "TOH_Game.h"

#include "TOH_Disk.inl"
#include "TOH_Tower.inl"

void TOH_Game::update(sml::Kernel &/*kernel*/) {
  // Go through all the commands we've received (if any) since we last ran Soar.
  const int num_commands = m_agent->GetNumberCommands();

  for(int i = 0; i < num_commands; ++i) {
    sml::Identifier * const command_ptr = m_agent->GetCommand(i);

    const std::string cmd_name = command_ptr->GetCommandName();

    if(cmd_name == "move-disk") {
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

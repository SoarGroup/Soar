#include "TOH_Game.inl"

#include <iostream>

static void toh_update_event_handler(sml::smlUpdateEventId /*id*/, void *user_data_ptr, sml::Kernel* kernel_ptr, sml::smlRunFlags /*run_flags*/) {
  assert(user_data_ptr);
  reinterpret_cast<TOH_Game *>(user_data_ptr)->update(*kernel_ptr);
}

TOH_Game::TOH_Game(const std::string &agent_productions)
: m_agent(m_kernel, "TOH")
#ifdef TOH_COUNT_STEPS
, m_command_count(0)
#endif
{
  const int num_towers = 3;
  const int num_disks = 11;

  m_agent.LoadProductions(agent_productions);

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

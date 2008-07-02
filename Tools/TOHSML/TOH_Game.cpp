#include "TOH_Game.inl"

#include <iostream>

static TOH_Game * toh_update_event_handlee = 0;
static void toh_update_event_handler(sml::smlUpdateEventId /*id*/, void * /*user_data_ptr*/, sml::Kernel* kernel_ptr, sml::smlRunFlags /*run_flags*/) {
  assert(toh_update_event_handlee);
  toh_update_event_handlee->update(*kernel_ptr);
}

TOH_Game::TOH_Game()
: m_agent(m_kernel, "TOH")
{
  const int &num_towers = 3;
  const int &num_disks = 11;

  m_kernel->RegisterForUpdateEvent(sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, toh_update_event_handler, 0);
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

int TOH_Game::run() {
//#ifdef _DEBUG
//  {
//    std::string rubbish;
//    std::getline(std::cin, rubbish);
//  }
//#endif

  assert(!toh_update_event_handlee);
  toh_update_event_handlee = this;

  //const std::string result = m_agent.RunSelfForever();
  const std::string result = m_agent->ExecuteCommandLine("time run");
  std::cout << result << std::endl;

  assert(toh_update_event_handlee == this);
  toh_update_event_handlee = 0;

  return 0;
}

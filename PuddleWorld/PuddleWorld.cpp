#include "portability.h"

/*
 * Invoking function main:
 *
 * ============================================================
 *
 * Option 1:
 *
 * If no arguments are given, it will simply run 3 trials locally and exit.
 *
 * ============================================================
 *
 * Option 2:
 *
 * --remote [ip[:port]]
 *
 * Defaults:
 *
 * ip = localhost (0)
 * port = 12121
 *
 * ============================================================
 *
 * Option 3:
 *
 * --ip xxx.xxx.xxx.xxx (implies remote)
 * --port xxxxx (implies remote)
 *
 * Defaults (if one is specified without the other):
 *
 * ip = localhost(0)
 * port = sml::Kernel::kDefaultSMLPort (12121)
 *
 * ============================================================
 *
 * Return Values:
 *
 * 0 = Success
 * 1 = Invalid argument provided
 * 2 = Missing mandatory argument for --ip
 * 3 = Missing mandatory argument for --port
 */

#include "misc.h"

#include "PuddleWorld.inl"

#include <iostream>
#include <sstream>
#include <cmath>

inline bool arg_help(char ** &arg)
{
  if(strcmp(*arg, "--help"))
    return false;

  std::cout << "Options:" << std::endl
            << "  --help                  prints this help" << std::endl
            << "  --remote [ip[:port]]    to use a remote Soar kernel" << std::endl
            << "  --ip                    to specify an IP address (implies remote Soar kernel)" << std::endl
            << "  --port                  to specify a port address (implies remote Soar kernel)" << std::endl
            << "  --episodes count        to specify the maximum number of episodes [1000]" << std::endl
            << "  --seed seed             to specify the random seed" << std::endl
            << "  --rules filename        to specify non-default rules" << std::endl
            << "  --rl-rules-out          to specify where to output the RL-rules when finished" << std::endl
            << "  --sp-special ep x y     to specify what RL breakdown to add, and when" << std::endl;

  exit(0);

  return true;
}

inline bool arg_remote(bool &remote,
                       std::string &ip_address,
                       int &port,
                       char ** &arg,
                       char ** const &arg_end)
{
  if(strcmp(*arg, "--remote"))
    return false;

  remote = true;

  if(arg + 1 == arg_end)
    return true;

  if(arg[1][1] && arg[1][0] == '-' && arg[1][1] == '-')
    return true;

  ++arg;

  const std::string ip_port = *arg;
  const size_t colon = ip_port.find(':');

  if(colon != std::string::npos) {
    ip_address = ip_port.substr(0, colon);
    from_string(port, *arg + (colon + 1));
  }
  else
    ip_address = ip_port;

  return true;
}

inline bool arg_ip(bool &remote,
                   std::string &ip_address,
                   char ** &arg,
                   char ** const &arg_end)
{
  if(strcmp(*arg, "--ip"))
    return false;

  remote = true;

  if(++arg == arg_end) {
    std::cerr << "'--ip' requires an argument of the form 'xxx.xxx.xxx.xxx'";
    exit(2);
  }

  ip_address = *arg;

  return true;
}

inline bool arg_port(bool &remote,
                     int &port,
                     char ** &arg,
                     char ** const &arg_end)
{
  if(strcmp(*arg, "--port"))
    return false;

  remote = true;

  if(++arg == arg_end) {
    std::cerr << "'--port' requires an argument of the form 'xxxxx'";
    exit(3);
  }

  from_string(port, *arg);

  return true;
}

inline bool arg_rules(std::string &rules,
                      char ** &arg,
                      char ** const &arg_end)
{
  if(strcmp(*arg, "--rules"))
    return false;

  if(++arg == arg_end) {
    std::cerr << "'--rules' requires an argument'";
    exit(2);
  }

  rules = *arg;

  return true;
}

inline bool arg_episodes(int &episodes,
                         char ** &arg,
                         char ** const &arg_end)
{
  if(strcmp(*arg, "--episodes"))
    return false;

  if(++arg == arg_end) {
    std::cerr << "'--episodes' requires an argument'";
    exit(2);
  }

  episodes = atoi(*arg);

  return true;
}

inline bool arg_seed(int &seed,
                     char ** &arg,
                     char ** const &arg_end)
{
  if(strcmp(*arg, "--seed"))
    return false;

  if(++arg == arg_end) {
    std::cerr << "'--seed' requires an argument'";
    exit(2);
  }

  seed = atoi(*arg);

  return true;
}

inline bool arg_rl_rules_out(std::string &rl_rules,
                             char ** &arg,
                             char ** const &arg_end)
{
  if(strcmp(*arg, "--rl-rules-out"))
    return false;

  if(++arg == arg_end) {
    std::cerr << "'--rl-rules-out' requires an argument'";
    exit(2);
  }

  rl_rules = *arg;

  return true;
}

inline bool arg_sp_special(int &episode,
                           float &x_div,
                           float &y_div,
                           char ** &arg,
                           char ** const &arg_end)
{
  if(strcmp(*arg, "--sp-special"))
    return false;

  if(++arg == arg_end) {
    std::cerr << "'--rl-rules-out' requires 3 arguments'";
    exit(2);
  }

  episode = atoi(*arg);

  if(++arg == arg_end) {
    std::cerr << "'--rl-rules-out' requires 3 arguments'";
    exit(2);
  }
  
  x_div = atof(*arg);

  if(++arg == arg_end) {
    std::cerr << "'--rl-rules-out' requires 3 arguments'";
    exit(2);
  }
  
  y_div = atof(*arg);

  return true;
}

int main(int argc, char ** argv) {
#ifdef WIN32
#ifdef _DEBUG
  //_crtBreakAlloc = 1441;
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
#endif

  // Defaults
  bool remote = false;
  std::string ip_address;
  int port = sml::Kernel::kDefaultSMLPort;
  std::string rules = PUDDLEWORLD_AGENT_PRODUCTIONS;
  int episodes = 1000;
  int seed = int(time(0));
  std::string rl_rules_out = "puddleworld-rl.soar";
  int episode = -1;
  float x_div = 1.0f;
  float y_div = 1.0f;

  for(char **arg = argv + 1, **arg_end = argv + argc; arg != arg_end; ++arg) {
    if(!arg_help         (                                        arg         ) &&
       !arg_remote       (remote, ip_address, port,               arg, arg_end) &&
       !arg_ip           (remote, ip_address,                     arg, arg_end) &&
       !arg_port         (remote,             port,               arg, arg_end) &&
       !arg_rules        (                          rules,        arg, arg_end) &&
       !arg_episodes     (                          episodes,     arg, arg_end) &&
       !arg_seed         (                          seed,         arg, arg_end) &&
       !arg_rl_rules_out (                          rl_rules_out, arg, arg_end) &&
       !arg_sp_special   (episode, x_div, y_div,                  arg, arg_end))
    {
      std::cerr << "Unrecognized argument: " << *arg;
      exit(1);
    }
  }

  std::cout << "SEED " << seed << std::endl;

  if(rules == PUDDLEWORLD_AGENT_PRODUCTIONS)
    set_working_directory_to_executable_path();

  PuddleWorld game(rules, remote);
  game.set_sp(episode, x_div, y_div);
  srand(seed);
  game.srand(seed);

  bool force_debugging = false;
  if(remote) {
    game.SpawnDebugger();
    force_debugging = true;
  }

  for(int episode = 0; episode != episodes; ++episode) {
    game.do_sp(episode);

    do {
      if(game.debugging()) {
#ifdef WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
        
        force_debugging = false;
      }
      else if(force_debugging) {
#ifdef WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
      }
      else
        game.run();
    }while(!game.is_finished());

//       if(game.is_success()) {
//         std::cout << "Success in episode " << episode + 1 << std::endl;
//         break;
//       }

    if(!(episode % 50))
      std::cerr << "\nEp " << episode << ' ';
    std::cerr << (game.is_success() ? 'S' : '.');

    game.reinit(true, episode);
  }

  game.ExecuteCommandLine("command-to-file " + rl_rules_out + " print --rl --full");

  std::cerr << std::endl;
  
  return 0;
}

bool PuddleWorld::is_finished() const {
  return m_state && strcmp("non-terminal", m_state->GetValue());
}

bool PuddleWorld::is_success() const {
  return m_x->GetValue() >= 0.95f && m_y->GetValue() <= 0.05f;
}

void PuddleWorld::set_sp(const int &episode, const float &x_div, const float &y_div) {
  m_sp_episode = episode;
  m_sp_x_div = x_div;
  m_sp_y_div = y_div;
}

void PuddleWorld::do_sp(const int &episode) {
  if(episode == m_sp_episode) {
    std::ostringstream oss;
    oss << "sp {elaborate*additional*puddleworld (state <s> ^superstate nil ^name puddleworld) --> (<s> ^div <d>) (<d> ^name additional ^x (/ 1.001 " << m_sp_x_div
        << ") ^y (/ 1.001 " << m_sp_y_div
        << "))}";
    std::cerr << oss.str() << std::endl;
    ExecuteCommandLine(oss.str());

    if(!m_agent->Commit())
      abort();
  }
}

void PuddleWorld::reinit(const bool &init_soar, const int &after_episode) {
  m_agent->DestroyWME(m_y);
  m_y = 0;
  m_agent->DestroyWME(m_x);
  m_x = 0;
  m_agent->DestroyWME(m_reward);
  m_reward = 0;
  m_agent->DestroyWME(m_step);
  m_step = 0;
  m_agent->DestroyWME(m_state);
  m_state = 0;

  if(!m_agent->Commit())
    abort();

  if(init_soar)
    m_agent->InitSoar();
  
  m_reward_total = 0.0f;

  m_state = m_agent->CreateStringWME(m_agent->GetInputLink(), "state", "non-terminal");
  m_step = m_agent->CreateIntWME(m_agent->GetInputLink(), "step", 0);
  m_reward = m_agent->CreateFloatWME(m_agent->GetInputLink(), "reward", 0.0f);

  float x, y;
  do {
    x = float(rand()) / RAND_MAX;
    y = float(rand()) / RAND_MAX;
  } while(x >= 0.95f && y <= 0.05f);
  m_x = m_agent->CreateFloatWME(m_agent->GetInputLink(), "x", x);
  m_y = m_agent->CreateFloatWME(m_agent->GetInputLink(), "y", y);

  if(!m_agent->Commit())
    abort();
}

void PuddleWorld::srand(const int &seed) {
  std::ostringstream oss;
  oss << "srand " << seed;
  ExecuteCommandLine(oss.str());
}

void PuddleWorld::ExecuteCommandLine(const std::string &command) {
  m_agent->ExecuteCommandLine(command.c_str());
}

bool PuddleWorld::SpawnDebugger() {
  return m_agent->SpawnDebugger();
}

bool PuddleWorld::debugging() {
  m_kernel->GetAllConnectionInfo();
  return m_kernel->GetNumberConnections() > 1;
}

void PuddleWorld::update() {
  // Go through all the commands we've received (if any) since we last ran Soar.
  const int num_commands = m_agent->GetNumberCommands();

  for(int i = 0; i < num_commands; ++i) {
    sml::Identifier * const command_ptr = m_agent->GetCommand(i);

    if(!strcmp("move", command_ptr->GetCommandName())) {
      const char * direction_name = command_ptr->GetParameterValue("direction");

      if(!direction_name)
        abort();
      else if(!strcmp("north", direction_name) ||
              !strcmp("south", direction_name) ||
              !strcmp("east", direction_name) ||
              !strcmp("west", direction_name))
      {
        if(is_finished()) {
          std::cout << "terminal" << std::endl;
        }
        else {
          // Change the state of the world and generate new input
          const int step = m_step->GetValue() + 1;
          float x = m_x->GetValue();
          float y = m_y->GetValue();

          {
            const float shift = 0.02f * rand() / RAND_MAX - 0.01f; ///< Should really be Gaussian, stddev = 0.01f

            switch(direction_name[0]) {
              case 'n': y -= 0.05f + shift; break;
              case 's': y += 0.05f + shift; break;
              case 'e': x += 0.05f + shift; break;
              case 'w': x -= 0.05f + shift; break;
              default: abort(); break;
            }

            if(x < 0.0f)
              x = 0.0f;
            else if(x > 1.0f)
              x = 1.0f;
            if(y < 0.0f)
              y = 0.0f;
            else if(y > 1.0f)
              y = 1.0f;
          }

          float reward = -1.0f;
          if(x >= 0.95f && y <= 0.05f) {
            m_state->Update("terminal");
            m_reward->Update(reward = 0.0f);
          }
          else {
            float dist;

            /// (.1, .25) to (.45, .25), radius 0.1
            if(x < 0.1f)
              dist = sqrt(pow(x - 0.1f, 2) + pow(y - 0.25f, 2));
            else if(x < 0.45f)
              dist = fabs(y - 0.25f);
            else
              dist = sqrt(pow(x - 0.45f, 2) + pow(y - 0.25f, 2));
            reward += -400.0f * std::max(0.0f, 0.1f - dist);

            /// (.45, .2) to (.45, .6), radius 0.1
            if(y < 0.2f)
              dist = sqrt(pow(x - 0.45f, 2) + pow(y - 0.2f, 2));
            else if(y < 0.6f)
              dist = fabs(x - 0.45f);
            else
              dist = sqrt(pow(x - 0.45f, 2) + pow(y - 0.6f, 2));
            reward += -400.0f * std::max(0.0f, 0.1f - dist);

            m_reward->Update(reward);
          }

          m_step->Update(step);
          m_x->Update(x);
          m_y->Update(y);

          m_reward_total += reward;

          if(is_finished()) {
            static int episode = 0;
            std::cout << "EPISODE " << ++episode
                      << " STEP " << step
                      << " REWARD " << m_reward_total
                      << " DIR " << (direction_name[0] == 'r' ? 1 : 0)
                      << " X " << x
                      << " Y " << y
                      << std::endl;
          }
        }
      }
      else
        abort();
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

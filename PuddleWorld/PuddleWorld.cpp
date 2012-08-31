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

using namespace std;

inline bool arg_help(char ** &arg)
{
  if(strcmp(*arg, "--help"))
    return false;

  cout << "Options:" << endl
            << "  --help                          prints this help" << endl
            << "  --remote [ip[:port]]            to use a remote Soar kernel" << endl
            << "  --port                          to specify a port address (does not imply" << endl
            << "                                       remote Soar kernel)" << endl
            << "  --episodes count                to specify the maximum number" << endl
            << "                                       of episodes [1000]" << endl
            << "  --steps count                   to specify the maximum number" << endl
            << "                                       of steps [-1]" << endl
            << "  --seed seed                     to specify the random seed" << endl
            << "  --rules filename                to specify non-default rules" << endl
            << "  --rl-rules-out                  to specify where to output the RL-rules" << endl
            << "                                       when finished" << endl
            << "  --sp-special ep x y             to specify what RL breakdown to add, and when" << endl
            << "  --credit-assignment even/fc/rl  to specify credit assignment" << endl
            << "  --alpha normal/adaptive         to specify credit assignment" << endl
            << "  --initial minx miny maxx maxy   to specify the starting location" << endl
            << "  --credit-modification none/variance  to specify any modification to credit" << endl
            << "  --variance bellman/simple       to specify the method of calculating variance" << endl
            << "  --tsdt                          to turn on TSDT" << endl
            << "  --refine uperf/td-error         to specify the how to determine when to" << endl
            << "  --refine-stddev #               to specify the threshold for refinement" << endl
            << "                                       refine Q(s,a)" << endl
            << "  --refine-require-episodes [1,)  to specify the delay before the refinement" << endl
            << "                                       criterion comes into play" << endl
            << "  --refine-decay-rate [0,1]       to specify the decay rate for td-error" << endl
            << "                                       uperf is unaffected" << endl
            << "  --refine-cycles-between-episodes [0,)  to specify the how many cycles" << endl
            << "                                       simulate an episode break" << endl
            << "  --refine-reinhibit              to specify that subsequent visits should" << endl
            << "                                       reinhibit refinement" << endl;

  exit(0);

  return true;
}

inline bool arg_remote(bool &remote,
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

  from_string(port, *arg);

  return true;
}

inline bool arg_port(int &port,
                     char ** &arg,
                     char ** const &arg_end)
{
  if(strcmp(*arg, "--port"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--port' requires an argument of the form 'xxxxx'" << std::endl;
    exit(3);
  }

  from_string(port, *arg);

  return true;
}

inline bool arg_rules(string &rules,
                      char ** &arg,
                      char ** const &arg_end)
{
  if(strcmp(*arg, "--rules"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--rules' requires an argument'" << std::endl;
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
    cerr << "'--episodes' requires an argument'" << std::endl;
    exit(2);
  }

  episodes = atoi(*arg);

  return true;
}

inline bool arg_steps(int &steps,
                      char ** &arg,
                      char ** const &arg_end)
{
  if(strcmp(*arg, "--steps"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--steps' requires an argument'" << std::endl;
    exit(2);
  }

  steps = atoi(*arg);

  return true;
}

inline bool arg_seed(int &seed,
                     char ** &arg,
                     char ** const &arg_end)
{
  if(strcmp(*arg, "--seed"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--seed' requires an argument'" << std::endl;
    exit(2);
  }

  seed = atoi(*arg);

  return true;
}

inline bool arg_rl_rules_out(string &rl_rules,
                             char ** &arg,
                             char ** const &arg_end)
{
  if(strcmp(*arg, "--rl-rules-out"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--rl-rules-out' requires an argument'" << std::endl;
    exit(2);
  }

  rl_rules = *arg;

  return true;
}

inline bool arg_sp_special(multimap<int, pair<float, float> > &sp,
                           char ** &arg,
                           char ** const &arg_end)
{
  if(strcmp(*arg, "--sp-special"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--rl-rules-out' requires 3 arguments'" << std::endl;
    exit(2);
  }

  const int episode = atoi(*arg);

  if(++arg == arg_end) {
    cerr << "'--rl-rules-out' requires 3 arguments'" << std::endl;
    exit(2);
  }
  
  const float x_div = atof(*arg);

  if(++arg == arg_end) {
    cerr << "'--rl-rules-out' requires 3 arguments'" << std::endl;
    exit(2);
  }
  
  const float y_div = atof(*arg);

  sp.insert(make_pair(episode, make_pair(x_div, y_div)));

  return true;
}

inline bool arg_credit_assignment(string &value,
                                  char ** &arg,
                                  char ** const &arg_end)
{
  if(strcmp(*arg, "--credit-assignment"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--credit-assignment' requires 1 arguments'" << std::endl;
    exit(2);
  }

  if(strcmp(*arg, "even") && strcmp(*arg, "fc") && strcmp(*arg, "rl") && strcmp(*arg, "log-rl")) {
    cerr << "--credit-assignment takes 'even', 'fc', or 'rl'" << std::endl;
    exit(3);
  }

  value = *arg;

  return true;
}

inline bool arg_alpha(string &value,
                      char ** &arg,
                      char ** const &arg_end)
{
  if(strcmp(*arg, "--alpha"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--alpha' requires 1 arguments'" << std::endl;
    exit(2);
  }

  if(strcmp(*arg, "normal") && strcmp(*arg, "adaptive")) {
    cerr << "--alpha takes 'normal' or 'adaptive'" << std::endl;
    exit(3);
  }

  value = *arg;

  return true;
}

inline bool arg_initial(float &initial_min_x,
                        float &initial_min_y,
                        float &initial_max_x,
                        float &initial_max_y,
                        char ** &arg,
                        char ** const &arg_end)
{
  if(strcmp(*arg, "--initial"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--initial' requires 4 arguments'" << std::endl;
    exit(2);
  }

  initial_min_x = atof(*arg);

  if(++arg == arg_end) {
    cerr << "'--rl-rules-out' requires 4 arguments'" << std::endl;
    exit(2);
  }
  
  initial_min_y = atof(*arg);

  if(++arg == arg_end) {
    cerr << "'--rl-rules-out' requires 4 arguments'" << std::endl;
    exit(2);
  }
  
  initial_max_x = atof(*arg);

  if(++arg == arg_end) {
    cerr << "'--rl-rules-out' requires 4 arguments'" << std::endl;
    exit(2);
  }
  
  initial_max_y = atof(*arg);

  return true;
}

inline bool arg_credit_mod(string &credit_mod,
                           char ** &arg,
                           char ** const &arg_end)
{
  if(strcmp(*arg, "--credit-modification"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--credit-modification' requires 1 arguments'" << std::endl;
    exit(2);
  }

  if(strcmp(*arg, "none") && strcmp(*arg, "variance")) {
    cerr << "--credit-modification takes 'none' or 'variance'" << std::endl;
    exit(3);
  }

  credit_mod = *arg;

  return true;
}

inline bool arg_variance(string &credit_mod,
                         char ** &arg,
                         char ** const &arg_end)
{
  if(strcmp(*arg, "--variance"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--variance' requires 1 arguments'" << std::endl;
    exit(2);
  }

  if(strcmp(*arg, "bellman") && strcmp(*arg, "simple")) {
    cerr << "--variance takes 'bellman' or 'simple'" << std::endl;
    exit(3);
  }

  credit_mod = *arg;

  return true;
}

inline bool arg_tsdt(bool &tsdt,
                     char ** &arg,
                     char ** const &arg_end)
{
  if(strcmp(*arg, "--tsdt"))
    return false;

  tsdt = true;

  return true;
}

inline bool arg_refine(string &refine,
                       char ** &arg,
                       char ** const &arg_end)
{
  if(strcmp(*arg, "--refine"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--refine' requires 1 arguments'";
    exit(2);
  }

  if(strcmp(*arg, "uperf") && strcmp(*arg, "td-error")) {
    cerr << "--refine takes 'uperf' or 'td-error'" << std::endl;
    exit(3);
  }

  refine = *arg;

  return true;
}

inline bool arg_refine_stddev(string &refine_stddev,
                              char ** &arg,
                              char ** const &arg_end)
{
  if(strcmp(*arg, "--refine-stddev"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--refine-stddev' requires 1 arguments'" << std::endl;
    exit(2);
  }

  refine_stddev = *arg;

  return true;
}

inline bool arg_refine_require_episodes(string &refine_require_episodes,
                                        char ** &arg,
                                        char ** const &arg_end)
{
  if(strcmp(*arg, "--refine-require-episodes"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--refine-require_episodes' requires 1 arguments'" << std::endl;
    exit(2);
  }

  refine_require_episodes = *arg;

  return true;
}

inline bool arg_refine_decay_rate(string &refine_decay_rate,
                                  char ** &arg,
                                  char ** const &arg_end)
{
  if(strcmp(*arg, "--refine-decay-rate"))
    return false;

  if(++arg == arg_end) {
    cerr << "'--refine-decay-rate' requires 1 arguments'" << std::endl;
    exit(2);
  }

  refine_decay_rate = *arg;

  return true;
}

inline bool arg_refine_cycles_between_episodes(std::string &refine_cycles_between_episodes,
                                               char ** &arg,
                                               char ** const &arg_end)
{
  if(strcmp(*arg, "--refine-cycles-between-episodes"))
    return false;

  if(++arg == arg_end) {
    std::cerr << "'--refine-cycles-between-episodes' requires 1 arguments'" << std::endl;
    exit(2);
  }

  refine_cycles_between_episodes = *arg;

  return true;
}

inline bool arg_refine_reinhibit(bool &refine_reinhibit,
                                 char ** &arg,
                                 char ** const &arg_end)
{
  if(strcmp(*arg, "--refine-reinhibit"))
    return false;

  refine_reinhibit = true;

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
  string ip_address;
  int port = sml::Kernel::kDefaultSMLPort;
  string rules = PUDDLEWORLD_AGENT_PRODUCTIONS;
  int episodes = 1000;
  int steps = -1;
  int seed = int(time(0));
  string rl_rules_out = "puddleworld-rl.soar";
  multimap<int, pair<float, float> > sp;
  string credit_assignment = "even";
  string alpha = "normal";
  float initial_min_x = 0.0f;
  float initial_min_y = 0.0f;
  float initial_max_x = 1.0f;
  float initial_max_y = 1.0f;
  string credit_mod = "none";
  string variance = "bellman";
  bool tsdt = false;
  string refine = "td-error";
  string refine_stddev = "0.84155";
  string refine_require_episodes = "10";
  string refine_decay_rate = "1.0";
  string refine_cycles_between_episodes = "100";
  bool refine_reinhibit = false;

  for(char **arg = argv + 1, **arg_end = argv + argc; arg != arg_end; ++arg) {
    if(!arg_help         (                                             arg         ) &&
       !arg_remote       (remote,                   port,              arg, arg_end) &&
       !arg_port         (                          port,              arg, arg_end) &&
       !arg_rules        (                          rules,             arg, arg_end) &&
       !arg_episodes     (                          episodes,          arg, arg_end) &&
       !arg_steps        (                          steps,             arg, arg_end) &&
       !arg_seed         (                          seed,              arg, arg_end) &&
       !arg_rl_rules_out (                          rl_rules_out,      arg, arg_end) &&
       !arg_sp_special   (                          sp,                arg, arg_end) &&
       !arg_credit_assignment (                     credit_assignment, arg, arg_end) &&
       !arg_alpha        (                          alpha,             arg, arg_end) &&
       !arg_initial      (initial_min_x, initial_min_y, initial_max_x, initial_max_y, arg, arg_end) &&
       !arg_credit_mod   (                          credit_mod,        arg, arg_end) &&
       !arg_variance     (                          variance,          arg, arg_end) &&
       !arg_tsdt         (                          tsdt,              arg, arg_end) &&
       !arg_refine       (                          refine,            arg, arg_end) &&
       !arg_refine_stddev(                          refine_stddev,     arg, arg_end) &&
       !arg_refine_require_episodes(          refine_require_episodes, arg, arg_end) &&
       !arg_refine_decay_rate(                      refine_decay_rate, arg, arg_end) &&
       !arg_refine_cycles_between_episodes(refine_cycles_between_episodes, arg, arg_end) &&
       !arg_refine_reinhibit(                       refine_reinhibit,  arg, arg_end))
    {
      cerr << "Unrecognized argument: " << *arg;
      exit(1);
    }
  }

  cout << "SEED " << seed << endl;

  if(rules == PUDDLEWORLD_AGENT_PRODUCTIONS)
    set_working_directory_to_executable_path();

  PuddleWorld game(initial_min_x, initial_min_y, initial_max_x, initial_max_y, rules, remote, port);
  srand(seed);
  game.srand(seed);

  bool force_debugging = false;
  if(remote) {
    game.SpawnDebugger();
    force_debugging = true;
  }

  game.ExecuteCommandLine(("rl --set credit-assignment " + credit_assignment).c_str());
  game.ExecuteCommandLine(("rl --set decay-mode " + alpha).c_str());
  game.ExecuteCommandLine(("rl --set credit-modification " + credit_mod).c_str());
  game.ExecuteCommandLine(("rl --set variance-bellman " + string(variance == "bellman" ? "on" : "off")).c_str());
  if(tsdt)
    game.ExecuteCommandLine("rl --set trace tsdt");
  game.ExecuteCommandLine(("rl --set refine " + refine).c_str());
  game.ExecuteCommandLine(("rl --set refine-stddev " + refine_stddev).c_str());
  game.ExecuteCommandLine(("rl --set refine-require-episodes " + refine_require_episodes).c_str());
  game.ExecuteCommandLine(("rl --set refine-decay-rate " + refine_decay_rate).c_str());
  game.ExecuteCommandLine(("rl --set refine-cycles-between-episodes " + refine_cycles_between_episodes).c_str());
  game.ExecuteCommandLine(("rl --set refine-reinhibit " + string(refine_reinhibit ? "on" : "off")).c_str());

  for(int episode = 0; episode != episodes; ++episode) {
    for(std::pair<multimap<int, pair<float, float> >::const_iterator,
                  multimap<int, pair<float, float> >::const_iterator>
          st = sp.equal_range(episode);
        st.first != st.second;
        ++st.first)
    {
      game.do_sp(st.first->second.first, st.first->second.second);
    }

    do {
      if(force_debugging && game.debugging())
        force_debugging = false;
      if(force_debugging || game.debugging()) {
#ifdef WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
      }
      else {
        if(game.is_running())
          game.StopSelf();
        game.run(steps - game.step_count());

//         if(episode == 57) {
//           game.SpawnDebugger();
//           force_debugging = true;
//         }

        if(steps == game.step_count())
          goto DONE;
      }
    }while(!game.is_finished());

//       if(game.is_success()) {
//         cout << "Success in episode " << episode + 1 << endl;
//         break;
//       }

//     if(!(episode % 50))
//       cerr << "\nEp " << episode << ' ';
//     cerr << (game.is_success() ? 'S' : '.');

    game.reinit(true, episode);
  }

DONE:

  game.ExecuteCommandLine("command-to-file " + rl_rules_out + " print --rl --full");

  cerr << endl;
  
  return 0;
}

bool PuddleWorld::is_finished() const {
  return m_state && strcmp("non-terminal", m_state->GetValue());
}

bool PuddleWorld::is_success() const {
  return m_x->GetValue() >= 0.95f && m_y->GetValue() <= 0.05f;
}

void PuddleWorld::do_sp(const float &x_div, const float &y_div) {
  static int i = 0;
  ++i;
  
  ostringstream oss;
  oss << "sp {elaborate*additional*puddleworld*" << i
      << " (state <s> ^superstate nil ^name puddleworld) --> (<s> ^div <d>) (<d> ^name additional-" << i
      << " ^x (/ 1.001 " << x_div
      << ") ^y (/ 1.001 " << y_div
      << "))}";
//   cerr << oss.str() << endl;
  ExecuteCommandLine(oss.str());

  if(!m_agent->Commit())
    abort();
}

void PuddleWorld::reinit(const bool &init_soar, const int &after_episode) {
  m_agent->DestroyWME(m_y);
  m_y = 0;
  m_agent->DestroyWME(m_x);
  m_x = 0;
  m_agent->DestroyWME(m_terminal);
  m_terminal = 0;
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
  
  m_reward_counter = 0;
  m_reward_total = 0.0f;

  m_state = m_agent->CreateStringWME(m_agent->GetInputLink(), "state", "non-terminal");
  m_step = m_agent->CreateIntWME(m_agent->GetInputLink(), "step", 0);
  m_reward = m_agent->CreateFloatWME(m_agent->GetInputLink(), "reward", 0.0f);

  float x, y;
  do {
    /// Normal
    x = m_initial_min_x + float(rand()) / RAND_MAX * (m_initial_max_x - m_initial_min_x);
    y = m_initial_min_y + float(rand()) / RAND_MAX * (m_initial_max_y - m_initial_min_y);

    /// Simplified, Deterministic
//     x = int(float(rand()) / RAND_MAX * 20) * 0.05f;
//     y = int(float(rand()) / RAND_MAX * 20) * 0.05f;
  } while(x >= 0.95f && y <= 0.05f);
  m_x = m_agent->CreateFloatWME(m_agent->GetInputLink(), "x", x);
  m_y = m_agent->CreateFloatWME(m_agent->GetInputLink(), "y", y);

  if(!m_agent->Commit())
    abort();
}

void PuddleWorld::srand(const int &seed) {
  ostringstream oss;
  oss << "srand " << seed;
  ExecuteCommandLine(oss.str());
}

void PuddleWorld::ExecuteCommandLine(const string &command) {
  m_agent->ExecuteCommandLine(command.c_str());
}

bool PuddleWorld::SpawnDebugger() {
  return m_agent->SpawnDebugger();
}

bool PuddleWorld::debugging() {
  m_kernel->GetAllConnectionInfo();
  return m_kernel->GetNumberConnections() > 1;
}

bool PuddleWorld::is_running() {
  return m_agent->GetRunState() == sml::sml_RUNSTATE_RUNNING;
}

bool PuddleWorld::StopSelf() {
  return m_agent->StopSelf();
}

void PuddleWorld::update() {
  // Go through all the commands we've received (if any) since we last ran Soar.
  const int num_commands = m_agent->GetNumberCommands();

  float reward = 0.0f;

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
          cout << "terminal" << endl;
        }
        else {
          // Change the state of the world and generate new input
          const int step = m_step->GetValue() + 1;
          float x = m_x->GetValue();
          float y = m_y->GetValue();

          {
            const float shift = 0.02f * rand() / RAND_MAX - 0.01f; ///< Should really be Gaussian, stddev = 0.01f
//             const float shift = 0.0f; ///< Simplified, Deterministic

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

          if(x >= 0.95f && y <= 0.05f) {
            m_state->Update("terminal");
            if(!m_terminal) {
              m_terminal = m_agent->CreateFloatWME(m_agent->GetInputLink(), "terminal", 0.0f);
//               std::cerr << "PuddleWorld marked ^terminal " << 0.0f << std::endl;
            }

            reward += m_terminal_reward;
          }
          else {
            if(step == 5000) {
              /// HACK: Force eventual termination
              m_state->Update("terminal");
            }

            float dist;

            reward += -1.0f;

            /// Standard Puddle World
            /// (.1, .25) to (.45, .25), radius 0.1
            if(x < 0.1f)
              dist = sqrt(pow(x - 0.1f, 2) + pow(y - 0.25f, 2));
            else if(x < 0.45f)
              dist = fabs(y - 0.25f);
            else
              dist = sqrt(pow(x - 0.45f, 2) + pow(y - 0.25f, 2));
            reward += -400.0f * max(0.0f, 0.1f - dist);

            /// (.45, .2) to (.45, .6), radius 0.1
            if(y < 0.2f)
              dist = sqrt(pow(x - 0.45f, 2) + pow(y - 0.2f, 2));
            else if(y < 0.6f)
              dist = fabs(x - 0.45f);
            else
              dist = sqrt(pow(x - 0.45f, 2) + pow(y - 0.6f, 2));
            reward += -400.0f * max(0.0f, 0.1f - dist);

//             /// Simplified Puddle World
//             /// (.375, 0) to (.375, 1), radius 0.125
// //             if(0.25f <= x && x < 0.5f)
// //               reward += -10.0f;

            /// Simple Puddle World
            /// (.35, 0) to (.35, 1), radius 0.1
//             dist = fabs(x - 0.35f);
//             reward += -400.0f * max(0.0f, 0.1f - dist);

            /// Semi-Complex Puddle World
            /// (.35, 0) to (.35, .8275), radius 0.1
//             if(y < 0.8275f)
//               dist = fabs(x - 0.35f);
//             else
//               dist = sqrt(pow(x - 0.35f, 2) + pow(y - 0.8275f, 2));
//             reward += -400.0f * max(0.0f, 0.1f - dist);
          }

          m_step->Update(step);
          m_x->Update(x);
          m_y->Update(y);

          m_reward_total += reward;

          ++m_step_count;
          cout << m_step_count << ' ' << m_episode_count << ' ' << step << ' ' << reward << endl;

          if(is_finished()) {
//             cout << "EPISODE " << m_episode_count
//                       << " STEP " << step
//                       << " REWARD " << m_reward_total
//                       << " DIR " << (direction_name[0] == 'r' ? 1 : 0)
//                       << " X " << x
//                       << " Y " << y
//                       << endl;
            ++m_episode_count;
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

  if(num_commands || m_reward_counter == 1)
    m_reward->Update(reward);

  if(num_commands)
    m_reward_counter = 0;
  else if(m_reward_counter < 2)
    ++m_reward_counter;

  if(!m_agent->Commit())
    abort();
}

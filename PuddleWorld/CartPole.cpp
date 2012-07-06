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

#include "CartPole.inl"

#include <iostream>
#include <sstream>

void cart_pole(int action, float *x, float *x_dot, float *theta, float *theta_dot);

int get_box(float x, float x_dot, float theta, float theta_dot);

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
            << "  --sp-special ep x xd t td  to specify what RL breakdown to add, and when" << std::endl;

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
                           float &x_dot_div,
                           float &theta_div,
                           float &theta_dot_div,
                           char ** &arg,
                           char ** const &arg_end)
{
  if(strcmp(*arg, "--sp-special"))
    return false;

  if(++arg == arg_end) {
    std::cerr << "'--rl-rules-out' requires 5 arguments'";
    exit(2);
  }

  episode = atoi(*arg);

  if(++arg == arg_end) {
    std::cerr << "'--rl-rules-out' requires 5 arguments'";
    exit(2);
  }
  
  x_div = atof(*arg);

  if(++arg == arg_end) {
    std::cerr << "'--rl-rules-out' requires 5 arguments'";
    exit(2);
  }
  
  x_dot_div = atof(*arg);

  if(++arg == arg_end) {
    std::cerr << "'--rl-rules-out' requires 5 arguments'";
    exit(2);
  }
  
  theta_div = atof(*arg);

  if(++arg == arg_end) {
    std::cerr << "'--rl-rules-out' requires 5 arguments'";
    exit(2);
  }
  
  theta_dot_div = atof(*arg);

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
  std::string rules = CARTPOLE_AGENT_PRODUCTIONS;
  int episodes = 1000;
  int seed = int(time(0));
  std::string rl_rules_out = "cartpole-rl.soar";
  int episode = -1;
  float x_div = 1.0f;
  float x_dot_div = 1.0f;
  float theta_div = 1.0f;
  float theta_dot_div = 1.0f;

  for(char **arg = argv + 1, **arg_end = argv + argc; arg != arg_end; ++arg) {
    if(!arg_help         (                                        arg         ) &&
       !arg_remote       (remote, ip_address, port,               arg, arg_end) &&
       !arg_ip           (remote, ip_address,                     arg, arg_end) &&
       !arg_port         (remote,             port,               arg, arg_end) &&
       !arg_rules        (                          rules,        arg, arg_end) &&
       !arg_episodes     (                          episodes,     arg, arg_end) &&
       !arg_seed         (                          seed,         arg, arg_end) &&
       !arg_rl_rules_out (                          rl_rules_out, arg, arg_end) &&
       !arg_sp_special   (episode, x_div, x_dot_div, theta_div, theta_dot_div, arg, arg_end))
    {
      std::cerr << "Unrecognized argument: " << *arg;
      exit(1);
    }
  }

  std::cout << "SEED " << seed << std::endl;

  if(rules == CARTPOLE_AGENT_PRODUCTIONS)
    set_working_directory_to_executable_path();

  if(remote) {
//     CartPole::remote_trials(3, ip_address, port, rules);

//     CartPole game(rules,
//                   sml::Kernel::CreateRemoteConnection(true,
//                                                       ip_address.empty() ? 0 : ip_address.c_str(),
//                                                       port,
//                                                       false));

    CartPole game(rules, true);
    game.set_sp(episode, x_div, x_dot_div, theta_div, theta_dot_div);
    game.srand(seed);
    game.SpawnDebugger();

    for(int episode = 0; episode != episodes; ++episode) {
      game.do_sp(episode);
      
      while(!game.is_finished()) {
  #ifdef WIN32
        Sleep(100);
  #else
        usleep(100000);
  #endif
      }

//       if(game.is_success()) {
//         std::cout << "Success in episode " << episode + 1 << std::endl;
//         break;
//       }

      if(!(episode % 50))
        std::cerr << "\nEp " << episode << ' ';
      std::cerr << (game.is_success() ? 'S' : '.');

      game.reinit(false, episode);
    }
  
    game.ExecuteCommandLine("command-to-file " + rl_rules_out + " print --rl --full");
  }
  else {
    // CartPole::run_trials(3);

    CartPole game(rules, false);
    game.set_sp(episode, x_div, x_dot_div, theta_div, theta_dot_div);
    game.srand(seed);

    for(int episode = 0; episode != episodes; ++episode) {
      game.do_sp(episode);

      game.run();

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
  }

  std::cerr << std::endl;
  
  return 0;
}

bool CartPole::is_finished() const {
  return m_state && strcmp("non-terminal", m_state->GetValue());
}

bool CartPole::is_success() const {
  return m_step && m_step->GetValue() > 10000;
}

void CartPole::set_sp(const int &episode, const float &x_div, const float &x_dot_div, const float &theta_div, const float &theta_dot_div) {
  m_sp_episode = episode;
  m_sp_x_div = x_div;
  m_sp_x_dot_div = x_dot_div;
  m_sp_theta_div = theta_div;
  m_sp_theta_dot_div = theta_dot_div;
}

void CartPole::do_sp(const int &episode) {
  if(episode == m_sp_episode) {
    std::ostringstream oss;
    oss << "sp {elaborate*additional*cartpole (state <s> ^superstate nil ^name cartpole) --> (<s> ^div <d>) (<d> ^name additional ^x (/ 10 " << m_sp_x_div
        << ") ^x-dot (/ 1 " << m_sp_x_dot_div
        << ") ^theta (/ 3.1415926 " << m_sp_theta_div
        << ") ^theta-dot (/ 3.141526 " << m_sp_theta_dot_div << "))}";
    std::cerr << oss.str() << std::endl;
    ExecuteCommandLine(oss.str());

    if(!m_agent->Commit())
      abort();
  }
}

void CartPole::reinit(const bool &init_soar, const int &after_episode) {
  m_agent->DestroyWME(m_theta_dot);
  m_theta_dot = 0;
  m_agent->DestroyWME(m_theta);
  m_theta = 0;
  m_agent->DestroyWME(m_x_dot);
  m_x_dot = 0;
  m_agent->DestroyWME(m_x);
  m_x = 0;
  m_agent->DestroyWME(m_step);
  m_step = 0;
  m_agent->DestroyWME(m_state);
  m_state = 0;

  if(!m_agent->Commit())
    abort();

  if(init_soar)
    m_agent->InitSoar();

  m_state = m_agent->CreateStringWME(m_agent->GetInputLink(), "state", "non-terminal");
  m_step = m_agent->CreateIntWME(m_agent->GetInputLink(), "step", 0);
  m_x = m_agent->CreateFloatWME(m_agent->GetInputLink(), "x", 0.0f);
  m_x_dot = m_agent->CreateFloatWME(m_agent->GetInputLink(), "x-dot", 0.0f);
  m_theta = m_agent->CreateFloatWME(m_agent->GetInputLink(), "theta", 0.0f);
  m_theta_dot = m_agent->CreateFloatWME(m_agent->GetInputLink(), "theta-dot", 0.0f);

  if(!m_agent->Commit())
    abort();
}

bool CartPole::SpawnDebugger() {
  return m_agent->SpawnDebugger(-1, "/home/bazald/Documents/Work/soar-group/SoarSuite/out/SoarJavaDebugger.jar");
}

void CartPole::srand(const int &seed) {
  std::ostringstream oss;
  oss << "srand " << seed;
  ExecuteCommandLine(oss.str());
}

void CartPole::ExecuteCommandLine(const std::string &command) {
  m_agent->ExecuteCommandLine(command.c_str());
}

void CartPole::update() {
  // Go through all the commands we've received (if any) since we last ran Soar.
  const int num_commands = m_agent->GetNumberCommands();

  for(int i = 0; i < num_commands; ++i) {
    sml::Identifier * const command_ptr = m_agent->GetCommand(i);

    if(!strcmp("move", command_ptr->GetCommandName())) {
      const char * direction_name = command_ptr->GetParameterValue("direction");

      if(!direction_name)
        abort();
      else if(!strcmp("left", direction_name) ||
              !strcmp("right", direction_name))
      {
        if(is_finished()) {
          std::cout << "terminal" << std::endl;
        }
        else {
          // Change the state of the world and generate new input
          const int step = m_step->GetValue() + 1;
          float x = m_x->GetValue();
          float x_dot = m_x_dot->GetValue();
          float theta = m_theta->GetValue();
          float theta_dot = m_theta_dot->GetValue();
          cart_pole(direction_name[0] == 'r', &x, &x_dot, &theta, &theta_dot);

          if(step > 10000 ||
             get_box(x, x_dot, theta, theta_dot) < 0)
          {
//             m_agent->DestroyWME(m_state);
//             m_state = m_agent->CreateStringWME(m_agent->GetInputLink(), "state", "terminal");
            m_state->Update("terminal");
          }
//           m_agent->DestroyWME(m_step);
//           m_step = m_agent->CreateIntWME(m_agent->GetInputLink(), "step", step);
//           m_agent->DestroyWME(m_x);
//           m_x = m_agent->CreateFloatWME(m_agent->GetInputLink(), "x", x);
//           m_agent->DestroyWME(m_x_dot);
//           m_x_dot = m_agent->CreateFloatWME(m_agent->GetInputLink(), "x-dot", x_dot);
//           m_agent->DestroyWME(m_theta);
//           m_theta = m_agent->CreateFloatWME(m_agent->GetInputLink(), "theta", theta);
//           m_agent->DestroyWME(m_theta_dot);
//           m_theta_dot = m_agent->CreateFloatWME(m_agent->GetInputLink(), "theta-dot", theta_dot);
          m_step->Update(step);
          m_x->Update(x);
          m_x_dot->Update(x_dot);
          m_theta->Update(theta);
          m_theta_dot->Update(theta_dot);

          if(m_min_x > x)
            m_min_x = x;
          else if(m_max_x < x)
            m_max_x = x;
          if(m_min_x_dot > x_dot)
            m_min_x_dot = x_dot;
          else if(m_max_x_dot < x_dot)
            m_max_x_dot = x_dot;
          if(m_min_theta > theta)
            m_min_theta = theta;
          else if(m_max_theta < theta)
            m_max_theta = theta;
          if(m_min_theta_dot > theta_dot)
            m_min_theta_dot = theta_dot;
          else if(m_max_theta_dot < theta_dot)
            m_max_theta_dot = theta_dot;
          
          if(is_finished()) {
            static int episode = 0;
            std::cout << "EPISODE " << ++episode
                      << " STEP " << step
                      << " DIR " << (direction_name[0] == 'r' ? 1 : 0)
                      << " X " << x
                      << " X-DOT " << x_dot
                      << " THETA " << theta
                      << " THETA-DOT " << theta_dot
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

/*----------------------------------------------------------------------
    This file contains a simulation of the cart and pole dynamic system and 
 a procedure for learning to balance the pole.  Both are described in 
 Barto, Sutton, and Anderson, "Neuronlike Adaptive Elements That Can Solve
 Difficult Learning Control Problems," IEEE Trans. Syst., Man, Cybern.,
 Vol. SMC-13, pp. 834--846, Sept.--Oct. 1983, and in Sutton, "Temporal
 Aspects of Credit Assignment in Reinforcement Learning", PhD
 Dissertation, Department of Computer and Information Science, University
 of Massachusetts, Amherst, 1984.  The following routines are included:

       main:              controls simulation interations and implements 
                          the learning system.

       cart_and_pole:     the cart and pole dynamics; given action and
                          current state, estimates next state

       get_box:           The cart-pole's state space is divided into 162
                          boxes.  get_box returns the index of the box into
                          which the current state appears.

 These routines were written by Rich Sutton and Chuck Anderson.  Claude Sammut 
 translated parts from Fortran to C.  Please address correspondence to
 sutton@gte.com or anderson@cs.colostate.edu
---------------------------------------
Changes:
  1/93: A bug was found and fixed in the state -> box mapping which resulted 
        in array addressing outside the range of the array.  It's amazing this
        program worked at all before this bug was fixed.  -RSS
----------------------------------------------------------------------*/

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

using namespace std;

double prob_push_right(const double &s) {
  return 1.0 / (1.0 + exp(-max(-50.0, min(s, 50.0))));
}

float pole_random() {
  return float(rand()) / ((1u << 31u) - 1.0f);
}

const int N_BOXES         = 162;         /* Number of disjoint boxes of state space. */
const int ALPHA   = 1000;        /* Learning rate for action weights, w. */
const double BETA    = 0.5;         /* Learning rate for critic weights, v. */
const double GAMMA   = 0.95;        /* Discount factor for critic. */
const double LAMBDAw   = 0.9;         /* Decay rate for w eligibility trace. */
const double LAMBDAv   = 0.8;         /* Decay rate for v eligibility trace. */

const int MAX_FAILURES     = 100;         /* Termination criterion. */
const int MAX_STEPS        = 100000;

typedef float Boxes[N_BOXES];

int pole_main(int, char **)
{
  float x,      /* cart position, meters */
        x_dot,      /* cart velocity */
        theta,      /* pole angle, radians */
        theta_dot;    /* pole angular velocity */
  Boxes   w,      /* vector of action weights */
          v,      /* vector of critic weights */
          e,      /* vector of action weight eligibilities */
          xbar;     /* vector of critic weight eligibilities */
  float p, oldp, rhat, r;
  int box, i, y, steps = 0, failures=0, failed;

  printf("Seed? ");
  scanf("%d",&i);
  srand(i);

  /*--- Initialize action and heuristic critic weights and traces. ---*/
  for (i = 0; i < N_BOXES; i++)
    w[i] = v[i] = xbar[i] = e[i] = 0.0;

  /*--- Starting state is (0 0 0 0) ---*/
  x = x_dot = theta = theta_dot = 0.0;

  /*--- Find box in state space containing start state ---*/
  box = get_box(x, x_dot, theta, theta_dot);

  /*--- Iterate through the action-learn loop. ---*/
  while (steps++ < MAX_STEPS && failures < MAX_FAILURES)
    {
      /*--- Choose action randomly, biased by current weight. ---*/
      y = (pole_random() < prob_push_right(w[box]));

      /*--- Update traces. ---*/
      e[box] += (1.0 - LAMBDAw) * (y - 0.5);
      xbar[box] += (1.0 - LAMBDAv);

      /*--- Remember prediction of failure for current state ---*/
      oldp = v[box];

      /*--- Apply action to the simulated cart-pole ---*/
      cart_pole(y, &x, &x_dot, &theta, &theta_dot);

      /*--- Get box of state space containing the resulting state. ---*/
      box = get_box(x, x_dot, theta, theta_dot);

      if (box < 0)    
  {
    /*--- Failure occurred. ---*/
    failed = 1;
    failures++;
    printf("Trial %d was %d steps.\n", failures, steps);
    steps = 0;

    /*--- Reset state to (0 0 0 0).  Find the box. ---*/
    x = x_dot = theta = theta_dot = 0.0;
    box = get_box(x, x_dot, theta, theta_dot);

    /*--- Reinforcement upon failure is -1. Prediction of failure is 0. ---*/
    r = -1.0;
    p = 0.;
  }
      else
  {
    /*--- Not a failure. ---*/
    failed = 0;

    /*--- Reinforcement is 0. Prediction of failure given by v weight. ---*/
    r = 0;
    p= v[box];
  }

      /*--- Heuristic reinforcement is:   current reinforcement
        + gamma * new failure prediction - previous failure prediction ---*/
      rhat = r + GAMMA * p - oldp;

      for (i = 0; i < N_BOXES; i++)
  {
    /*--- Update all weights. ---*/
    w[i] += ALPHA * rhat * e[i];
    v[i] += BETA * rhat * xbar[i];
    if (v[i] < -1.0)
      v[i] = v[i];

    if (failed)
      {
        /*--- If failure, zero all traces. ---*/
        e[i] = 0.;
        xbar[i] = 0.;
      }
    else
      {
        /*--- Otherwise, update (decay) the traces. ---*/       
        e[i] *= LAMBDAw;
        xbar[i] *= LAMBDAv;
      }
  }

    }
  if (failures == MAX_FAILURES)
    printf("Pole not balanced. Stopping after %d failures.",failures);
  else
    printf("Pole balanced successfully for at least %d steps\n", steps);

  return 0;
}


/*----------------------------------------------------------------------
   cart_pole:  Takes an action (0 or 1) and the current values of the
 four state variables and updates their values by estimating the state
 TAU seconds later.
----------------------------------------------------------------------*/

/*** Parameters for simulation ***/

const double GRAVITY = 9.8;
const double MASSCART = 1.0;
const double MASSPOLE = 0.1;
const double TOTAL_MASS = MASSPOLE + MASSCART;
const double LENGTH = 0.5;      /* actually half the pole's length */
const double POLEMASS_LENGTH = MASSPOLE * LENGTH;
const double FORCE_MAG = 10.0;
const double TAU = 0.02;      /* seconds between state updates */
const double FOURTHIRDS = 1.3333333333333;


void cart_pole(int action, float *x, float *x_dot, float *theta, float *theta_dot)
{
    float xacc,thetaacc,force,costheta,sintheta,temp;

    force = (action>0)? FORCE_MAG : -FORCE_MAG;
    costheta = cos(double(*theta));
    sintheta = sin(double(*theta));

    temp = (force + POLEMASS_LENGTH * *theta_dot * *theta_dot * sintheta)
             / TOTAL_MASS;

    thetaacc = (GRAVITY * sintheta - costheta* temp)
         / (LENGTH * (FOURTHIRDS - MASSPOLE * costheta * costheta
                                              / TOTAL_MASS));

    xacc  = temp - POLEMASS_LENGTH * thetaacc* costheta / TOTAL_MASS;

/*** Update the four state variables, using Euler's method. ***/

    *x  += TAU * *x_dot;
    *x_dot += TAU * xacc;
    *theta += TAU * *theta_dot;
    *theta_dot += TAU * thetaacc;
}

/*----------------------------------------------------------------------
   get_box:  Given the current state, returns a number from 1 to 162
  designating the region of the state space encompassing the current state.
  Returns a value of -1 if a failure state is encountered.
----------------------------------------------------------------------*/

const double one_degree = 0.0174532;  /* 2pi/360 */
const double six_degrees = 0.1047192;
const double twelve_degrees = 0.2094384;
const double fifty_degrees = 0.87266;

int get_box(float x, float x_dot, float theta, float theta_dot)
{
  int box=0;

  if (x < -2.4 ||
      x > 2.4  ||
      theta < -twelve_degrees ||
      theta > twelve_degrees)          return(-1); /* to signal failure */

  if (x < -0.8)            box = 0;
  else if (x < 0.8)              box = 1;
  else                         box = 2;

  if (x_dot < -0.5)            ;
  else if (x_dot < 0.5)                box += 3;
  else                     box += 6;

  if (theta < -six_degrees)          ;
  else if (theta < -one_degree)        box += 9;
  else if (theta < 0)            box += 18;
  else if (theta < one_degree)         box += 27;
  else if (theta < six_degrees)        box += 36;
  else                   box += 45;

  if (theta_dot < -fifty_degrees)   ;
  else if (theta_dot < fifty_degrees)  box += 54;
  else                                 box += 108;

  return(box);
}

/*----------------------------------------------------------------------
  Result of:  cc -o pole pole.c -lm          (assuming this file is pole.c)
              pole
----------------------------------------------------------------------*/
/*  
Trial 1 was 21 steps.
Trial 2 was 12 steps.
Trial 3 was 28 steps.
Trial 4 was 44 steps.
Trial 5 was 15 steps.
Trial 6 was 9 steps.
Trial 7 was 10 steps.
Trial 8 was 16 steps.
Trial 9 was 59 steps.
Trial 10 was 25 steps.
Trial 11 was 86 steps.
Trial 12 was 118 steps.
Trial 13 was 218 steps.
Trial 14 was 290 steps.
Trial 15 was 19 steps.
Trial 16 was 180 steps.
Trial 17 was 109 steps.
Trial 18 was 38 steps.
Trial 19 was 13 steps.
Trial 20 was 144 steps.
Trial 21 was 41 steps.
Trial 22 was 323 steps.
Trial 23 was 172 steps.
Trial 24 was 33 steps.
Trial 25 was 1166 steps.
Trial 26 was 905 steps.
Trial 27 was 874 steps.
Trial 28 was 758 steps.
Trial 29 was 758 steps.
Trial 30 was 756 steps.
Trial 31 was 165 steps.
Trial 32 was 176 steps.
Trial 33 was 216 steps.
Trial 34 was 176 steps.
Trial 35 was 185 steps.
Trial 36 was 368 steps.
Trial 37 was 274 steps.
Trial 38 was 323 steps.
Trial 39 was 244 steps.
Trial 40 was 352 steps.
Trial 41 was 366 steps.
Trial 42 was 622 steps.
Trial 43 was 236 steps.
Trial 44 was 241 steps.
Trial 45 was 245 steps.
Trial 46 was 250 steps.
Trial 47 was 346 steps.
Trial 48 was 384 steps.
Trial 49 was 961 steps.
Trial 50 was 526 steps.
Trial 51 was 500 steps.
Trial 52 was 321 steps.
Trial 53 was 455 steps.
Trial 54 was 646 steps.
Trial 55 was 1579 steps.
Trial 56 was 1131 steps.
Trial 57 was 1055 steps.
Trial 58 was 967 steps.
Trial 59 was 1061 steps.
Trial 60 was 1009 steps.
Trial 61 was 1050 steps.
Trial 62 was 4815 steps.
Trial 63 was 863 steps.
Trial 64 was 9748 steps.
Trial 65 was 14073 steps.
Trial 66 was 9697 steps.
Trial 67 was 16815 steps.
Trial 68 was 21896 steps.
Trial 69 was 11566 steps.
Trial 70 was 22968 steps.
Trial 71 was 17811 steps.
Trial 72 was 11580 steps.
Trial 73 was 16805 steps.
Trial 74 was 16825 steps.
Trial 75 was 16872 steps.
Trial 76 was 16827 steps.
Trial 77 was 9777 steps.
Trial 78 was 19185 steps.
Trial 79 was 98799 steps.
Pole balanced successfully for at least 100001 steps
*/

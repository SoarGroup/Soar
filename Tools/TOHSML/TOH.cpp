/*
 * C++ Towers of Hanoi
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * Hopefully a "good" C++ version of the TOH_Towers of Hanoi example, 
 * taken primarily from the JavaTOH code, but some information
 * taken from SML Quick Start Guide.doc and JavaTOHTutorial.doc
 */

#include "TOH_Game.inl"

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

  ++arg;

  if(strcmp(*arg, "--ip") &&
     strcmp(*arg, "--port"))
  {
    const std::string ip_port = *arg;
    const size_t colon = ip_port.find(':');

	if(colon != std::string::npos) {
      ip_address = ip_port.substr(0, colon);
      port = atoi(*arg + (colon + 1));
    }
    else
      ip_address = ip_port;
  }

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

  port = atoi(*arg);

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

  for(char **arg = argv + 1, **arg_end = argv + argc; arg != arg_end; ++arg) {
    if(!arg_remote(remote, ip_address, port, arg, arg_end) &&
       !arg_ip    (remote, ip_address,       arg, arg_end) &&
       !arg_port  (remote,             port, arg, arg_end))
    {
      std::cerr << "Unrecognized argument: " << *arg;
      exit(1);
    }
  }

  if(remote)
    TOH_Game::remote_trials(3, ip_address, port);
  else
    TOH_Game::run_trials(3);

  return 0;
}

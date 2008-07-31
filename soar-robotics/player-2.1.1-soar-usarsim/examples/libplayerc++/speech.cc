#include <libplayerc++/playerc++.h>
#include "args.h"
#include <iostream>
#include <unistd.h>
int main(int argc, char** argv)
{
  parse_args(argc, argv);

  try
  {
    PlayerCc::PlayerClient client(gHostname, gPort);
    PlayerCc::SpeechProxy sp(&client, gIndex);

    sp.Say("All the world is a stage\n");
    usleep(1000000);
    sp.Say("And all the men and women merely players\n");
    usleep(1000000);
    sp.Say("They have their exits and their entrances\n");
    usleep(1000000);
    sp.Say("And one man in his time plays many parts\n");
    usleep(1000000);
  }
  catch (PlayerCc::PlayerError e)
  {
    std::cerr << e << std::endl;
    return -1;
  }
  return 1;
}

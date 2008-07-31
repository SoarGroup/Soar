#include <libplayerc++/playerc++.h>
#include "args.h"
#include <iostream>

int main(int argc, char** argv)
{
  parse_args(argc, argv);

  try
  {
    PlayerCc::PlayerClient client(gHostname, gPort);
    PlayerCc::CameraProxy cp(&client, gIndex);

    for (uint32_t i=0; i<10; ++i)
    {
      client.Read();
      cp.SaveFrame("camera");
      std::cout << cp << std::endl;
    }

  }
  catch (PlayerCc::PlayerError e)
  {
    std::cerr << e << std::endl;
    return -1;
  }
  return 1;
}

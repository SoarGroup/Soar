#include <libplayerc++/playerc++.h>
#include <iostream>
#include <list>

int main(int argc, char** argv)
{
  try
  {
    using namespace PlayerCc;
    // Let's subscribe to a couple of different clients, and
    // set up a proxy or two.
    PlayerClient client1("feyd");
    CameraProxy  cp1(&client1);

    PlayerClient client2("rabban");
    CameraProxy  cp2(&client2);

    // We can now create a list of pointers to PlayerClient,
    // and add two elements
    std::list<PlayerClient*> m_client;
    m_client.push_back(&client1);
    m_client.push_back(&client2);

    while (1)
    {
      // this will now iterate through the list of clients and read from
      // all of them that have data waiting
      std::for_each(m_client.begin(),
                    m_client.end(),
                    std::mem_fun(&PlayerClient::ReadIfWaiting));

      // output the proxies just for fun
      std::cout << cp1 << std::endl;
      std::cout << cp2 << std::endl;
    }

  }
  catch (PlayerCc::PlayerError e)
  {
    std::cerr << e << std::endl;
    return -1;
  }
  return 1;
}

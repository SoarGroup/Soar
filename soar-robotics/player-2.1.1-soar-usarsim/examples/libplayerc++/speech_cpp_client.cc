#include <libplayerc++/playerc++.h>
#include <iostream>
#include <unistd.h>

using namespace PlayerCc;

template<typename T>
void
Print(T t)
{
	  std::cout << *t << std::endl;
}

int main(int argc, char** argv)
{
  try
  {
    PlayerCc::PlayerClient client("127.0.0.1", 6665);
    PlayerCc::SpeechRecognitionProxy srp(&client, 0);
    srp.ConnectReadSignal(boost::bind(&Print<SpeechRecognitionProxy*>, &srp));
    for(;;){
      client.Read();
      timespec sleep = {0, 200000000}; // 200 ms
      nanosleep(&sleep, NULL);
    }
  }
  catch (PlayerCc::PlayerError e)
  {
    std::cerr << e << std::endl;
    return -1;
  }
  return 1;
}

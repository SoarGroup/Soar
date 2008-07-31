#include <iostream>
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <libplayerc++/playerc++.h>

// we have a basic command line parser here
#include "args.h"

// These are our callback functions.
// Currently, they all must return void.
void cb1()
  { std::cout << "cb1" << std::endl; }

// Callbacks can also be passed parameters
void cb2(uint32_t &aI)
  { std::cout << "cb2 " << ++aI << std::endl; }

// we can also have callbacks in objects
class TestCb
{
    int mId;

  public:

    TestCb(int aId) : mId(aId) {};

    // We'll output the ID of the TestCb, so we can see
    // which object is being called.
    void Cb()
      { std::cout << "TestCb " << mId << std::endl; }

    void Cb(uint32_t aOpt)
      { std::cout << "TestCb " << mId << " " << aOpt << std::endl; }

};

// we'll use this to stop the client
void stop_cb(PlayerCc::PlayerClient* c, uint32_t &i)
{
  // after 10 iterations, stop the client
  if (++i>10)
  {
    std::cout << "stop: " << i << std::endl;
    c->Stop();
  }
}

int main(int argc, char** argv)
{
  parse_args(argc, argv);
  
  // it's always good to put the code in a try block
  // libplayerc++ throws a PlayerError exception when
  // it runs into trouble
  try
  {
    // let's setup a client
    // by default PlayerClient uses localhost and 6665
    PlayerCc::PlayerClient client(gHostname, gPort);
    PlayerCc::CameraProxy  cp(&client, 0);

    // Here, we're connecting a signal to a function.
    // We keep the connection_t so we can later disconnect.
    PlayerCc::ClientProxy::connection_t conn;
    conn = cp.ConnectReadSignal(&cb1);

    // Signals can also be connected without storing the connection_t,
    // but you can no longer disconnect them.
    // In order to use a callback with a argument, you need to bind
    // the argument to the callback.
    uint32_t count = 0;
    cp.ConnectReadSignal(boost::bind(&cb2, count));

    // here we're connecting a signal to a member function
    TestCb test1(1), test2(2);
    // just like functions, member functions can also be bound
    // with or without arguments
    cp.ConnectReadSignal(boost::bind(&TestCb::Cb, boost::ref(test1)));
    cp.ConnectReadSignal(boost::bind(&TestCb::Cb, boost::ref(test2), 1));

    // now, we should see some signals each time Read() is called for the
    // object that receives the data

    // libplayerc++ has three different ways for handling messages from the
    // server.  The first is by manually calling Read() each time we would
    // like to process data.
    std::cout << "Read()" << std::endl;
    for (uint32_t i=0; i<10; ++i)
    {
      client.Read();
      // an example of disconnecting a signal
      if (4==i)
        cp.DisconnectReadSignal(conn);
    }

    // The PlayerClient can also be used as a messaging loop through the use of
    // a blocking Run() method.  The correct way to tell Run() to quit is
    // through a stop callback.  In this case, we only have a single thread,
    // so we don't have to worry about multithreading
    std::cout << "Run()" << std::endl;

    // Let's connect our stop_cb() signal.  This signal tells the client
    // to exit after 10 iterations
    uint32_t i = 0;
    conn = cp.ConnectReadSignal(boost::bind(&stop_cb, &client, i));

    // Now, let's run the client.  This exits when the client->Stop() function
    // is called from the callback.
    client.Run();

    // We can also access the client in a multithreaded fashion.
    std::cout << "StartThread()" << std::endl;

    // let's first disconnect our previous stop
    cp.DisconnectReadSignal(conn);

    // Start the thread, which will handle all message processing.
    client.StartThread();

    // Sleep for 5 seconds.  During this time, callbacks should be
    // fired on every read and output should show up on the display.
    timespec sleep = {5, 0};
    nanosleep(&sleep, NULL);

    // Instead of sleeping here, we could also be sending commands and reading
    // directly from the proxy.
    for (uint32_t j=0; j<10; ++j)
    {
      cp.SaveFrame("test");
      // all proxies have a iostream operator
      std::cout << cp << std::endl;
    }

    // Now, let's stop the thread.  This function only returns after
    // the thread has been stopped
    client.StopThread();

    std::cout << "finished!" << std::endl;
  }
  catch (PlayerCc::PlayerError e)
  {
    // let's output the error
    std::cerr << e << std::endl;
    return -1;
  }
  return 1;
}

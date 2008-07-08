#ifndef SOAR_PLAYER_CLIENT_H
#define SOAR_PLAYER_CLIENT_H

#include "sml_Client.h"
#include "RunThread.h"
#include "SoarRunThreadInterface.h"

#include <libplayerc++/playerc++.h>

class SoarPlayerClient: public SoarRunThreadInterface
{
public:
    SoarPlayerClient( sml::Kernel* kernel, sml::Agent* agent );

    virtual std::string command_run();
    virtual std::string command_stop();
    virtual std::string command_step();
    virtual std::string command_debug();
    virtual std::string command_reset();
    
    void update();
    
private:
    bool update_and_check_running();
    
    sml::Kernel* m_kernel;
    sml::Agent* m_agent;
    
    PlayerCc::PlayerClient    m_robot;
    PlayerCc::SonarProxy      m_sp;
    PlayerCc::Position2dProxy m_pp;    

    RunThread* m_run_thread;
    
    bool m_stop_issued;
};

#endif // SOAR_PLAYER_CLIENT_H

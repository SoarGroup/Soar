#ifndef SOAR_PLAYER_CLIENT_H
#define SOAR_PLAYER_CLIENT_H

#include "sml_Client.h"
#include "RunThread.h"
#include "SoarRunThreadInterface.h"

#include <libplayerc++/playerc++.h>
#include <string>

class SoarPlayerClient: public SoarRunThreadInterface
{
public:
    SoarPlayerClient( const std::string& productions );
    virtual ~SoarPlayerClient();
    
    virtual std::string command_run();
    virtual std::string command_stop();
    virtual std::string command_step();
    virtual std::string command_debug();
    virtual std::string command_reset();
    virtual std::string command_reload();
    
    void update();
    
private:
    bool update_and_check_running();
    bool reload_productions();

    std::string m_productions;
    sml::Kernel* m_kernel;
    sml::Agent* m_agent;
    
    PlayerCc::PlayerClient    m_robot;
    PlayerCc::SonarProxy      m_sp;
    PlayerCc::Position2dProxy m_pp;    

    RunThread* m_run_thread;
    
    bool m_stop_issued;
};

#endif // SOAR_PLAYER_CLIENT_H

#ifndef SOAR_PLAYER_CLIENT_H
#define SOAR_PLAYER_CLIENT_H

#include "sml_Client.h"
#include "RunThread.h"
#include "SoarRunThreadInterface.h"

#include <libplayerc++/playerc++.h>
#include <string>

class InputLinkManager;
class OutputLinkManager;

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
    void agent_event( sml::smlAgentEventId id );
    
private:
    bool update_and_check_running();
    bool reload_productions();

    std::string m_productions;
    sml::Kernel* m_kernel;
    sml::Agent* m_agent;
    
    InputLinkManager* m_input_link;
    OutputLinkManager* m_output_link;
    
    PlayerCc::PlayerClient    m_robot;
    PlayerCc::Position2dProxy m_pp;    
    PlayerCc::FiducialProxy   m_fp;    
    PlayerCc::LaserProxy      m_lp;    
    PlayerCc::GripperProxy    m_gp;    

    RunThread* m_run_thread;
    
    bool m_stop_issued;
};

#endif // SOAR_PLAYER_CLIENT_H

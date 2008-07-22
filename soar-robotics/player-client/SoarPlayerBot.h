#ifndef SOAR_PLAYER_BOT_H
#define SOAR_PLAYER_BOT_H

#include "sml_Client.h"
#include "RunThread.h"
#include "SoarRunThreadInterface.h"

#include <libplayerc++/playerc++.h>
#include <string>

class InputLinkManager;
class OutputLinkManager;

class SoarPlayerBot
{
public:
    SoarPlayerBot( int port, sml::Agent& agent, const std::string& productions );
    ~SoarPlayerBot();
    
    void reload_productions();
    void reset();
    void update();
    void clear_io_links();
    void create_io_links();
    
private:
    PlayerCc::PlayerClient    m_robot;
    PlayerCc::Position2dProxy m_pp;    
    PlayerCc::FiducialProxy   m_fp;    
    PlayerCc::LaserProxy      m_lp;    
    PlayerCc::GripperProxy    m_gp;    

    InputLinkManager* m_input_link;
    OutputLinkManager* m_output_link;
    
    //bool m_move_to;
    player_pose2d m_move_to_destination;
    
    std::string m_productions;
    sml::Agent& m_agent;
    
    static const double MOVE_TO_TOLERANCE;
};

#endif // SOAR_PLAYER_BOT_H

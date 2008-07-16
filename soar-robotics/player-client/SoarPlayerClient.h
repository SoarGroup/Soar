#ifndef SOAR_PLAYER_CLIENT_H
#define SOAR_PLAYER_CLIENT_H

#include "sml_Client.h"
#include "RunThread.h"
#include "SoarRunThreadInterface.h"

#include <libplayerc++/playerc++.h>
#include <string>

class InputLinkManager;
class OutputLinkManager;
class SoarPlayerBot;

class SoarPlayerClient: public SoarRunThreadInterface
{
public:
    SoarPlayerClient( const std::string& productions, int number_of_bots );
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

    sml::Kernel* m_kernel;
    
	int m_bot_count;	
	std::vector< SoarPlayerBot* > m_bot_list;
    
    RunThread* m_run_thread;
    
    bool m_stop_issued;
};

#endif // SOAR_PLAYER_CLIENT_H

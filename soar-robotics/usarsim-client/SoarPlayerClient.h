#ifndef SOAR_PLAYER_CLIENT_H
#define SOAR_PLAYER_CLIENT_H

#include "sml_Client.h"
#include "SoarRunThreadInterface.h"
#include "ConfigFile.h"

#include <libplayerc++/playerc++.h>
#include <string>
#include <exception>

class InputLinkManager;
class OutputLinkManager;
class SoarPlayerBot;
class RunThread;

class SoarPlayerClient: public SoarRunThreadInterface
{
public:
    SoarPlayerClient( ConfigFile& config );
    virtual ~SoarPlayerClient();
    
    virtual std::string command_run();
    virtual std::string command_stop();
    virtual std::string command_step();
    virtual std::string command_debug();
    virtual std::string command_reset();
    virtual std::string command_reload();
    virtual std::string command_output( const std::string& command );
    
    void update();
    void agent_event( sml::smlAgentEventId id );
    
private:
    bool update_and_check_running();

	ConfigFile& m_config;
    sml::Kernel* m_kernel;
    
	int m_bot_count;	
	std::vector< SoarPlayerBot* > m_bot_list;
    
    RunThread* m_run_thread;
    
    bool m_stop_issued;
    
public:
	struct soar_error: public std::exception
	{
		const char* message;
		soar_error( const char* error_message )
		: message( error_message ) {}
		virtual const char* what() { return message; }
	};
};

#endif // SOAR_PLAYER_CLIENT_H

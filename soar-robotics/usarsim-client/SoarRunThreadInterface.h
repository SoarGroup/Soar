#ifndef SOAR_RUN_THREAD_INTERFACE_H
#define SOAR_RUN_THREAD_INTERFACE_H

#include <string>

class SoarRunThreadInterface
{
public:
    virtual ~SoarRunThreadInterface() {};

    virtual std::string command_run() = 0;
    virtual std::string command_stop() = 0;
    virtual std::string command_step() = 0;
    virtual std::string command_debug() = 0;
    virtual std::string command_reset() = 0;
    virtual std::string command_reload() = 0;
    virtual std::string command_output( const std::string& command ) =0;
};

#endif // SOAR_RUN_THREAD_INTERFACE_H


#ifndef CONSOLE_H
#define CONSOLE_H

#include "SoarRunThreadInterface.h"

#include <map>

class Console
{
public:
    Console( SoarRunThreadInterface& client );

    int run();

private:
    SoarRunThreadInterface& m_client;
};

#endif // CONSOLE_H


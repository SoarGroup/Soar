#ifndef DRAWER_H
#define DRAWER_H

/*
 A class that interfaces with the viewer through TCP sockets.
*/

#include <string>

class sgnode;
class ipcsocket;

class drawer
{
    public:
        enum change_type
        {
            POS   = 1,
            ROT   = 1 << 2,
            SCALE = 1 << 3,
            COLOR = 1 << 4,
            SHAPE = 1 << 5,
        };
        
        drawer();
        ~drawer();
        
        bool connect(const std::string& addr);
        void disconnect();
        void add(const std::string& scn, const sgnode* n);
        void del(const std::string& scn, const sgnode* n);
        void change(const std::string& scn, const sgnode* n, int props);
        void delete_scene(const std::string& scn);
        void send(const std::string& s);
        
    private:
        bool connected;
        ipcsocket* sock;
};

#endif

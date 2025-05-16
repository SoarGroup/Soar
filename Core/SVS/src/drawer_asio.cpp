#include <iostream>
#include <cstdlib>
#include <asio.hpp>
#include "drawer.h"
#include "common.h"
#include "sgnode.h"

/**** 
 * Note: This is a new implementation of the drawer socket that uses the asio library
 *          it will be used in the cmake build process, replacing the old platform_specific code 
 ****/

std::ostream& write_vec3(std::ostream& os, const vec3& v)
{
    os << v(0) << " " << v(1) << " " << v(2);
    return os;
}

class ipcsocket
{
    public:
        ipcsocket() : socket(io_context), is_connected(false) {}

        ~ipcsocket()
        {
            disconnect();
        }

        bool connect(const std::string& port)
        {
            if (is_connected) {
                disconnect();
            }
            try {
                asio::ip::tcp::resolver resolver(io_context);
                auto endpoints = resolver.resolve("127.0.0.1", port);

                asio::connect(socket, endpoints);
                is_connected = true;
            } catch (std::exception& e) {
                // ignore
            }
            return is_connected;
        }

        void disconnect()
        {
            if (!is_connected) {
                return;
            }
            try {
                socket.close();
            } catch (std::exception& e) {
                // ignore
            }
            is_connected = false;
        }

        bool send(const std::string& s)
        {
            if (!is_connected) {
                return false;
            }
            try {
                asio::write(socket, asio::buffer(s));
                return true;
            } catch (std::exception& e) {
                is_connected = false;
            }
            return false;
        }

    private:
        asio::io_context io_context;
        asio::ip::tcp::socket socket;
        bool is_connected;
};

drawer::drawer() : connected(false)
{
    sock = new ipcsocket();
}

drawer::~drawer()
{
    delete sock;
}

bool drawer::connect(const std::string& path)
{
    connected = sock->connect(path);
    return connected;
}

void drawer::disconnect()
{
    if (connected)
    {
        sock->disconnect();
    }
    connected = false;
}

void drawer::add(const std::string& scn, const sgnode* n)
{
    if (!connected || !n->get_parent())
    {
        return;
    }
    change(scn, n, SHAPE | POS | ROT | SCALE);
}

void drawer::del(const std::string& scn, const sgnode* n)
{
    if (!connected)
    {
        return;
    }

    std::stringstream ss;
    ss << scn << " -" << n->get_id() << std::endl;
    send(ss.str());
}

void drawer::change(const std::string& scn, const sgnode* n, int props)
{
    if (!connected)
    {
        return;
    }

    vec3 p, s;
    vec4 q;
    std::stringstream ss;

    n->get_world_trans().to_prs(p, q, s);
    ss << "+" << scn << " +" << n->get_id() << " ";
    if (props & SHAPE)
    {
        std::string shape;
        n->get_shape_sgel(shape);
        ss << " " << shape << " ";
    }
    if (props & POS)
    {
        ss << " p ";
        write_vec3(ss, p);
    }
    if (props & ROT)
    {
        ss << " r " << q(0) << " " << q(1) << " " << q(2) << " " << q(3) << " ";
    }
    if (props & SCALE)
    {
        ss << " s ";
        write_vec3(ss, s);
    }
    ss << std::endl;
    send(ss.str());
}

void drawer::delete_scene(const std::string& scn)
{
    if (!connected)
    {
        return;
    }

    send(std::string("-") + scn + "\n");
}

void drawer::send(const std::string& s)
{
    if (!connected)
    {
        return;
    }
    if (s[s.size() - 1] != '\n')
    {
        connected = sock->send(s + '\n');
    }
    else
    {
        connected = sock->send(s);
    }
}

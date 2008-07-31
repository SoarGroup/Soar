/*
 * goto.cc - a simple (and bad) goto program
 *
 * but, it demonstrates one multi-threaded structure
 *
 * @todo: this has been ported to libplayerc++, but not tested AT ALL
 */

#include <libplayerc++/playerc++.h>

#include <iostream>
#include <stdlib.h> // for atof()
#include <unistd.h>

#include <math.h>
#include <string>

#define USAGE \
  "USAGE: goto [-x <x>] [-y <y>] [-h <host>] [-p <port>] [-m]\n" \
  "       -x <x>: set the X coordinate of the target to <x>\n"\
  "       -y <y>: set the Y coordinate of the target to <y>\n"\
  "       -h <host>: connect to Player on this host\n" \
  "       -p <port>: connect to Player on this TCP port\n" \
  "       -m       : turn on motors (be CAREFUL!)"

PlayerCc::PlayerClient* robot;
PlayerCc::Position2dProxy* pp;
PlayerCc::SonarProxy* sp;

bool         gMotorEnable(false);
bool         gGotoDone(false);
std::string  gHostname(PlayerCc::PLAYER_HOSTNAME);
uint32_t        gPort(PlayerCc::PLAYER_PORTNUM);
uint32_t        gIndex(0);
uint32_t        gDebug(0);
uint32_t        gFrequency(10); // Hz

player_pose2d_t gTarget = {0, 0, 0};

void
print_usage(int argc, char** argv)
{
  std::cout << USAGE << std::endl;
}

int
parse_args(int argc, char** argv)
{
  const char* optflags = "h:p:i:d:u:x:y:m";
  int ch;

  while(-1 != (ch = getopt(argc, argv, optflags)))
  {
    switch(ch)
    {
      /* case values must match long_options */
      case 'h':
          gHostname = optarg;
          break;
      case 'p':
          gPort = atoi(optarg);
          break;
      case 'i':
          gIndex = atoi(optarg);
          break;
      case 'd':
          gDebug = atoi(optarg);
          break;
      case 'u':
          gFrequency = atoi(optarg);
          break;
      case 'x':
          gTarget.px = atof(optarg);
          break;
      case 'y':
          gTarget.py = atof(optarg);
          break;
      case 'm':
          gMotorEnable = true;
          break;
      case '?':
      case ':':
      default:
        print_usage(argc, argv);
        return (-1);
    }
  }

  return (0);
}

/*
 * very bad goto.  target is arg (as pos_t*)
 *
 * sets global 'gGotoDone' when it's done
 */
void
position_goto(player_pose2d_t target)
{
  using namespace PlayerCc;

  double dist, angle;

  dist = sqrt((target.px - pp->GetXPos())*
              (target.px - pp->GetXPos()) +
              (target.py - pp->GetYPos())*
              (target.py - pp->GetYPos()));

  angle = atan2(target.py - pp->GetYPos(),
                target.px - pp->GetXPos());

  double newturnrate = 0;
  double newspeed = 0;

  if (fabs(rtod(angle)) > 10.0)
  {
    newturnrate = limit((angle/M_PI) * 40.0, -40.0, 40.0);
    newturnrate = dtor(newturnrate);
  }
  else
    newturnrate = 0.0;

  if (dist > 0.05)
  {
    newspeed = limit(dist * 0.200, -0.2, 0.2);
  }
  else
    newspeed = 0.0;

  if (fabs(newspeed) < 0.01)
    gGotoDone = true;

  pp->SetSpeed(newspeed, newturnrate);

}

/*
 * sonar avoid.
 *   policy:
 *     if(object really close in front)
 *       backup and turn away;
 *     else if(object close in front)
 *       stop and turn away
 */
void
sonar_avoid(void)
{
  double min_front_dist = 0.500;
  double really_min_front_dist = 0.300;
  bool avoid = false;

  double newturnrate = 10.0;
  double newspeed = 10.0;

  if ((sp->GetScan(2) < really_min_front_dist) ||
      (sp->GetScan(3) < really_min_front_dist) ||
      (sp->GetScan(4) < really_min_front_dist) ||
      (sp->GetScan(5) < really_min_front_dist))
  {
    avoid = true;
    std::cerr << "really avoiding" << std::endl;
    newspeed = -0.100;
  }
  else if((sp->GetScan(2) < min_front_dist) ||
          (sp->GetScan(3) < min_front_dist) ||
          (sp->GetScan(4) < min_front_dist) ||
          (sp->GetScan(5) < min_front_dist))
  {
    avoid = true;
    std::cerr << "avoiding" << std::endl;
    newspeed = 0;
  }

  if(avoid)
  {
    if ((sp->GetScan(0) + sp->GetScan(1)) < (sp->GetScan(6) + sp->GetScan(7)))
      newturnrate = PlayerCc::dtor(30);
    else
      newturnrate = PlayerCc::dtor(-30);
  }

  if(newspeed < 10.0 && newturnrate < 10.0)
    pp->SetSpeed(newspeed, newturnrate);
}

template<typename T>
void
Print(T t)
{
  std::cout << *t << std::endl;
}

int
main(int argc, char **argv)
{
  try
  {
    using namespace PlayerCc;

    parse_args(argc,argv);

    // Connect to Player server
    robot = new PlayerClient(gHostname, gPort);

    // Request sensor data
    pp = new Position2dProxy(robot, gIndex);
    sp = new SonarProxy(robot, gIndex);

    if(gMotorEnable)
      pp->SetMotorEnable(true);

    // output the data
    pp->ConnectReadSignal(boost::bind(&Print<Position2dProxy*>, pp));
    sp->ConnectReadSignal(boost::bind(&Print<SonarProxy*>, sp));

    // callback to avoid obstacles whenever we have new sonar data
    sp->ConnectReadSignal(&sonar_avoid);

    // start the read thread
    robot->StartThread();

    std::cout << "goto starting, target: " << gTarget.px
              << ", " << gTarget.py << std::endl;

    for (;;)
    {
      position_goto(gTarget);
      if (gGotoDone)
        robot->StopThread();

      timespec sleep = {0, 100000000}; // 100 ms
      nanosleep(&sleep, NULL);
    }
  }
  catch (PlayerCc::PlayerError e)
  {
    std::cerr << e << std::endl;
    return -1;
  }

  return(0);
}


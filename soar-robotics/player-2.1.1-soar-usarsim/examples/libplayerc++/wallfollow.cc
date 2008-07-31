#include <iostream>
#include <libplayerc++/playerc++.h>

const uint32_t WALL_FOLLOWING = 0;
const uint32_t COLLISION_AVOIDANCE = 1;

// parameters
const double VEL       = 0.3; // normal_advance_speed
const double K_P       = 1000; // kp_wall_following
const double DIST      = 0.5; // preferred_wall_following_distance
const double TURN_RATE = 30; // maximal_wall_following_turnrate
const int FOV          = 45; // collision_avoidance_fov
const double STOP_DIST = 0.6; // stop_distance
const double STOP_ROT  = 50; // stop_rotation_speed

int
main(int argc, char *argv[])
{
  using namespace PlayerCc;

  PlayerClient    robot("localhost");
  LaserProxy      lp(&robot,0);
  Position2dProxy pp(&robot,0);

  for (;;)
  {
    double speed = VEL;
    double turnrate;
    bool escape_direction;
    uint32_t previous_mode = WALL_FOLLOWING;

    // read from the proxies
    robot.Read();

    std::cout << "Left: "   << lp[179]
              << " Front: " << lp[90]
              << " Right: " << lp[0]
              << std::endl;

    // do simple wall following
    turnrate = dtor(K_P*(lp[135]-DIST));
    if (turnrate > dtor(TURN_RATE))
      turnrate = dtor(TURN_RATE);

    // avoid collision: find closest range in the collision avoidance field of
    // view, calculate statistical mean to select escape direction
    double min_dist = lp[90 - FOV/2];
    double left_mean = 0;
    double right_mean = 0;
    int left_count = 0;
    int right_count = 0;

    for (int theta = 0; theta < 180; ++theta)
    {
       if (theta < 90)
       {
         left_mean += lp[theta];
         ++left_count;
       }
       else
       {
         right_mean += lp[theta];
         ++right_count;
       }
       if (theta > 90 - FOV/2 &&
           theta < 90 + FOV/2 &&
           lp[theta] < min_dist)
       {
         min_dist = lp[theta];
       }
    }
    // compute mean of each direction
    left_mean /= left_count;
    right_mean /= right_count;

    if (min_dist < STOP_DIST)
    {
      speed = 0;
      // selection of escape direction (done once for each object encounter)
      if (previous_mode == WALL_FOLLOWING)
      {
        // go towards direction with most open space
        escape_direction = left_mean < right_mean;
        // change this so that we know we have chosen the escape direction
        previous_mode = COLLISION_AVOIDANCE;
      }
      
      if (escape_direction) // right turn
        turnrate = dtor(STOP_ROT);
      else // left turn
        turnrate = dtor(-STOP_ROT);
    }
    else
      previous_mode = WALL_FOLLOWING;

    // command the motors
    pp.SetSpeed(speed, turnrate);
  }
}

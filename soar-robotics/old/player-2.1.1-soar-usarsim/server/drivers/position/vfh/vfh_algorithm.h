/*
 *  Orca-Components: Components for robotics.
 *  
 *  Copyright (C) 2004
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef VFH_ALGORITHM_H
#define VFH_ALGORITHM_H

#include <vector>
#include <libplayercore/playercore.h>

class VFH_Algorithm
{
public:
    VFH_Algorithm( double cell_size,
                   int window_diameter,
                   int sector_angle,
                   double safety_dist_0ms,
                   double safety_dist_1ms, 
                   int max_speed,
                   int max_speed_narrow_opening,
                   int max_speed_wide_opening,
                   int max_acceleration,
                   int min_turnrate,
                   int max_turnrate_0ms,
                   int max_turnrate_1ms,
                   double min_turn_radius_safety_factor,
                   double free_space_cutoff_0ms,
                   double obs_cutoff_0ms,
                   double free_space_cutoff_1ms,
                   double obs_cutoff_1ms,
                   double weight_desired_dir,
                   double weight_current_dir );

    ~VFH_Algorithm();

    int Init();
    
    // Choose a new speed and turnrate based on the given laser data and current speed.
    //
    // Units/Senses:
    //  - goal_direction in degrees, 0deg is to the right.
    //  - goal_distance  in mm.
    //  - goal_distance_tolerance in mm.
    //
    int Update_VFH( double laser_ranges[361][2], 
                    int current_speed,  
                    float goal_direction,
                    float goal_distance,
                    float goal_distance_tolerance,
                    int &chosen_speed, 
                    int &chosen_turnrate );

    // Get methods
    int   GetMinTurnrate() { return MIN_TURNRATE; }
    // Angle to goal, in degrees.  0deg is to our right.
    float GetDesiredAngle() { return Desired_Angle; }
    float GetPickedAngle() { return Picked_Angle; }

    // Max Turnrate depends on speed
    int GetMaxTurnrate( int speed );
    int GetCurrentMaxSpeed() { return Current_Max_Speed; }

    // Set methods
    void SetRobotRadius( float robot_radius ) { this->ROBOT_RADIUS = robot_radius; }
    void SetMinTurnrate( int min_turnrate ) { MIN_TURNRATE = min_turnrate; }
    void SetCurrentMaxSpeed( int Current_Max_Speed );

    // The Histogram.
    // This is public so that monitoring tools can get at it; it shouldn't
    // be modified externally.
    // Sweeps in an anti-clockwise direction.
    float *Hist;

private:

    // Functions

    int VFH_Allocate();

    float Delta_Angle(int a1, int a2);
    float Delta_Angle(float a1, float a2);
    int Bisect_Angle(int angle1, int angle2);

    bool Cant_Turn_To_Goal();

    // Returns 0 if something got inside the safety distance, else 1.
    int Calculate_Cells_Mag( double laser_ranges[361][2], int speed );
    // Returns 0 if something got inside the safety distance, else 1.
    int Build_Primary_Polar_Histogram( double laser_ranges[361][2], int speed );
    int Build_Binary_Polar_Histogram(int speed);
    int Build_Masked_Polar_Histogram(int speed);
    int Select_Candidate_Angle();
    int Select_Direction();
    int Set_Motion( int &speed, int &turnrate, int current_speed );

    // AB: This doesn't seem to be implemented anywhere...
    // int Read_Min_Turning_Radius_From_File(char *filename);

    void Print_Cells_Dir();
    void Print_Cells_Mag();
    void Print_Cells_Dist();
    void Print_Cells_Sector();
    void Print_Cells_Enlargement_Angle();
    void Print_Hist();

    // Returns the speed index into Cell_Sector, for a given speed in mm/sec.
    // This exists so that only a few (potentially large) Cell_Sector tables must be stored.
    int Get_Speed_Index( int speed );

    // Returns the safety dist in mm for this speed.
    int Get_Safety_Dist( int speed );

    float Get_Binary_Hist_Low( int speed );
    float Get_Binary_Hist_High( int speed );

    // Data

    float ROBOT_RADIUS;           // millimeters
    int CENTER_X;                 // cells
    int CENTER_Y;                 // cells
    int HIST_SIZE;                // sectors (over 360deg)

    float CELL_WIDTH;             // millimeters
    int WINDOW_DIAMETER;          // cells
    int SECTOR_ANGLE;             // degrees
    float SAFETY_DIST_0MS;        // millimeters
    float SAFETY_DIST_1MS;        // millimeters
    int Current_Max_Speed;        // mm/sec
    int MAX_SPEED;                // mm/sec
    int MAX_SPEED_NARROW_OPENING; // mm/sec
    int MAX_SPEED_WIDE_OPENING;   // mm/sec
    int MAX_ACCELERATION;         // mm/sec/sec
    int MIN_TURNRATE;             // deg/sec -- not actually used internally

    int NUM_CELL_SECTOR_TABLES;

    // Scale turnrate linearly between these two
    int MAX_TURNRATE_0MS;       // deg/sec
    int MAX_TURNRATE_1MS;       // deg/sec
    double MIN_TURN_RADIUS_SAFETY_FACTOR;
    float Binary_Hist_Low_0ms, Binary_Hist_High_0ms;
    float Binary_Hist_Low_1ms, Binary_Hist_High_1ms;
    float U1, U2;
    float Desired_Angle, Dist_To_Goal, Goal_Distance_Tolerance;
    float Picked_Angle, Last_Picked_Angle;
    int   Max_Speed_For_Picked_Angle;

    // Radius of dis-allowed circles, either side of the robot, which
    // we can't enter due to our minimum turning radius.
    float Blocked_Circle_Radius;

    std::vector<std::vector<float> > Cell_Direction;
    std::vector<std::vector<float> > Cell_Base_Mag;
    std::vector<std::vector<float> > Cell_Mag;
    std::vector<std::vector<float> > Cell_Dist;      // millimetres
    std::vector<std::vector<float> > Cell_Enlarge;

    // Cell_Sector[x][y] is a vector of indices to sectors that are effected if cell (x,y) contains
    // an obstacle.  
    // Cell enlargement is taken into account.
    // Acess as: Cell_Sector[speed_index][x][y][sector_index]
    std::vector<std::vector<std::vector<std::vector<int> > > > Cell_Sector;
    std::vector<float> Candidate_Angle;
    std::vector<int> Candidate_Speed;

    double dist_eps;
    double ang_eps;

    float *Last_Binary_Hist;

    // Minimum turning radius at different speeds, in millimeters
    std::vector<int> Min_Turning_Radius;

    // Keep track of last update, so we can monitor acceleration
    timeval last_update_time;

    int last_chosen_speed;
};

#endif

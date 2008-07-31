/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
 *                      
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*  Kinematics calculator class for the Pioneer arm.
 */

#include <stdio.h>

// Basic vector
typedef struct
{
  double x, y, z;
} KineVector;

// Struct that describes the pose of the end effector
// (result of FK)
typedef struct
{
  KineVector p;
  KineVector n;
  KineVector o;
  KineVector a;
} EndEffector;

class KineCalc
{
  public:
    KineCalc (void);

    // Kinematics functions
    void CalculateFK (const double fromJoints[]);
    bool CalculateIK (const EndEffector &fromPosition);

    // Accessor functions
    const KineVector& GetP (void) const   { return endEffector.p; }
    const KineVector& GetN (void) const   { return endEffector.n; }
    const KineVector& GetO (void) const   { return endEffector.o; }
    const KineVector& GetA (void) const   { return endEffector.a; }
    void SetP (const KineVector &newP)    { endEffector.p.x = newP.x; endEffector.p.y = newP.y; endEffector.p.z = newP.z; }
    void SetN (const KineVector &newN)    { endEffector.n.x = newN.x; endEffector.n.y = newN.y; endEffector.n.z = newN.z; }
    void SetO (const KineVector &newO)    { endEffector.o.x = newO.x; endEffector.o.y = newO.y; endEffector.o.z = newO.z; }
    void SetA (const KineVector &newA)    { endEffector.a.x = newA.x; endEffector.a.y = newA.y; endEffector.a.z = newA.z; }
    void SetP (double newPX, double newPY, double newPZ);
    void SetN (double newNX, double newNY, double newNZ);
    void SetO (double newOX, double newOY, double newOZ);
    void SetA (double newAX, double newAY, double newAZ);

    double GetTheta (unsigned int index);
    const double* GetThetas (void) const  { return joints; }
    void SetTheta (unsigned int index, double newVal);
    void SetLinkLengths (double newLink1, double newLink2, double newLink3, double newLink4, double newLink5);
    void SetOffset (unsigned int joint, double newOffset);
    void SetJointRange (unsigned int joint, double min, double max);

    // Use this to calculate N
    KineVector CalculateN (const EndEffector &pose);
    KineVector Normalise (const KineVector &vector);

  protected:
    void CalcTheta4and5 (double angles[], const EndEffector &fromPosition);
    int ChooseSolution (const EndEffector &fromPosition, const double solutions[][5]);
    double CalcSolutionError (const double solution[], const EndEffector &fromPosition);
    EndEffector CalcFKForJoints (const double angles[]);
    bool SolutionInRange (const double angles[]);

    void PrintEndEffector (const EndEffector &endEffector);

    // The 4 vectors that describe the position and orientation of the
    // end effector.
    // These should be computed when performing forward kinematics
    // and are used to provide data to the client.
    EndEffector endEffector;

    // The 5 joint angles.
    // These are computed when performing inverse kinematics and
    // are used for positioning the arm.
    double joints[5];
    // Joint offsets, as calibrated and supplied in the config file
    double jointOffsets[5];
    // Joint ranges
    double jointMin[5];
    double jointMax[5];

    // The link lengths are required for any of the kinematics to be useful.
    // I can't see them changing, but just in case better to have the ability to
    // specify them in the config file.
    double link1, link2, link3, link4, link5;
};

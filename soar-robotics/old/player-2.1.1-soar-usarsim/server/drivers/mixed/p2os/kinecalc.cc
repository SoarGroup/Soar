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

/*  Kinematics calculator implementation based on the analytical method by
 *  Gan et al. See:
 *  J.Q. Gan, E. Oyama, E.M. Rosales, and H. Hu, "A complete analytical
 *  solution to the inverse kinematics of the Pioneer 2 robotic arm,"
 *  Robotica, vol.23, no.1, pp.123-129, 2005.
 */

#include <libplayercore/playercommon.h>
#include "kinecalc.h"

#include <math.h>

#include <stdio.h>

KineCalc::KineCalc (void)
{
  link1 = 0.06875f;
  link2 = 0.16f;
  link3 = 0.0f;
  link4 = 0.13775f;
  link5 = 0.11321f;

  endEffector.p.x = 0.0f; endEffector.p.y = 0.0f; endEffector.p.z = 0.0f;
  endEffector.n.x = 0.0f; endEffector.n.y = 0.0f; endEffector.n.z = 0.0f;
  endEffector.o.x = 0.0f; endEffector.o.y = -1.0f; endEffector.o.z = 1.0f;
  endEffector.a.x = 1.0f; endEffector.a.y = 0.0f; endEffector.a.z = 0.0f;

  for (int ii = 0; ii < 5; ii++)
  {
    joints[ii] = 0.0f;
    jointOffsets[ii] = 0.0f;
    jointMin[ii] = 0.0f;
    jointMax[ii] = 0.0f;
  }
}


///////////////////////////////////////////////////////////////////////////////
//  Accessor functions
///////////////////////////////////////////////////////////////////////////////

void KineCalc::SetP (double newPX, double newPY, double newPZ)
{
  endEffector.p.x = newPX;
  endEffector.p.y = newPY;
  endEffector.p.z = newPZ;
}

void KineCalc::SetN (double newNX, double newNY, double newNZ)
{
  endEffector.n.x = newNX;
  endEffector.n.y = newNY;
  endEffector.n.z = newNZ;
}

void KineCalc::SetO (double newOX, double newOY, double newOZ)
{
  endEffector.o.x = newOX;
  endEffector.o.y = newOY;
  endEffector.o.z = newOZ;
}

void KineCalc::SetA (double newAX, double newAY, double newAZ)
{
  endEffector.a.x = newAX;
  endEffector.a.y = newAY;
  endEffector.a.z = newAZ;
}

double KineCalc::GetTheta (unsigned int index)
{
  return joints[index];
}

void KineCalc::SetTheta (unsigned int index, double newVal)
{
  joints[index] = newVal;
}

void KineCalc::SetLinkLengths (double newLink1, double newLink2, double newLink3, double newLink4, double newLink5)
{
  link1 = newLink1;
  link2 = newLink2;
  link3 = newLink3;
  link4 = newLink4;
  link5 = newLink5;
}

void KineCalc::SetOffset (unsigned int joint, double newOffset)
{
  jointOffsets[joint] = newOffset;
}

void KineCalc::SetJointRange (unsigned int joint, double min, double max)
{
  jointMin[joint] = MIN (min, max);   // So that if min > max we reverse them
  jointMax[joint] = MAX (min, max);
}



///////////////////////////////////////////////////////////////////////////////
//  Utility helper functions
///////////////////////////////////////////////////////////////////////////////

KineVector KineCalc::Normalise (const KineVector &vector)
{
  KineVector result;
  double length = sqrt (vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
  result.x = vector.x / length;
  result.y = vector.y / length;
  result.z = vector.z / length;
  return result;
}

KineVector KineCalc::CalculateN (const EndEffector &pose)
{
  KineVector result;
  result.x = pose.o.y * pose.a.z - pose.a.y * pose.o.z;
  result.y = pose.o.z * pose.a.x - pose.a.z * pose.o.x;
  result.z = pose.o.x * pose.a.y - pose.a.x * pose.o.y;
  return Normalise (result);
}

void KineCalc::PrintEndEffector (const EndEffector &endEffector)
{
  printf ("P: (%f, %f, %f)\tA: (%f, %f, %f)\tO: (%f, %f, %f)\tN: (%f, %f, %f)\n",
          endEffector.p.x, endEffector.p.y, endEffector.p.z,
          endEffector.a.x, endEffector.a.y, endEffector.a.z,
          endEffector.o.x, endEffector.o.y, endEffector.o.z,
          endEffector.n.x, endEffector.n.y, endEffector.n.z);
}


///////////////////////////////////////////////////////////////////////////////
//  The important functions
///////////////////////////////////////////////////////////////////////////////

//  Calculate the forward kinematics
//  The result is stored in endEffector
//  fromJoints[]:   An array of 5 joint values
void KineCalc::CalculateFK (const double fromJoints[])
{
  double adjustedJoints[5];

  adjustedJoints[0] = (fromJoints[0] - jointOffsets[0]) * -1;
  adjustedJoints[1] = fromJoints[1] - jointOffsets[1];
  adjustedJoints[2] = fromJoints[2] - jointOffsets[2];
  adjustedJoints[3] = (fromJoints[3] - jointOffsets[3]) * -1;;
  adjustedJoints[4] = (fromJoints[4] - jointOffsets[4]) * -1;;

  endEffector = CalcFKForJoints (adjustedJoints);
//  printf ("Result of FK:\n");
//  PrintEndEffector (endEffector);
}

//  Calculate the inverse kinematics
//  The result is stored in joints
//  fromPosition:   An EndEffector structure describing the pose
//                  of the end effector
bool KineCalc::CalculateIK (const EndEffector &fromPosition)
{
  // Some references to make the code neater
  const KineVector &p = fromPosition.p;
  const KineVector &a = fromPosition.a;
  // These are the four possible solutions to the IK
  // solution1 = {1a, 2a, 3a, 4a, 5a}
  // solution2 = {1a, 2b, 3b, 4b, 5b}
  // solution3 = {1b, 2c, 3c, 4c, 5c}
  // solution4 = {1b, 2d, 3d, 4d, 5d}
  double solutions[4][5];
  double temp = 0.0f;

  // First calculate the two possible values for theta1, theta1a and theta1b
  temp = atan2 (p.y - link5 * a.y, p.x - link5 * a.x);
  solutions[0][0] = solutions[1][0] = temp;
  temp = atan2 (link5 * a.y - p.y, link5 * a.x - p.x);
  solutions[2][0] = solutions[3][0] = temp;

  // Next, using theta1_a, calculate thetas 2 and 3 (a and b)
  // First up is calculating r and rz
  double r = 0.0f, rz = 0.0f;
  if (sin (solutions[0][0]) < 0.1f || sin(solutions[0][0]) > -0.1f)
  {
    r = ((p.x - (link5 * a.x)) / cos (solutions[0][0])) - link1;
  }
  else
  {
    r = ((p.y - (link5 * a.y)) / sin (solutions[0][0])) - link1;
  }
  rz = p.z - (link5 * a.z);
  // Then calculate theta2a and 3a
  temp = (r * r + rz * rz + link2 * link2 - link4 * link4) / (2 * link2 * sqrt (r * r + rz * rz));
  temp = MIN (MAX (temp, -1.0f), 1.0f);
  temp = atan2 (rz, r) - acos (temp);
  int m1 = -1;
  do
  {
    if (m1 > 1)
    {
      printf ("m1 > 1!\n");
      break;
    }
    solutions[0][1] = temp + 2 * m1 * M_PI;
    m1 += 1;  // So that within the 3 iterations we get m1 = -1, 0, 1
  } // Put a catchall here to prevent infinite loops by checking if m1 has gone past 1 (shouldn't happen)
  while ((solutions[0][1] < -(M_PI) || solutions[0][1] > M_PI));// && m1 < 1);
  temp = (link2 * link2 + link4 * link4 - r * r - rz * rz) / (2 * link2 * link4);
  temp = MIN (MAX (temp, -1.0f), 1.0f);
  solutions[0][2] = M_PI - acos (temp);
  // Followed by theta2b and 3b
  temp = (r * r + rz * rz + link2 * link2 - link4 * link4) / (2 * link2 * sqrt (r * r + rz * rz));
  temp = MIN (MAX (temp, -1.0f), 1.0f);
  temp = atan2 (rz, r) + acos (temp);
  m1 = -1;
  do
  {
    if (m1 > 1)
    {
      break;
    }
    solutions[1][1] = temp + 2 * m1 * M_PI;
    m1 += 1;  // So that within the 3 iterations we get m1 = -1, 0, 1
  }
  while ((solutions[1][1] < -(M_PI) || solutions[1][1] > M_PI));// && m1 < 1);
  temp = (link2 * link2 + link4 * link4 - r * r - rz * rz) / (2 * link2 * link4);
  temp = MIN (MAX (temp, -1.0f), 1.0f);
  solutions[1][2] = -(M_PI) + acos (temp);

  // Using theta2a and 3a, calculate 4a and 5a to complete solution1
  CalcTheta4and5 (solutions[0], fromPosition);
  // Using theta2b and 3b, calculate 4b and 5b to complete solution2
  CalcTheta4and5 (solutions[1], fromPosition);

  // That's two of the possible solutions. To get the other two, repeat with theta1b
  // First up is calculating r and rz
  r = 0.0f;
  rz = 0.0f;
  if (sin (solutions[2][0]) < 0.1f || sin(solutions[2][0]) > -0.1f)
  {
    r = (p.x - link5 * a.x) / cos (solutions[2][0]) - link1;
  }
  else
  {
    r = (p.y - link5 * a.y) / sin (solutions[2][0]) - link1;
  }
  rz = p.z - (link5 * a.z);
  // Then calculate theta2c and 3c
  temp = (r * r + rz * rz + link2 * link2 - link4 * link4) / (2 * link2 * sqrt (r * r + rz * rz));
  temp = MIN (MAX (temp, -1.0f), 1.0f);
  temp = atan2 (rz, r) - acos (temp);
  m1 = -1;
  do
  {
    if (m1 > 1)
    {
      break;
    }
    solutions[2][1] = temp + 2 * m1 * M_PI;
    m1 += 1;  // So that within the 3 iterations we get m1 = -1, 0, 1
  } // Put a catchall here to prevent infinite loops by checking if m1 has gone past 1 (shouldn't happen)
  while ((solutions[2][1] < -(M_PI) || solutions[2][1] > M_PI));// && m1 < 1);
  temp = (link2 * link2 + link4 * link4 - r * r - rz * rz) / (2 * link2 * link4);
  temp = MIN (MAX (temp, -1.0f), 1.0f);
  solutions[2][2] = M_PI - acos (temp);
  // Followed by theta2d and 3d
  temp = (r * r + rz * rz + link2 * link2 - link4 * link4) / (2 * link2 * sqrt (r * r + rz * rz));
  temp = MIN (MAX (temp, -1.0f), 1.0f);
  temp = atan2 (rz, r) + acos (temp);
  m1 = -1;
  do
  {
    if (m1 > 1)
    {
      break;
    }
    solutions[3][1] = temp + 2 * m1 * M_PI;
    m1 += 1;  // So that within the 3 iterations we get m1 = -1, 0, 1
  }
  while ((solutions[3][1] < -(M_PI) || solutions[3][1] > M_PI));// && m1 < 1);
  temp = (link2 * link2 + link4 * link4 - r * r - rz * rz) / (2 * link2 * link4);
  temp = MIN (MAX (temp, -1.0f), 1.0f);
  solutions[3][2] = -(M_PI) + acos (temp);

  // Using theta2c and 3c, calculate 4c and 5c to complete solution1
  CalcTheta4and5 (solutions[2], fromPosition);
  // Using theta2d and 3d, calculate 4d and 5d to complete solution2
  CalcTheta4and5 (solutions[3], fromPosition);

  // Choose the best of the four solutions
  int chosenSolution = ChooseSolution (fromPosition, solutions);
  if (chosenSolution == -1)
    // Couldn't find a valid solution
    return false;

  // Offsets and so forth
  joints[0] = (solutions[chosenSolution][0] * -1) + jointOffsets[0];
  joints[1] = solutions[chosenSolution][1] + jointOffsets[1];
  joints[2] = solutions[chosenSolution][2] + jointOffsets[2];
  joints[3] = (solutions[chosenSolution][3] * -1) + jointOffsets[3];
  joints[4] = (solutions[chosenSolution][4] * -1) + jointOffsets[4];

  return true;
}

//  Calculates thetas 4 and 5 based on supplied thetas 1, 2 and 3 and the desired end effector pose
//  angles[]:       A 5-element array, of which elements 0, 1 and 2 should be filled already
//  fromPosition:   The desired end effector pose
void KineCalc::CalcTheta4and5 (double angles[], const EndEffector &fromPosition)
{
  const KineVector &n = fromPosition.n;
  const KineVector &o = fromPosition.o;
  const KineVector &a = fromPosition.a;

  double cos1 = cos (angles[0]);
  double cos23 = cos (angles[1] + angles[2]);
  double sin1 = sin (angles[0]);
  double sin23 = sin (angles[1] + angles[2]);

  if (cos23 != 0.0f)
  {
    if (sin1 < -0.1f || sin1 > 0.1f)
    {
      angles[3] = atan2 (n.z / cos23, -(n.x + ((n.z * cos1 * sin23) / cos23)) / sin1);
    }
    else
    {
      angles[3] = atan2 (n.z / cos23, (n.y + ((n.z * sin1 * sin23) / cos23)) / cos1);
    }

    double cos4 = cos (angles[3]);
    double sin4 = sin (angles[3]);
    if (cos4 != 0 || sin23 != 0)
    {
      angles[4] = atan2 (a.z * cos23 * cos4 - o.z * sin23, o.z * cos23 * cos4 + a.z * sin23);
    }
    else
    {
      angles[4] = atan2 (-(o.x * cos1 + o.y * sin1) / cos23, (o.x * sin1 - o.y * cos1) / sin4);
    }
  }
  else
  {
    angles[4] = atan2 (-o.z / sin23, a.z / sin23);

    double cos5 = cos (angles[4]);
    double sin5 = sin (angles[4]);
    if (cos5 > -0.1f || cos5 < 0.1f)
    {
      angles[3] = atan2 ((a.x * sin1 - a.y * cos1) / sin5, -(n.x * sin1) + n.y * cos1);
    }
    else
    {
      angles[3] = atan2 ((o.x * sin1 - o.y * cos1) / cos5, -(n.x * sin1) + n.y * cos1);
    }
  }
}

//  Choose the best solution from the 4 available based on error and reachability
//  fromPosition:   The desired end effector pose
//  solutions[][]:  The four solutions (each with 5 angles) in an array
int KineCalc::ChooseSolution (const EndEffector &fromPosition, const double solutions[][5])
{
  double errors[4];
  int order[4], jj;

  // We have 4 solutions, calculate the error for each one
  errors[0] = CalcSolutionError (solutions[0], fromPosition);
  errors[1] = CalcSolutionError (solutions[1], fromPosition);
  errors[2] = CalcSolutionError (solutions[2], fromPosition);
  errors[3] = CalcSolutionError (solutions[3], fromPosition);

  for (int ii = 0; ii < 4; ii++)
  {
    double min = MIN (errors[0], MIN (errors[1], MIN (errors[2], errors[3])));
    for (jj = 0; min != errors[jj]; jj++);  // Find the index at which the min is at
    errors[jj] = 999999;
    order[ii] = jj;
  }

  for (int ii = 0; ii < 4; ii++)
  {
    if (SolutionInRange (solutions[order[ii]]))
    {
      return order[ii];
    }
  }

  return -1;
}

//  Calculate the error for a solution from the desired pose
//  solution[]:       An array of 5 angles
//  fromPosition[]:   The end effector pose
double KineCalc::CalcSolutionError (const double solution[], const EndEffector &fromPosition)
{
  EndEffector solutionPos;
  double error = 0.0f;

  // Calculate the position of the end effector this solution gives using FK
  solutionPos = CalcFKForJoints (solution);
  // Calculate the distance from this to the desired position
  double xOffset = solutionPos.p.x - fromPosition.p.x;
  double yOffset = solutionPos.p.y - fromPosition.p.y;
  double zOffset = solutionPos.p.z - fromPosition.p.z;

  error = sqrt (xOffset * xOffset + yOffset * yOffset + zOffset * zOffset);
  if (isnan (error))
    error = 9999;

  return error;
}

//  Calculates the forward kinematics of a set of joint angles
//  angles[]:   The 5 angles to calculate from
EndEffector KineCalc::CalcFKForJoints (const double angles[])
{
  EndEffector result;

  double cos1 = cos (angles[0]);
  double cos2 = cos (angles[1]);
  double cos23 = cos (angles[1] + angles[2]);
  double cos4 = cos (angles[3]);
  double cos5 = cos (angles[4]);
  double sin1 = sin (angles[0]);
  double sin2 = sin (angles[1]);
  double sin23 = sin (angles[1] + angles[2]);
  double sin4 = sin (angles[3]);
  double sin5 = sin (angles[4]);

  result.p.x = link5 * ((cos1 * cos23 * cos5) + (sin1 * sin4 * sin5) - (cos1 * sin23 * cos4 * sin5)) +
      cos1 * ((link4 * cos23) + (link2 * cos2) + link1);
  result.p.y = link5 * ((sin1 * cos23 * cos5) + (cos1 * sin4 * sin5) - (sin1 * sin23 * cos4 * sin5)) +
      sin1 * ((link4 * cos23) + (link2 * cos2) + link1);
  result.p.z = link5 * ((sin23 * cos5) + (cos23 * cos4 * sin5)) + (link4 * sin23) + (link2 * sin2);

  result.n.x = -(sin1 * cos4) - (cos1 * sin23 * sin4);
  result.n.y = (cos1 * cos4) - (sin1 * sin23 * sin4);
  result.n.z = (cos23 * sin4);

  result.o.x = -(cos1 * cos23 * sin5) + (sin1 * sin4 * cos5) - (cos1 * sin23 * cos4 * cos5);
  result.o.y = -(sin1 * cos23 * sin5) - (cos1 * sin4 * cos5) - (sin1 * sin23 * cos4 * cos5);
  result.o.z = -(sin23 * sin5) + (cos23 * cos4 * cos5);

  result.a.x = (cos1 * cos23 * cos5) + (sin1 * sin4 * sin5) - (cos1 * sin23 * cos4 * sin5);
  result.a.y = (sin1 * cos23 * cos5) + (cos1 * sin4 * sin5) - (sin1 * sin23 * cos4 * sin5);
  result.a.z = (sin23 * cos5) + (cos23 * cos4 * sin5);

  return result;
}

//  Checks if the angles for a solution are reachable by the arm
//  angles[]:   The 5 angles to check
bool KineCalc::SolutionInRange (const double angles[])
{
  for (int ii = 0; ii < 5; ii++)
  {
    if (angles[ii] < jointMin[ii] || angles[ii] > jointMax[ii])
      return false;
  }

  return true;
}


/*************************************************************************************/
/*                                                                                   */
/*  File:	   nd.h                                                                  */
/*  Author:       Javier Minguez                                                     */
/*  Modified:      20/10/2005                                                        */
/*                                                                                   */
/*  This library implements a mixture between:                                       */
/*																		             */
/*	J. Minguez, L. Montano.                                                          */
/*	Nearness Diagram Navigation (ND): Collision Avoidance in Troublesome Scenarios.  */
/*	IEEE Transactions on Robotics and Automation, pp 154, 2004.                      */
/*                                                                                   */
/*                                                                                   */
/*	J. Minguez, J. Osuna, L. Montano.                                                */
/*	A Divide and Conquer Strategy based on Situations								 */
/*	to Achieve Reactive Collision Avoidance in Troublesome Scenarios.				 */
/*	IEEE International Conference on Robotics and Automation (ICRA 2004),   		 */
/*	2004. New Orleans, USA.															 */
/*                                                                                   */
/*************************************************************************************/


/*****************************************************************************/
//
//	EVERYTHING IN THE INTERNATIONAL SYSTEM (METERS AND RADIANS)
//
/*****************************************************************************/


#ifndef nd_h
#define nd_h

// ----------------------------------------------------------------------------
// GENERIC TYPES
// ----------------------------------------------------------------------------

// Cartesian coordinates. 

typedef struct {
  float x;
  float y;
} TCoordenadas;

// System of reference

typedef struct {
  TCoordenadas posicion;
  float orientacion;
} TSR; 



// ----------------------------------------------------------------------------
// SPECIFIC TYPES.
// ----------------------------------------------------------------------------



// ************************

// TParametrosND	(information of the robot and laser for the ND)

typedef struct {

  // GEOMETRY
  // The vehicle is considered to be symetric at both sides of the X axis.
  // The flag is 1 if the robot is resctangular, 0 if it is circular
  short int geometryRect;		
  
  // --- RECTANGULAR --- 
  // distance (m) from the wheels to the:
  // front: frontal part
  // back: back part
  // left: left side. Notice that the vehicle is symetric
  float front,back,left; 

  // --- CIRCULAR --- 
  // radius of the robot is is circular
  float R;						
  
  // MOTION
  // The falg is 1 if the robot is holonomous, or 0 is diff-drive or syncro
  short int holonomic;			

  // Maximum linear and angular velocities
  float vlmax,vamax;

  // Maximum linear and angular acelerations
  float almax,aamax;
  
  // OTHER STUFF

  // -- SECURITY DISTANCE ---
  // Distance to consider an obstacle dangerous (i.e. to start the avoidance maneouvre)
  // dsmax: Distance from the frontal robot bounds.
  // dsmin: Distance from the back robot bounds.
  // engorde: Inner value. The suggestion is 20% of the dsmin (i.e. 0.2*dsmin)
  float dsmax,dsmin,enlarge;

  // -- DISCONTINUITY --
  // Minimum space where the robot fits. I suggest same value than "izquierda" value.
  float discontinuity;

  // -- SAMPLING PERIOD --
  float T;

  // LASER
  // Distance from the wheels axis to the laser, X axis.
  //float laser;					

} TParametersND;

// **************************************






// ************************

// TVelocities	(information of linear v, and angular velocities w)

typedef struct {
  float v;			// linear velocity
  float w;			// angular velocity
  float v_theta;	// velocity angle (just if holonomous vehicle)
} TVelocities;

// **************************************





// ************************

// TInfoMovimiento	(information of the robot)

typedef struct {
  TSR SR1;					// Current vehicle location in GLOBAL coordinates
  TVelocities velocidades;	// Current vehicle velocities
} TInfoMovimiento;

// **************************************




// ************************

// TInfoEntorno	(list of obstacle points)

// Maximum number of points of the environment
// This number depends on the maximum number of obstacle points that 
// you want to give to the ND

//#define MAX_POINTS_SCENARIO 1440
#define MAX_POINTS_SCENARIO 10000

typedef struct {
  int longitud;
  TCoordenadas punto[MAX_POINTS_SCENARIO];
} TInfoEntorno;

// **************************************





// ----------------------------------------------------------------------------
// FUNCTIONS
// ----------------------------------------------------------------------------




// **********************************
// This function initialites the ND
// Input--
//		parametros:: information of the robot and laser used by the ND 
// Ouput--

void InicializarND(TParametersND *parametros);

// **********************************






// **********************************
// This runs the ND. The input is the current obstacle list and the goal location
// and the output the motion command. 
// Input--
//		objetivo::  current objective in GLOBAL coordinates. Notice that this
//					location can change each time you call ND.
//		movimiento:: this is the current velocity of the robot.
//		mapa::  this is a list of the obstacle points in global coordinates. 
//				You can use the current sensor reading or implement a kind of memory
//				to remember last scans. Whatever, ND wants a list of points in GLOBAL coordinates.					 
//		information:: variable for debug.
//		
// Ouput--
//		movimiento:: this is the output of the ND. 
//					 * Linear and angular velocities (and direction if holonomic).
//					 * NULL an emergency stop is required
//					 * pointer to (0,0) goal reached.

extern TVelocities *IterarND(TCoordenadas objetivo,
                             float goal_tol,
                             TInfoMovimiento *movimiento,
                             TInfoEntorno *mapa,
                             void *informacion);
    // if you do not want to see the internal information in nh2.h informacion = NULL

// **********************************


#endif 


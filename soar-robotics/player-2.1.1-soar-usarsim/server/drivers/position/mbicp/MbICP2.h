/*************************************************************************************/
/*                                                                                   */
/*  File:	   MbICP.h                                                               */
/*  Authors:       Luis Montesano and Javier Minguez                                 */
/*  Modified:      1/3/2006                                                          */
/*                                                                                   */
/*  This library implements the:                                                     */
/*																		             */
/*	J. Minguez, F. Lamiraux and L. Montesano										 */
/*	Metric-Based Iterative Closest Point, 											 */
/*  Scan Matching for Mobile Robot Displacement Estimation							 */
/*	IEEE Transactions on Roboticics (2006)										     */
/*                                                                                   */
/*************************************************************************************/


/* **************************************************************************************** */
//	This file contains inner information of the MbICP that you want to see from the outside
/* **************************************************************************************** */

#ifndef MbICP2
#define MbICP2

//#include "MbICP.h"
#include "TData.h"

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------
// ---------------------------------------------------------------
// Types definition
// ---------------------------------------------------------------
// ---------------------------------------------------------------


// ************************
// Associations information

/*
typedef struct{
  float rx,ry,nx,ny,dist;		// Point (nx,ny), static corr (rx,ry), dist
  int numDyn;					// Number of dynamic associations
  float unknown;				// Unknown weight
  int index;					// Index within the original scan
  int L,R;
}TAsoc;
*/

// ************************
// Scan inner matching parameters
typedef struct{
	/* --------------------- */
	/* --- Thresold parameters */
	/* Bw: maximum angle diference between points of different scans */
	/* Points with greater Bw cannot be correspondent (eliminate spurius asoc.) */
	/* This is a speed up parameter */
	float Bw;

	/* Br: maximum distance difference between points of different scans */
	/* Points with greater Br cannot be correspondent (eliminate spurius asoc.) */
	float Br;

	/* --------------------- */
	/* --- Inner parameters */

	/* L: value of the metric */
	/* When L tends to infinity you are using the standart ICP */
    /* When L tends to 0 you use the metric (more importance to rotation */
	float LMET;

	/* laserStep: selects points of each scan with an step laserStep  */
	/* When laserStep=1 uses all the points of the scans */
	/* When laserStep=2 uses one each two ... */
	/* This is an speed up parameter */
	int laserStep;

	/* ProjectionFilter: */
	/* Eliminate the points that cannot be seen given the two scans (see Lu&Millios 97) */
	/* It works well for angles < 45 \circ*/
	/* 1 : activates the filter */
	/* 0 : desactivates the filter */
	int ProjectionFilter;

	/* MaxDistInter: maximum distance to interpolate between points in the ref scan */
	/* Consecutive points with less Euclidean distance than MaxDistInter are considered to be a segment */
	float MaxDistInter;

	/* filtrado: in [0,1] sets the % of asociations NOT considered spurious */
	float filter;

	/* AsocError: in [0,1] */
	/* One way to check if the algorithm diverges if to supervise if the number of associatios goes below a thresold */
	/* When the number of associations is below AsocError, the main function will return error in associations step */
	float AsocError;

	/* --------------------- */
	/* --- Exit parameters */
	/* MaxIter: sets the maximum number of iterations for the algorithm to exit */
	/* More iterations more chance you give the algorithm to be more accurate   */
	int MaxIter;

	/* error_th: in [0,1] sets the maximum error ratio between iterations to exit */
	/* In each iteration, the error is the residual of the minimization */
	/* When error_th tends to 1 more precise is the solution of the scan matching */
	float error_th;

	/* errx_out,erry_out, errt_out: minimum error of the asociations to exit */
	/* In each iteration, the error is the residual of the minimization in each component */
	/* The condition is (lower than errx_out && lower than erry_out && lower than errt_out */
	/* When error_XXX tend to 0 more precise is the solution of the scan matching */
	float errx_out,erry_out, errt_out;

	/* IterSmoothConv: number of consecutive iterations that satisfity the error criteria */
	/* (error_th) OR (errorx_out && errory_out && errt_out) */
	/* With this parameter >1 avoids random solutions */
	int IterSmoothConv;

}TSMparams;

// ************************
// Structure to store the scans in polar and cartesian coordinates

/*
typedef struct {
  int numPuntos;
  Tpf laserC[MAXLASERPOINTS];  // Cartesian coordinates
  Tpfp laserP[MAXLASERPOINTS]; // Polar coordinates
}Tscan;
*/

// ---------------------------------------------------------------
// ---------------------------------------------------------------
// Variables definition
// ---------------------------------------------------------------
// ---------------------------------------------------------------


// ************************
// Static structure to initialize the SM parameters
extern TSMparams params;

// Original points to be aligned
extern Tscan ptosRef;
extern Tscan ptosNew;

// At each step::

// Those points removed by the projection filter (see Lu&Millios -- IDC)
extern Tscan ptosNoView; // Only with ProjectionFilter=1;

// Structure of the associations before filtering
extern TAsoc cp_associations[MAXLASERPOINTS];
extern int cntAssociationsT;

// Filtered Associations
extern TAsoc cp_associationsTemp[MAXLASERPOINTS];
extern int cntAssociationsTemp;

// Current motion estimation
extern Tsc motion2;

#ifdef __cplusplus
}
#endif

#endif

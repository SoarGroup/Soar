/*************************************************************************************/
/*                                                                                   */
/*  File:	   MbICP.h                                                           */
/*  Authors:       Luis Montesano and Javier Minguez                                 */
/*  Modified:      1/3/2006                                                          */
/*                                                                                   */
/*  This library implements the:                                                     */
/*										     */
/*      		    							     */
/*      J. Minguez, L. Montesano, and F. Lamiraux, "Metric-based iterative		*/
/*	closest point scan matching for sensor displacement estimation," IEEE		*/
/*	Transactions on Robotics, vol. 22, no. 5, pp. 1047 \u2013 1054, 2006. 		*/
/*************************************************************************************/


/*****************************************************************************/
//
//	EVERYTHING IN THE INTERNATIONAL SYSTEM (METERS AND RADIANS)
//
/*****************************************************************************/

#ifndef MbICP
#define MbICP
#include "TData.h"

#ifdef __cplusplus
extern "C" {
#endif


// ----------------------------------------------------------------------------
// DEFINES
// ----------------------------------------------------------------------------

/*
#define MAXLASERPOINTS 361
*/

// ----------------------------------------------------------------------------
// GENERIC TYPES
// ----------------------------------------------------------------------------

/*typedef struct {
  float x;
  float y;
}Tpf;

typedef struct {
  float r;
  float t;
}Tpfp;

typedef struct {
  int x;
  int y;
}Tpi;

typedef struct {
  float x;
  float y;
  float tita;
}Tsc;
*/

// ----------------------------------------------------------------------------
// SPECIFIC TYPES
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// GLOBAL FUNCTIONS
// ----------------------------------------------------------------------------


// ************************
// Function that initializes the SM parameters
// ************************

/* void InitScanMatching(float Bw, float Br,
						 float L, int laserStep,float MaxDistInter, float filtrado,
					     int MaxIter, float error_th, float exo, float eyo, float etitao, int IterSmoothConv); */
// in:::

	/* --------------------- */
	/* --- Thresold parameters */
	/* --------------------- */
	/* Bw: maximum angle diference between points of different scans */
	/* Points with greater Bw cannot be correspondent (eliminate spurius asoc.) */
	/* This is a speed up parameter */
	//float Bw;

	/* Br: maximum distance difference between points of different scans */
	/* Points with greater Br cannot be correspondent (eliminate spurius asoc.) */
	//float Br;

	/* --------------------- */
	/* --- Inner parameters */
	/* --------------------- */
	/* L: value of the metric */
	/* When L tends to infinity you are using the standart ICP */
    /* When L tends to 0 you use the metric (more importance to rotation) */
	//float L;

	/* laserStep: selects points of each scan with an step laserStep  */
	/* When laserStep=1 uses all the points of the scans */
	/* When laserStep=2 uses one each two ... */
	/* This is an speed up parameter */
	//int laserStep;

	/* ProjectionFilter: */
	/* Eliminate the points that cannot be seen given the two scans (see Lu&Millios 97) */
	/* It works well for angles < 45 \circ*/
	/* 1 : activates the filter */
	/* 0 : desactivates the filter */
	// int ProjectionFilter;

	/* MaxDistInter: maximum distance to interpolate between points in the ref scan */
	/* Consecutive points with less Euclidean distance than MaxDistInter are considered to be a segment */
	//float MaxDistInter;

	/* filter: in [0,1] sets the % of asociations NOT considered spurious */
	/* E.g. if filter=0.9 you use 90% of the associations */
	/* The associations are ordered by distance and the (1-filter) with greater distance are not used */
	/* This type of filtering is called "trimmed-ICP" */
	//float filter;

	/* AsocError: in [0,1] */
	/* One way to check if the algorithm diverges if to supervise if the number of associatios goes below a thresold */
	/* When the number of associations is below AsocError, the main function will return error in associations step */
	// float AsocError;

	/* --------------------- */
	/* --- Exit parameters */
	/* --------------------- */
	/* MaxIter: sets the maximum number of iterations for the algorithm to exit */
	/* The more iterations, the more chance you give the algorithm to be more accurate   */
	//int MaxIter;

	/* errorRatio: in [0,1] sets the maximum error ratio between iterations to exit */
	/* In iteration K, let be errorK the residual of the minimization */
	/* Error_th=(errorK-1/errorK). When error_th tends to 1 more precise is the solution of the scan matching */
	//float error_th;

	/* errx_out,erry_out, errt_out: minimum error of the asociations to exit */
	/* In each iteration, the error is the residual of the minimization in each component */
	/* The condition is (errorKx<errx_out && errorKx<erry_out && errorKx<errt_out) */
	/* When errorK tends to 0 the more precise is the solution of the scan matching */
	//float errx_out,erry_out, errt_out;

	/* IterSmoothConv: number of consecutive iterations that satisfity the error criteria (the two above criteria) */
	/* (error_th) OR (errorx_out && errory_out && errt_out) */
	/* With this parameter >1 avoids random solutions and estabilices the algorithm */
	//int IterSmoothConv;



void Init_MbICP_ScanMatching(
			     float max_laser_range,
			     float Bw,
			     float Br,
			     float L,
			     int   laserStep,
			     float MaxDistInter,
			     float filter,
			     int   ProjectionFilter,
			     float AsocError,
			     int   MaxIter,
			     float errorRatio,
			     float errx_out,
			     float erry_out,
			     float errt_out,
			     int IterSmoothConv);

// -------------------------------------------------------------

// ************************
// Function that does the scan matching
// ************************

/* int MbICPmatcher(Tpfp *laserK, Tpfp *laserK1,
				 Tsc *sensorMotion, Tsc *solution); */

// in:::
//      laserK: is the reference scan in polar coordinates (max. num points is MAXLASERPOINTS)
//		laserK1: is the new scan in polar coordinates (max. num points is MAXLASERPOINTS)
//		sensorMotion: initial SENSOR motion estimation from location K to location K1
//		solution: SENSOR motion solution from location K to location K1
// out:::
//		1 : Everything OK in less that the Maximum number of iterations
//		2 : Everything OK but reached the Maximum number of iterations
//		-1: Failure in the association step
//		-2: Failure in the minimization step

int MbICPmatcher(Tpfp *laserK, Tpfp *laserK1,
				 Tsc *sensorMotion, Tsc *solution);

#ifdef __cplusplus
}
#endif

#endif

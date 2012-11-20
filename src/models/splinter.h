#ifndef SPLINTER_H
#define SPLINTER_H

#include "mat.h"

const double wheel_diameter     = 0.25;
const double baseline           = 0.35;
const double torque_constant    = 3.0; // torque (Nm) per amp
const double emf_constant       = 2.0; // volts per rad_per_sec
const double winding_resistance = 5.5; // ohms
const double inertia            = 0.5; // kg*m^2
const double drag_constant      = 1.0; // drag (Nm per rad_per_sec) ( >= 0)
const double dt                 = 0.016; // need a better way to figure this out

inline void calc_rps(double &rps, double input_volts) {
	double volts_emf = rps * emf_constant;
	double amps = (input_volts - volts_emf) / winding_resistance;
	double torque0 = amps * torque_constant;
	double torque_drag = rps * drag_constant;
	double torque_net = torque0 - torque_drag;
	double acceleration = torque_net / inertia;
	rps += acceleration * dt;
}

inline void splinter_update(double &px, double &py, double &vx, double &vy, double &rz, double &rtz, double &lrps, double &rrps, double lvolt, double rvolt) {
	calc_rps(lrps, lvolt * 12);
	calc_rps(rrps, rvolt * 12);
	double dleft  = dt * lrps * wheel_diameter;
	double dright = dt * rrps * wheel_diameter;
	
	Eigen::Matrix3d orient(Eigen::AngleAxis<double>(rz, vec3(0, 0, 1)));
	vec3 vel = orient * vec3((dleft + dright) / 2, 0., 0.);
	rtz = (dright - dleft) / baseline;
	px += vel[0];
	py += vel[1];
	rz += rtz;
	vx = vel[0];
	vy = vel[1];
}

#endif

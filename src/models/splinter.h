#ifndef SPLINTER_H
#define SPLINTER_H

#include "linalg.h"

const float wheel_diameter     = 0.25;
const float baseline           = 0.35;
const float torque_constant    = 3.0; // torque (Nm) per amp
const float emf_constant       = 2.0; // volts per rad_per_sec
const float winding_resistance = 5.5; // ohms
const float inertia            = 0.5; // kg*m^2
const float drag_constant      = 1.0; // drag (Nm per rad_per_sec) ( >= 0)
const float dt                 = 0.016; // need a better way to figure this out

inline void calc_rps(float &rps, float input_volts) {
	float volts_emf = rps * emf_constant;
	float amps = (input_volts - volts_emf) / winding_resistance;
	float torque0 = amps * torque_constant;
	float torque_drag = rps * drag_constant;
	float torque_net = torque0 - torque_drag;
	float acceleration = torque_net / inertia;
	rps += acceleration * dt;
}

inline void splinter_update(float &px, float &py, float &vx, float &vy, float &rz, float &rtz, float &lrps, float &rrps, float lvolt, float rvolt) {
	calc_rps(lrps, lvolt * 12);
	calc_rps(rrps, rvolt * 12);
	float dleft  = dt * lrps * wheel_diameter;
	float dright = dt * rrps * wheel_diameter;
	
	quaternion orient(vec3(0., 0., rz));
	vec3 vel = orient.rotate(vec3((dleft + dright) / 2, 0., 0.));
	rtz = (dright - dleft) / baseline;
	px += vel[0];
	py += vel[1];
	rz += rtz;
	vx = vel[0];
	vy = vel[1];
}

#endif

#ifndef LINALG_H
#define LINALG_H

#include <string.h>
#include <cmath>
#include <cassert>
#include <ostream>
#include <iterator>
#include <vector>
#include "mat.h"

class quaternion {
public:
	float a[4];
	
	quaternion() {
		memset(a, 0, 4 * sizeof(float));
	}
	
	quaternion(float w, float x, float y, float z) {
		a[0] = w; a[1] = x; a[2] = y; a[3] = z;
	}
	
	quaternion(const quaternion &q) {
		memcpy(a, q.a, 4 * sizeof(float));
	}
	
	/* from april.jmat.LinAlg.rollPitchYawToQuat */
	quaternion(const vec3 &rpy) {
		float halfroll = rpy[0] / 2;
		float halfpitch = rpy[1] / 2;
		float halfyaw = rpy[2] / 2;
	
		float sin_r2 = sin( halfroll );
		float sin_p2 = sin( halfpitch );
		float sin_y2 = sin( halfyaw );
	
		float cos_r2 = cos( halfroll );
		float cos_p2 = cos( halfpitch );
		float cos_y2 = cos( halfyaw );
	
		a[0] = cos_r2 * cos_p2 * cos_y2 + sin_r2 * sin_p2 * sin_y2;
		a[1] = sin_r2 * cos_p2 * cos_y2 - cos_r2 * sin_p2 * sin_y2;
		a[2] = cos_r2 * sin_p2 * cos_y2 + sin_r2 * cos_p2 * sin_y2;
		a[3] = cos_r2 * cos_p2 * sin_y2 - sin_r2 * sin_p2 * cos_y2;
	}
	
	/* from april.jmat.LinAlg.quatRotate */
	vec3 rotate(const vec3 &v) const {
		float t2, t3, t4, t5, t6, t7, t8, t9, t10;
		t2  =  a[0] * a[1];
		t3  =  a[0] * a[2];
		t4  =  a[0] * a[3];
		t5  = -a[1] * a[1];
		t6  =  a[1] * a[2];
		t7  =  a[1] * a[3];
		t8  = -a[2] * a[1];
		t9  =  a[2] * a[3];
		t10 = -a[3] * a[3];
	
		return vec3(2*((t8+t10)*v[0] + (t6-t4)*v[1]  + (t3+t7)*v[2]) + v[0],
		            2*((t4+t6)*v[0]  + (t5+t10)*v[1] + (t9-t2)*v[2]) + v[1],
		            2*((t7-t3)*v[0]  + (t2+t9)*v[1]  + (t5+t8)*v[2]) + v[2]);
	}
	
	/* from april.jmat.LinAlg.quatToRollPitchYaw */
	vec3 to_rpy() const {
		float roll_a =     2 * (a[0] * a[1] + a[2] * a[3]);
		float roll_b = 1 - 2 * (a[1] * a[1] + a[2] * a[2]);
		float pitch_sin =  2 * (a[0] * a[2] - a[3] * a[1]);
		float yaw_a =      2 * (a[0] * a[3] + a[1] * a[2]);
		float yaw_b =  1 - 2 * (a[2] * a[2] + a[3] * a[3]);
	
		return vec3(atan2(roll_a, roll_b), asin(pitch_sin), atan2(yaw_a, yaw_b));
	}
	
	/* from april.jmat.LinAlg.quatMultiply */
	quaternion operator*(const quaternion &q) const {
		quaternion r;
		r[0] = a[0]*q[0] - a[1]*q[1] - a[2]*q[2] - a[3]*q[3];
		r[1] = a[0]*q[1] + a[1]*q[0] + a[2]*q[3] - a[3]*q[2];
		r[2] = a[0]*q[2] - a[1]*q[3] + a[2]*q[0] + a[3]*q[1];
		r[3] = a[0]*q[3] + a[1]*q[2] - a[2]*q[1] + a[3]*q[0];
		return r;
	}
	
	/* from april.jmat.LinAlg.quatMultiply */
	void operator*=(const quaternion &q) {
		float t[4];
		t[0] = a[0]*q[0] - a[1]*q[1] - a[2]*q[2] - a[3]*q[3];
		t[1] = a[0]*q[1] + a[1]*q[0] + a[2]*q[3] - a[3]*q[2];
		t[2] = a[0]*q[2] - a[1]*q[3] + a[2]*q[0] + a[3]*q[1];
		t[3] = a[0]*q[3] + a[1]*q[2] - a[2]*q[1] + a[3]*q[0];
		memcpy(a, t, 4 * sizeof(float));
	}
	
	float& operator[](int i) {
		return a[i];
	}
	
	float operator[](int i) const {
		return a[i];
	}
};

class transform3 {
public:
	class Translation {};
	class Rotation {};
	class Scaling {};
	class Identity {};
	
	transform3() {
		memset(m, 0, 12 * sizeof(float));
		for (int i = 0; i < 3; ++i) {
			m[i][i] = 1.;
		}
	}
	
	transform3(const transform3 &t) {
		memcpy(m, t.m, 12 * sizeof(float));
	}
	
	transform3(char type, vec3 v) {
		memset(m, 0, 12 * sizeof(float));
		switch(type) {
			case 'p':
				for (int i = 0; i < 3; ++i) {
					m[i][i] = 1.;
				}
				for (int i = 0; i < 3; ++i) {
					m[i][3] = v[i];
				}
				break;
			case 'r':
				/*
				 Application order - roll, pitch, yaw
				 http://mathworld.wolfram.com/EulerAngles.html
				*/
				{
					float sinr = sin(v[0]), sinp = sin(v[1]), siny = sin(v[2]),
					      cosr = cos(v[0]), cosp = cos(v[1]), cosy = cos(v[2]);
					
					m[0][0] = cosy*cosp;
					m[0][1] = -siny*cosr+cosy*sinp*sinr;
					m[0][2] = -siny*-sinr+cosy*sinp*cosr;
					m[1][0] = siny*cosp;
					m[1][1] = cosy*cosr+siny*sinp*sinr;
					m[1][2] = cosy*-sinr+siny*sinp*cosr;
					m[2][0] = -sinp;
					m[2][1] = cosp*sinr;
					m[2][2] = cosp*cosr;
				}
				break;
			case 's':
				for (int i = 0; i < 3; ++i) {
					m[i][i] = v[i];
				}
				break;
			default:
				assert(false);
		}
	}
	
	vec3 operator()(const vec3 &v) const {
		vec3 r;
		for (int i = 0; i < 3; ++i) {
			r[i] = m[i][3];
			for (int j = 0; j < 3; ++j) {
				r[i] += m[i][j] * v[j];
			}
		}
		return r;
	}
	
	transform3 operator*(const transform3 &t) const {
		transform3 r;
		r.m[0][0] = m[0][0]*t.m[0][0]+m[0][1]*t.m[1][0]+m[0][2]*t.m[2][0];
		r.m[0][1] = m[0][0]*t.m[0][1]+m[0][1]*t.m[1][1]+m[0][2]*t.m[2][1];
		r.m[0][2] = m[0][0]*t.m[0][2]+m[0][1]*t.m[1][2]+m[0][2]*t.m[2][2];
		r.m[0][3] = m[0][0]*t.m[0][3]+m[0][1]*t.m[1][3]+m[0][2]*t.m[2][3]+m[0][3];
		r.m[1][0] = m[1][0]*t.m[0][0]+m[1][1]*t.m[1][0]+m[1][2]*t.m[2][0];
		r.m[1][1] = m[1][0]*t.m[0][1]+m[1][1]*t.m[1][1]+m[1][2]*t.m[2][1];
		r.m[1][2] = m[1][0]*t.m[0][2]+m[1][1]*t.m[1][2]+m[1][2]*t.m[2][2];
		r.m[1][3] = m[1][0]*t.m[0][3]+m[1][1]*t.m[1][3]+m[1][2]*t.m[2][3]+m[1][3];
		r.m[2][0] = m[2][0]*t.m[0][0]+m[2][1]*t.m[1][0]+m[2][2]*t.m[2][0];
		r.m[2][1] = m[2][0]*t.m[0][1]+m[2][1]*t.m[1][1]+m[2][2]*t.m[2][1];
		r.m[2][2] = m[2][0]*t.m[0][2]+m[2][1]*t.m[1][2]+m[2][2]*t.m[2][2];
		r.m[2][3] = m[2][0]*t.m[0][3]+m[2][1]*t.m[1][3]+m[2][2]*t.m[2][3]+m[2][3];
		return r;
	}
	
private:
	float m[3][4];
};

/*
# Used to generate the unrolled matrix multiplication

awk 'BEGIN {
	for (i = 0; i <= 3; i++) {
		for (j = 0; j <= 3; j++) {
			op = ""
			for (r = 0; r <= 3; r++) {
				if ((i == 3 && r != 3) || (r == 3 && j != 3))
					continue
				if (i == 3 && r == 3)
					printf("%sx.m%d%d", op, r, j)
				else if (r == 3 && j == 3)
					printf("%sm%d%d", op, i, r)
				else
					printf("%sm%d%d*x.m%d%d", op, i, r, r, j)
				op = "+"
			}
			print ","
		}
	}
	exit
}'

# Poor man's symbolic matrix multiplication with awk. Unrolls
# the transform matrix for 3D rotation using roll, pitch, and yaw.

awk 'BEGIN {
	ys="cosy -siny 0.0 0.0 " \
	   "siny  cosy 0.0 0.0 " \
	   " 0.0   0.0 1.0 0.0 " \
	   " 0.0   0.0 0.0 1.0 "

	ps=" cosp 0.0 sinp 0.0 " \
	   "  0.0 1.0  0.0 0.0 " \
	   "-sinp 0.0 cosp 0.0 " \
	   "  0.0 0.0  0.0 1.0 "

	rs="1.0  0.0   0.0 0.0 " \
	   "0.0 cosr -sinr 0.0 " \
	   "0.0 sinr  cosr 0.0 " \
	   "0.0  0.0   0.0 1.0 "
	
	parse(RY, ys)
	parse(RP, ps)
	parse(RR, rs)

	mult(RY, RP, F)
	mult(F, RR, FF)
	
	printmat(FF)
	exit
}

function printmat(M) {
	print "{"
	for (i = 0; i <= 3; i++) {
		printf "{ "
		sep = ""
		for (j = 0; j <= 3; j++) {
			printf sep M[i,j]
			sep = ", "
		}
		print "},"
	}
	print "}"
}

function parse(M, s) {
	split(s, xxx)
	for (i = 0; i <= 3; i++)
		for (j = 0; j <= 3; j++)
			M[i, j] = xxx[i * 4 + j + 1]
}

function mult(A, B, AB) {
	for (i = 0; i <= 3; i++) {
		for (j = 0; j <= 3; j++) {
			line=""
			op = ""
			for (r = 0; r <= 3; r++) {
				if (A[i,r] == 0.0 || B[r,j] == 0.0)
					continue
				if (A[i,r] == 1.0) {
					line = line sprintf("%s%s", op, B[r, j])
				} else if (B[r, j] == 1.0) {
					line = line sprintf("%s%s", op, A[i, r])
				} else {
					line = line sprintf("%s%s*%s", op, A[i, r], B[r, j])
				}
				op = "+"
			}
			if (length(line) == 0)
				AB[i,j] = 0.0
			else
				AB[i,j]=line
		}
	}
}'


*/

#endif

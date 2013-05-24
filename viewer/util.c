#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "viewer.h"

static char *qhull_in_path = NULL;
static char *qhull_out_path = NULL;
static char qhull_cmd[1000];

void fix_literal_quotes(char *s);
void cleanup_qhull();

/*
 split using whitespace and double quotes. \" is a literal quote in quoted
 fields
*/
int split(char *s, char *fields[], int maxfields) {
	char *p;
	int n, quoted, done;
	
	memset(fields, 0, maxfields * sizeof(char*));
	for (n = 0, quoted = 0, done = 0, p = s; !done && n < maxfields; ++p) {
		if (*p == '\0') done = 1;
		
		if (!fields[n] && *p != '\0' && !isspace(*p)) {
			if (*p == '"') {
				fields[n] = p + 1;
				quoted = 1;
			} else {
				fields[n] = p;
				quoted = 0;
			}
		} else if (fields[n] && !quoted && (isspace(*p) || *p == '\0')) {
			*p = '\0';
			++n;
		} else if (fields[n] && quoted && ((*p == '"' && *(p-1) != '\\') || *p == '\0')) {
			*p = '\0';
			fix_literal_quotes(fields[n]);
			++n;
		}
	}
	return n;
}

void fix_literal_quotes(char *s) {
	char *t;
	for (t = s; *s != '\0'; ++s, ++t) {
		if (*s == '\\' && *(s+1) == '"') {
			++s;
		}
		if (t != s) {
			*t = *s;
		}
	}
	*t = '\0';
}

int match(char *pat, char *s) {
	char *patcopy, *p1, *p2, *sp1, *sp2;
	
	if ((patcopy = strdup(pat)) == NULL) {
		perror("match: ");
		exit(1);
	}
	
	p1 = patcopy;
	sp1 = s;
	while (*p1 != '\0') {
		p2 = strchr(p1, '*');
		if (p2 == NULL)
			return (strcmp(p1, sp1) == 0);
		*p2 = '\0';
		sp2 = strstr(sp1, p1);
		if (sp2 == NULL || (p1 == patcopy && sp2 != sp1))
			return 0;
		sp1 = sp2 + strlen(p1);
		p1 = p2 + 1;
	}
	/* will only get here if pat ends with '*' */
	return 1;
}

int parse_int(char *s, int *v) {
	char *end;
	*v = strtol(s, &end, 10);
	return *s != '\0' && *end == '\0';
}

void set_vec3(vec3 a, real x, real y, real z) {
	a[0] = x; a[1] = y; a[2] = z;
}

void set_vec4(vec3 a, real x, real y, real z, real w) {
	a[0] = x; a[1] = y; a[2] = z; a[3] = w;
}

void copy_vec3(vec3 a, vec3 b) {
	set_vec3(b, a[0], a[1], a[2]);
}

void copy_vec4(vec4 a, vec4 b) {
	set_vec4(b, a[0], a[1], a[2], a[3]);
}

void add_vec3(vec3 a, vec3 b) {
	a[0] += b[0]; a[1] += b[1]; a[2] += b[2];
}

void subtract_vec3(vec3 a, vec3 b) {
	a[0] -= b[0]; a[1] -= b[1]; a[2] -= b[2];
}

void scale_vec3(vec3 a, real b) {
	a[0] *= b; a[1] *= b; a[2] *= b;
}

void cross_prod(vec3 a, vec3 b, vec3 c) {
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
}

void normalize(vec3 a) {
	real d = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	if (d == 0.0) {
		fprintf(stderr, "division by zero\n");
		exit(1);
	}
	a[0] /= d; a[1] /= d; a[2] /= d;
}

void norm_cross_prod(vec3 a, vec3 b, vec3 c) {
	cross_prod(a, b, c);
	if (c[0] != 0.0 || c[1] != 0.0 || c[2] != 0.0)
		normalize(c);
}

void set_quat(quaternion q, real x, real y, real z, real w) {
	q[0] = x; q[1] = y; q[2] = z; q[3] = w;
}

void quat_to_axis_angle(quaternion q, vec3 axis, real *angle) {
	real s = sqrt(1 - q[3] * q[3]);
	*angle = 2.0 * acos(q[3]);
	
	set_vec3(axis, q[0], q[1], q[2]);
	if (s > 0.00001) {
		axis[0] /= s;
		axis[1] /= s;
		axis[2] /= s;
	}
}

void quat_to_mat(quaternion q, real m[16]) {
	m[0] = 1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]);
	m[1] = 2.0 * (q[0] * q[1] - q[2] * q[3]);
	m[2] = 2.0 * (q[2] * q[0] + q[1] * q[3]);
	m[3] = 0.0;

	m[4] = 2.0 * (q[0] * q[1] + q[2] * q[3]);
	m[5]= 1.0 - 2.0 * (q[2] * q[2] + q[0] * q[0]);
	m[6] = 2.0 * (q[1] * q[2] - q[0] * q[3]);
	m[7] = 0.0;

	m[8] = 2.0 * (q[2] * q[0] - q[1] * q[3]);
	m[9] = 2.0 * (q[1] * q[2] + q[0] * q[3]);
	m[10] = 1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]);
	m[11] = 0.0;

	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = 0.0;
	m[15] = 1.0;
}

void rotate_vec3(quaternion q, vec3 v) {
	real m[16];
	
	quat_to_mat(q, m);
	v[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2];
	v[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2];
	v[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2];
}

void init_semaphore(semaphore *s) {
	s->count = 0;
	if (!(s->mutex = glfwCreateMutex()))
		error("Failed to create semaphore mutex.\n");
}

void semaphore_P(semaphore *s) {
	glfwLockMutex(s->mutex);
	while (s->count == 0) {
		glfwUnlockMutex(s->mutex);
		glfwSleep(0.0001);
		glfwLockMutex(s->mutex);
	}
	s->count = 0;
	glfwUnlockMutex(s->mutex);
}

void semaphore_V(semaphore *s) {
	glfwLockMutex(s->mutex);
	s->count = 1;
	glfwUnlockMutex(s->mutex);
}

int qhull(real verts[], int nverts, int indexes[], int max_indexes) {

	FILE *input, *output;
	int ret, ntriangles, i, j, k;
	char readbuf[4096];
	char *fields[4];  // should be 3 fields per line, one more to detect errors
	
	if (!qhull_in_path) {
		qhull_in_path = get_temp("qhull_in_");
		qhull_out_path = get_temp("qhull_out_");
		fprintf(stderr, "qhull_in = %s, qhull_out = %s\n", qhull_in_path, qhull_out_path);
		if (sprintf(qhull_cmd, "qconvex i Qt <%s >%s", qhull_in_path, qhull_out_path) < 0) {
			fprintf(stderr, "error constructing qhull command\n");
			exit(1);
		}
		atexit(cleanup_qhull);
	}
	
	input = fopen(qhull_in_path, "w");
	if (!input) {
		perror("qhull");
		exit(1);
	}
	
	fprintf(input, "3\n%ld\n", nverts / 3);
	for (i = 0; i < nverts; i += 3) {
		fprintf(input, "%f %f %f\n", verts[i], verts[i+1], verts[i+2]);
	}
	fclose(input);
	
	if ((ret = run_shell(qhull_cmd)) != 0) {
		fprintf(stderr, "qhull returned with non-zero status %d\n", ret);
		exit(1);
	}
	
	output = fopen(qhull_out_path, "r");
	if (!output) {
		perror("qhull");
		exit(1);
	}
	
	if (!fgets(readbuf, sizeof(readbuf), output) || 
	    split(readbuf, fields, 2) != 1 ||
	    !parse_int(fields[0], &ntriangles))
	{
		fprintf(stderr, "unexpected qhull output\n");
		exit(1);
	}
	
	if (ntriangles * 3 > max_indexes) {
		fprintf(stderr, "too many triangles in convex hull\n");
		exit(1);
	}
	
	for (i = 0, k = 0; i < ntriangles; ++i) {
		if (!fgets(readbuf, sizeof(readbuf), output) || 
		    split(readbuf, fields, 4) != 3)
		{
			fprintf(stderr, "unexpected qhull output\n");
			exit(1);
		}
		for (j = 2; j >= 0; --j) {   /* qhull outputs clockwise triangles */
			if (!parse_int(fields[j], &indexes[k++])) {
				fprintf(stderr, "unexpected qhull output\n");
				exit(1);
			}
		}
	}
	
	fclose(output);
	return ntriangles * 3;
}

void cleanup_qhull() {
	delete_file(qhull_in_path);
	delete_file(qhull_out_path);
	free(qhull_in_path);
	free(qhull_out_path);
}

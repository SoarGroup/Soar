#ifndef VIEWER_H
#define VIEWER_H

#include <stdlib.h>
#include <stdio.h>
#include <GL/glfw.h>

#define PAN_FACTOR  1.0e-1
#define GRID_LINES  10
#define MAX_COMMAND 4096
#define FONT_WIDTH  6
#define FONT_HEIGHT 12
#define PI          3.141592654
#define FOVY_MIN    5
#define FOVY_MAX    175
#define FOVY_DEF    60
#define NEAR_CLIP   0.1
#define FAR_CLIP    100.0
#define NLAYERS     10

typedef GLdouble real;
typedef real vec3[3];
typedef real vec4[4];
typedef real quaternion[4];

typedef struct Camera {
	quaternion q;
	vec3 pos;
	real rot_mat[16];
	real fovy;
	int ortho;
	real orthoy;
} camera;

typedef struct Geometry {
	char *name;
	int layer;
	double line_width;
	
	vec3 pos;
	vec3 axis;
	real angle;
	vec3 scale;
	vec3 color;
	
	/* for polyhedrons */
	real *vertices;
	GLuint *indexes;
	int ninds;
	real *normals;
	
	/* for spheres */
	GLUquadricObj *quadric;
	real radius;
	
	/* for text */
	char *text;
	
	struct Geometry *next;
} geometry;

typedef struct Layer {
	/* drawing options */
	int lighting;
	int flat;
	int clear_depth;
	int draw_names;
	int wireframe;
	
	/*
	 storage for various matrices between object drawing and label drawing
	*/
	GLint last_view[4];
	real last_modelview[16], last_projection[16];
} layer;

typedef struct Scene {
	char *name;
	geometry *geoms;
	struct Scene *next;
} scene;

typedef struct Semaphore {
	GLFWmutex mutex;
	int count;
} semaphore;

extern GLFWmutex scene_lock;
extern semaphore redraw_semaphore;
extern int debug;

/* viewer.c */
void pan_camera(camera *c, real x, real y);
void rotate_camera(camera *c, int x, int y, int dx, int dy);
void zoom_camera(camera *c, real f);
void pull_camera(camera *c, real z);
void reset_camera(camera *c, int key);
void apply_camera(camera *c);

void init_geom(geometry *g, char *name);
void set_geom_vertices(geometry *g, real *vertices, int nverts);
void set_geom_radius(geometry *g, real radius);
void set_geom_text(geometry *g, char *text);
void destroy_geom(geometry *g);
void draw_geom(geometry *g);

scene *find_or_add_scene(char *name);
int delete_scenes(char *pattern);
int match_scenes(char *pattern, scene **scns, int n);
void init_scene(scene *s, char *name);
void destroy_scene(scene *s);
void request_screenshot(char *path, int i);  /* 0 = from keyboard, 1 = from input */

geometry *find_or_add_geom(scene *s, char *name);
int delete_geoms(scene *s, char *pattern);
int match_geoms(scene *s, char *pattern, geometry **geoms, int n);

int set_layer(int layer_num, char option, int value);

/* input.c */
void GLFWCALL proc_input(void *unused);

/* text.c */
void init_font(void);
void draw_text(char *s, int x, int y);

/* util.c */
void error(const char *msg);
int match(char *pattern, char *s);
int split(char *s, char *fields[], int maxfields);

void set_vec3(vec3 a, real x, real y, real z);
void set_vec4(vec3 a, real x, real y, real z, real w);
void copy_vec3(vec3 a, vec3 b);
void copy_vec4(vec4 a, vec4 b);
void scale_vec3(vec3 a, real b);
void add_vec3(vec3 a, vec3 b);
void subtract_vec3(vec3 a, vec3 b);
void normalize(vec3 a);
void cross_prod(vec3 a, vec3 b, vec3 c);
void norm_cross_prod(vec3 a, vec3 b, vec3 c);
void quat_to_axis_angle(quaternion q, vec3 axis, real *angle);
void quat_to_mat(quaternion q, real m[16]);
void rotate_vec3(quaternion q, vec3 v);
void set_quat(quaternion q, real x, real y, real z, real w);

void init_semaphore(semaphore *s);
void semaphore_P(semaphore *s);
void semaphore_V(semaphore *s);

int qhull(real verts[], int nverts, int indexes[], int max_indexes);

/* platform specific */
int init_input(int argc, char *argv[]);
int get_input(char *buf, int n);
int run_shell(const char *cmd);
char *get_temp(const char *prefix);
void delete_file(const char *path);

#endif

#include "viewer.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "viewer.h"
#include "trackball.h"

/*
 For GRID_LINES = 10, there would be 20 lines along x axis and 20 lines
 along y axis (exclude axes). Each line is composed of 4 doubles (x1, y1), (x2, y2)
*/
#define GRID_VERTS_SIZE (4 * 2 * (GRID_LINES * 2))
#define SCENE_MENU_OFFSET 20
#define MAX_INDS 3000
#define DEPTH_BITS 16

GLFWmutex scene_lock;
semaphore redraw_semaphore;
int debug;

static real grid_verts[GRID_VERTS_SIZE];
static camera cam;
static layer layers[NLAYERS];

/* Colors of all objects */
static GLfloat light_color[] =          { 0.2, 0.2, 0.2, 1.0 };
static real bg_color[] =                { 0.2, 0.2, 0.2, 1.0 };
static real grid_color[] =              { 0.3, 0.3, 0.3, 0.5 };
static real grid_x_color[] =            { 0.7, 0.0, 0.0, 0.5 };
static real grid_y_color[] =            { 0.0, 0.7, 0.0, 0.5 };
static real scene_text_color[] =        { 1.0, 0.0, 0.0 };
static real active_scene_text_color[] = { 1.0, 1.0, 0.0 };
static real geom_default_color[] =      { 0.8, 0.8, 0.8 };
static real geom_label_color[] =        { 1.0, 0.0, 0.0 };
static GLfloat geom_specular[] =        { 1.0, 1.0, 1.0, 1.0 };
static GLfloat geom_shininess[] =       { 100.0 };

static scene *scene_head = NULL;
static scene *curr_scene = NULL;
static int scr_width = 640;
static int scr_height = 480;
static int show_grid = 1;
static real grid_size = 1.0;
static int redraw;

/*
 0 = screenshots requested by keyboard
 1 = screenshots requested by input
*/
static char screenshot_path[2][1000] = { { '\0' }, { '\0' } };

void reshape ();
void calc_normals(geometry *g);
void init_grid(void);
void draw_grid(void);
void draw_screen(void);
void draw_layer(scene *s, int layer_num);
void draw_labels(void);
void draw_scene_buttons(GLuint x, GLuint y);
int scene_button_hit_test(GLuint x0, GLuint y0, GLuint x, GLuint y);
void free_geom_shape(geometry *g);
void setup3d();
void init_layers();
void screenshot(char *path);

void GLFWCALL keyboard_callback(int key, int state);
void GLFWCALL mouse_button_callback(int button, int state);
void GLFWCALL mouse_wheel_callback(int pos);
void GLFWCALL mouse_position_callback(int x, int y);
void GLFWCALL win_resize_callback(int w, int h);
void GLFWCALL win_refresh_callback(void);

int main(int argc, char *argv[]) {
	int i, signal_redraw;
	GLFWthread input_thread;
	
	if (glfwInit() == GL_FALSE) {
		error("Failed to init glfw");
	}
	atexit(glfwTerminate);
	
	for (i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-d") == 0) {
			debug = 1;
		}
	}
	
	if (!init_input(argc, argv))
		exit(1);
	
	if (glfwOpenWindow(scr_width, scr_height, 0, 0, 0, 0, DEPTH_BITS, 0, GLFW_WINDOW) == GL_FALSE)
	{
		error("Failed to open glfw window");
	}
	glfwSetWindowTitle("SVS viewer");
	glfwSetMouseWheel(0);
	
	glfwSetWindowSizeCallback(win_resize_callback);
	glfwSetWindowRefreshCallback(win_refresh_callback);
	glfwSetKeyCallback(keyboard_callback);
	glfwSetMouseButtonCallback(mouse_button_callback);
	glfwSetMouseWheelCallback(mouse_wheel_callback);
	glfwSetMousePosCallback(mouse_position_callback);
	
	glClearColor(bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_color);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glMaterialfv(GL_FRONT, GL_SPECULAR, geom_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, geom_shininess);
	
	init_grid();
	init_font();
	init_layers();
	
	if (0) { /* for debugging single threaded */
		proc_input(NULL);
	} else {
		if (!(scene_lock = glfwCreateMutex()))
			error("Failed to create scene lock mutex.");
		init_semaphore(&redraw_semaphore);
		input_thread = glfwCreateThread(proc_input, NULL);
		if (input_thread < 0) {
			error("Unable to create input thread");
		}
	}
	
	reset_camera(&cam, '1');
	cam.ortho = 0;
	redraw = 1;
	signal_redraw = 0;
	while(glfwGetWindowParam(GLFW_OPENED)) {
		glfwWaitEvents();
		if (redraw) {
			draw_screen();
			redraw = 0;
		}
	}

	return 0;
}

void GLFWCALL keyboard_callback(int key, int state) {
	if (state == GLFW_RELEASE)
		return;
	
	switch (key) {
		case '1':
		case '2':
		case '3':
			reset_camera(&cam, key);
			redraw = 1;
			break;
		case 'G':
			show_grid = !show_grid;
			redraw = 1;
			break;
		case 'N':
			layers[0].draw_names = !layers[0].draw_names;
			redraw = 1;
			break;
		case '-':
			grid_size /= 10.0;
			init_grid();
			redraw = 1;
			break;
		case '=':
			grid_size *= 10.0;
			init_grid();
			redraw = 1;
			break;
		case 'W':
			layers[0].wireframe = !layers[0].wireframe;
			redraw = 1;
			break;
		case 'S':
			request_screenshot("screen.ppm", 0);
			redraw = 1;
			break;
		case 'O':
			cam.ortho = 1 - cam.ortho;
			redraw = 1;
	}
}

void GLFWCALL mouse_button_callback(int button, int state) {
	int x, y;
	
	if (state == GLFW_RELEASE)
		return;
	
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		glfwGetMousePos(&x, &y);
		if (scene_button_hit_test(SCENE_MENU_OFFSET, scr_height - SCENE_MENU_OFFSET, x, scr_height - y))
			redraw = 1;
	}
}

void GLFWCALL mouse_wheel_callback(int pos) {
	if (pos < 0) {
		zoom_camera(&cam, -10);
	} else if (pos > 0) {
		zoom_camera(&cam, 10);
	}
	redraw = 1;
	glfwSetMouseWheel(0);
}

void GLFWCALL mouse_position_callback(int x, int y) {
	static int ox = -1, oy = -1;
	int shift, ctrl, dx, dy;
	
	dx = x - ox;
	dy = y - oy;
	ox = x;
	oy = y;
	
	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
		return;
	
	shift = (glfwGetKey(GLFW_KEY_LSHIFT) == GLFW_PRESS) ||
	        (glfwGetKey(GLFW_KEY_RSHIFT) == GLFW_PRESS);

	ctrl = (glfwGetKey(GLFW_KEY_LCTRL) == GLFW_PRESS) ||
	       (glfwGetKey(GLFW_KEY_RCTRL) == GLFW_PRESS);

	if (shift) {
		pan_camera(&cam, dx, dy);
		redraw = 1;
	} else if (ctrl) {
		if (!cam.ortho) {
			pull_camera(&cam, dy);
			redraw = 1;
		}
	} else {
		rotate_camera(&cam, x, y, dx, dy);
		redraw = 1;
	}
}

void init_grid() {
	int i, j;
	real g;
	
	g = GRID_LINES * grid_size;
	i = 0;
	for (j = -GRID_LINES; j <= GRID_LINES; ++j) {
		if (j == 0)
			continue;
		grid_verts[i++] = j * grid_size; /* x1 */
		grid_verts[i++] = -g;            /* y1 */
		grid_verts[i++] = j * grid_size; /* x2 */
		grid_verts[i++] = g;             /* y2 */
	}
	
	for (j = -GRID_LINES; j <= GRID_LINES; ++j) {
		if (j == 0)
			continue;
		grid_verts[i++] = -g;             /* x1 */
		grid_verts[i++] = j * grid_size;  /* y1 */
		grid_verts[i++] = g;              /* x2 */
		grid_verts[i++] = j * grid_size;  /* y2 */
	}
	
	assert(i == GRID_VERTS_SIZE);
}

void draw_grid() {
	real g = GRID_LINES * grid_size;
	
	setup3d();
	glDisable(GL_LIGHTING);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColor4dv(grid_color);
	glVertexPointer(2, GL_DOUBLE, 0, grid_verts);
	glDrawArrays(GL_LINES, 0, GRID_VERTS_SIZE / 2);
	
	glColor4dv(grid_x_color);
	glBegin(GL_LINES);
		glVertex3f(0.0, -g, 0.0);
		glVertex3f(0.0, g, 0.0);
	glEnd();
	
	glColor4dv(grid_y_color);
	glBegin(GL_LINES);
		glVertex3f(-g, 0.0, 0.0);
		glVertex3f(g, 0.0, 0.0);
	glEnd();
	glDisableClientState(GL_VERTEX_ARRAY);
}

void draw_screen() {
	int i;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (show_grid)
		draw_grid();
	
	glfwLockMutex(scene_lock);
	if (curr_scene) {
		for (i = 0; i < NLAYERS; ++i) {
			draw_layer(curr_scene, i);
		}
		draw_labels();
	}
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLfloat) scr_width, 0.0, (GLfloat) scr_height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	draw_scene_buttons(SCENE_MENU_OFFSET, scr_height - SCENE_MENU_OFFSET);
	
	glFinish();
	for (i = 0; i < 2; ++i) {
		if (screenshot_path[i][0] != '\0') {
			screenshot(screenshot_path[i]);
			screenshot_path[i][0] = '\0';
			if (i == 1)
				semaphore_V(&redraw_semaphore);
		}
	}
	glfwUnlockMutex(scene_lock);
	glfwSwapBuffers();
}

void GLFWCALL win_resize_callback(int w, int h) {
	scr_width = w;
	scr_height = h;
	glViewport (0, 0, (GLsizei) scr_width, (GLsizei) scr_height);
	redraw = 1;
}

void GLFWCALL win_refresh_callback(void) {
	redraw = 1;
}

void pan_camera(camera *c, real dx, real dy) {
	c->pos[0] += PAN_FACTOR * dx;
	c->pos[1] -= PAN_FACTOR * dy;
}

void rotate_camera(camera *c, int x, int y, int dx, int dy) {
	int lastx, lasty;
	double p1x, p1y, p2x, p2y, q1[4], q2[4];

	lastx = x - dx;
	lasty = y - dy;
	p1x = (2.0 * lastx - scr_width) / (double) scr_width;
	p1y = (scr_height - 2.0 * lasty) / (double) scr_height;
	p2x = (2.0 * x - scr_width) / (double) scr_width;
	p2y = (scr_height - 2.0 * y) / (double) scr_height;
	
	/* printf("rotate in: %g %g %g %g\n", p1x, p1y, p2x, p2y); */
	trackball(q1, 0.8, p1x, p1y, p2x, p2y);
	q2[0] = c->q[0]; q2[1] = c->q[1]; q2[2] = c->q[2]; q2[3] = c->q[3];
	add_quats(q1, q2, c->q);
	quat_to_mat(c->q, c->rot_mat);
}

void pull_camera(camera *c, real dz) {
	c->pos[2] += PAN_FACTOR * dz;
}

void zoom_camera(camera *c, real df) {
	c->fovy += df;
	if (c->fovy < FOVY_MIN) {
		c->fovy = FOVY_MIN;
	} else if (c->fovy > FOVY_MAX) {
		c->fovy = FOVY_MAX;
	}
	c->orthoy = (int) (tan(c->fovy * PI / 360.0) * abs(c->pos[2]));
}

void reset_camera(camera *c, int key) {
	set_vec3(c->pos, 0.0, 0.0, -10.0);
	c->fovy = FOVY_DEF;
	zoom_camera(c, 0);
	switch (key) {
		case '1':
			set_quat(c->q, 0.0, 0.0, 0.0, 1.0);
			break;
		case '2':
			set_quat(c->q, 0.5, 0.5, 0.5, 0.5);
			break;
		case '3':
			set_quat(c->q, 0.7071067811865476, 0.0, 0.0, 0.7071067811865476);
			break;
	}
	quat_to_mat(c->q, c->rot_mat);
}

void apply_camera(camera *c) {
	glMatrixMode(GL_MODELVIEW);
	glTranslated(c->pos[0], c->pos[1], c->pos[2]);
	glMultMatrixd(c->rot_mat);
}

void init_geom(geometry *g, char *name) {
	g->name = strdup(name);
	if (!g->name) {
		perror("init_geom: ");
		exit(1);
	}
	g->layer = 0;
	g->line_width = 1.0;
	
	set_vec3(g->pos, 0.0, 0.0, 0.0);
	set_vec3(g->scale, 1.0, 1.0, 1.0);
	set_vec3(g->axis, 0.0, 0.0, 1.0);
	g->angle = 0.0;
	copy_vec3(geom_default_color, g->color);
	
	g->vertices = NULL;
	g->indexes = NULL;
	g->ninds = 0;
	
	g->quadric = NULL;
	g->radius = -1.0;
	
	g->text = NULL;
	
	g->next = NULL;
}

void free_geom_shape(geometry *g) {
	if (g->vertices) {
		free(g->vertices);
		free(g->indexes);
		g->vertices = NULL;
		g->indexes = NULL;
		if (g->normals) {
			free(g->normals);
			g->normals = NULL;
		}
	}
	g->ninds = -1;
	if (g->quadric) {
		gluDeleteQuadric(g->quadric);
		g->quadric = NULL;
	}
	g->radius = 0.0;
	if (g->text) {
		free(g->text);
		g->text = NULL;
	}
}

void set_geom_vertices(geometry *g, real *vertices, int nverts) {
	int i, ninds, indexes[MAX_INDS];
	
	free_geom_shape(g);
	g->vertices = (real *) malloc(sizeof(real) * nverts);
	for (i = 0; i < nverts; ++i) {
		g->vertices[i] = vertices[i];
	}
	if (nverts < 3) {
		ninds = nverts / 3;
		for (i = 0; i < ninds; ++i) {
			indexes[i] = i;
		}
	} else {
		ninds = qhull(vertices, nverts, indexes, MAX_INDS);
	}
	g->indexes = (GLuint *) malloc(sizeof(GLuint) * ninds);
	for (i = 0; i < ninds; ++i) {
		g->indexes[i] = indexes[i];
	}
	g->ninds = ninds;
	
	if (g->ninds >= 3) {
		/*
		 Since each vertex in a triangle share the same normal, there are
		 ninds/3 normals, each of size 3, so the normals array is length
		 ninds
		*/
		g->normals = (real *) malloc(sizeof(real) * g->ninds);
		if (!g->normals) {
			perror("init_geom: ");
			exit(1);
		}
		calc_normals(g);
	}
}

void set_geom_radius(geometry *g, real radius) {
	free_geom_shape(g);
	g->radius = radius;
	g->quadric = gluNewQuadric();
	if (!g->quadric) {
		error("Not enough memory for quadric");
	}
	gluQuadricNormals(g->quadric, GLU_SMOOTH);
}

void set_geom_text(geometry *g, char *text) {
	free_geom_shape(g);
	if (!(g->text = strdup(text))) {
		error("Not enough memory for text");
	}
}

void calc_normals(geometry *g) {
	int i, j;
	vec3 e1, e2;
	real *v1, *v2, *v3, *n;
	
	for (i = 0; i < g->ninds; i += 3) {
		v1 = &g->vertices[g->indexes[i] * 3];
		v2 = &g->vertices[g->indexes[i+1] * 3];
		v3 = &g->vertices[g->indexes[i+2] * 3];
		n = &g->normals[i];
		
		copy_vec3(v1, e1);
		subtract_vec3(e1, v2);
		copy_vec3(v2, e2);
		subtract_vec3(e2, v3);
		norm_cross_prod(e1, e2, n);
	}
}

void draw_geom(geometry *g) {
	int i;
	
	glColor3dv(g->color);
	glLineWidth(g->line_width);
	glPushMatrix();
	glTranslated(g->pos[0], g->pos[1], g->pos[2]);
	glRotated(g->angle, g->axis[0], g->axis[1], g->axis[2]);
	glScaled(g->scale[0], g->scale[1], g->scale[2]);
	
	if (g->ninds >= 0) {
		glVertexPointer(3, GL_DOUBLE, 0, g->vertices);
		if (g->ninds == 1) {
			glBegin(GL_POINTS);
			glArrayElement(g->indexes[0]);
			glEnd();
		} else if (g->ninds == 2) {
			glBegin(GL_LINES);
			glArrayElement(g->indexes[0]);
			glArrayElement(g->indexes[1]);
			glEnd();
		} else {
			for (i = 0; i < g->ninds; i += 3) {
				glNormal3dv(&g->normals[i]);
				glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &g->indexes[i]);
			}
		}
	} else if (g->quadric) {
		gluSphere(g->quadric, g->radius, 10, 10);
	} else if (g->text) {
		draw_text(g->text, 0, 0);
	} else {
		fprintf(stderr, "geometry %s has no shape\n", g->name);
	}
	glPopMatrix();
	glLineWidth(1.0);
}

scene *find_or_add_scene(char *name) {
	scene *s;
	
	for (s = scene_head; s && strcmp(s->name, name) != 0; s = s->next)
		;
	
	if (!s) {
		if (!(s = (scene *) malloc(sizeof(scene)))) {
			perror("find_or_add_scene: ");
			exit(1);
		}
		init_scene(s, name);
		s->next = scene_head;
		scene_head = s;
		if (!curr_scene)
			curr_scene = s;
	}
	return s;
}

int delete_scenes(char *pattern) {
	int n;
	scene *p, *s;
	
	for (n = 0, p = NULL, s = scene_head; s ; ) {
		if (match(pattern, s->name)) {
			if (p) {
				p->next = s->next;
				if (curr_scene == s)
					curr_scene = p;
				destroy_scene(s);
				s = p->next;
			} else {
				scene_head = s->next;
				if (curr_scene == s)
					curr_scene = scene_head;
				destroy_scene(s);
				s = scene_head;
			}
			++n;
		} else {
			p = s;
			s = s->next;
		}
	}
	return n;
}

void init_scene(scene *s, char *name) {
	int i;
	
	s->name = strdup(name);
	if (!s->name) {
		perror("init_scene: ");
		exit(1);
	}
	s->geoms = NULL;
	s->next = NULL;
}

void destroy_geom(geometry *g) {
	free_geom_shape(g);
	free(g->name);
	free(g);
}
	
void destroy_scene(scene *s) {
	geometry *p, *pn;
	
	free(s->name);
	for (p = s->geoms; p; ) {
		pn = p->next;
		destroy_geom(p);
		p = pn;
	}
	free(s);
}

void setup3d() {
	real aspect;
	
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	aspect = scr_width / (double) scr_height;
	if (cam.ortho) {
		glOrtho(-cam.orthoy * aspect, cam.orthoy * aspect, -cam.orthoy, cam.orthoy, NEAR_CLIP, FAR_CLIP );
	} else {
		gluPerspective(cam.fovy, aspect, NEAR_CLIP, FAR_CLIP);
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	apply_camera(&cam);
}

void draw_layer(scene *s, int layer_num) {
	GLint view[4];
	real modelview[16], proj[16];
	layer *l;
	geometry *g;
	int empty;
	
	if (layer_num > 0) {
		for (empty = 1, g = s->geoms; g; g = g->next) {
			if (g->layer == layer_num) {
				empty = 0;
				break;
			}
		}
		if (empty) {
			return;
		}
	}
	
	l = &layers[layer_num];
	if (l->lighting) {
		glEnable(GL_LIGHTING);
	} else {
		glDisable(GL_LIGHTING);
	}
	if (l->clear_depth) {
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	
	if (l->flat) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0.0, (GLfloat) scr_width, 0.0, (GLfloat) scr_height);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	} else {
		setup3d();
	}
	
	if (!l->wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	} else {
		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	glEnableClientState(GL_VERTEX_ARRAY);
	for (g = s->geoms; g; g = g->next) {
		if (g->layer == layer_num) {
			draw_geom(g);
		}
	}
	glDisableClientState(GL_VERTEX_ARRAY);
		
	glGetDoublev(GL_PROJECTION_MATRIX, l->last_projection);
	glGetDoublev(GL_MODELVIEW_MATRIX, l->last_modelview);
	glGetIntegerv(GL_VIEWPORT, l->last_view);
}

void draw_labels() {
	int i;
	layer *l;
	geometry *g;
	real wx, wy, wz;
	
	if (!curr_scene) {
		return;
	}
	
	for (i = 0; i < NLAYERS; ++i) {
		l = &layers[i];
		if (!l->draw_names) {
			continue;
		}
		glClear(GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0.0, (GLfloat) scr_width, 0.0, (GLfloat) scr_height);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glColor3dv(geom_label_color);
		glDisable(GL_LIGHTING);
		
		for (g = curr_scene->geoms; g; g = g->next) {
			if (g->layer == i) {
				gluProject(g->pos[0], g->pos[1], g->pos[2], l->last_modelview, l->last_projection, l->last_view, &wx, &wy, &wz);
				draw_text(g->name, (int) wx, (int) wy);
			}
		}
	}
}

int match_scenes(char *pattern, scene *scns[], int n) {
	int m;
	scene *p;
	
	for (m = 0, p = scene_head; p && m < n; p = p->next) {
		if (match(pattern, p->name))
			scns[m++] = p;
	}
	return m;
}

geometry *find_or_add_geom(scene *s, char *name) {
	int i;
	geometry *g;
	
	for (g = s->geoms; g; g = g->next) {
		if (strcmp(g->name, name) == 0) {
			return g;
		}
	}
	
	if (!(g = (geometry *) malloc(sizeof(geometry)))) {
		perror("find_or_add_geom: ");
		exit(1);
	}
	init_geom(g, name);
	g->next = s->geoms;
	s->geoms = g;
	return g;
}

int match_geoms(scene *s, char *pattern, geometry **geoms, int n) {
	int m;
	geometry *g;
	
	for (m = 0, g = s->geoms; g && m < n; g = g->next) {
		if (match(pattern, g->name))
			geoms[m++] = g;
	}
	return m;
}

int delete_geoms(scene *s, char *pattern) {
	int n;
	geometry *p, *g;
	
	for (n = 0, p = NULL, g = s->geoms; g; ) {
		if (match(pattern, g->name)) {
			if (p) {
				p->next = g->next;
				destroy_geom(g);
				g = p->next;
			} else {
				s->geoms = g->next;
				destroy_geom(g);
				g = s->geoms;
			}
			++n;
		} else {
			p = g;
			g = g->next;
		}
	}
	return n;
}

int scene_button_hit_test(GLuint x0, GLuint y0, GLuint x, GLuint y) {
	scene *p;
	GLuint yb, w;
	
	for (yb = y0, p = scene_head; p; yb -= FONT_HEIGHT, p = p->next) {
		w = strlen(p->name) * FONT_WIDTH;
		if (p != curr_scene &&
		    (x0 <= x && x <= x0 + w) &&
		    (yb <= y && y <= yb + FONT_HEIGHT))
		{
			curr_scene = p;
			return 1;
		}
	}
	return 0;
}

void draw_scene_buttons(GLuint x, GLuint y) {
	scene *p;
	
	glDisable(GL_LIGHTING);
	glColor3dv(scene_text_color);
	for (p = scene_head; p; p = p->next, y -= FONT_HEIGHT) {
		if (p == curr_scene)
			glColor3dv(active_scene_text_color);
		
		draw_text(p->name, x, y);
		glColor3dv(scene_text_color);
	}
}

void screenshot(char *path) {
	FILE *out;
	unsigned char *pixels;
	int i, j, k;
	
	pixels = calloc(3 * scr_width * scr_height, sizeof(unsigned char));
	if (!pixels) {
		error("out of memory");
	}
	out = fopen(path, "w");
	if (!out) {
		perror("screenshot");
		return;
	}
	fprintf(out, "P6\n%d %d\n255\n", scr_width, scr_height);
	
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, scr_width, scr_height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	
	for (i = scr_height - 1; i >= 0; --i) {
		for (j = 0; j < scr_width; ++j) {
			k = (i * scr_width + j) * 3;
			fwrite(&pixels[k], sizeof(unsigned char), 3, out);
		}
	}
	fclose(out);
	free(pixels);
	fprintf(stderr, "screen shot saved to %s\n", path);
}

void request_screenshot(char *path, int i) {
	strncpy(screenshot_path[i], path, sizeof(screenshot_path[i]));
}

void init_layers() {
	int i;
	for (i = 0; i < NLAYERS; ++i) {
		layers[i].lighting = 0;
		layers[i].flat = 0;
		layers[i].clear_depth = 1;
		layers[i].draw_names = 0;
		layers[i].wireframe = 0;
	}
	layers[0].lighting = 1;
	layers[0].draw_names = 1;
}

int set_layer(int layer_num, char option, int value) {
	assert(0 <= layer_num && layer_num < NLAYERS);
	switch (option) {
		case 'l':
			layers[layer_num].lighting = value;
			return 1;
		case 'f':
			layers[layer_num].flat = value;
			return 1;
		case 'd':
			layers[layer_num].clear_depth = value;
			return 1;
		case 'n':
			layers[layer_num].draw_names = value;
			return 1;
		case 'w':
			layers[layer_num].wireframe = value;
			return 1;
	}
	return 0;  /* no such option */
}


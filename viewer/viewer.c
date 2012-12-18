#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#include <GL/glu.h>

#include "viewer.h"
#include "trackball.h"

/*
 For GRID_LINES = 10, there would be 20 lines along x axis and 20 lines
 along y axis (exclude axes). Each line is composed of 4 doubles (x1, y1), (x2, y2)
*/
#define GRID_VERTS_SIZE (4 * 2 * (GRID_LINES * 2))
#define SCENE_MENU_OFFSET 20

static real grid_verts[GRID_VERTS_SIZE];

static camera cam = {
	{ 0.0, 0.0, 0.0, 1.0 },
	{ 0.0, 0.0, -10.0 }
};

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

SDL_mutex *scene_lock;
static scene *scene_head = NULL;
static scene *curr_scene = NULL;
static int scr_width = 640;
static int scr_height = 480;
static int show_grid = 1;
static int show_labels = 1;
static real grid_size = 1.0;

void calc_normals(geometry *g);
void init_grid(void);
void draw_grid(void);
void draw_screen(void);
void reshape (int w, int h);
void draw_geom_labels(scene *s, real *modelview, real *proj, GLint *view);
void draw_scene_buttons(GLuint x, GLuint y);
int scene_button_hit_test(GLuint x0, GLuint y0, GLuint x, GLuint y);
void free_geom_shape(geometry *g);

int main(int argc, char* argv[]) {
	int flags, bpp;
	int buttons[4] = {0, 0, 0, 0 };
	int redraw;
	const SDL_VideoInfo* info;
	SDL_Event evt;
	SDL_Surface *screen;
	SDL_Thread *input_thread;
	SDLMod mod;
	
	atexit(SDL_Quit);
	
	if (!init_input(argc, argv))
		exit(1);
	
	flags = SDL_OPENGL | SDL_RESIZABLE;
	if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Video initialization failed: %s\n", SDL_GetError());
		return 1;
	}

	if(!(info = SDL_GetVideoInfo())) {
		fprintf(stderr, "Video query failed: %s\n", SDL_GetError());
		return 1;
	}
	bpp = info->vfmt->BitsPerPixel;
	
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 32 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	if(!(screen = SDL_SetVideoMode(scr_width, scr_height, bpp, flags))) {
		fprintf( stderr, "Video mode set failed: %s\n", SDL_GetError() );
		return 1;
	}
	SDL_WM_SetCaption("SVS viewer", NULL);
	
	reshape(screen->w, screen->h);
	glClearColor(bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
	glShadeModel(GL_FLAT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_color);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	
	glMaterialfv(GL_FRONT, GL_SPECULAR, geom_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, geom_shininess);
		
	init_grid();
	init_font();
	
	if (0) { /* for debugging single threaded */
		proc_input(NULL);
	} else {
		scene_lock = SDL_CreateMutex();
		input_thread = SDL_CreateThread(proc_input, NULL);
		if (input_thread == NULL) {
			fprintf(stderr, "Unable to create input thread: %s\n", SDL_GetError());
			exit(1);
		}
	}
	
	redraw = 1;
	while(1) {
		SDL_WaitEvent(&evt);
		do {
			switch (evt.type) {
				case SDL_QUIT:
					return 0;
				case SDL_VIDEORESIZE:
					SDL_SetVideoMode(evt.resize.w, evt.resize.h, bpp, flags);
					reshape(evt.resize.w, evt.resize.h);
					redraw = 1;
					break;
				case SDL_VIDEOEXPOSE:
					redraw = 1;
					break;
				case SDL_KEYDOWN:
					switch (evt.key.keysym.sym) {
						case SDLK_1:
						case SDLK_2:
						case SDLK_3:
							reset_camera(&cam, evt.key.keysym.sym);
							redraw = 1;
							break;
						case SDLK_g:
							show_grid = !show_grid;
							redraw = 1;
							break;
						case SDLK_l:
							show_labels = !show_labels;
							redraw = 1;
							break;
						case SDLK_MINUS:
							grid_size /= 10.0;
							init_grid();
							redraw = 1;
							break;
						case SDLK_EQUALS:
							grid_size *= 10.0;
							init_grid();
							redraw = 1;
							break;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (evt.button.button == 1) {
						if (scene_button_hit_test(SCENE_MENU_OFFSET, scr_height - SCENE_MENU_OFFSET, 
						                          evt.motion.x, scr_height - evt.motion.y))
							redraw = 1;
					}
					
					if (evt.button.button <= 3) {
						buttons[evt.button.button] = 1;
					} else if (evt.button.button == 4) {
						zoom_camera(&cam, 20);
						redraw = 1;
					} else if (evt.button.button == 5) {
						zoom_camera(&cam, -20);
						redraw = 1;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if (evt.button.button <= 3)
						buttons[evt.button.button] = 0;
					break;
				case SDL_MOUSEMOTION:
					mod = SDL_GetModState();
					if (buttons[3] && (mod & KMOD_LSHIFT || mod & KMOD_RSHIFT)) {
						pan_camera(&cam, evt.motion.xrel, evt.motion.yrel);
						redraw = 1;
					} else if (buttons[3] && (mod & KMOD_LCTRL || mod & KMOD_RCTRL)) {
						zoom_camera(&cam, evt.motion.yrel);
						redraw = 1;
					} else if (buttons[3]) {
						rotate_camera(&cam, evt.motion.x, evt.motion.y, evt.motion.xrel, evt.motion.yrel);
						redraw = 1;
					}
					break;
				case SDL_USEREVENT:
					redraw = 1;
					break;
			}
		} while (SDL_PollEvent(&evt));
		
		if (redraw) {
			draw_screen();
			redraw = 0;
		}
	}

	return 0;
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
}

void draw_screen() {
	GLint view[4];
	real modelview[16], proj[16];
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, scr_width / (double) scr_height, 0.1, 100.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	apply_camera(&cam);
	
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetIntegerv(GL_VIEWPORT, view);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	
	if (show_grid)
		draw_grid();
	
	if (curr_scene) {
		glEnable(GL_LIGHTING);
		SDL_mutexP(scene_lock);
		draw_scene(curr_scene);
		SDL_mutexV(scene_lock);
		glDisable(GL_LIGHTING);
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	
	/* draw all text */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLfloat) scr_width, 0.0, (GLfloat) scr_height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	draw_scene_buttons(SCENE_MENU_OFFSET, scr_height - SCENE_MENU_OFFSET);
	if (curr_scene && show_labels) {
		draw_geom_labels(curr_scene, modelview, proj, view);
	}
	
	glFinish();
	SDL_GL_SwapBuffers();
}

void reshape (int w, int h) {
	scr_width = w;
	scr_height = h;
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
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
}

void zoom_camera(camera *c, real dz) {
	c->pos[2] += ZOOM_FACTOR * dz;
}

void reset_camera(camera *c, SDLKey k) {
	set_vec3(c->pos, 0.0, 0.0, -10.0);
	switch (k) {
		case SDLK_1:
			set_quat(c->q, 0.0, 0.0, 0.0, 1.0);
			break;
		case SDLK_2:
			set_quat(c->q, 0.5, 0.5, 0.5, 0.5);
			break;
		case SDLK_3:
			set_quat(c->q, 0.7071067811865476, 0.0, 0.0, 0.7071067811865476);
			break;
	}
}

void apply_camera(camera *c) {
	real rot_mat[16];
	quat_to_mat(c->q, rot_mat);
	
	glMatrixMode(GL_MODELVIEW);
	glTranslated(c->pos[0], c->pos[1], c->pos[2]);
	glMultMatrixd(rot_mat);
}

void init_geom(geometry *g, char *name) {
	g->name = strdup(name);
	if (!g->name) {
		perror("init_geom: ");
		exit(1);
	}
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
}

void set_geom_vertices(geometry *g, real *vertices, int nverts, GLuint *indexes, int ninds) {
	int i;
	
	free_geom_shape(g);
	g->vertices = (real *) malloc(sizeof(real) * nverts);
	for (i = 0; i < nverts; ++i) {
		g->vertices[i] = vertices[i];
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
		fprintf(stderr, "Not enough memory for quadric\n");
		exit(1);
	}
	gluQuadricNormals(g->quadric, GLU_SMOOTH);
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

void destroy_geom(geometry *g) {
	free_geom_shape(g);
	free(g->name);
	free(g);
}
	
void draw_geom(geometry *g) {
	int i;
	
	glColor3dv(g->color);
	glPushMatrix();
	glTranslated(g->pos[0], g->pos[1], g->pos[2]);
	glRotated(g->angle, g->axis[0], g->axis[1], g->axis[2]);
	glScaled(g->scale[0], g->scale[1], g->scale[2]);
	
	if (g->ninds == -1) {
		gluSphere(g->quadric, g->radius, 10, 10);
	} else {
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
	}
	glPopMatrix();
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
	s->name = strdup(name);
	if (!s->name) {
		perror("init_scene: ");
		exit(1);
	}
	s->geoms = NULL;
	s->next = NULL;
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

void draw_scene(scene *s) {
	geometry *p;
	for (p = s->geoms; p; p = p->next) {
		draw_geom(p);
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
	geometry *g;
	for (g = s->geoms; g && strcmp(g->name, name) != 0; g = g->next)
		;
	
	if (!g) {
		if (!(g = (geometry *) malloc(sizeof(geometry)))) {
			perror("find_or_add_geom: ");
			exit(1);
		}
		init_geom(g, name);
		g->next = s->geoms;
		s->geoms = g;
	}
	return g;
}

int match_geoms(scene *s, char *pattern, geometry **geoms, int n) {
	int m;
	geometry *p;
	for (m = 0, p = s->geoms; p && m < n; p = p->next) {
		if (match(pattern, p->name))
			geoms[m++] = p;
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

void draw_geom_labels(scene *s, real *modelview, real *proj, GLint *view) {
	geometry *g;
	real wx, wy, wz;
	
	glColor3dv(geom_label_color);
	for (g = s->geoms; g; g = g->next) {
		gluProject(g->pos[0], g->pos[1], g->pos[2], modelview, proj, view, &wx, &wy, &wz);
		draw_text(g->name, wx, wy);
	}
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
	
	glColor3dv(scene_text_color);
	for (p = scene_head; p; p = p->next, y -= FONT_HEIGHT) {
		if (p == curr_scene)
			glColor3dv(active_scene_text_color);
		
		draw_text(p->name, x, y);
		glColor3dv(scene_text_color);
	}
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

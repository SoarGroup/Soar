#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>
#include "viewer.h"

#define MAX_FIELDS 1000
#define MAX_SCENES 1000
#define MAX_GEOMS 1000
#define WHITESPACE " \t\n"

int proc_cmd(char *fields[]);
int proc_geom_cmd(geometry *gs[], int ngeoms, char *fields[]);
int parse_nums(char *fields[], int n, real *v);
void request_redraw();
int split(char *s, char *fields[]);
void fix_literal_quotes(char *s);
int proc_layer_cmd(char *fields[]);

int wait_for_redraw = 0;

/*
 split using whitespace and double quotes. \" is a literal quote in quoted
 fields
*/
int split(char *s, char *fields[]) {
	char *p;
	int n, quoted, done;
	
	memset(fields, 0, MAX_FIELDS * sizeof(char*));
	for (n = 0, quoted = 0, done = 0, p = s; !done && n < MAX_FIELDS; ++p) {
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

int proc_input(void *unused) {
	char cmd[MAX_COMMAND];
	char *fields[MAX_FIELDS];
	char *endp, *startp, *startp2, *cp;
	int left, n, i;
	
	startp = endp = cmd;
	left = MAX_COMMAND;
	
	for(;;) {
		n = get_input(endp, left);
		if (n < 0) {
			/* error */
			break;
		}
		
		*(endp + n) = '\0';
		for (; n > 0; ++endp, --n) {
			if (*endp == '\n') {
				/* for now assume newline denotes end of complete command */
				*endp = '\0';
				/* strip comment */
				cp = strchr(startp, '#');
				if (cp) {
					*cp = '\0';
				}
				if (debug)
					printf("cmd: %s\n", startp);
				split(startp, fields);
				
				SDL_mutexP(scene_lock);
				proc_cmd(fields);
				SDL_mutexV(scene_lock);
				startp = endp + 1;
				
				request_redraw();
			}
		}
		
		/* shift remaining input left */
		if (startp != cmd) {
			for (startp2 = cmd; startp != endp; ++startp, ++startp2)
				*startp2 = *startp;
			startp = cmd;
			endp = startp2;
		}
		left = MAX_COMMAND - (endp - startp);
	}
	fprintf(stderr, "input thread has exited\n");
	return 0;
}

int proc_cmd(char *fields[]) {
	char *scene_pat, *geom_pat, **args;
	scene *scenes[MAX_SCENES], *sp;
	geometry *geoms[MAX_GEOMS], *gp;
	int nscenes, ngeoms, i;
	
	if (!fields[0]) {
		return 1;
	} else if (strcmp(fields[0], "save") == 0) {
		if (!fields[1]) {
			fprintf(stderr, "specify save path\n");
			return 0;
		}
		save_on_redraw(fields[1]);
		wait_for_redraw = 1;
		return 1;
	} else if (strcmp(fields[0], "layer") == 0) {
		return proc_layer_cmd(fields + 1);
	} else if (strcmp(fields[0], "draw") == 0) {
		scene_pat = fields[1];
		geom_pat = fields[2];
		args = fields + 3;
	} else {
		scene_pat = fields[0];
		geom_pat = fields[1];
		args = fields + 2;
	}
	
	if (!scene_pat) {
		return 1;
	}
	
	if (*scene_pat == '-') {
		delete_scenes(scene_pat + 1);
		return 1;
	} else if (*scene_pat == '+') {
		scenes[0] = find_or_add_scene(scene_pat + 1);
		nscenes = 1;
	} else {
		nscenes = match_scenes(scene_pat, scenes, MAX_SCENES);
	}
	
	if (!geom_pat) {
		return 1;
	}
	
	for (ngeoms = 0, i = 0; i < nscenes; ++i) {
		if (*geom_pat == '+') {
			geoms[ngeoms++] = find_or_add_geom(scenes[i], geom_pat + 1);
		} else if (*geom_pat == '-') {
			delete_geoms(scenes[i], geom_pat + 1);
		} else {
			ngeoms += match_geoms(scenes[i], geom_pat, geoms + ngeoms, MAX_GEOMS);
		}
	}
	
	return proc_geom_cmd(geoms, ngeoms, args);
}

int proc_geom_cmd(geometry *gs[], int ngeoms, char *fields[]) {
	real pos[3], rot[4], scale[3], color[3], verts[MAX_FIELDS], finds[MAX_FIELDS];
	GLuint inds[MAX_FIELDS];
	real radius, line_width, layer;
	int i, f, nverts, ninds, pos_set, rot_set, scale_set, color_set;
	char *text;
	
	pos_set = 0;
	rot_set = 0;
	scale_set = 0;
	color_set = 0;
	radius = -1.0;
	line_width = -1.0;
	layer = -1.0;
	nverts = -1;
	ninds = -1;
	text = NULL;
	
	f = 0;
	while (fields[f]) {
		if (strlen(fields[f]) != 1) {
			fprintf(stderr, "unexpected field: %s\n", fields[f]);
			return 0;
		}
		
		switch (fields[f][0]) {
			case 'p':  /* position */
				if (parse_nums(&fields[f+1], 3, pos) != 3) {
					fprintf(stderr, "invalid position\n");
					return 0;
				}
				pos_set = 1;
				f += 4;
				break;
			case 'r':  /* rotation */
				if (parse_nums(&fields[f+1], 4, rot) != 4) {
					fprintf(stderr, "invalid rotation\n");
					return 0;
				}
				rot_set = 1;
				f += 5;
				break;
			case 's':  /* scale */
				if (parse_nums(&fields[f+1], 3, scale) != 3) {
					fprintf(stderr, "invalid scale\n");
					return 0;
				}
				scale_set = 1;
				f += 4;
				break;
			case 'c':  /* color */
				if (parse_nums(&fields[f+1], 3, color) != 3) {
					fprintf(stderr, "invalid color\n");
					return 0;
				}
				color_set = 1;
				f += 4;
				break;
			case 'v':  /* vertices */
				if (nverts != -1) {
					fprintf(stderr, "you can only specify one set of vertices\n");
					return 0;
				}
				nverts = parse_nums(&fields[f+1], -1, verts);
				if (nverts % 3 != 0) {
					fprintf(stderr, "vertices must be have 3 components\n");
					return 0;
				}
				f += nverts + 1;
				break;
			case 'i':  /* vertex indexes */
				if (ninds != -1) {
					fprintf(stderr, "you can only specify one set of indexes\n");
					return 0;
				}
				ninds = parse_nums(&fields[f+1], -1, finds);
				if (ninds >= 3 && ninds % 3 != 0) {
					fprintf(stderr, "indexes must be in sets of 3\n");
					return 0;
				}
				for (i = 0; i < ninds; ++i) {
					inds[i] = finds[i];
				}
				f += ninds + 1;
				break;
			case 'b':  /* ball */
				if (parse_nums(&fields[f+1], 1, &radius) < 1 || radius < 0.0) {
					fprintf(stderr, "invalid radius\n");
					return 0;
				}
				f += 2;
				break;
			case 't':  /* text */
				text = fields[f+1];
				f += 2;
				break;
			case 'l':  /* layer */
				if (parse_nums(&fields[f+1], 1, &layer) < 1 || layer < 0.0) {
					fprintf(stderr, "expecting layer number (0 - %d)\n", NLAYERS);
					return 0;
				}
				f += 2;
				break;
			case 'w':  /* line width */
				if (parse_nums(&fields[f+1], 1, &line_width) < 1 || line_width < 0.0) {
					fprintf(stderr, "invalid line width\n");
					return 0;
				}
				f += 2;
				break;
			default:
				return 0;
		}
	}
	
	if ((nverts >= 0 && ninds == -1) || (nverts == -1 && ninds >= 0)) {
		fprintf(stderr, "you have to specify vertices and indexes together\n");
		return 0;
	} else if (ninds >= 0) {
		for (i = 0; i < ninds; ++i) {
			if (inds[i] < 0 || inds[i] >= nverts / 3) {
				fprintf(stderr, "index out of bounds: %d\n", inds[i]);
				return 0;
			}
		}
	}
	
	for (i = 0; i < ngeoms; ++i) {
		if (pos_set)
			copy_vec3(pos, gs[i]->pos);
		if (scale_set)
			copy_vec3(scale, gs[i]->scale);
		if (color_set)
			copy_vec3(color, gs[i]->color);
		if (rot_set)
			quat_to_axis_angle(rot, gs[i]->axis, &gs[i]->angle);
		
		if (radius >= 0.0) {
			set_geom_radius(gs[i], radius);
		} else if (nverts != -1 && ninds != -1) {
			set_geom_vertices(gs[i], verts, nverts, inds, ninds);
		} else if (text != NULL) {
			set_geom_text(gs[i], text);
		}
		
		if (layer >= 0.0) {
			gs[i]->layer = layer;
		}
		if (line_width >= 0.0) {
			gs[i]->line_width = line_width;
		}
	}
	return 1;
}


int parse_nums(char *fields[], int n, real *v) {
	int i;
	char *s, *end;
	
	for (i = 0; i != n && fields[i]; ++i) {
		v[i] = strtod(fields[i], &end);
		if (*end != '\0') {
			return i;
		}
	}
	return i;
}

/* wait until the command is processed by main thread */
void request_redraw() {
	SDL_Event e;
	e.type = SDL_USEREVENT;
	e.user.code = wait_for_redraw;
	SDL_PushEvent(&e);
	if (wait_for_redraw) {
		semaphore_P(&redraw_semaphore);
		wait_for_redraw = 0;
	}
}

int proc_layer_cmd(char *fields[]) {
	int layer_num, val, i;
	real num;
	
	if (!fields[0] || parse_nums(&fields[0], 1, &num) != 1) {
		fprintf(stderr, "expecting layer number\n");
		return 0;
	}
	layer_num = (int) num;
	if (layer_num < 0 || layer_num >= NLAYERS) {
		fprintf(stderr, "layer number has to be between 0 and %d", NLAYERS);
		return 0;
	}
	for (i = 1; i < MAX_FIELDS && fields[i]; i += 2) {
		if (!fields[i+1] || parse_nums(&fields[i+1], 1, &num) != 1) {
			fprintf(stderr, "expecting 0/1\n");
			return 0;
		}
		val = ((num == 0.0) ? 0 : 1);
		if (!set_layer(layer_num, fields[i][0], val)) {
			fprintf(stderr, "no such layer option: %s\n", fields[i]);
			return 0;
		}
	}
	return 1;
}

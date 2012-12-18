#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>
#include "viewer.h"

#define MAX_FIELDS 1024
#define MAX_SCENES 1000
#define MAX_GEOMS 1000
#define WHITESPACE " \t\n"

int proc_cmd(char *cmd);
int proc_geom_cmd(geometry *gs[], int ngeoms);
int parse_nums(real *v, int n, char **last);
void gen_request_event();

int proc_input(void *unused) {
	char cmd[MAX_COMMAND];
	char *endp, *startp, *startp2;
	int left, n;
	
	startp = endp = cmd;
	left = MAX_COMMAND;
	while (1) {
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
				SDL_mutexP(scene_lock);
				gen_request_event();
				proc_cmd(startp);
				SDL_mutexV(scene_lock);
				startp = endp + 1;
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

int proc_cmd(char *cmd) {
	char *scene_pat, *geom_pat;
	scene *scenes[MAX_SCENES], *sp;
	geometry *geoms[MAX_GEOMS], *gp;
	int scenes_matched, geoms_matched, i;
	
	/* printf("cmd: %s\n", cmd); */
	if (!(scene_pat = strtok(cmd, WHITESPACE)))
		return 1;
	
	if (*scene_pat == '-') {
		delete_scenes(scene_pat + 1);
		return 1;
	} else if (*scene_pat == '+') {
		scenes[0] = find_or_add_scene(scene_pat + 1);
		scenes_matched = 1;
	} else {
		scenes_matched = match_scenes(scene_pat, scenes, MAX_SCENES);
	}
	
	if (!(geom_pat = strtok(NULL, WHITESPACE)))
		return 1;
	
	for (geoms_matched = 0, i = 0; i < scenes_matched; ++i) {
		if (*geom_pat == '+') {
			geoms[geoms_matched++] = find_or_add_geom(scenes[i], geom_pat + 1);
		} else if (*geom_pat == '-') {
			delete_geoms(scenes[i], geom_pat + 1);
		} else {
			geoms_matched += match_geoms(scenes[i], geom_pat, geoms + geoms_matched, MAX_GEOMS);
		}
	}
	
	return proc_geom_cmd(geoms, geoms_matched);
}

int proc_geom_cmd(geometry *gs[], int ngeoms) {
	char *token;
	real pos[3], rot[4], scale[3], color[3];
	real numbers[MAX_FIELDS];
	real *verts, radius;
	GLuint *inds;
	int i, nverts, ninds, pos_set, rot_set, scale_set, color_set;
	
	pos_set = 0;
	rot_set = 0;
	scale_set = 0;
	color_set = 0;
	verts = NULL;
	inds = NULL;
	token = strtok(NULL, WHITESPACE);
	radius = -1.0;
	while (token) {
		if (*token == '#')
			break;
		
		if (strlen(token) != 1) {
			fprintf(stderr, "unexpected token: %s\n", token);
			goto Error;
		}
		
		switch (*token) {
			case 'p':
				if (parse_nums(pos, 3, &token) != 3) {
					fprintf(stderr, "invalid position\n");
					goto Error;
				}
				pos_set = 1;
				break;
			case 'r':
				if (parse_nums(rot, 4, &token) != 4) {
					fprintf(stderr, "invalid rotation\n");
					goto Error;
				}
				rot_set = 1;
				break;
			case 's':
				if (parse_nums(scale, 3, &token) != 3) {
					fprintf(stderr, "invalid scale\n");
					goto Error;
				}
				scale_set = 1;
				break;
			case 'c':
				if (parse_nums(color, 3, &token) != 3) {
					fprintf(stderr, "invalid color\n");
					goto Error;
				}
				color_set = 1;
				break;
			case 'v':
				if (verts) {
					fprintf(stderr, "you can only specify one set of vertices\n");
					goto Error;
				}
				nverts = parse_nums(numbers, -1, &token);
				if (nverts % 3 != 0) {
					fprintf(stderr, "vertices must be have 3 components\n");
					goto Error;
				}
				if (!(verts = (real *) malloc(sizeof(real) * nverts))) {
					perror("proc_cmd: ");
					goto Error;
				}
				memcpy(verts, numbers, sizeof(real) * nverts);
				break;
			case 'i':
				if (inds) {
					fprintf(stderr, "you can only specify one set of indexes\n");
					goto Error;
				}
				ninds = parse_nums(numbers, -1, &token);
				if (ninds >= 3 && ninds % 3 != 0) {
					fprintf(stderr, "indexes must be in sets of 3\n");
					goto Error;
				}
				if (!(inds = (GLuint *) malloc(sizeof(GLuint) * ninds))) {
					perror("proc_cmd: ");
					goto Error;
				}
				for (i = 0; i < ninds; ++i) {
					inds[i] = numbers[i];
				}
				break;
			case 'b':
				if (parse_nums(&radius, 1, &token) < 1 || radius < 0.0) {
					fprintf(stderr, "invalid radius\n");
					goto Error;
				}
				break;
			default:
				return 0;
		}
		
		if (!token)
			token = strtok(NULL, WHITESPACE);
	}
	
	if ((verts && !inds) || (!verts && inds)) {
		fprintf(stderr, "you have to specify vertices and indexes together\n");
		goto Error;
	} else if (verts && inds) {
		for (i = 0; i < ninds; ++i) {
			if (inds[i] < 0 || inds[i] >= nverts) {
				fprintf(stderr, "index out of bounds: %d\n", inds[i]);
				goto Error;
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
		} else if (verts && inds) {
			set_geom_vertices(gs[i], verts, nverts, inds, ninds);
		}
	}
	return 1;
	
Error:
	if (verts)
		free(verts);
	if (inds)
		free(inds);
	return 0;
}


int parse_nums(real *v, int n, char **last) {
	int i;
	char *s, *end;
	
	*last = NULL;
	for (i = 0; i != n && i < MAX_FIELDS; ++i) {
		if (!(s = strtok(NULL, WHITESPACE))) {
			break;
		}
		v[i] = strtod(s, &end);
		if (*end != '\0') {
			*last = s;
			break;
		}
	}
	return i;
}

void gen_request_event() {
	SDL_Event e;
	e.type = SDL_USEREVENT;
	SDL_PushEvent(&e);
}

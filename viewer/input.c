#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>
#include "viewer.h"

#define MAX_FIELDS 1024
#define WHITESPACE " \t\n"

int proc_cmd(char *cmd);
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
	char *scene_name, *geom_name, *token;
	scene *scn;
	geometry *g;
	real numbers[MAX_FIELDS];
	real *verts, radius;
	GLuint *inds;
	int i, nverts, ninds;
	
	/* printf("cmd: %s\n", cmd); */
	if (!(scene_name = strtok(cmd, WHITESPACE)))
		return 1;
	
	if (*scene_name == '!') {
		return delete_scene(scene_name + 1);
	}
	
	scn = find_or_add_scene(scene_name);
	if (!(geom_name = strtok(NULL, WHITESPACE)))
		return 0;
		
	if (*geom_name == '!') {
		return delete_geom(scn, geom_name + 1);
	}
	
	g = find_or_add_geom(scn, geom_name);
	
	verts = NULL;
	inds = NULL;
	token = strtok(NULL, WHITESPACE);
	radius = -1.0;
	while (token) {
		if (strlen(token) != 1) {
			fprintf(stderr, "unexpected token: %s\n", token);
			goto Error;
		}
		
		switch (*token) {
			case 'p':
				if (parse_nums(g->pos, 3, &token) != 3) {
					fprintf(stderr, "invalid position\n");
					goto Error;
				}
				break;
			case 'r':
				if (parse_nums(numbers, 4, &token) != 4) {
					fprintf(stderr, "invalid rotation\n");
					goto Error;
				}
				quat_to_axis_angle(numbers, g->axis, &g->angle);
				break;
			case 's':
				if (parse_nums(g->scale, 3, &token) != 3) {
					fprintf(stderr, "invalid scale\n");
					goto Error;
				}
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
				if (parse_nums(numbers, 1, &token) < 1 || numbers[0] < 0.0) {
					fprintf(stderr, "invalid radius\n");
					goto Error;
				}
				radius = numbers[0];
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
		set_geom_vertices(g, verts, inds, ninds);
	} else if (radius >= 0.0) {
		set_geom_radius(g, radius);
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

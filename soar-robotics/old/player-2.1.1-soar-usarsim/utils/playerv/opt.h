/* 
 *  PlayerViewer
 *  Copyright (C) Andrew Howard 2002
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
/***************************************************************************
 * Desc: Read program options from command line and config file.
 * Author: Andrew Howard
 * Date: 28 Mar 2002
 * CVS: $Id: opt.h 639 2002-05-17 00:40:35Z inspectorg $
 **************************************************************************/

#ifndef OPT_H
#define OPT_H


// Structure describing a single option.
// This structure is opaque.
typedef struct
{
  char *section;
  char *key;
  char *value;
  char *comment;
  int used;
  int save;
} opt_item_t;


// Structure describing all options.
// This structure is opaque.
typedef struct
{
  const char *filename;
  int item_size;
  int item_count;
  opt_item_t *items;
} opt_t;


// Read options from command line
opt_t *opt_init(int argc, char **argv, const char *filename);

// Clean up
void opt_term(opt_t *opt);

// Load a config file
int opt_load(opt_t *opt, const char *filename);

// Save a config file.
// Set filename to NULL to save it back with the same name.
int opt_save(opt_t *opt, const char *filename);

// Issue a warning about unrecognized options
int opt_warn_unused(opt_t *opt);

// Write a string
void opt_set_string(opt_t *opt, const char *section,
                    const char *key, const char *value);

// Read a string
const char *opt_get_string(opt_t *opt, const char *section,
                           const char *key, const char *defvalue);

// Read an integer
int opt_get_int(opt_t *opt, const char *section,
                const char *key, int defvalue);

// Write an integer
void opt_set_int(opt_t *opt, const char *section,
                 const char *key, int value);

// Read a double
double opt_get_double(opt_t *opt, const char *section,
                      const char *key, double defvalue);

// Read in two integers
// Returns 0 if successful
int opt_get_int2(opt_t *opt, const char *section,
                 const char *key, int *p1, int *p2);

// Write two integers
void opt_set_int2(opt_t *opt, const char *section,
                  const char *key, int p1, int p2);

// Read in three integers
// Returns 0 if successful
int opt_get_int3(opt_t *opt, const char *section,
                 const char *key, int *p1, int *p2, int *p3);

// Write three integers
void opt_set_int3(opt_t *opt, const char *section,
                  const char *key, int p1, int p2, int p3);

// Read in two doubles
// Returns 0 if successful
int opt_get_double2(opt_t *opt, const char *section,
                    const char *key, double *p1, double *p2);

// Read in three doubles
// Returns 0 if successful
int opt_get_double3(opt_t *opt, const char *section, const char *key,
                    double *p1, double *p2, double *p3);

// Read in four doubles
// Returns 0 if successful
int opt_get_double4(opt_t *opt, const char *section, const char *key,
                    double *p1, double *p2, double *p3, double *p4);



#endif

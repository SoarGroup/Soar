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
 * CVS: $Id: opt.c 2085 2004-05-31 23:23:10Z gerkey $
 * Notes:
 * - This will leak mem due to the strdup's; should fix this sometime.
 **************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "opt.h"


// Parse options from a command line argument.
int opt_parse_short_args(opt_t *opt, const char *arg1, const char *arg2);

// Parse options from a command line argument.
int opt_parse_long_args(opt_t *opt, const char *arg1, const char *arg2);

// Add an item
void opt_add_item(opt_t *opt, const char *section, const char *key,
                  const char *value, const char *comment, int save);

// Set the value of an existing item
void opt_set_item(opt_t *opt, const char *section, const char *key,
                  const char *value, const char *comment, int save);


// Read options from command line
opt_t *opt_init(int argc, char **argv, const char *filename)
{
  opt_t *opt;
  int i;

  opt = malloc(sizeof(opt_t));
  
  // Initialise the option list
  opt->item_size = 1000;
  opt->item_count = 0;
  opt->items = malloc(opt->item_size * sizeof(opt_item_t));
  
  // Find the configuration file name;
  // we assume it is the first non-option argument
  opt->filename = filename;
  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
      i += 1;
    else
    {
      opt->filename = strdup(argv[i]);
      break;
    }
  }

  // Load the config file
  if (opt->filename)
  {
    if (opt_load(opt, opt->filename) != 0)
    {
      free(opt);
      return NULL;
    }
  }

  // Now read command line arguments
  // Command line options override the config file values.
  for (i = 1; i < argc; i++)
  {
    // Look for long-form arguments
    if (strncmp(argv[i], "--", 2) == 0)
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
          opt_parse_long_args(opt, argv[i] + 2, argv[i + 1]);
      else
          opt_parse_long_args(opt, argv[i] + 2, NULL);
    }

    // Look for short-form arguments
    else if (argv[i][0] == '-')
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
          opt_parse_short_args(opt, argv[i] + 1, argv[i + 1]);
      else
          opt_parse_short_args(opt, argv[i] + 1, NULL);
    }
  }

  /*
  // Debugging -- list all options
  for (i = 0; i < opt->item_count; i++)
  {
    printf("[%s].[%s] = [%s]\n", opt->items[i].section,
           opt->items[i].key, opt->items[i].value);
  }
  */
  
  return opt;
}


// Clean up
void opt_term(opt_t *opt)
{
  // TODO: free mem here
}


// Parse short form options from the command line.
int opt_parse_short_args(opt_t *opt, const char *arg1, const char *arg2)
{
  const char *key, *value;

  key = arg1;
  value = arg2;

  if (value)
    opt_add_item(opt, "", key, value, NULL, 0);
  else
    opt_add_item(opt, "", key, "1", NULL, 0);
  
  return 0;
}


// Parse long form options from the command line.
// arg2 may be NULL, in which case we will add an item with a blank
// key and a value of 1.  This allows for options of the form
// '--foobar' as opposed to '--foobar "enable 1"'.
int opt_parse_long_args(opt_t *opt, const char *arg1, const char *arg2)
{
  const char *section, *key, *value;
  char *tmp;
  
  section = arg1;

  opt_add_item(opt, section, "", "1", NULL, 0);
  if (!arg2)
    return 0;

  // Extract all the <key, value> pairs from the argument.
  tmp = strdup(arg2);
  key = strtok(tmp, " \t");
  value = strtok(NULL, " \t");
  while (key && value)
  {
    opt_add_item(opt, section, key, value, NULL, 0);
    key = strtok(NULL, " \t");
    value = strtok(NULL, " \t");
  }
  free(tmp);
  
  return 0;
}


// Load the config file
int opt_load(opt_t *opt, const char *filename)
{
  int i;
  FILE *file;
  char line[2048], temp[2048];
  char *tokens[3];
  char *section, *key, *value;
  
  opt->filename = strdup(filename);
  
  file = fopen(filename, "r");
  if (file == NULL)
  {
    PRINT_ERR1("unable to open config file %s", filename);
    PRINT_ERRNO("%s");
    return -1;
  }

  PRINT_MSG1("Loading config file [%s]", filename);

  for (i = 0; 1; i++)
  {
    if (fgets(line, sizeof(line), file) == NULL)
      break;

    strcpy(temp, line);
    tokens[0] = strtok(temp, " .\n");
    tokens[1] = strtok(NULL, " =\n");
    tokens[2] = strtok(NULL, "=#\n");

    //printf("[%s] [%s] [%s]\n", tokens[0], tokens[1], tokens[2]);

    // Handle blank lines
    if (tokens[0] == NULL)
      opt_add_item(opt, NULL, NULL, NULL, line, 1);

    // Handle comment lines
    else if (tokens[0][0] == '#')
      opt_add_item(opt, NULL, NULL, NULL, line, 1);

    // Parse regular lines
    else if (tokens[0] && tokens[1] && tokens[2])
    {
      section = tokens[0];
      key = tokens[1];
      value = tokens[2];

      // Strip leading white-space
      while (*value == ' ')
        value++;
      
      opt_add_item(opt, section, key, value, NULL, 1);
    }
    else
    {
      PRINT_ERR1("syntax error in config file, line %d", i);
      fclose(file);
      return -1;
    }
  }

  fclose(file);
  
  return 0;
}


// Save a config file
int opt_save(opt_t *opt, const char *filename)
{
  int i;
  FILE *file;
  opt_item_t *item;

  if (filename == NULL)
  {
    if (opt->filename == NULL)
      return 0;
    file = fopen(opt->filename, "w+");
  }
  else
    file = fopen(filename, "w+");
  
  if (file == NULL)
  {
    PRINT_ERR1("unable to open/create config file %s", filename);
    PRINT_ERRNO("%s");
    return -1;
  }

  for (i = 0; i < opt->item_count; i++)
  {
    item = opt->items + i;
    if (!item->save)
      continue;    if (item->section)
      fprintf(file, "%s.%s = %s\n", item->section, item->key, item->value);
    else
      fprintf(file, "%s", item->comment);
  }
  
  fclose(file);
  return 0;
}


// Issue a warning about unrecognized options
int opt_warn_unused(opt_t *opt)
{
  int i, count;

  // See how many unused options there are
  count = 0;
  for (i = 0; i < opt->item_count; i++)
  {
    if (opt->items[i].used == 0)
      count++;
  }
  if (count == 0)
    return 0;
  
  // Print out the unused options
  PRINT_MSG("Warning : the following options were defined but not used");
  for (i = 0; i < opt->item_count; i++)
  {
    if (opt->items[i].used == 0)
      PRINT_MSG2("%s.%s", opt->items[i].section, opt->items[i].key);
  }
  return count;
}


// Write an item
void opt_add_item(opt_t *opt, const char *section, const char *key,
                    const char *value, const char *comment, int save)
{
  opt_item_t *item;
  
  // Make more space if we need it
  if (opt->item_count >= opt->item_size)
  {
    opt->item_size *= 2;
    opt->items = realloc(opt->items, opt->item_size * sizeof(opt_item_t));
  }

  // Add the new option
  item = &opt->items[opt->item_count];
  item->section = (section ? strdup(section) : NULL);
  item->key = (key ? strdup(key) : NULL);
  item->value = (value ? strdup(value) : NULL);
  item->comment = (comment ? strdup(comment) : NULL);
  item->save = save;
  item->used = (section == NULL && key == NULL);
  opt->item_count++;
}


// Set the value of an existing item
void opt_set_item(opt_t *opt, const char *section, const char *key,
                    const char *value, const char *comment, int save)
{
  int i;
  
  for (i = 0; i < opt->item_count; i++)
  {
    if (opt->items[i].section == NULL)
      continue;
    if (strcmp(section, opt->items[i].section) == 0 && strcmp(key, opt->items[i].key) == 0)
    {
      if (opt->items[i].value)
        free(opt->items[i].value);
      opt->items[i].value = strdup(value);
      opt->items[i].save |= save;
    }
  }
}


// Write a string
// See if we already have this option, and if so, update it.
// Otherwise, we will have to add it.
void opt_set_string(opt_t *opt, const char *section,
                      const char *key, const char *value)
{
  if (opt_get_string(opt, section, key, NULL) == NULL)
    opt_add_item(opt, section, key, value, NULL, 1);
  opt_set_item(opt, section, key, value, NULL, 1);
}


// Read a string
// In the case of duplicates, we use the last one
// in the list.
const char *opt_get_string(opt_t *opt, const char *section,
                             const char *key, const char *defvalue)
{
  int i;
  
  for (i = 0; i < opt->item_count; i++)
  {
    if (opt->items[i].section == NULL)
      continue;
    if (strcmp(section, opt->items[i].section) != 0)
      continue;
    if (strcmp(key, opt->items[i].key) != 0)
      continue;
    opt->items[i].used++;
    defvalue = opt->items[i].value;
  }
  return defvalue;
}


// Read an integer
int opt_get_int(opt_t *opt, const char *section,
                  const char *key, int defvalue)
{
  const char *value;
  value = opt_get_string(opt, section, key, NULL);
  if (value != NULL)
    return atoi(value);
  else
    return defvalue;
}


// Write an integer
void opt_set_int(opt_t *opt, const char *section, const char *key,
                   int value)
{
  char svalue[64];
  snprintf(svalue, sizeof(svalue), "%d", value);
  opt_set_string(opt, section, key, svalue);
}

// Read in two integers
// Returns 0 if successful
int opt_get_int2(opt_t *opt, const char *section, const char *key,
                   int *p1, int *p2)
{
  const char *value;

  value = opt_get_string(opt, section, key, NULL);
  if (!value)
    return -1;
  if (sscanf(value, " ( %d , %d ) ", p1, p2) < 2)
    return -1;
  
  return 0;
}


// Write two integers
void opt_set_int2(opt_t *opt, const char *section, const char *key,
                    int p1, int p2)
{
  char svalue[64];
  snprintf(svalue, sizeof(svalue), "(%d, %d)", p1, p2);
  opt_set_string(opt, section, key, svalue);
}


// Read in three integers
// Returns 0 if successful
int opt_get_int3(opt_t *opt, const char *section,
                 const char *key, int *p1, int *p2, int *p3)
{
  const char *value;

  value = opt_get_string(opt, section, key, NULL);
  if (!value)
    return -1;
  if (sscanf(value, " ( %d , %d , %d ) ", p1, p2, p3) < 3)
    return -1;
  
  return 0;
}


// Read a double
double opt_get_double(opt_t *opt, const char *section, const char *key,
                        double defvalue)
{
  const char *value;
  value = opt_get_string(opt, section, key, NULL);
  if (value != NULL)
    return atof(value);
  else
    return defvalue;
}

// Read in two doubles
// Returns 0 if successful
int opt_get_double2(opt_t *opt, const char *section, const char *key,
                      double *p1, double *p2)
{
  const char *value;

  value = opt_get_string(opt, section, key, NULL);
  if (!value)
    return -1;
  if (sscanf(value, " ( %lf , %lf ) ", p1, p2) < 2)
    return -1;
  
  return 0;
}


// Write two doubles
void opt_set_double2(opt_t *opt, const char *section,
                       const char *key, double p1, double p2, int places)
{
  char svalue[64];
  snprintf(svalue, sizeof(svalue), "(%.*f, %.*f)", places, p1, places, p2);
  opt_set_string(opt, section, key, svalue);
}


// Read in three doubles
// Returns 0 if successful
int opt_get_double3(opt_t *opt, const char *section, const char *key,
                      double *p1, double *p2, double *p3)
{
  const char *value;

  value = opt_get_string(opt, section, key, NULL);
  if (!value)
    return -1;
  if (sscanf(value, " ( %lf , %lf , %lf ) ", p1, p2, p3) < 3)
    return -1;
  
  return 0;
}


// Read in four doubles
int opt_get_double4(opt_t *opt, const char *section, const char *key,
                      double *p1, double *p2, double *p3, double *p4)
{
  const char *value;

  value = opt_get_string(opt, section, key, NULL);
  if (!value)
    return -1;
  
  if (sscanf(value, " ( %lf , %lf , %lf , %lf) ", p1, p2, p3, p4) < 4)
    return -1;
  
  return 0;
}

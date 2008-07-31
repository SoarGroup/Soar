/*
 *  RTK2 : A GUI toolkit for robotics
 *  Copyright (C) 2001  Andrew Howard  ahoward@usc.edu
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * Desc: Rtk table functions
 * Author: Andrew Howard
 * CVS: $Id: rtk_table.c 2686 2005-02-03 23:44:17Z gerkey $
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rtk.h"
#include "rtkprivate.h"


// Local functions
static gboolean rtk_table_on_destroy(GtkWidget *widget, rtk_table_t *table);


// Create a new table
rtk_table_t *rtk_table_create(rtk_app_t *app, int width, int height)
{
  rtk_table_t *table;

  table = malloc(sizeof(rtk_table_t));
  RTK_LIST_APPEND(app->table, table);

  table->app = app;
  table->item = NULL;
  table->item_count = 0;
  table->row_count = 1;

  // Create the gtk objects
  table->destroyed = FALSE;
	table->frame = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  table->table = gtk_table_new(table->row_count, 2, TRUE);
  gtk_container_add(GTK_CONTAINER(table->frame), table->table);
  gtk_widget_set_usize(table->table, width, height);

  // Add signal handlers
	gtk_signal_connect(GTK_OBJECT(table->frame), "destroy",
                     GTK_SIGNAL_FUNC(rtk_table_on_destroy), table);
  
  return table;
}


// Delete the table
void rtk_table_destroy(rtk_table_t *table)
{
  // TODO: clean up items
  
  // Remove ourself from the linked list in the app
  RTK_LIST_REMOVE(table->app->table, table);   

  // Free ourself
  free(table);
}

// Handle destroy events
gboolean rtk_table_on_destroy(GtkWidget *widget, rtk_table_t *table)
{
  table->destroyed = TRUE;
  return FALSE;
}


// Create a new item in the table
rtk_tableitem_t *rtk_tableitem_create_int(rtk_table_t *table,
                                          const char *label, int low, int high)
{
  rtk_tableitem_t *item;
  
  item = malloc(sizeof(rtk_tableitem_t));
  item->table = table;
  RTK_LIST_APPEND(table->item, item);

  // Create the gtk objects
  item->label = gtk_label_new(label);
  item->adj = gtk_adjustment_new(0, low, high, 1, 10, 10);
  item->spin = gtk_spin_button_new(GTK_ADJUSTMENT(item->adj), 1, 0);
  gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(item->spin), TRUE); 

  // Resize the table to make it big enough
  if (table->item_count >= table->row_count)
    gtk_table_resize(GTK_TABLE(table->table), ++table->row_count, 2);
  
  // Now stick the item in the table
  gtk_table_attach(GTK_TABLE(table->table), item->label,
                   0, 1, table->item_count, table->item_count + 1,
                   GTK_FILL, GTK_FILL, 0, 0);
  gtk_table_attach(GTK_TABLE(table->table), item->spin,
                   1, 2, table->item_count, table->item_count + 1,
                   GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  table->item_count++;

  return item;
}


// Set the value of a table item
void rtk_tableitem_set_int(rtk_tableitem_t *item, int value)
{
  if (item->table->destroyed)
    return;
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(item->spin), value);
}


// Get the value of a table item
int rtk_tableitem_get_int(rtk_tableitem_t *item)
{
  if (item->table->destroyed)
    return item->value;
  item->value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(item->spin));
  return item->value;
}

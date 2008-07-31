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
 * Desc: Menu and menuitem functions
 * Author: Andrew Howard
 * CVS: $Id: rtk_menu.c 2686 2005-02-03 23:44:17Z gerkey $
 */

#include <stdlib.h>

//#define DEBUG
#include "rtk.h"
#include "rtkprivate.h"


// Some local functions
static void rtk_on_activate(GtkWidget *widget, rtk_menuitem_t *item);


// Create a menu
rtk_menu_t *rtk_menu_create(rtk_canvas_t *canvas, const char *label)
{
  rtk_menu_t *menu;

  menu = malloc(sizeof(rtk_menu_t));
  menu->canvas = canvas;
  menu->item = gtk_menu_item_new_with_label(label);
  menu->menu = gtk_menu_new();

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu->item), menu->menu);
  gtk_menu_bar_append(GTK_MENU_BAR(canvas->menu_bar), menu->item);

  return menu;
}


// Create a sub menu
rtk_menu_t *rtk_menu_create_sub(rtk_menu_t *pmenu, const char *label)
{
  rtk_menu_t *menu;

  menu = malloc(sizeof(rtk_menu_t));
  menu->canvas = pmenu->canvas;
  menu->item = gtk_menu_item_new_with_label(label);
  menu->menu = gtk_menu_new();

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu->item), menu->menu);
  gtk_menu_append(GTK_MENU(pmenu->menu), menu->item);

  return menu;
}


// Delete a menu
void rtk_menu_destroy(rtk_menu_t *menu)
{
  // Hmmm, how do I destroy menus?
  free(menu);
}


// Create a new menu item
rtk_menuitem_t *rtk_menuitem_create(rtk_menu_t *menu,
                                    const char *label, int check)
{
  rtk_menuitem_t *item;

  item = malloc(sizeof(rtk_menuitem_t));
  item->menu = menu;
  item->activated = FALSE;
  item->checked = FALSE;
  item->callback = NULL;

  if (check)
  {
    item->checkitem = 1;
    item->item = gtk_check_menu_item_new_with_label(label);
    gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(item->item), 1);
  }
  else
  {
    item->checkitem = 0;
    item->item = gtk_menu_item_new_with_label(label);
  }
  
  gtk_menu_append(GTK_MENU(menu->menu), item->item);

  gtk_signal_connect(GTK_OBJECT(item->item), "activate",
                     GTK_SIGNAL_FUNC(rtk_on_activate), item);

  return item;
}


// Delete a menu item
void rtk_menuitem_destroy(rtk_menuitem_t *item)
{
  // Hmmm, there doesnt seem to be any way to remove
  // an item from a menu.
  //gtk_container_remove(GTK_CONTAINER(item->menu->menu), item->item);
  free(item);
}


// Set the callback for a menu item.  This function will be called
// when the user selects the menu item.
void rtk_menuitem_set_callback(rtk_menuitem_t *item,
                               rtk_menuitem_fn_t callback)
{
  item->callback = callback;
  return;
}


// Test to see if the menu item has been activated
// Calling this function will reset the flag
int rtk_menuitem_isactivated(rtk_menuitem_t *item)
{
  if (item->activated)
  {
    item->activated = FALSE;
    return TRUE;
  }
  return FALSE;
}


// Set the check state of a menu item.
// We have to be careful not to change the activation state.
void rtk_menuitem_check(rtk_menuitem_t *item, int check)
{
  int tmp;

  if (!item->menu->canvas->destroyed)
  {
    tmp = item->activated;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item->item), check);
    item->activated = tmp;
  }
}


// Test to see if the menu item is checked
int rtk_menuitem_ischecked(rtk_menuitem_t *item)
{
  return item->checked;
}


// Enable/disable the menu item
int rtk_menuitem_enable(rtk_menuitem_t *item, int enable)
{
  gtk_widget_set_sensitive(GTK_WIDGET(item->item), enable);
  return;
}


// Handle activation
void rtk_on_activate(GtkWidget *widget, rtk_menuitem_t *item)
{
  if (!item->menu->canvas->destroyed)
  {
    item->activated = TRUE;

    if (item->checkitem)
      item->checked = GTK_CHECK_MENU_ITEM(item->item)->active;
    
    if (item->callback)
      (*item->callback) (item);
  }
  return;
}

/*
 * vala-panel
 * Copyright (C) 2015-2016 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "menu-maker.h"
#include <gdk/gdk.h>
#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <string.h>

static const GtkTargetEntry MENU_TARGETS[] = {
	{ "text/uri-list", 0, 0 }, { "application/x-desktop", 0, 0 },
};

typedef struct
{
	GMenuModel *section;
	GtkMenuItem *menuitem;
	int item_pos;
	volatile int ref_count;
} DragData;

static DragData *drag_data_ref(DragData *data)
{
	g_atomic_int_inc(&(data->ref_count));
	return data;
}
static void drag_data_unref(DragData *data)
{
	if (g_atomic_int_dec_and_test(&(data->ref_count)))
		g_slice_free(DragData, data);
}
static void drag_data_destroy(GtkWidget *w, DragData *data)
{
	g_signal_handlers_disconnect_by_data(data->menuitem, data);
	gtk_drag_source_unset(GTK_WIDGET(data->menuitem));
	data->ref_count = 1;
	drag_data_unref(data);
}
static DragData *drag_data_new(GtkMenuItem *item, GMenuModel *section, int model_item)
{
	DragData *data  = (DragData *)g_slice_alloc0(sizeof(DragData));
	data->section   = section;
	data->menuitem  = item;
	data->item_pos  = model_item;
	data->ref_count = 1;
	return data;
}
void drag_data_get(GtkWidget *item, GdkDragContext *context, GtkSelectionData *sdata, uint info,
                   uint time_, DragData *data)
{
	GStrv uri_list              = NULL;
	g_autofree char *action     = NULL;
	g_autofree char *target     = NULL;
	g_autofree char *launch_str = NULL;
	g_menu_model_get_item_attribute(data->section,
	                                data->item_pos,
	                                G_MENU_ATTRIBUTE_ACTION,
	                                "s",
	                                &action);
	g_menu_model_get_item_attribute(data->section,
	                                data->item_pos,
	                                G_MENU_ATTRIBUTE_TARGET,
	                                "s",
	                                &target);
	if (!strcmp(action, "app.launch-id"))
	{
		g_autoptr(GDesktopAppInfo) info = g_desktop_app_info_new(target);
		launch_str = g_filename_to_uri(g_desktop_app_info_get_filename(info), NULL, NULL);
	}
	uri_list    = (char **)g_malloc0(sizeof(char *));
	uri_list[0] = launch_str;
	gtk_selection_data_set_uris(sdata, uri_list);
}
static void drag_data_begin(GtkWidget *item, GdkDragContext *context, DragData *data)
{
	g_autoptr(GVariant) val = g_menu_model_get_item_attribute_value(data->section,
	                                                                data->item_pos,
	                                                                G_MENU_ATTRIBUTE_ICON,
	                                                                NULL);
	g_autoptr(GIcon) icon = g_icon_deserialize(val);
	if (icon)
		gtk_drag_source_set_icon_gicon(GTK_WIDGET(item), icon);
	else
		gtk_drag_source_set_icon_name(GTK_WIDGET(item), "system-run-symbolic");
}

void append_all_sections(GMenu *menu1, GMenuModel *menu2)
{
	for (int i = 0; i < g_menu_model_get_n_items(menu2); i++)
	{
		g_autoptr(GMenuModel) link =
		    g_menu_model_get_item_link(menu2, i, G_MENU_LINK_SECTION);
		g_autoptr(GVariant) labelv =
		    g_menu_model_get_item_attribute_value(menu2, i, "label", G_VARIANT_TYPE_STRING);
		const char *label = labelv != NULL ? g_variant_get_string(labelv, NULL) : NULL;
		if (link != NULL)
			g_menu_append_section(menu1, label, link);
	}
}
static void apply_menu_dnd(GtkMenuItem *item, GMenuModel *section, int model_item)
{
	// Make the this widget a DnD source.
	gtk_drag_source_set(GTK_WIDGET(item), // widget will be drag-able
	                    GDK_BUTTON1_MASK, // modifier that will start a drag
	                    MENU_TARGETS,     // lists of target to support
	                    G_N_ELEMENTS(MENU_TARGETS),
	                    GDK_ACTION_COPY // what to do with data after dropped
	                    );
	DragData *data = drag_data_new(item, section, model_item);
	g_signal_connect(item, "drag-begin", G_CALLBACK(drag_data_begin), data);
	g_signal_connect(item, "drag-data-get", G_CALLBACK(drag_data_get), data);
	g_signal_connect(item, "destroy", G_CALLBACK(drag_data_destroy), data);
}
void apply_menu_properties(GList *w, GMenuModel *menu)
{
	GList *l = w;
	for (int i = 0; i < g_menu_model_get_n_items(menu); i++)
	{
		uint jumplen = 1;
		if (GTK_IS_SEPARATOR_MENU_ITEM(l->data))
			l                     = l->next;
		GtkMenuItem *shell            = GTK_MENU_ITEM(l->data);
		const char *str               = NULL;
		bool has_section              = false;
		bool has_submenu              = false;
		GtkMenuShell *menuw           = GTK_MENU_SHELL(gtk_menu_item_get_submenu(shell));
		g_autoptr(GMenuLinkIter) iter = g_menu_model_iterate_item_links(menu, i);
		GMenuModel *link_menu         = NULL;
		while (g_menu_link_iter_get_next(iter, &str, &link_menu))
		{
			has_section = has_section || !(strcmp(str, G_MENU_LINK_SECTION));
			has_submenu = has_submenu || !(strcmp(str, G_MENU_LINK_SUBMENU));
			if (menuw != NULL && has_submenu)
				apply_menu_properties(gtk_container_get_children(
				                          GTK_CONTAINER(menuw)),
				                      link_menu);
			else if (has_section)
			{
				jumplen += ((uint)g_menu_model_get_n_items(link_menu) - 1);
				apply_menu_properties(l, link_menu);
			}
			g_object_unref(link_menu);
		}
		GVariant *val = NULL;
		g_autoptr(GMenuAttributeIter) attr_iter =
		    g_menu_model_iterate_item_attributes(menu, i);
		while (g_menu_attribute_iter_get_next(attr_iter, &str, &val))
		{
			if (!strcmp(str, G_MENU_ATTRIBUTE_ICON) && (has_submenu || has_section))
			{
				g_autoptr(GIcon) icon = g_icon_deserialize(val);
				g_object_set(shell, "icon", icon, NULL);
			}
			if (!strcmp(str, ATTRIBUTE_TOOLTIP))
				gtk_widget_set_tooltip_text(GTK_WIDGET(shell),
				                            g_variant_get_string(val, NULL));
			if (!strcmp(str, ATTRIBUTE_DND_SOURCE) && g_variant_get_boolean(val))
				apply_menu_dnd(GTK_MENU_ITEM(l->data), menu, i);
			g_variant_unref(val);
		}
		l = g_list_nth(l, jumplen);
		if (l == NULL)
			break;
	}
}

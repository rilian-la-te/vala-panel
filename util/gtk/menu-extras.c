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
#include "menu-extras.h"
#include <gdk/gdk.h>
#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <string.h>

static const GtkTargetEntry MENU_TARGETS[] = {
	{ "text/uri-list", 0, 0 },
	{ "application/x-desktop", 0, 0 },
};

typedef struct
{
	GMenuModel *section;
	GtkMenuItem *menuitem;
	int item_pos;
} DragData;

static void drag_data_destroy(G_GNUC_UNUSED GtkWidget *w, DragData *data)
{
	g_signal_handlers_disconnect_by_data(data->menuitem, data);
	gtk_drag_source_unset(GTK_WIDGET(data->menuitem));
	g_slice_free(DragData, data);
}
static DragData *drag_data_new(GtkMenuItem *item, GMenuModel *section, int model_item)
{
	DragData *data = (DragData *)g_slice_alloc0(sizeof(DragData));
	data->section  = section;
	data->menuitem = item;
	data->item_pos = model_item;
	return data;
}
void drag_data_get(G_GNUC_UNUSED GtkWidget *item, G_GNUC_UNUSED GdkDragContext *context,
                   GtkSelectionData *sdata, G_GNUC_UNUSED uint info, G_GNUC_UNUSED uint time_,
                   DragData *data)
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
static void drag_data_begin(GtkWidget *item, G_GNUC_UNUSED GdkDragContext *context, DragData *data)
{
	g_autoptr(GVariant) val = g_menu_model_get_item_attribute_value(data->section,
	                                                                data->item_pos,
	                                                                G_MENU_ATTRIBUTE_ICON,
	                                                                NULL);
	g_autoptr(GIcon) icon   = g_icon_deserialize(val);
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
		g_autofree char *label = NULL;
		g_menu_model_get_item_attribute(menu2, i, "label", "s", &label, NULL);
		if (link)
			g_menu_append_section(menu1, label, link);
	}
}
static void copy_attribute(gpointer key, gpointer value, gpointer user_data)
{
	GMenuItem *item = G_MENU_ITEM(user_data);
	g_menu_item_set_attribute_value(item, (const char *)key, (GVariant *)value);
}
static void copy_link(gpointer key, gpointer value, gpointer user_data)
{
	GMenuItem *item = G_MENU_ITEM(user_data);
	g_menu_item_set_link(item, (const char *)key, G_MENU_MODEL(value));
}
void copy_model_items(GMenu *dst, GMenuModel *src)
{
	g_menu_remove_all(dst);
	for (int i = 0; i < g_menu_model_get_n_items(src); i++)
	{
		GHashTable *attributes = NULL;
		GHashTable *links      = NULL;
		G_MENU_MODEL_GET_CLASS(src)->get_item_attributes(src, i, &attributes);
		G_MENU_MODEL_GET_CLASS(src)->get_item_links(src, i, &links);
		g_autoptr(GMenuItem) item = g_menu_item_new(NULL, NULL);
		g_hash_table_foreach(attributes, copy_attribute, item);
		g_hash_table_foreach(links, copy_link, item);
		g_menu_append_item(dst, item);
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
			l = l->next;
		GtkMenuItem *shell            = GTK_MENU_ITEM(l->data);
		const char *str               = NULL;
		bool has_section              = false;
		bool has_submenu              = false;
		GtkMenuShell *menuw           = GTK_MENU_SHELL(gtk_menu_item_get_submenu(shell));
		g_autoptr(GMenuLinkIter) iter = g_menu_model_iterate_item_links(menu, i);
		GMenuModel *link_menu         = NULL;
		while (g_menu_link_iter_get_next(iter, &str, &link_menu))
		{
			bool is_section = !(strcmp(str, G_MENU_LINK_SECTION));
			bool is_submenu = !(strcmp(str, G_MENU_LINK_SUBMENU));
			if (menuw != NULL && is_submenu)
			{
				g_autoptr(GList) children =
				    gtk_container_get_children(GTK_CONTAINER(menuw));
				apply_menu_properties(children, link_menu);
			}
			if (is_section)
			{
				jumplen += ((uint)g_menu_model_get_n_items(link_menu) - 1);
				apply_menu_properties(l, link_menu);
			}
			g_clear_object(&link_menu);
			has_section = has_section || is_section;
			has_submenu = has_submenu || is_submenu;
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

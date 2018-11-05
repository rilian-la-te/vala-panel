/*
 * vala-panel
 * Copyright (C) 2017 Konstantin Pugin <ria.freelander@gmail.com>
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

#include "toplevel-config.h"
#include "applet-manager.h"
#include "applet-plugin.h"
#include "definitions.h"
#include "panel-layout.h"
#include "toplevel.h"
#include "util-gtk.h"

#include "private.h"

G_DEFINE_TYPE(ValaPanelToplevelConfig, vala_panel_toplevel_config, GTK_TYPE_DIALOG);

static GQuark applet_quark;
#define config_row_get_applet(row)                                                                 \
	VALA_PANEL_APPLET(g_object_get_qdata((GObject *)row, applet_quark))
#define config_row_set_applet(row, applet) g_object_set_qdata((GObject *)row, applet_quark, applet)

#define config_row_get_info_data(row)                                                              \
	((AppletInfoData *)(g_object_get_qdata((GObject *)row, applet_quark)))
#define config_row_set_info(row, applet) g_object_set_qdata((GObject *)row, applet_quark, applet)

#define NEW_APPLET_NAME "new-applet"
#define NO_SETTINGS "no-settings"

#define COLUMN_ICON 0
#define COLUMN_NAME 1
#define COLUMN_EXPAND 2
#define COLUMN_DATA 3

enum
{
	DUMMY_PROPERTY,
	TOPLEVEL_PROPERTY,
	NUM_PROPERTIES
};
static GParamSpec *vala_panel_toplevel_config_properties[NUM_PROPERTIES];
static void state_configure_monitor(GSimpleAction *act, GVariant *param, void *data);
static void init_plugin_list(ValaPanelToplevelConfig *self);
static void on_add_plugin_stack_box_generate(ValaPanelToplevelConfig *self);
static void on_remove_plugin(GtkButton *btn, void *user_data);

void vala_panel_toplevel_config_select_applet(ValaPanelToplevelConfig *self, const char *uuid)
{
	GList *ch               = gtk_container_get_children(self->plugin_list);
	GtkListBoxRow *selected = NULL;
	for (GList *l = ch; l != NULL; l = g_list_next(l))
	{
		ValaPanelApplet *pl = config_row_get_applet(l->data);
		if (!g_strcmp0(uuid, vala_panel_applet_get_uuid(pl)))
			selected = GTK_LIST_BOX_ROW(l->data);
	}
	gtk_list_box_select_row(self->plugin_list, selected);
	g_list_free(ch);
}

static void vala_panel_toplevel_config_init(ValaPanelToplevelConfig *self)
{
	applet_quark = g_quark_from_static_string("vala-panel-applet-property");
	gtk_widget_init_template(GTK_WIDGET(self));
}

static void vala_panel_configure_dialog_finalize(GObject *obj)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(obj);
	g_clear_object(&self->monitors_box);
	g_clear_object(&self->spin_iconsize);
	g_clear_object(&self->spin_height);
	g_clear_object(&self->store_monitors);
	g_clear_object(&self->spin_width);
	g_clear_object(&self->spin_corners);
	g_clear_object(&self->font_selector);
	g_clear_object(&self->font_box);
	g_clear_object(&self->color_background);
	g_clear_object(&self->color_foreground);
	g_clear_object(&self->file_background);
	g_clear_object(&self->plugin_list);
	g_clear_object(&self->adding_button);
	g_clear_object(&self->up_button);
	g_clear_object(&self->down_button);
	g_clear_object(&self->about_button);
	g_clear_object(&self->prefs_stack);
	G_OBJECT_CLASS(vala_panel_toplevel_config_parent_class)->finalize(obj);
}

static void on_monitors_changed(GtkComboBox *box, void *data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(data);
	int panel_gravity, monitor, request_mon;
	g_object_get(self->_toplevel,
	             VP_KEY_MONITOR,
	             &monitor,
	             VP_KEY_GRAVITY,
	             &panel_gravity,
	             NULL);

	/* change monitor */
	GtkTreeIter iter;
	gtk_combo_box_get_active_iter(box, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(self->store_monitors), &iter, 0, &request_mon, -1);
	ValaPanelPlatform *pf = vala_panel_toplevel_get_current_platform();
	if (vala_panel_platform_edge_available(pf,
	                                       GTK_WINDOW(self->_toplevel),
	                                       panel_gravity,
	                                       request_mon))
	{
		g_object_set(self->_toplevel, VP_KEY_MONITOR, request_mon, NULL);
		gtk_combo_box_set_active(box, request_mon + 1);
	}
}
static void background_color_connector(GtkColorButton *colorb, void *data)
{
	GdkRGBA color;
	gtk_color_chooser_get_rgba(colorb, &color);
	g_autofree char *chr_str = gdk_rgba_to_string(&color);
	g_object_set(G_OBJECT(data), VP_KEY_BACKGROUND_COLOR, chr_str, NULL);
}

static void foreground_color_connector(GtkColorButton *colorb, void *data)
{
	GdkRGBA color;
	gtk_color_chooser_get_rgba(colorb, &color);
	g_autofree char *chr_str = gdk_rgba_to_string(&color);
	g_object_set(G_OBJECT(data), VP_KEY_FOREGROUND_COLOR, chr_str, NULL);
}
static void background_file_connector(GtkFileChooser *colorb, void *data)
{
	g_autofree char *chr_str = gtk_file_chooser_get_filename(colorb);
	g_object_set(G_OBJECT(data), VP_KEY_BACKGROUND_FILE, chr_str, NULL);
}
static GObject *vala_panel_configure_dialog_constructor(GType type, guint n_construct_properties,
                                                        GObjectConstructParam *construct_properties)
{
	GObjectClass *parent_class = G_OBJECT_CLASS(vala_panel_toplevel_config_parent_class);
	GObject *obj =
	    parent_class->constructor(type, n_construct_properties, construct_properties);
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(obj);
	GdkRGBA color;
	g_autoptr(GSimpleActionGroup) conf = g_simple_action_group_new();
	vala_panel_apply_window_icon(GTK_WINDOW(self));
	gtk_window_set_transient_for(self, self->_toplevel);
	/* monitors */
	int monitors       = 0;
	GdkDisplay *screen = gtk_widget_get_display(GTK_WIDGET(self->_toplevel));
	if (screen != NULL)
		monitors = gdk_display_get_n_monitors(screen);
	g_assert(monitors >= 1);
	for (int i = 0; i < monitors; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(self->store_monitors, &iter);
		gtk_list_store_set(self->store_monitors,
		                   &iter,
		                   0,
		                   i,
		                   1,
		                   gdk_monitor_get_model(gdk_display_get_monitor(screen, i)),
		                   -1);
	}
	int true_monitor;
	g_object_get(self->_toplevel, VP_KEY_MONITOR, &true_monitor, NULL);

	gtk_combo_box_set_active(self->monitors_box, true_monitor + 1);
	/* update monitor */
	on_monitors_changed(self->monitors_box, self);

	/* size */
	g_object_bind_property(self->_toplevel,
	                       VP_KEY_WIDTH,
	                       self->spin_width,
	                       "value",
	                       G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self->_toplevel,
	                       VP_KEY_DYNAMIC,
	                       self->spin_width,
	                       "sensitive",
	                       G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);
	g_object_bind_property(self->_toplevel,
	                       VP_KEY_HEIGHT,
	                       self->spin_height,
	                       "value",
	                       G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self->_toplevel,
	                       VP_KEY_ICON_SIZE,
	                       self->spin_iconsize,
	                       "value",
	                       G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self->_toplevel,
	                       VP_KEY_CORNER_RADIUS,
	                       self->spin_corners,
	                       "value",
	                       G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	/* background */
	char scol[120];
	g_object_get(self->_toplevel, VP_KEY_BACKGROUND_COLOR, &scol, NULL);
	gdk_rgba_parse(&color, scol);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(self->color_background), &color);
	gtk_button_set_relief(GTK_BUTTON(self->color_background), GTK_RELIEF_NONE);
	g_signal_connect(self->color_background,
	                 "color-set",
	                 G_CALLBACK(background_color_connector),
	                 self->_toplevel);
	g_object_bind_property(self->_toplevel,
	                       VP_KEY_USE_BACKGROUND_COLOR,
	                       self->color_background,
	                       "sensitive",
	                       G_BINDING_SYNC_CREATE);
	char *file = NULL;
	bool use_background_file;
	g_object_get(self->_toplevel,
	             VP_KEY_BACKGROUND_FILE,
	             &file,
	             VP_KEY_USE_BACKGROUND_FILE,
	             &use_background_file,
	             NULL);
	if (use_background_file && file != NULL)
		gtk_file_chooser_set_filename(self->file_background, file);
	gtk_widget_set_sensitive(self->file_background, use_background_file);
	g_object_bind_property(self->_toplevel,
	                       VP_KEY_USE_BACKGROUND_FILE,
	                       self->file_background,
	                       "sensitive",
	                       G_BINDING_SYNC_CREATE);
	g_signal_connect(self->file_background,
	                 "file-set",
	                 G_CALLBACK(background_file_connector),
	                 self->_toplevel);
	//    if(file)
	//        g_free0(file);
	/* foregorund */
	g_object_get(self->_toplevel, VP_KEY_FOREGROUND_COLOR, &scol, NULL);
	gdk_rgba_parse(&color, scol);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(self->color_foreground), &color);
	gtk_button_set_relief(GTK_BUTTON(self->color_foreground), GTK_RELIEF_NONE);
	g_signal_connect(self->color_foreground,
	                 "color-set",
	                 G_CALLBACK(foreground_color_connector),
	                 self->_toplevel);
	g_object_bind_property(self->_toplevel,
	                       VP_KEY_USE_FOREGROUND_COLOR,
	                       self->color_foreground,
	                       "sensitive",
	                       G_BINDING_SYNC_CREATE);
	/* fonts */
	g_object_bind_property(self->_toplevel,
	                       VP_KEY_FONT,
	                       self->font_selector,
	                       "font",
	                       G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	gtk_button_set_relief(GTK_BUTTON(self->font_selector), GTK_RELIEF_NONE);
	g_object_bind_property(self->_toplevel,
	                       VP_KEY_USE_FONT,
	                       self->font_box,
	                       "sensitive",
	                       G_BINDING_SYNC_CREATE);
	/* plugin list */
	init_plugin_list(self);
	on_add_plugin_stack_box_generate(self);
	gtk_widget_insert_action_group(self, "conf", conf);
	gtk_widget_insert_action_group(self, "win", self->_toplevel);
	gtk_widget_insert_action_group(self, "app", gtk_window_get_application(self->_toplevel));
	return obj;
}

static void vala_panel_toplevel_config_get_property(GObject *object, guint property_id,
                                                    GValue *value, GParamSpec *pspec)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(object);
	switch (property_id)
	{
	case TOPLEVEL_PROPERTY:
		g_value_set_object(value, self->_toplevel);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void vala_panel_toplevel_config_set_property(GObject *object, guint property_id,
                                                    const GValue *value, GParamSpec *pspec)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(object);
	switch (property_id)
	{
	case TOPLEVEL_PROPERTY:
		self->_toplevel = VALA_PANEL_TOPLEVEL(g_value_get_object(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static ValaPanelAppletInfo *get_info_from_applet(ValaPanelApplet *pl)
{
	return vp_applet_manager_get_applet_info(vala_panel_layout_get_manager(),
	                                         pl,
	                                         vala_panel_toplevel_get_core_settings());
}

static GtkListBoxRow *create_info_representation(ValaPanelAppletInfo *pl_info)
{
	GtkListBoxRow *row = gtk_list_box_row_new();
	GtkBox *box        = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_widget_show(box);
	gtk_container_add(row, box);

	gtk_widget_set_margin_top(box, 4);
	gtk_widget_set_margin_bottom(box, 4);

	GtkImage *image =
	    gtk_image_new_from_icon_name(vala_panel_applet_info_get_icon_name(pl_info),
	                                 GTK_ICON_SIZE_MENU);
	gtk_widget_set_margin_start(image, 12);
	gtk_widget_set_margin_end(image, 14);
	gtk_widget_show(image);
	gtk_box_pack_start(box, image, false, false, 0);

	GtkLabel *label = gtk_label_new(vala_panel_applet_info_get_name(pl_info));
	gtk_widget_set_margin_end(label, 18);
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_widget_show(label);
	gtk_box_pack_start(box, label, false, false, 0);
	gtk_widget_show(box);
	gtk_widget_show(row);
	return row;
}

static GtkListBoxRow *create_applet_representation(ValaPanelToplevelConfig *self,
                                                   ValaPanelApplet *pl)
{
	ValaPanelAppletInfo *pl_info =
	    vp_applet_manager_get_applet_info(vala_panel_layout_get_manager(),
	                                      pl,
	                                      vala_panel_toplevel_get_core_settings());
	GtkListBoxRow *row = create_info_representation(pl_info);
	GtkBox *box        = GTK_BOX(gtk_bin_get_child(row));
	GtkButton *button  = GTK_BUTTON(gtk_button_new());
	GtkImage *image = gtk_image_new_from_icon_name("list-remove-symbolic", GTK_ICON_SIZE_MENU);
	gtk_widget_show(image);
	gtk_container_add(button, image);
	gtk_widget_show(button);
	GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(button));
	gtk_style_context_add_class(context, "circular");
	gtk_style_context_add_class(context, "destructive-action");
	gtk_button_set_relief(button, GTK_RELIEF_NONE);
	gtk_box_pack_end(box, button, false, false, 0);
	g_signal_connect(button, "clicked", on_remove_plugin, self);
	config_row_set_applet(row, pl);
	return row;
}

/**
 * Sort the list in accordance with pack and actual position
 */
static int plugin_list_sort(GtkListBoxRow *before, GtkListBoxRow *after, void *user_data)
{
	ValaPanelLayout *layout      = VALA_PANEL_LAYOUT(user_data);
	ValaPanelApplet *before_info = before ? config_row_get_applet(before) : NULL;
	ValaPanelApplet *after_info  = after ? config_row_get_applet(after) : NULL;
	uint bpos                    = vala_panel_layout_get_applet_position(layout, before_info);
	uint apos                    = vala_panel_layout_get_applet_position(layout, after_info);
	int bi                       = vala_panel_layout_get_applet_pack_type(before_info);

	if (before_info != NULL && after_info != NULL &&
	    vala_panel_layout_get_applet_pack_type(before_info) !=
	        vala_panel_layout_get_applet_pack_type(after_info))
	{
		int ai = vala_panel_layout_get_applet_pack_type(after_info);
		if (ai != bi)
		{
			if (bi == PACK_START || ai == PACK_END)
				return -1;
			else if (ai == PACK_START || bi == PACK_END)
				return 1;
		}
		return 0;
	}

	if (after_info == NULL)
		return 0;

	if (bpos < apos)
		return -1;
	else if (bpos > apos)
		return 1;

	return 0;
}

/**
 * Provide headers in the list to separate the visual positions
 */
static void plugin_list_headers(GtkListBoxRow *before, GtkListBoxRow *after, void *user_data)
{
	ValaPanelApplet *before_info = before ? config_row_get_applet(before) : NULL;
	ValaPanelApplet *after_info  = after ? config_row_get_applet(after) : NULL;
	int prev        = before_info ? vala_panel_layout_get_applet_pack_type(before_info) : -1;
	int next        = after_info ? vala_panel_layout_get_applet_pack_type(after_info) : -1;
	GtkLabel *label = GTK_LABEL(gtk_list_box_row_get_header(before));

	if (after == NULL || prev != next)
	{
		if (!label)
			label = GTK_LABEL(gtk_label_new(""));
		switch (prev)
		{
		case PACK_START:
			gtk_label_set_text(label, _("Start"));
			break;
		case PACK_CENTER:
			gtk_label_set_text(label, _("Center"));
			break;
		default:
			gtk_label_set_text(label, _("End"));
			break;
		}
		GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(label));
		gtk_style_context_add_class(context, "dim-label");
		gtk_style_context_add_class(context, "applet-row-header");
		gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_START);
		gtk_widget_set_valign(GTK_WIDGET(label), GTK_ALIGN_CENTER);
		gtk_widget_set_margin_start(GTK_WIDGET(label), 4);
		gtk_widget_set_margin_top(GTK_WIDGET(label), 2);
		gtk_widget_set_margin_bottom(GTK_WIDGET(label), 2);
		gtk_label_set_use_markup(label, true);
		gtk_list_box_row_set_header(before, GTK_WIDGET(label));
	}
	else
		gtk_list_box_row_set_header(before, NULL);
}

static void on_plugin_list_row_selected(GtkListBox *box, GtkListBoxRow *row, gpointer data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(data);
	if (!row)
	{
		gtk_widget_set_sensitive(self->about_button, false);
		gtk_widget_set_sensitive(self->up_button, false);
		gtk_widget_set_sensitive(self->down_button, false);
		gtk_stack_set_visible_child_name(self->applet_info_stack, NO_SETTINGS);
	}
	else
	{
		ValaPanelApplet *pl     = config_row_get_applet(row);
		ValaPanelLayout *layout = vala_panel_toplevel_get_layout(self->_toplevel);
		gtk_widget_set_sensitive(self->about_button, true);
		GtkListBoxRow *down_row =
		    gtk_list_box_get_row_at_index(self->plugin_list,
		                                  gtk_list_box_row_get_index(row) + 1);
		ValaPanelApplet *down = down_row ? config_row_get_applet(down_row) : NULL;
		GtkListBoxRow *up_row =
		    gtk_list_box_get_row_at_index(self->plugin_list,
		                                  gtk_list_box_row_get_index(row) - 1);
		ValaPanelApplet *up = up_row ? config_row_get_applet(up_row) : NULL;
		gtk_widget_set_sensitive(self->up_button,
		                         vala_panel_layout_can_move_to_direction(layout,
		                                                                 pl,
		                                                                 up,
		                                                                 GTK_PACK_START));
		gtk_widget_set_sensitive(self->down_button,
		                         vala_panel_layout_can_move_to_direction(layout,
		                                                                 pl,
		                                                                 down,
		                                                                 GTK_PACK_END));
		if (vala_panel_applet_is_configurable(pl))
			gtk_stack_set_visible_child_name(self->applet_info_stack,
			                                 vala_panel_applet_get_uuid(pl));
		else
			gtk_stack_set_visible_child_name(self->applet_info_stack, NO_SETTINGS);
	}
}
static void plugin_list_generate_applet_settings(ValaPanelApplet *pl, ValaPanelToplevelConfig *self)
{
	if (vala_panel_applet_is_configurable(pl))
		gtk_stack_add_named(self->applet_info_stack,
		                    vala_panel_applet_get_settings_ui(pl),
		                    vala_panel_applet_get_uuid(pl));
}

static void plugin_list_add_applet(const char *type, ValaPanelToplevelConfig *self,
                                   GtkListBoxRow *prev)
{
	ValaPanelLayout *layout  = vala_panel_toplevel_get_layout(self->_toplevel);
	ValaPanelApplet *prev_pl = prev ? config_row_get_applet(prev) : NULL;
	ValaPanelAppletPackType prev_pack =
	    prev ? vala_panel_layout_get_applet_pack_type(prev_pl) : PACK_START;
	uint prev_pos       = prev ? vala_panel_layout_get_applet_position(layout, prev_pl) : 0;
	ValaPanelApplet *pl = vala_panel_layout_insert_applet(layout, type, prev_pack, prev_pos);
	GtkListBoxRow *row  = create_applet_representation(self, pl);
	gtk_container_add(self->plugin_list, row);
	plugin_list_generate_applet_settings(pl, self);
	gtk_list_box_invalidate_sort(self->plugin_list);
}

static void init_plugin_list(ValaPanelToplevelConfig *self)
{
	GList *plugins =
	    vala_panel_layout_get_applets_list(vala_panel_toplevel_get_layout(self->_toplevel));
	GtkListBoxRow *prev;
	for (GList *l = plugins; l != NULL; l = g_list_next(l))
	{
		ValaPanelApplet *w = VALA_PANEL_APPLET(l->data);
		GtkListBoxRow *r   = create_applet_representation(self, w);
		gtk_container_add(GTK_CONTAINER(self->plugin_list), GTK_WIDGET(r));
		plugin_list_generate_applet_settings(w, self);
	}
	gtk_list_box_set_sort_func(self->plugin_list,
	                           plugin_list_sort,
	                           vala_panel_toplevel_get_layout(self->_toplevel),
	                           NULL);
	gtk_list_box_set_header_func(self->plugin_list, plugin_list_headers, self, NULL);
	on_plugin_list_row_selected(self->plugin_list, NULL, self);
}

static void on_about_plugin(GtkButton *btn, void *data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(data);
	GtkListBoxRow *row            = gtk_list_box_get_selected_row(self->plugin_list);
	if (!row)
		return;
	ValaPanelApplet *pl       = config_row_get_applet(row);
	ValaPanelAppletInfo *info = get_info_from_applet(pl);
	vala_panel_applet_info_show_about_dialog(info);
}

static int listbox_new_name_sort(GtkListBoxRow *before, GtkListBoxRow *after, void *user_data)
{
	AppletInfoData *before_info = before ? config_row_get_info_data(before) : NULL;
	AppletInfoData *after_info  = after ? config_row_get_info_data(after) : NULL;
	if (before_info && after_info)
		return g_strcmp0(vala_panel_applet_info_get_name(before_info->info),
		                 vala_panel_applet_info_get_name(after_info->info));
	else if (before_info)
		return -1;
	else
		return 1;
}

static void listbox_new_applet_row_activated(GtkListBox *box, GtkListBoxRow *row,
                                             gpointer user_data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(user_data);
	const char *type =
	    vala_panel_applet_info_get_module_name(config_row_get_info_data(row)->info);
	GtkListBoxRow *prev = gtk_list_box_get_selected_row(self->plugin_list);
	plugin_list_add_applet(type, self, prev);
	gtk_list_box_invalidate_filter(self->listbox_new_applet);
}

static bool listbox_new_filter(GtkListBoxRow *row, void *data)
{
	AppletInfoData *d = config_row_get_info_data(row);
	bool ret          = false;
	if ((d->count < 1) || !vala_panel_applet_info_is_exclusive(d->info))
		ret = true;
	return ret;
}

static void available_plugins_reload(ValaPanelToplevelConfig *self)
{
	/* Populate list of available plugins.
	 * Omit plugins that can only exist once per system if it is already configured. */
	vp_applet_manager_reload_applets(vala_panel_layout_get_manager());
	gtk_container_foreach(GTK_CONTAINER(self->listbox_new_applet),
	                      (GtkCallback)gtk_widget_destroy,
	                      NULL);
	GList *all_types = vp_applet_manager_get_all_types(vala_panel_layout_get_manager());
	for (GList *l = all_types; l != NULL; l = g_list_next(l))
	{
		AppletInfoData *d  = (AppletInfoData *)l->data;
		GtkListBoxRow *row = create_info_representation(d->info);
		config_row_set_info(row, d);
		gtk_container_add(self->listbox_new_applet, row);
	}
	g_list_free(all_types);
}

static void on_add_plugin_stack_box_generate(ValaPanelToplevelConfig *self)
{
	available_plugins_reload(self);
	gtk_list_box_set_sort_func(self->listbox_new_applet, listbox_new_name_sort, self, NULL);
	gtk_list_box_set_filter_func(self->listbox_new_applet,
	                             (GtkListBoxFilterFunc)listbox_new_filter,
	                             self,
	                             NULL);
}

static void on_add_plugin(GtkButton *btn, ValaPanelToplevelConfig *self)
{
	const char *name = gtk_stack_get_visible_child_name(self->applet_info_stack);
	if (!g_strcmp0(name, NEW_APPLET_NAME))
	{
		GtkListBoxRow *row = gtk_list_box_get_selected_row(self->plugin_list);
		on_plugin_list_row_selected(self->plugin_list, row, self);
		return;
	}
	available_plugins_reload(self);
	gtk_widget_set_sensitive(self->about_button, false);
	gtk_widget_set_sensitive(self->up_button, false);
	gtk_widget_set_sensitive(self->down_button, false);
	gtk_stack_set_visible_child_name(self->applet_info_stack, NEW_APPLET_NAME);
}

static inline GtkListBoxRow *get_row_from_btn(GtkButton *self)
{
	return GTK_LIST_BOX_ROW(gtk_widget_get_ancestor(GTK_WIDGET(self), GTK_TYPE_LIST_BOX_ROW));
}

static void on_remove_plugin(GtkButton *btn, void *user_data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(user_data);
	GtkListBoxRow *row            = get_row_from_btn(btn);
	if (!row)
		return;
	ValaPanelApplet *pl = config_row_get_applet(row);
	GtkWidget *w =
	    gtk_stack_get_child_by_name(self->applet_info_stack, vala_panel_applet_get_uuid(pl));
	int index              = gtk_list_box_row_get_index(row);
	GtkListBoxRow *sel_row = gtk_list_box_get_selected_row(self->plugin_list);
	int sel_index          = sel_row ? gtk_list_box_row_get_index(sel_row) : -1;
	gtk_widget_destroy0(row);
	gtk_widget_destroy0(w);
	if (index == sel_index)
		gtk_list_box_select_row(self->plugin_list,
		                        gtk_list_box_get_row_at_index(self->plugin_list, index));
	ValaPanelLayout *layout      = vala_panel_toplevel_get_layout(self->_toplevel);
	ValaPanelAppletPackType pack = vala_panel_layout_get_applet_pack_type(pl);
	uint pos                     = vala_panel_layout_get_applet_position(layout, pl);
	vala_panel_layout_remove_applet(layout, pl);
	gtk_list_box_invalidate_sort(self->plugin_list);
	gtk_list_box_invalidate_filter(self->listbox_new_applet);
}

static void on_moveup_plugin(GtkButton *btn, void *user_data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(user_data);
	ValaPanelLayout *layout       = vala_panel_toplevel_get_layout(self->_toplevel);
	GtkListBoxRow *row            = gtk_list_box_get_selected_row(self->plugin_list);
	if (!row)
		return;
	ValaPanelApplet *pl = config_row_get_applet(row);
	int index           = gtk_list_box_row_get_index(row);
	index--;
	GtkListBoxRow *nxt      = gtk_list_box_get_row_at_index(self->plugin_list, index);
	ValaPanelApplet *nxt_pl = nxt ? config_row_get_applet(nxt) : NULL;
	vala_panel_layout_move_applet_one_step(layout, pl, nxt_pl, GTK_PACK_START);
	gtk_list_box_invalidate_headers(self->plugin_list);
	gtk_list_box_invalidate_sort(self->plugin_list);
}

static void on_movedown_plugin(GtkButton *btn, void *user_data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(user_data);
	ValaPanelLayout *layout       = vala_panel_toplevel_get_layout(self->_toplevel);
	GtkListBoxRow *row            = gtk_list_box_get_selected_row(self->plugin_list);
	if (!row)
		return;
	ValaPanelApplet *pl = config_row_get_applet(row);
	int index           = gtk_list_box_row_get_index(row);
	index++;
	GtkListBoxRow *nxt      = gtk_list_box_get_row_at_index(self->plugin_list, index);
	ValaPanelApplet *nxt_pl = nxt ? config_row_get_applet(nxt) : NULL;
	vala_panel_layout_move_applet_one_step(layout, pl, nxt_pl, GTK_PACK_END);
	gtk_list_box_invalidate_headers(self->plugin_list);
	gtk_list_box_invalidate_sort(self->plugin_list);
}

static void vala_panel_toplevel_config_class_init(ValaPanelToplevelConfigClass *klass)
{
	vala_panel_toplevel_config_parent_class = g_type_class_peek_parent(klass);
	G_OBJECT_CLASS(klass)->get_property     = vala_panel_toplevel_config_get_property;
	G_OBJECT_CLASS(klass)->set_property     = vala_panel_toplevel_config_set_property;
	G_OBJECT_CLASS(klass)->constructor      = vala_panel_configure_dialog_constructor;
	G_OBJECT_CLASS(klass)->finalize         = vala_panel_configure_dialog_finalize;
	vala_panel_toplevel_config_properties[TOPLEVEL_PROPERTY] =
	    g_param_spec_object(VP_KEY_TOPLEVEL,
	                        VP_KEY_TOPLEVEL,
	                        VP_KEY_TOPLEVEL,
	                        VALA_PANEL_TYPE_TOPLEVEL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READABLE |
	                                      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                TOPLEVEL_PROPERTY,
	                                vala_panel_toplevel_config_properties[TOPLEVEL_PROPERTY]);
	gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass),
	                                            "/org/vala-panel/lib/pref.ui");
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "combo-monitors",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          monitors_box));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "spin-iconsize",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          spin_iconsize));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "spin-height",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          spin_height));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "spin-width",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          spin_width));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "spin-corners",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          spin_corners));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "font-selector",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          font_selector));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "font-box",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          font_box));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "color-background",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          color_background));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "color-foreground",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          color_foreground));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "chooser-background",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          file_background));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "plugin-list",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          plugin_list));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "applet-info-stack",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          applet_info_stack));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "listbox-new-applet",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          listbox_new_applet));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "add-button",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          adding_button));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "up-button",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          up_button));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "down-button",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          down_button));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "about-button",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          about_button));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "prefs",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          prefs_stack));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "liststore-monitor",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          store_monitors));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_monitor_changed",
	                                             G_CALLBACK(on_monitors_changed));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_about_plugin",
	                                             G_CALLBACK(on_about_plugin));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_add_plugin",
	                                             G_CALLBACK(on_add_plugin));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_moveup_plugin",
	                                             G_CALLBACK(on_moveup_plugin));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_movedown_plugin",
	                                             G_CALLBACK(on_movedown_plugin));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "plugin_list_row_selected",
	                                             G_CALLBACK(on_plugin_list_row_selected));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "listbox_new_applet_row_activated",
	                                             G_CALLBACK(listbox_new_applet_row_activated));
}

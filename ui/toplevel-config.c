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

G_DEFINE_TYPE(ValaPanelToplevelConfig, vala_panel_toplevel_config, GTK_TYPE_DIALOG);

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

static void vala_panel_toplevel_config_init(ValaPanelToplevelConfig *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
}

static void vala_panel_configure_dialog_finalize(GObject *obj)
{
	ValaPanelToplevelConfig *self;
	self = G_TYPE_CHECK_INSTANCE_CAST(obj,
	                                  vala_panel_toplevel_config_get_type(),
	                                  ValaPanelToplevelConfig);
	g_object_unref0(self->monitors_box);
	g_object_unref0(self->spin_iconsize);
	g_object_unref0(self->spin_height);
	g_object_unref0(self->store_monitors);
	g_object_unref0(self->spin_width);
	g_object_unref0(self->spin_corners);
	g_object_unref0(self->font_selector);
	g_object_unref0(self->font_box);
	g_object_unref0(self->color_background);
	g_object_unref0(self->color_foreground);
	g_object_unref0(self->file_background);
	g_object_unref0(self->plugin_list);
	g_object_unref0(self->plugin_desc);
	g_object_unref0(self->adding_button);
	g_object_unref0(self->configure_button);
	g_object_unref0(self->prefs_stack);
	G_OBJECT_CLASS(vala_panel_toplevel_config_parent_class)->finalize(obj);
}

static void on_monitors_changed(GtkComboBox *box, void *data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(data);
	int panel_gravity, monitor, request_mon;
	g_object_get(self->_toplevel,
	             VALA_PANEL_KEY_MONITOR,
	             &monitor,
	             VALA_PANEL_KEY_GRAVITY,
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
		g_object_set(self->_toplevel, VALA_PANEL_KEY_MONITOR, request_mon, NULL);
		gtk_combo_box_set_active(box, request_mon + 1);
	}
}
static void background_color_connector(GtkColorButton *colorb, void *data)
{
	GdkRGBA color;
	gtk_color_chooser_get_rgba(colorb, &color);
	g_autofree char *chr_str = gdk_rgba_to_string(&color);
	g_object_set(G_OBJECT(data), VALA_PANEL_KEY_BACKGROUND_COLOR, chr_str, NULL);
}

static void foreground_color_connector(GtkColorButton *colorb, void *data)
{
	GdkRGBA color;
	gtk_color_chooser_get_rgba(colorb, &color);
	g_autofree char *chr_str = gdk_rgba_to_string(&color);
	g_object_set(G_OBJECT(data), VALA_PANEL_KEY_FOREGROUND_COLOR, chr_str, NULL);
}
static void background_file_connector(GtkFileChooser *colorb, void *data)
{
	g_autofree char *chr_str = gtk_file_chooser_get_filename(colorb);
	g_object_set(G_OBJECT(data), VALA_PANEL_KEY_BACKGROUND_FILE, chr_str, NULL);
}
static GObject *vala_panel_configure_dialog_constructor(GType type, guint n_construct_properties,
                                                        GObjectConstructParam *construct_properties)
{
	GObjectClass *parent_class = G_OBJECT_CLASS(vala_panel_toplevel_config_parent_class);
	GObject *obj =
	    parent_class->constructor(type, n_construct_properties, construct_properties);
	ValaPanelToplevelConfig *self =
	    G_TYPE_CHECK_INSTANCE_CAST(obj,
	                               vala_panel_toplevel_config_get_type(),
	                               ValaPanelToplevelConfig);
	GdkRGBA color;
	g_autoptr(GSimpleActionGroup) conf = g_simple_action_group_new();
	vala_panel_apply_window_icon(GTK_WINDOW(self));
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
	g_object_get(self->_toplevel, VALA_PANEL_KEY_MONITOR, &true_monitor, NULL);

	gtk_combo_box_set_active(self->monitors_box, true_monitor + 1);
	/* update monitor */
	on_monitors_changed(self->monitors_box, self);

	/* size */
	g_object_bind_property(self->_toplevel,
	                       VALA_PANEL_KEY_WIDTH,
	                       self->spin_width,
	                       "value",
	                       G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self->_toplevel,
	                       VALA_PANEL_KEY_DYNAMIC,
	                       self->spin_width,
	                       "sensitive",
	                       G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);
	g_object_bind_property(self->_toplevel,
	                       VALA_PANEL_KEY_HEIGHT,
	                       self->spin_height,
	                       "value",
	                       G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self->_toplevel,
	                       VALA_PANEL_KEY_ICON_SIZE,
	                       self->spin_iconsize,
	                       "value",
	                       G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self->_toplevel,
	                       VALA_PANEL_KEY_CORNER_RADIUS,
	                       self->spin_corners,
	                       "value",
	                       G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	/* background */
	char scol[120];
	g_object_get(self->_toplevel, VALA_PANEL_KEY_BACKGROUND_COLOR, &scol, NULL);
	gdk_rgba_parse(&color, scol);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(self->color_background), &color);
	gtk_button_set_relief(GTK_BUTTON(self->color_background), GTK_RELIEF_NONE);
	g_signal_connect(self->color_background,
	                 "color-set",
	                 G_CALLBACK(background_color_connector),
	                 self->_toplevel);
	g_object_bind_property(self->_toplevel,
	                       VALA_PANEL_KEY_USE_BACKGROUND_COLOR,
	                       self->color_background,
	                       "sensitive",
	                       G_BINDING_SYNC_CREATE);
	char *file = NULL;
	bool use_background_file;
	g_object_get(self->_toplevel,
	             VALA_PANEL_KEY_BACKGROUND_FILE,
	             &file,
	             VALA_PANEL_KEY_USE_BACKGROUND_FILE,
	             &use_background_file,
	             NULL);
	if (use_background_file && file != NULL)
		gtk_file_chooser_set_filename(self->file_background, file);
	gtk_widget_set_sensitive(self->file_background, use_background_file);
	g_object_bind_property(self->_toplevel,
	                       VALA_PANEL_KEY_USE_BACKGROUND_FILE,
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
	g_object_get(self->_toplevel, VALA_PANEL_KEY_FOREGROUND_COLOR, &scol, NULL);
	gdk_rgba_parse(&color, scol);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(self->color_foreground), &color);
	gtk_button_set_relief(GTK_BUTTON(self->color_foreground), GTK_RELIEF_NONE);
	g_signal_connect(self->color_foreground,
	                 "color-set",
	                 G_CALLBACK(foreground_color_connector),
	                 self->_toplevel);
	g_object_bind_property(self->_toplevel,
	                       VALA_PANEL_KEY_USE_FOREGROUND_COLOR,
	                       self->color_foreground,
	                       "sensitive",
	                       G_BINDING_SYNC_CREATE);
	/* fonts */
	g_object_bind_property(self->_toplevel,
	                       VALA_PANEL_KEY_FONT,
	                       self->font_selector,
	                       "font",
	                       G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	gtk_button_set_relief(GTK_BUTTON(self->font_selector), GTK_RELIEF_NONE);
	g_object_bind_property(self->_toplevel,
	                       VALA_PANEL_KEY_USE_FONT,
	                       self->font_box,
	                       "sensitive",
	                       G_BINDING_SYNC_CREATE);
	/* plugin list */
	init_plugin_list(self);
	gtk_widget_insert_action_group(self, "conf", conf);
	gtk_widget_insert_action_group(self, "win", self->_toplevel);
	gtk_widget_insert_action_group(self, "app", gtk_window_get_application(self->_toplevel));
	return obj;
}

static void vala_panel_toplevel_config_get_property(GObject *object, guint property_id,
                                                    GValue *value, GParamSpec *pspec)
{
	ValaPanelToplevelConfig *self;
	self = G_TYPE_CHECK_INSTANCE_CAST(object,
	                                  vala_panel_toplevel_config_get_type(),
	                                  ValaPanelToplevelConfig);
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
	ValaPanelToplevelConfig *self;
	self = G_TYPE_CHECK_INSTANCE_CAST(object,
	                                  vala_panel_toplevel_config_get_type(),
	                                  ValaPanelToplevelConfig);
	switch (property_id)
	{
	case TOPLEVEL_PROPERTY:
		self->_toplevel = G_TYPE_CHECK_INSTANCE_CAST(g_value_get_object(value),
		                                             VALA_PANEL_TYPE_TOPLEVEL,
		                                             ValaPanelToplevel);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void on_sel_plugin_changed(GtkTreeSelection *tree_sel, void *data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(data);
	GtkTreeIter it;
	GtkTreeModel *model;
	ValaPanelApplet *pl;
	if (gtk_tree_selection_get_selected(tree_sel, &model, &it))
	{
		gtk_tree_model_get(model, &it, COLUMN_DATA, &pl, -1);
		ValaPanelAppletInfo *apl = vala_panel_applet_manager_get_plugin_info(
		    vala_panel_layout_get_manager(), pl, vala_panel_toplevel_get_core_settings());
		const char *desc = vala_panel_applet_info_get_description(apl);
		gtk_label_set_text(self->plugin_desc, desc);
		gtk_widget_set_sensitive(self->configure_button,
		                         vala_panel_applet_is_configurable(pl));
	}
}
static void on_plugin_expand_toggled(const char *path, void *data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(data);
	GtkTreeIter it;
	g_autoptr(GtkTreePath) tp = gtk_tree_path_new_from_string(path);
	GtkTreeModel *model       = gtk_tree_view_get_model(self->plugin_list);
	if (gtk_tree_model_get_iter(model, &it, tp))
	{
		ValaPanelApplet *pl;
		bool expand;
		gtk_tree_model_get(model, &it, COLUMN_DATA, &pl, COLUMN_EXPAND, &expand, -1);
		ValaPanelAppletInfo *pl_info = vala_panel_applet_manager_get_plugin_info(
		    vala_panel_layout_get_manager(), pl, vala_panel_toplevel_get_core_settings());
		if (vala_panel_applet_info_is_expandable(pl_info))
		{
			expand = !expand;
			gtk_list_store_set(model, &it, COLUMN_EXPAND, expand, -1);
			ValaPanelUnitSettings *s = vala_panel_layout_get_applet_settings(pl);
			g_settings_set_boolean(s->default_settings, VALA_PANEL_KEY_EXPAND, expand);
		}
	}
}
static void on_stretch_render(GtkCellLayout *layout, GtkCellRenderer *renderer, GtkTreeModel *model,
                              GtkTreeIter *iter, void *data)
{
	/* Set the control visible depending on whether stretch is available for the plugin.
	 * The g_object_set method is touchy about its parameter, so we can't pass the boolean
	 * directly. */
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(data);
	ValaPanelApplet *pl;
	gtk_tree_model_get(model, iter, COLUMN_DATA, &pl, -1);
	ValaPanelAppletInfo *pl_info =
	    vala_panel_applet_manager_get_plugin_info(vala_panel_layout_get_manager(),
	                                              pl,
	                                              vala_panel_toplevel_get_core_settings());
	gtk_cell_renderer_set_visible(renderer, vala_panel_applet_info_is_expandable(pl_info));
}

static void update_plugin_list_model(ValaPanelToplevelConfig *self)
{
	GtkTreeIter it;
	GtkListStore *list = gtk_list_store_new(4,
	                                        G_TYPE_STRING,
	                                        G_TYPE_STRING,
	                                        G_TYPE_BOOLEAN,
	                                        VALA_PANEL_TYPE_APPLET);
	GList *plugins =
	    vala_panel_layout_get_applets_list(vala_panel_toplevel_get_layout(self->_toplevel));
	for (GList *l = plugins; l != NULL; l = g_list_next(l))
	{
		ValaPanelApplet *w = VALA_PANEL_APPLET(l->data);
		bool expand        = gtk_widget_get_hexpand(w) && gtk_widget_get_vexpand(w);
		gtk_list_store_append(list, &it);
		ValaPanelAppletInfo *pl_info = vala_panel_applet_manager_get_plugin_info(
		    vala_panel_layout_get_manager(), w, vala_panel_toplevel_get_core_settings());
		const char *name = vala_panel_applet_info_get_name(pl_info);
		const char *icon = vala_panel_applet_info_get_icon_name(pl_info);
		gtk_list_store_set(list,
		                   &it,
		                   COLUMN_ICON,
		                   icon,
		                   COLUMN_NAME,
		                   _(name),
		                   COLUMN_EXPAND,
		                   expand,
		                   COLUMN_DATA,
		                   w,
		                   -1);
	}
	gtk_tree_view_set_model(self->plugin_list, list);
}

static void on_plugin_list_row_activated(GtkTreeView *tree_view, GtkTreePath *path,
                                         GtkTreeViewColumn *column, gpointer user_data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(user_data);
	GtkTreeSelection *tree_sel    = gtk_tree_view_get_selection(tree_view);
	GtkTreeModel *model;
	GtkTreeIter iter;
	ValaPanelApplet *pl;
	if (!gtk_tree_selection_get_selected(tree_sel, &model, &iter))
		return;
	gtk_tree_model_get(model, &iter, COLUMN_DATA, &pl, -1);
	if (vala_panel_applet_is_configurable(pl))
		vala_panel_applet_show_config_dialog(pl);
}

static void init_plugin_list(ValaPanelToplevelConfig *self)
{
	GtkTreeIter it;
	GtkCellRenderer *textrender = gtk_cell_renderer_pixbuf_new();
	GtkTreeViewColumn *col      = gtk_tree_view_column_new_with_attributes(_("Icon"),
                                                                          textrender,
                                                                          "icon-name",
                                                                          COLUMN_ICON,
                                                                          NULL);
	gtk_tree_view_column_set_expand(col, true);
	gtk_tree_view_append_column(self->plugin_list, col);
	textrender = gtk_cell_renderer_text_new();
	col        = gtk_tree_view_column_new_with_attributes(_("Currently loaded plugins"),
                                                       textrender,
                                                       "text",
                                                       COLUMN_NAME,
                                                       NULL);
	gtk_tree_view_column_set_expand(col, true);
	gtk_tree_view_append_column(self->plugin_list, col);
	GtkCellRendererToggle *render = gtk_cell_renderer_toggle_new();
	gtk_cell_renderer_toggle_set_activatable(render, true);
	g_signal_connect(render, "toggled", G_CALLBACK(on_plugin_expand_toggled), self);
	col = gtk_tree_view_column_new_with_attributes(_("Stretch"),
	                                               render,
	                                               "active",
	                                               COLUMN_EXPAND,
	                                               NULL);
	gtk_tree_view_column_set_expand(col, true);
	gtk_tree_view_column_set_cell_data_func(col, render, on_stretch_render, self, NULL);
	gtk_tree_view_append_column(self->plugin_list, col);
	update_plugin_list_model(self);
	g_signal_connect(self->plugin_list,
	                 "row-activated",
	                 G_CALLBACK(on_plugin_list_row_activated),
	                 self);
	GtkTreeModel *list         = gtk_tree_view_get_model(self->plugin_list);
	GtkTreeSelection *tree_sel = gtk_tree_view_get_selection(self->plugin_list);
	gtk_tree_selection_set_mode(tree_sel, GTK_SELECTION_BROWSE);
	g_signal_connect(tree_sel, "changed", G_CALLBACK(on_sel_plugin_changed), self);
	if (gtk_tree_model_get_iter_first(list, &it))
		gtk_tree_selection_select_iter(tree_sel, &it);
}

static void on_configure_plugin(GtkButton *btn, void *data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(data);
	GtkTreeSelection *tree_sel    = gtk_tree_view_get_selection(self->plugin_list);
	GtkTreeModel *model;
	GtkTreeIter iter;
	ValaPanelApplet *pl;
	if (!gtk_tree_selection_get_selected(tree_sel, &model, &iter))
		return;
	gtk_tree_model_get(model, &iter, COLUMN_DATA, &pl, -1);
	if (vala_panel_applet_is_configurable(pl))
		vala_panel_applet_show_config_dialog(pl);
}
static int sort_by_name(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, void *unused)
{
	char *str_a, *str_b;
	gtk_tree_model_get(model, a, 0, &str_a, -1);
	gtk_tree_model_get(model, b, 0, &str_b, -1);
	int ret = g_utf8_collate(str_a, str_b);
	g_free(str_a);
	g_free(str_b);
	return ret;
}

static void update_widget_position_keys(ValaPanelToplevelConfig *self)
{
	ValaPanelLayout *layout       = vala_panel_toplevel_get_layout(self->_toplevel);
	g_autoptr(GList) applets_list = vala_panel_layout_get_applets_list(layout);
	for (GList *l = applets_list; l != NULL; l = g_list_next(l))
	{
		ValaPanelApplet *applet  = VALA_PANEL_APPLET(l->data);
		uint idx                 = vala_panel_layout_get_applet_position(layout, applet);
		ValaPanelUnitSettings *s = vala_panel_layout_get_applet_settings(applet);
		g_settings_set_uint(s->default_settings, VALA_PANEL_KEY_POSITION, idx);
	}
}

static void on_add_plugin_row_activated(GtkTreeView *tree_view, GtkTreePath *path,
                                        GtkTreeViewColumn *column, gpointer user_data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(user_data);
	GtkTreeIter it;
	GtkTreeModel *model;
	GtkTreeSelection *sel = gtk_tree_view_get_selection(tree_view);
	if (gtk_tree_selection_get_selected(sel, &model, &it))
	{
		const char *type;
		gtk_tree_model_get(model, &it, 2, &type, -1);
		ValaPanelLayout *layout = vala_panel_toplevel_get_layout(self->_toplevel);
		vala_panel_layout_add_applet(layout, type);
		vala_panel_layout_update_applet_positions(layout);
		update_plugin_list_model(self);
	}
	update_widget_position_keys(self);
}

static void on_add_plugin(GtkButton *btn, ValaPanelToplevelConfig *self)
{
	GtkPopover *dlg           = gtk_popover_new(self->adding_button);
	GtkScrolledWindow *scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(scroll, GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(dlg, scroll);
	GtkTreeView *view = gtk_tree_view_new();
	gtk_container_add(scroll, view);
	GtkTreeSelection *tree_sel = gtk_tree_view_get_selection(view);
	gtk_tree_selection_set_mode(tree_sel, GTK_SELECTION_BROWSE);

	GtkCellRenderer *render = gtk_cell_renderer_pixbuf_new();
	GtkTreeViewColumn *col  = gtk_tree_view_column_new_with_attributes(_("Icon"),
                                                                          render,
                                                                          "icon-name",
                                                                          COLUMN_ICON,
                                                                          NULL);

	gtk_tree_view_append_column(view, col);
	render = gtk_cell_renderer_text_new();
	col    = gtk_tree_view_column_new_with_attributes(_("Available plugins"),
                                                       render,
                                                       "text",
                                                       1,
                                                       NULL);
	gtk_tree_view_append_column(view, col);

	GtkListStore *list = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	/* Populate list of available plugins.
	 * Omit plugins that can only exist once per system if it is already configured. */
	vala_panel_applet_manager_reload_applets(vala_panel_layout_get_manager());
	GList *all_types = vala_panel_applet_manager_get_all_types(vala_panel_layout_get_manager());
	for (GList *l = all_types; l != NULL; l = g_list_next(l))
	{
		AppletInfoData *d = (AppletInfoData *)l->data;
		if (!vala_panel_applet_info_is_exclusive(d->info) || (d->count < 1))
		{
			GtkTreeIter it;
			gtk_list_store_append(list, &it);
			/* it is safe to put classes data here - they will be valid until restart */
			const char *icon_name   = vala_panel_applet_info_get_icon_name(d->info);
			const char *name        = _(vala_panel_applet_info_get_name(d->info));
			const char *module_name = vala_panel_applet_info_get_module_name(d->info);

			gtk_list_store_set(list, &it, 0, icon_name, 1, name, 2, module_name, -1);
		}
	}
	g_list_free(all_types);
	gtk_tree_sortable_set_default_sort_func(list, sort_by_name, self, NULL);
	gtk_tree_sortable_set_sort_column_id(list,
	                                     GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
	                                     GTK_SORT_ASCENDING);
	gtk_tree_view_set_activate_on_single_click(view, true);
	gtk_tree_view_set_model(view, list);
	g_signal_connect(view, "row-activated", G_CALLBACK(on_add_plugin_row_activated), self);
	gtk_scrolled_window_set_min_content_width(scroll, 320);
	gtk_scrolled_window_set_min_content_height(scroll, 200);
	gtk_widget_show_all(dlg);
}

static void on_remove_plugin(GtkButton *btn, void *user_data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(user_data);
	GtkTreeIter it;
	GtkTreeModel *model;
	GtkTreeSelection *tree_sel = gtk_tree_view_get_selection(self->plugin_list);
	ValaPanelApplet *pl;
	if (gtk_tree_selection_get_selected(tree_sel, &model, &it))
	{
		GtkTreePath *tree_path = gtk_tree_model_get_path(model, &it);
		gtk_tree_model_get(model, &it, COLUMN_DATA, &pl, -1);
		if (gtk_tree_path_get_indices(tree_path)[0] >=
		    gtk_tree_model_iter_n_children(model, NULL))
			gtk_tree_path_prev(tree_path);
		gtk_list_store_remove(GTK_LIST_STORE(model), &it);
		gtk_tree_selection_select_path(tree_sel, tree_path);
		ValaPanelLayout *layout = vala_panel_toplevel_get_layout(self->_toplevel);
		vala_panel_layout_remove_applet(layout, pl);
	}
}

static void on_moveup_plugin(GtkButton *btn, void *user_data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(user_data);
	GtkTreeIter it, prev;
	GtkTreeModel *model        = gtk_tree_view_get_model(self->plugin_list);
	GtkTreeSelection *tree_sel = gtk_tree_view_get_selection(self->plugin_list);
	if (!gtk_tree_model_get_iter_first(model, &it))
		return;
	if (gtk_tree_selection_iter_is_selected(tree_sel, &it))
		return;
	do
	{
		if (gtk_tree_selection_iter_is_selected(tree_sel, &it))
		{
			ValaPanelApplet *pl;
			ValaPanelLayout *layout = vala_panel_toplevel_get_layout(self->_toplevel);
			gtk_tree_model_get(model, &it, COLUMN_DATA, &pl, -1);
			gtk_list_store_move_before(GTK_LIST_STORE(model), &it, &prev);

			uint i = vala_panel_layout_get_applet_position(layout, pl);
			/* reorder in config, 0 is Global */
			i = i > 0 ? i : 0;

			/* reorder in panel */
			vala_panel_layout_set_applet_position(layout, pl, (int)i - 1);
			update_widget_position_keys(self);
			return;
		}
		prev = it;
	} while (gtk_tree_model_iter_next(model, &it));
}

static void on_movedown_plugin(GtkButton *btn, void *user_data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(user_data);
	GtkTreeIter it, next;
	GtkTreeModel *model;
	GtkTreeSelection *tree_sel = gtk_tree_view_get_selection(self->plugin_list);
	if (!gtk_tree_selection_get_selected(tree_sel, &model, &it))
		return;
	next = it;
	if (!gtk_tree_model_iter_next(model, &next))
		return;

	ValaPanelApplet *pl;
	ValaPanelLayout *layout = vala_panel_toplevel_get_layout(self->_toplevel);
	gtk_tree_model_get(model, &it, COLUMN_DATA, &pl, -1);
	gtk_list_store_move_after(GTK_LIST_STORE(model), &it, &next);

	uint i = vala_panel_layout_get_applet_position(layout, pl);
	/* reorder in panel */
	vala_panel_layout_set_applet_position(layout, pl, (int)i + 1);
	update_widget_position_keys(self);
	return;
}

static void vala_panel_toplevel_config_class_init(ValaPanelToplevelConfigClass *klass)
{
	vala_panel_toplevel_config_parent_class = g_type_class_peek_parent(klass);
	G_OBJECT_CLASS(klass)->get_property     = vala_panel_toplevel_config_get_property;
	G_OBJECT_CLASS(klass)->set_property     = vala_panel_toplevel_config_set_property;
	G_OBJECT_CLASS(klass)->constructor      = vala_panel_configure_dialog_constructor;
	G_OBJECT_CLASS(klass)->finalize         = vala_panel_configure_dialog_finalize;
	vala_panel_toplevel_config_properties[TOPLEVEL_PROPERTY] =
	    g_param_spec_object(VALA_PANEL_KEY_TOPLEVEL,
	                        VALA_PANEL_KEY_TOPLEVEL,
	                        VALA_PANEL_KEY_TOPLEVEL,
	                        VALA_PANEL_TYPE_TOPLEVEL,
	                        (GParamFlags)(G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
	                                      G_PARAM_STATIC_BLURB | G_PARAM_READABLE |
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
	                                          "plugin-desc",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          plugin_desc));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "add-button",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          adding_button));
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "configure-button",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          configure_button));
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
	                                             "on_configure_plugin",
	                                             G_CALLBACK(on_configure_plugin));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_add_plugin",
	                                             G_CALLBACK(on_add_plugin));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_remove_plugin",
	                                             G_CALLBACK(on_remove_plugin));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_moveup_plugin",
	                                             G_CALLBACK(on_moveup_plugin));
	gtk_widget_class_bind_template_callback_full(GTK_WIDGET_CLASS(klass),
	                                             "on_movedown_plugin",
	                                             G_CALLBACK(on_movedown_plugin));
}

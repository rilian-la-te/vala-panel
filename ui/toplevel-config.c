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
#include "definitions.h"
#include "toplevel.h"
#include "util-gtk.h"
#include "vala-panel-compat.h"

G_DEFINE_TYPE(ValaPanelToplevelConfig, vala_panel_toplevel_config, GTK_TYPE_DIALOG);

#define COLUMN_ICON 0
#define COLUMN_NAME 1
#define COLUMN_EXPAND 2
#define COLUMN_DATA 3
enum
{
	TOPLEVEL_PROPERTY,
	NUM_PROPERTIES
};
static GParamSpec *vala_panel_toplevel_config_properties[NUM_PROPERTIES];
static void state_configure_monitor(GSimpleAction *act, GVariant *param, void *data);

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
	gtk_tree_model_get(self->store_monitors, &iter, &request_mon, NULL);
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
		gtk_list_store_insert_with_values(self->store_monitors,
		                                  &iter,
		                                  0,
		                                  i,
		                                  1,
		                                  gdk_monitor_get_model(
		                                      gdk_display_get_monitor(screen, i)),
		                                  NULL);
	}
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
	                       VALA_PANEL_KEY_CORNERS_SIZE,
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
	if (file)
		gtk_file_chooser_set_filename(self->file_background, file);
	gtk_widget_set_sensitive(self->file_background, use_background_file);
	g_object_bind_property(self->_toplevel,
	                       VALA_PANEL_KEY_USE_BACKGROUND_FILE,
	                       self->file_background,
	                       "sensitive",
	                       G_BINDING_SYNC_CREATE);
	g_signal_connect(self->color_background,
	                 "file-set",
	                 G_CALLBACK(background_file_connector),
	                 self->_toplevel);
	g_free0(file);
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
	//    init_plugin_list();
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

extern ValaPanelAppletHolder *vala_panel_layout_holder;
static void on_sel_plugin_changed(GtkTreeSelection *tree_sel, void *data)
{
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(data);
	GtkTreeIter it;
	GtkTreeModel *model;
	ValaPanelApplet *pl;
	if (gtk_tree_selection_get_selected(tree_sel, &model, &it))
	{
		gtk_tree_model_get(model, &it, COLUMN_DATA, &pl, -1);
		ValaPanelAppletPlugin *apl =
		    vala_panel_applet_holder_get_plugin(vala_panel_layout_holder,
		                                        pl,
		                                        vala_panel_toplevel_get_core_settings());
		PeasPluginInfo *pl_info = peas_extension_base_get_plugin_info(apl);
		char *desc              = peas_plugin_info_get_description(pl_info);
		//        plugin_desc.set_text(_(desc) );
		//        configure_button.set_sensitive(pl.is_configurable());
	}
}
// private void on_plugin_expand_toggled(string path)
//{
//    TreeIter it;
//    TreePath tp = new TreePath.from_string( path );
//    var model = plugin_list.get_model();
//    if( model.get_iter(out it, tp) )
//    {
//        Applet pl;
//        bool expand;
//        model.get(it, Column.DATA, out pl, Column.EXPAND, out expand, -1 );
//        if
//        (Layout.holder.get_plugin(pl,Toplevel.core_settings).plugin_info.get_external_data(Data.EXPANDABLE)!=null)
//        {
//            expand = !expand;
//            (model as Gtk.ListStore).set(it,Column.EXPAND,expand,-1);
//            unowned UnitSettings s = toplevel.layout.get_applet_settings(pl);
//            s.default_settings.set_boolean(Key.EXPAND,expand);
//        }
//    }
//}
static void on_stretch_render(GtkCellLayout *layout, GtkCellRenderer *renderer, GtkTreeModel *model,
                              GtkTreeIter *iter, void *data)
{
	/* Set the control visible depending on whether stretch is available for the plugin.
	 * The g_object_set method is touchy about its parameter, so we can't pass the boolean
	 * directly. */
	ValaPanelToplevelConfig *self = VALA_PANEL_TOPLEVEL_CONFIG(data);
	ValaPanelApplet *pl;
	gtk_tree_model_get(model, iter, COLUMN_DATA, &pl, -1);
	ValaPanelAppletPlugin *apl =
	    vala_panel_applet_holder_get_plugin(vala_panel_layout_holder,
	                                        pl,
	                                        vala_panel_toplevel_get_core_settings());
	PeasPluginInfo *pl_info = peas_extension_base_get_plugin_info(apl);
	char *ext = peas_plugin_info_get_external_data(apl, VALA_PANEL_DATA_EXPANDABLE);
	gtk_cell_renderer_set_visible(renderer, ext != NULL);
}

static void vala_panel_toplevel_config_class_init(ValaPanelToplevelConfigClass *klass)
{
	vala_panel_toplevel_config_parent_class = g_type_class_peek_parent(klass);
	G_OBJECT_CLASS(klass)->get_property     = vala_panel_toplevel_config_get_property;
	G_OBJECT_CLASS(klass)->set_property     = vala_panel_toplevel_config_set_property;
	G_OBJECT_CLASS(klass)->constructor      = vala_panel_configure_dialog_constructor;
	G_OBJECT_CLASS(klass)->finalize         = vala_panel_configure_dialog_finalize;
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                TOPLEVEL_PROPERTY,
	                                vala_panel_toplevel_config_properties[TOPLEVEL_PROPERTY] =
	                                    g_param_spec_object("toplevel",
	                                                        "toplevel",
	                                                        "toplevel",
	                                                        VALA_PANEL_TYPE_TOPLEVEL,
	                                                        G_PARAM_STATIC_NAME |
	                                                            G_PARAM_STATIC_NICK |
	                                                            G_PARAM_STATIC_BLURB |
	                                                            G_PARAM_READABLE |
	                                                            G_PARAM_WRITABLE |
	                                                            G_PARAM_CONSTRUCT_ONLY));
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
	//        gtk_widget_class_bind_template_callback_full (GTK_WIDGET_CLASS (klass),
	//        "on_configure_plugin",
	//        G_CALLBACK(_vala_panel_configure_dialog_on_configure_plugin_gtk_button_clicked));
	//    gtk_widget_class_bind_template_callback_full (GTK_WIDGET_CLASS (klass),
	//    "on_add_plugin",
	//    G_CALLBACK(_vala_panel_configure_dialog_on_add_plugin_gtk_button_clicked));
	//    gtk_widget_class_bind_template_callback_full (GTK_WIDGET_CLASS (klass),
	//    "on_remove_plugin",
	//    G_CALLBACK(_vala_panel_configure_dialog_on_remove_plugin_gtk_button_clicked));
	//    gtk_widget_class_bind_template_callback_full (GTK_WIDGET_CLASS (klass),
	//    "on_moveup_plugin",
	//    G_CALLBACK(_vala_panel_configure_dialog_on_moveup_plugin_gtk_button_clicked));
	//    gtk_widget_class_bind_template_callback_full (GTK_WIDGET_CLASS (klass),
	//    "on_movedown_plugin",
	//    G_CALLBACK(_vala_panel_configure_dialog_on_movedown_plugin_gtk_button_clicked));
}

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

struct _ValaPanelToplevelConfig
{
	GtkDialog parent_instance;
	GtkStack *prefs_stack;
	ValaPanelToplevel *_toplevel;
	GtkMenuButton *monitors_button;
	GtkSpinButton *spin_iconsize;
	GtkSpinButton *spin_height;
	GtkSpinButton *spin_width;
	GtkSpinButton *spin_corners;
	GtkFontButton *font_selector;
	GtkBox *font_box;
	GtkColorButton *color_background;
	GtkColorButton *color_foreground;
	GtkFileChooserButton *file_background;
	GtkTreeView *plugin_list;
	GtkLabel *plugin_desc;
	GtkButton *adding_button;
	GtkButton *configure_button;
};

G_DEFINE_TYPE(ValaPanelToplevelConfig, vala_panel_toplevel_config, GTK_TYPE_DIALOG);

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
	g_object_unref0(self->monitors_button);
	g_object_unref0(self->spin_iconsize);
	g_object_unref0(self->spin_height);
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

static void vala_panel_toplevel_config_class_init(ValaPanelToplevelConfigClass *klass)
{
	vala_panel_toplevel_config_parent_class = g_type_class_peek_parent(klass);
	//    G_OBJECT_CLASS (klass)->get_property = _vala_vala_panel_configure_dialog_get_property;
	//    G_OBJECT_CLASS (klass)->set_property = _vala_vala_panel_configure_dialog_set_property;
	//    G_OBJECT_CLASS (klass)->constructor = vala_panel_configure_dialog_constructor;
	G_OBJECT_CLASS(klass)->finalize = vala_panel_configure_dialog_finalize;
	//    g_object_class_install_property (G_OBJECT_CLASS (klass),
	//    VALA_PANEL_CONFIGURE_DIALOG_TOPLEVEL_PROPERTY,
	//    vala_panel_configure_dialog_properties[VALA_PANEL_CONFIGURE_DIALOG_TOPLEVEL_PROPERTY]
	//    = g_param_spec_object ("toplevel", "toplevel", "toplevel", VALA_PANEL_TYPE_TOPLEVEL,
	//    G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE |
	//    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass),
	                                            "/org/vala-panel/lib/pref.ui");
	gtk_widget_class_bind_template_child_full(GTK_WIDGET_CLASS(klass),
	                                          "monitors-button",
	                                          FALSE,
	                                          G_STRUCT_OFFSET(ValaPanelToplevelConfig,
	                                                          monitors_button));
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
	//    gtk_widget_class_bind_template_callback_full (GTK_WIDGET_CLASS (klass),
	//    "on_configure_plugin",
	//    G_CALLBACK(_vala_panel_configure_dialog_on_configure_plugin_gtk_button_clicked));
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

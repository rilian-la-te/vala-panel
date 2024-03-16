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

#include "toplevel.h"
#include "definitions.h"
#include "panel-layout.h"
#include "private.h"
#include "toplevel-config.h"
#include "util-gtk.h"
#include "vala-panel-enums.h"

#include <glib.h>
#include <math.h>
#include <stdbool.h>

static const uint PERIOD = 200;

typedef enum
{
	XXS  = 16,
	XS   = 22,
	S    = 24,
	M    = 32,
	L    = 48,
	XL   = 96,
	XXL  = 128,
	XXXL = 256
} ValaPanelIconSizeHints;
typedef enum
{
	AH_HIDDEN  = 0,
	AH_WAITING = 1,
	AH_GRAB    = 2,
	AH_VISIBLE = 3,
} PanelAutohideState;

static ValaPanelPlatform *platform = NULL;

static ValaPanelToplevel *vala_panel_toplevel_create(GtkApplication *app, const char *name, int mon,
                                                     ValaPanelGravity e);
static void vala_panel_toplevel_update_geometry(ValaPanelToplevel *self);
static void activate_new_panel(GSimpleAction *act, GVariant *param, void *data);
static void activate_remove_panel(GSimpleAction *act, GVariant *param, void *data);
static void activate_panel_settings(GSimpleAction *act, GVariant *param, void *data);

static const GActionEntry panel_entries[] = {
	{ "new-panel", activate_new_panel, NULL, NULL, NULL, { 0 } },
	{ "remove-panel", activate_remove_panel, NULL, NULL, NULL, { 0 } },
	{ "panel-settings", activate_panel_settings, "s", NULL, NULL, { 0 } }
};

enum
{
	TOP_DUMMY,
	TOP_UUID,
	TOP_HEIGHT,
	TOP_WIDTH,
	TOP_USE_FONT,
	TOP_USE_BG_COLOR,
	TOP_USE_FG_COLOR,
	TOP_USE_BG_FILE,
	TOP_FONT_SIZE_ONLY,
	TOP_FONT_SIZE,
	TOP_CORNER_RAD,
	TOP_FONT,
	TOP_BG_COLOR,
	TOP_FG_COLOR,
	TOP_ICON_SIZE,
	TOP_TB_LOOK,
	TOP_BG_FILE,
	TOP_GRAVITY,
	TOP_ORIENTATION,
	TOP_MONITOR,
	TOP_DOCK,
	TOP_STRUT,
	TOP_IS_DYNAMIC,
	TOP_AUTOHIDE,
	TOP_LAST
};
static GParamSpec *top_specs[TOP_LAST];

struct _ValaPanelToplevel
{
	GtkApplicationWindow __parent__;
	ValaPanelLayout *layout;
	GtkRevealer *ah_rev;
	PanelAutohideState ah_state;
	ValaPanelUnitSettings *settings;
	GtkCssProvider *provider;
	bool initialized;
	bool dock;
	bool autohide;
	bool is_dynamic;
	bool strut;
	bool use_background_color;
	bool use_foreground_color;
	bool use_background_file;
	bool use_font;
	bool font_size_only;
	bool use_toolbar_appearance;
	uint corner_radius;
	ValaPanelIconSizeHints icon_size_hints;
	GdkRGBA background_color;
	GdkRGBA foreground_color;
	char *background_file;
	char *uuid;
	int mon;
	int height;
	int width;
	ValaPanelGravity gravity;
	ValaPanelToplevelConfig *pref_dialog;
	GtkMenu *context_menu;
	char *font;
};

G_DEFINE_TYPE(ValaPanelToplevel, vala_panel_toplevel, GTK_TYPE_APPLICATION_WINDOW)
/*****************************************************************************************
 *                                   Common functions
 *****************************************************************************************/

G_GNUC_INTERNAL ValaPanelCoreSettings *vp_toplevel_get_core_settings()
{
	return vala_panel_platform_get_settings(platform);
}
G_GNUC_INTERNAL ValaPanelAppletManager *vp_toplevel_get_manager()
{
	return vp_platform_get_manager(platform);
}

G_GNUC_INTERNAL bool vp_toplevel_is_initialized(ValaPanelToplevel *self)
{
	return self->initialized;
}

G_GNUC_INTERNAL ValaPanelPlatform *vp_toplevel_get_current_platform()
{
	return platform;
}

const char *vala_panel_get_current_platform_name()
{
	return vala_panel_platform_get_name(platform);
}

static void stop_ui(ValaPanelToplevel *self)
{
	if (self->pref_dialog != NULL)
		gtk_dialog_response(GTK_DIALOG(self->pref_dialog), GTK_RESPONSE_CLOSE);
	if (self->initialized)
	{
		gdk_display_flush(gtk_widget_get_display(GTK_WIDGET(self)));
		self->initialized = false;
	}
	GtkWidget *ch = gtk_bin_get_child(GTK_BIN(self));
	g_clear_pointer(&ch, gtk_widget_destroy);
}

static void vala_panel_toplevel_destroy(GObject *base)
{
	ValaPanelToplevel *self = VALA_PANEL_TOPLEVEL(base);
	stop_ui(self);
	G_OBJECT_CLASS(vala_panel_toplevel_parent_class)->dispose(base);
}

static void vala_panel_toplevel_finalize(GObject *obj)
{
	ValaPanelToplevel *self = VALA_PANEL_TOPLEVEL(obj);
	g_clear_object(&self->provider);
	g_clear_pointer(&self->uuid, g_free);
	g_clear_pointer(&self->font, g_free);
	g_clear_pointer(&self->background_file, g_free);
	G_OBJECT_CLASS(vala_panel_toplevel_parent_class)->finalize(obj);
}

static void start_ui(ValaPanelToplevel *self)
{
	vala_panel_style_from_res(GTK_WIDGET(self),
	                          "/org/vala-panel/lib/style.css",
	                          "-panel-transparent");
	vala_panel_style_class_toggle(GTK_WIDGET(self), "-panel-transparent", false);
	gtk_window_set_application(GTK_WINDOW(self), gtk_window_get_application(GTK_WINDOW(self)));
	gtk_widget_add_events(GTK_WIDGET(self),
	                      GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK |
	                          GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_realize(GTK_WIDGET(self));
	self->ah_rev = GTK_REVEALER(gtk_revealer_new());
	self->layout = vp_layout_new(VALA_PANEL_TOPLEVEL(self),
	                             vala_panel_orient_from_gravity(self->gravity),
	                             0);
	gtk_revealer_set_transition_type(self->ah_rev, GTK_REVEALER_TRANSITION_TYPE_CROSSFADE);
	g_signal_connect_swapped(self->ah_rev,
	                         "notify::child-revealed",
	                         G_CALLBACK(gtk_widget_queue_draw),
	                         self->layout);
	gtk_container_add(GTK_CONTAINER(self->ah_rev), GTK_WIDGET(self->layout));
	gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(self->ah_rev));
	vp_layout_init_applets(self->layout);
	gtk_widget_show(GTK_WIDGET(self->ah_rev));
	gtk_widget_show(GTK_WIDGET(self->layout));
	g_object_bind_property(self,
	                       "orientation",
	                       self->layout,
	                       "orientation",
	                       G_BINDING_SYNC_CREATE);
	gtk_revealer_set_reveal_child(self->ah_rev, true);
	gtk_window_set_type_hint(GTK_WINDOW(self),
	                         (self->dock) ? GDK_WINDOW_TYPE_HINT_DOCK
	                                      : GDK_WINDOW_TYPE_HINT_NORMAL);
	// End To Layout
	gtk_widget_show(GTK_WIDGET(self));
	gtk_window_stick(GTK_WINDOW(self));
	vp_layout_update_applet_positions(self->layout);
	gtk_window_present(GTK_WINDOW(self));
	bool autohide = g_settings_get_boolean(self->settings->common, VALA_PANEL_KEY_AUTOHIDE);
	g_object_set(self, VALA_PANEL_KEY_AUTOHIDE, autohide, NULL);
	vala_panel_toplevel_update_geometry(self);
	self->initialized = true;
}

static void init_actions(ValaPanelToplevel *self, bool use_internal_values)
{
	if (self->settings == NULL)
	{
		self->settings =
		    vp_core_settings_get_by_uuid(vala_panel_platform_get_settings(platform),
		                                 self->uuid);
	}
	if (use_internal_values)
	{
		g_settings_set_int(self->settings->common, VALA_PANEL_KEY_MONITOR, self->mon);
		g_settings_set_enum(self->settings->common,
		                    VALA_PANEL_KEY_GRAVITY,
		                    (int)self->gravity);
	}
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_GRAVITY);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_HEIGHT);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_WIDTH);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_DYNAMIC);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_AUTOHIDE);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_STRUT);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_DOCK);
	vala_panel_bind_gsettings(self, self->settings->common, VALA_PANEL_KEY_MONITOR);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_ICON_SIZE);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_BACKGROUND_COLOR);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_FOREGROUND_COLOR);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_BACKGROUND_FILE);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_FONT);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_CORNER_RADIUS);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_FONT_SIZE_ONLY);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_USE_BACKGROUND_COLOR);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_USE_FOREGROUND_COLOR);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_USE_FONT);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_USE_BACKGROUND_FILE);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->settings->common,
	                                   VALA_PANEL_KEY_USE_TOOLBAR_APPEARANCE);
	g_action_map_add_action_entries(G_ACTION_MAP(self),
	                                panel_entries,
	                                G_N_ELEMENTS(panel_entries),
	                                self);
}

void vala_panel_toplevel_init_ui(ValaPanelToplevel *self)
{
	if (self->mon < gdk_display_get_n_monitors(gtk_widget_get_display(GTK_WIDGET(self))))
		start_ui(self);
}

/**************************************************************************************
 *                                     Menus stuff
 **************************************************************************************/
static GtkMenu *vala_panel_toplevel_get_plugin_menu(ValaPanelToplevel *self, ValaPanelApplet *pl)
{
	g_autoptr(GtkBuilder) builder =
	    gtk_builder_new_from_resource("/org/vala-panel/lib/menus.ui");
	GMenu *gmenu = G_MENU(gtk_builder_get_object(builder, "panel-context-menu"));
	if (pl != NULL)
	{
		GMenu *gmenusection = G_MENU(gtk_builder_get_object(builder, "plugin-section"));
		vala_panel_applet_update_context_menu(pl, gmenusection);
	}
	if (GTK_IS_WIDGET(self->context_menu))
		gtk_widget_destroy(GTK_WIDGET(self->context_menu));
	self->context_menu = GTK_MENU(gtk_menu_new_from_model(G_MENU_MODEL(gmenu)));
	if (pl != NULL)
		gtk_menu_attach_to_widget(self->context_menu, GTK_WIDGET(pl), NULL);
	else
		gtk_menu_attach_to_widget(self->context_menu, GTK_WIDGET(self), NULL);
	gtk_widget_show(GTK_WIDGET(self->context_menu));
	return self->context_menu;
}

G_GNUC_INTERNAL bool vp_toplevel_release_event_helper(GtkWidget *_sender, GdkEventButton *e,
                                                      gpointer obj)
{
	ValaPanelToplevel *self = VALA_PANEL_TOPLEVEL(obj);
	ValaPanelApplet *pl     = NULL;
	if (VALA_PANEL_IS_APPLET(_sender))
		pl = VALA_PANEL_APPLET(_sender);
	if (e->button == 3)
	{
		GtkMenu *menu = vala_panel_toplevel_get_plugin_menu(self, pl);
		GdkGravity menug, widget;
		vala_panel_toplevel_get_menu_anchors(self, &menug, &widget);
		gtk_menu_popup_at_widget(menu, GTK_WIDGET(_sender), widget, menug, (GdkEvent *)e);
		return true;
	}
	return false;
}

static int button_release_event(GtkWidget *w, GdkEventButton *e)
{
	return vp_toplevel_release_event_helper(w, e, w);
}

/**************************************************************************************
 *                                     Actions stuff
 **************************************************************************************/
G_GNUC_INTERNAL void vp_toplevel_destroy_pref_dialog(ValaPanelToplevel *self)
{
	if (GTK_IS_WIDGET(self->pref_dialog))
		gtk_widget_destroy(GTK_WIDGET(self->pref_dialog));
	self->pref_dialog = NULL;
}

void vala_panel_toplevel_get_menu_anchors(ValaPanelToplevel *self, GdkGravity *menu_anchor,
                                          GdkGravity *widget_anchor)
{
	ValaPanelGravity gravity = self->gravity;
	switch (gravity)
	{
	case VALA_PANEL_GRAVITY_NORTH_LEFT:
	case VALA_PANEL_GRAVITY_NORTH_CENTER:
	case VALA_PANEL_GRAVITY_NORTH_RIGHT:
		*widget_anchor = GDK_GRAVITY_NORTH;
		*menu_anchor   = GDK_GRAVITY_SOUTH;
		break;
	case VALA_PANEL_GRAVITY_SOUTH_LEFT:
	case VALA_PANEL_GRAVITY_SOUTH_CENTER:
	case VALA_PANEL_GRAVITY_SOUTH_RIGHT:
		*menu_anchor   = GDK_GRAVITY_NORTH;
		*widget_anchor = GDK_GRAVITY_SOUTH;
		break;
	case VALA_PANEL_GRAVITY_WEST_UP:
	case VALA_PANEL_GRAVITY_WEST_CENTER:
	case VALA_PANEL_GRAVITY_WEST_DOWN:
		*widget_anchor = GDK_GRAVITY_NORTH_WEST;
		*menu_anchor   = GDK_GRAVITY_NORTH_EAST;
		break;
	case VALA_PANEL_GRAVITY_EAST_UP:
	case VALA_PANEL_GRAVITY_EAST_CENTER:
	case VALA_PANEL_GRAVITY_EAST_DOWN:
		*menu_anchor   = GDK_GRAVITY_NORTH_WEST;
		*widget_anchor = GDK_GRAVITY_NORTH_EAST;
		break;
	}
}

void vala_panel_toplevel_configure(ValaPanelToplevel *self, const char *page)
{
	if (self->pref_dialog == NULL)
		self->pref_dialog =
		    g_object_new(vp_toplevel_config_get_type(), "toplevel", self, NULL);
	vp_toplevel_config_select_page(self->pref_dialog, page);
	gtk_window_present(GTK_WINDOW(self->pref_dialog));
	g_signal_connect_swapped(self->pref_dialog,
	                         "hide",
	                         G_CALLBACK(vp_toplevel_destroy_pref_dialog),
	                         self);
}

void vala_panel_toplevel_configure_applet(ValaPanelToplevel *self, const char *uuid)
{
	if (self->pref_dialog == NULL)
		self->pref_dialog =
		    g_object_new(vp_toplevel_config_get_type(), "toplevel", self, NULL);
	vp_toplevel_config_select_page(self->pref_dialog, "applets");
	vp_toplevel_config_select_applet(self->pref_dialog, uuid);
	gtk_window_present(GTK_WINDOW(self->pref_dialog));
	g_signal_connect_swapped(self->pref_dialog,
	                         "hide",
	                         G_CALLBACK(vp_toplevel_destroy_pref_dialog),
	                         self);
}

static void activate_new_panel(G_GNUC_UNUSED GSimpleAction *act, G_GNUC_UNUSED GVariant *param,
                               void *data)
{
	ValaPanelToplevel *self  = VALA_PANEL_TOPLEVEL(data);
	int new_mon              = -2;
	GtkPositionType new_edge = GTK_POS_TOP;
	bool found               = false;
	/* Allocate the edge. */
	g_assert(gtk_widget_get_display(GTK_WIDGET(self)) != NULL);
	int monitors = gdk_display_get_n_monitors(gtk_widget_get_display(GTK_WIDGET(self)));
	/* try to allocate edge on current monitor first */
	int m = self->mon;
	for (int e = GTK_POS_BOTTOM; e >= GTK_POS_LEFT; e--)
	{
		if (vala_panel_platform_edge_available(platform, NULL, (ValaPanelGravity)e * 3, m))
		{
			new_edge = (GtkPositionType)e;
			new_mon  = m;
			found    = true;
		}
	}
	/* try all monitors */
	if (!found)
		for (m = 0; m < monitors; ++m)
		{
			/* try each of the four edges */
			for (int e = GTK_POS_BOTTOM; e >= GTK_POS_LEFT; e--)
			{
				if ((vala_panel_platform_edge_available(platform,
				                                        NULL,
				                                        (ValaPanelGravity)e * 3,
				                                        m)))
				{
					new_edge = (GtkPositionType)e;
					new_mon  = m;
					found    = true;
				}
			}
		}
	if (!found)
	{
		g_warning(
		    "Error adding panel: There is no room for another panel. All the edges are "
		    "taken.");
		g_autoptr(GtkWidget) msg = gtk_message_dialog_new(
		    GTK_WINDOW(self),
		    GTK_DIALOG_DESTROY_WITH_PARENT,
		    GTK_MESSAGE_ERROR,
		    GTK_BUTTONS_CLOSE,
		    _("There is no room for another panel. All the edges are taken."));
		vala_panel_apply_window_icon(GTK_WINDOW(msg));
		gtk_window_set_title(GTK_WINDOW(msg), _("Error"));
		gtk_dialog_run(GTK_DIALOG(msg));
		gtk_widget_destroy(GTK_WIDGET(msg));
		return;
	}
	g_autofree char *new_name = g_uuid_string_random();
	// FIXME: Translate after adding constructors
	ValaPanelToplevel *new_toplevel =
	    vala_panel_toplevel_create(gtk_window_get_application(GTK_WINDOW(self)),
	                               new_name,
	                               new_mon,
	                               (ValaPanelGravity)3 * new_edge);
	vala_panel_platform_register_unit(platform, GTK_WINDOW(new_toplevel));
	//        new_toplevel.configure("position");
	gtk_widget_show(GTK_WIDGET(new_toplevel));
	gtk_widget_queue_resize(GTK_WIDGET(new_toplevel));
}
static void activate_remove_panel(G_GNUC_UNUSED GSimpleAction *act, G_GNUC_UNUSED GVariant *param,
                                  void *data)
{
	ValaPanelToplevel *self = VALA_PANEL_TOPLEVEL(data);
	GtkMessageDialog *dlg   = GTK_MESSAGE_DIALOG(
            gtk_message_dialog_new_with_markup(GTK_WINDOW(self),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_QUESTION,
                                               GTK_BUTTONS_OK_CANCEL,
                                               _("Really delete this panel?\n<b>Warning:"
	                                           "This can not be recovered.</b>")));
	vala_panel_apply_window_icon(GTK_WINDOW(dlg));
	gtk_window_set_title(GTK_WINDOW(dlg), _("Confirm"));
	bool ok = (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK);
	gtk_widget_destroy(GTK_WIDGET(dlg));
	if (ok)
	{
		g_autofree char *uid = g_strdup(self->uuid);
		stop_ui(self);
		gtk_widget_hide(GTK_WIDGET(self));
		/* delete the config unit of this panel */
		ValaPanelCoreSettings *st = vala_panel_platform_get_settings(platform);
		vp_core_settings_remove_unit_settings_full(st, uid, true);
		/* Unregister unit in platform, causes panel destruction */
		vala_panel_platform_unregister_unit(platform, GTK_WINDOW(self));
	}
}
static void activate_panel_settings(G_GNUC_UNUSED GSimpleAction *act, G_GNUC_UNUSED GVariant *param,
                                    void *data)
{
	ValaPanelToplevel *self = VALA_PANEL_TOPLEVEL(data);
	vala_panel_toplevel_configure(self, g_variant_get_string(param, NULL));
}
/**************************************************************************
 * Appearance -------------------------------------------------------------
 **************************************************************************/
static void update_appearance(ValaPanelToplevel *self)
{
	if (self->provider)
		gtk_style_context_remove_provider(gtk_widget_get_style_context(GTK_WIDGET(self)),
		                                  GTK_STYLE_PROVIDER(self->provider));
	if (!self->font)
		return;
	g_autoptr(GString) str            = g_string_new("");
	g_autofree char *background_color = gdk_rgba_to_string(&self->background_color);
	g_string_append_printf(str, ".-vala-panel-background {\n");
	if (self->use_background_color)
		g_string_append_printf(str, " background-color: %s;\n", background_color);
	else
		g_string_append_printf(str, " background-color: transparent;\n");
	if (self->use_background_file)
	{
		g_string_append_printf(str,
		                       " background-image: url('%s');\n",
		                       self->background_file);
		/* Feature proposed: Background repeat */
		//~                 if (false)
		//~                     g_string_append_printf(str," background-repeat:
		// no-repeat;\n");
	}
	else
		g_string_append_printf(str, " background-image: none;\n");
	g_string_append_printf(str, "}\n");
	/* Feature proposed: Panel Layout and Shadow */
	//~             g_string_append_printf(str,".-vala-panel-shadow {\n");
	//~             g_string_append_printf(str," box-shadow: 0 0 0 3px alpha(0.3,
	//%s);\n",foreground_color);
	//~             g_string_append_printf(str," border-style: none;\n margin: 3px;\n");
	//~             g_string_append_printf(str,"}\n");
	g_string_append_printf(str, ".-vala-panel-round-corners {\n");
	g_string_append_printf(str, " border-radius: %upx;\n", self->corner_radius);
	g_string_append_printf(str, "}\n");
	PangoFontDescription *desc = pango_font_description_from_string(self->font);
	g_string_append_printf(str, ".-vala-panel-font-size {\n");
	g_string_append_printf(str,
	                       " font-size: %dpx;\n",
	                       pango_font_description_get_size(desc) / PANGO_SCALE);
	g_string_append_printf(str, "}\n");
	g_string_append_printf(str, ".-vala-panel-font {\n");
	const char *family   = pango_font_description_get_family(desc);
	PangoWeight weight   = pango_font_description_get_weight(desc);
	PangoStyle style     = pango_font_description_get_style(desc);
	PangoVariant variant = pango_font_description_get_variant(desc);
	g_string_append_printf(str,
	                       " font-style: %s;\n",
	                       (style == PANGO_STYLE_ITALIC)
	                           ? "italic"
	                           : ((style == PANGO_STYLE_OBLIQUE) ? "oblique" : "normal"));
	g_string_append_printf(str,
	                       " font-variant: %s;\n",
	                       (variant == PANGO_VARIANT_SMALL_CAPS) ? "small-caps" : "normal");
	g_string_append_printf(str,
	                       " font-weight: %s;\n",
	                       (weight <= PANGO_WEIGHT_SEMILIGHT)
	                           ? "light"
	                           : (weight >= PANGO_WEIGHT_SEMIBOLD ? "bold" : "normal"));
	g_string_append_printf(str, " font-family: %s;\n", family);
	g_string_append_printf(str, "}\n");
	g_string_append_printf(str, ".-vala-panel-foreground-color {\n");
	g_clear_pointer(&desc, pango_font_description_free);
	g_autofree char *foreground_color = gdk_rgba_to_string(&self->foreground_color);
	g_string_append_printf(str, " color: %s;\n", foreground_color);
	g_string_append_printf(str, "}\n");
	char *css = str->str;
	g_clear_object(&self->provider);
	self->provider = css_add_css_with_provider(GTK_WIDGET(self), css);
	vala_panel_style_class_toggle(GTK_WIDGET(self),
	                              "-vala-panel-background",
	                              self->use_background_color || self->use_background_file);
	vala_panel_style_class_toggle(GTK_WIDGET(self), "-vala-panel-shadow", false);
	vala_panel_style_class_toggle(GTK_WIDGET(self),
	                              "-vala-panel-round-corners",
	                              self->corner_radius > 0);
	vala_panel_style_class_toggle(GTK_WIDGET(self), "-vala-panel-font-size", self->use_font);
	vala_panel_style_class_toggle(GTK_WIDGET(self),
	                              "-vala-panel-font",
	                              self->use_font && !self->font_size_only);
	vala_panel_style_class_toggle(GTK_WIDGET(self),
	                              "-vala-panel-foreground-color",
	                              self->use_foreground_color);
	vala_panel_style_class_toggle(GTK_WIDGET(self),
	                              GTK_STYLE_CLASS_PRIMARY_TOOLBAR,
	                              self->use_toolbar_appearance);
}

static ValaPanelToplevel *vala_panel_toplevel_create_window(GtkApplication *app, const char *uuid)
{
	return VALA_PANEL_TOPLEVEL(g_object_new(vala_panel_toplevel_get_type(),
	                                        "border-width",
	                                        0,
	                                        "decorated",
	                                        false,
	                                        "name",
	                                        "ValaPanel",
	                                        "resizable",
	                                        false,
	                                        "title",
	                                        "ValaPanel",
	                                        "type-hint",
	                                        GDK_WINDOW_TYPE_HINT_DOCK,
	                                        "window-position",
	                                        GTK_WIN_POS_NONE,
	                                        "skip-taskbar-hint",
	                                        true,
	                                        "skip-pager-hint",
	                                        true,
	                                        "accept-focus",
	                                        false,
	                                        "application",
	                                        app,
	                                        "uuid",
	                                        uuid,
	                                        NULL));
}

static ValaPanelToplevel *vala_panel_toplevel_new_from_position(GtkApplication *app,
                                                                const char *uid, int mon,
                                                                ValaPanelGravity edge)
{
	ValaPanelToplevel *ret = vala_panel_toplevel_create_window(app, uid);
	ret->mon               = mon;
	ret->gravity           = edge;
	init_actions(ret, true);
	vala_panel_toplevel_init_ui(ret);
	return ret;
}
static ValaPanelToplevel *vala_panel_toplevel_create(GtkApplication *app, const char *name, int mon,
                                                     ValaPanelGravity e)
{
	vp_core_settings_add_unit_settings_full(vp_toplevel_get_core_settings(), name, name, true);
	return vala_panel_toplevel_new_from_position(app, name, mon, e);
}
ValaPanelToplevel *vala_panel_toplevel_new(GtkApplication *app, ValaPanelPlatform *plt,
                                           const char *uid)
{
	if (platform == NULL)
	{
		platform = plt;
	}
	ValaPanelToplevel *ret = vala_panel_toplevel_create_window(app, uid);
	init_actions(ret, false);
	return ret;
}

static inline int calc_width(int scrw, int panel_width, int panel_margin)
{
	int effective_width = (int)round(scrw * panel_width / 100.0);
	if ((effective_width + panel_margin) > scrw)
		effective_width = scrw - panel_margin;
	return effective_width;
}
static void measure(GtkWidget *w, GtkOrientation orient, G_GNUC_UNUSED int for_size, int *min,
                    int *nat, int *base_min, int *base_nat)
{
	ValaPanelToplevel *self = VALA_PANEL_TOPLEVEL(w);
	GdkRectangle marea      = { 0, 0, 0, 0 };
	GdkMonitor *mon         = vala_panel_platform_get_suitable_monitor(w, self->mon);
	if (self->mon < gdk_display_get_n_monitors(gtk_widget_get_display(GTK_WIDGET(self))))
		gdk_monitor_get_geometry(mon, &marea);
	int scrw = vala_panel_orient_from_gravity(self->gravity) == GTK_ORIENTATION_HORIZONTAL
	               ? marea.width
	               : marea.height;
	if (vala_panel_orient_from_gravity(self->gravity) != orient)
		*min = *nat = (!self->autohide || (self->ah_rev != NULL &&
		                                   gtk_revealer_get_reveal_child(self->ah_rev)))
		                  ? self->height
		                  : VALA_PANEL_AUTOHIDE_GAP;
	else
		*min = *nat = *base_min = *base_nat = calc_width(scrw, self->width, 0);
}
static void get_preferred_width_for_height(GtkWidget *w, int height, int *min, int *nat)
{
	int x = 0, y = 0;
	GtkOrientation eff_ori = GTK_ORIENTATION_HORIZONTAL;
	GTK_WIDGET_CLASS(vala_panel_toplevel_parent_class)
	    ->get_preferred_width_for_height(w, height, &x, &y);
	measure(w, eff_ori, height, min, nat, &x, &y);
}
static void get_preferred_height_for_width(GtkWidget *w, int height, int *min, int *nat)
{
	int x = 0, y = 0;
	GtkOrientation eff_ori = GTK_ORIENTATION_VERTICAL;
	GTK_WIDGET_CLASS(vala_panel_toplevel_parent_class)
	    ->get_preferred_width_for_height(w, height, &x, &y);
	measure(w, eff_ori, height, min, nat, &x, &y);
}
static void get_preferred_width(GtkWidget *w, int *min, int *nat)
{
	ValaPanelToplevel *self = VALA_PANEL_TOPLEVEL(w);
	*min = *nat = (!self->autohide ||
	               (self->ah_rev != NULL && gtk_revealer_get_reveal_child(self->ah_rev)))
	                  ? self->height
	                  : VALA_PANEL_AUTOHIDE_GAP;
}
static void get_preferred_height(GtkWidget *w, int *min, int *nat)
{
	ValaPanelToplevel *self = VALA_PANEL_TOPLEVEL(w);
	*min = *nat = (!self->autohide ||
	               (self->ah_rev != NULL && gtk_revealer_get_reveal_child(self->ah_rev)))
	                  ? self->height
	                  : VALA_PANEL_AUTOHIDE_GAP;
}
static GtkSizeRequestMode get_request_mode(GtkWidget *w)
{
	ValaPanelToplevel *self = VALA_PANEL_TOPLEVEL(w);
	return (vala_panel_orient_from_gravity(self->gravity) == GTK_ORIENTATION_HORIZONTAL)
	           ? GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT
	           : GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}
static void vala_panel_toplevel_update_geometry_no_orient(ValaPanelToplevel *self)
{
	GdkRectangle marea = { 0 };
	GdkMonitor *mon    = vala_panel_platform_get_suitable_monitor(GTK_WIDGET(self), self->mon);
	if (self->mon < gdk_display_get_n_monitors(gtk_widget_get_display(GTK_WIDGET(self))))
		gdk_monitor_get_geometry(mon, &marea);
	gtk_widget_queue_resize(GTK_WIDGET(self));
	while (gtk_events_pending())
		gtk_main_iteration_do(false);
	vala_panel_platform_move_to_side(platform, GTK_WINDOW(self), self->gravity, self->mon);
	vala_panel_platform_update_strut(platform, GTK_WINDOW(self));
	while (gtk_events_pending())
		gtk_main_iteration_do(false);
}

static void vala_panel_toplevel_update_geometry(ValaPanelToplevel *self)
{
	vala_panel_toplevel_update_geometry_no_orient(self);
	g_object_notify(G_OBJECT(self), "orientation");
}

void vala_panel_update_visibility(ValaPanelToplevel *panel, int mons)
{
	int monitor;
	g_object_get(panel, VALA_PANEL_KEY_MONITOR, &monitor, NULL);
	if (monitor < mons && !vp_toplevel_is_initialized(panel) && mons > 0)
		start_ui(panel);
	else if ((monitor >= mons && vp_toplevel_is_initialized(panel)) || mons == 0)
		stop_ui(panel);
	else
	{
		vala_panel_toplevel_update_geometry(panel);
	}
}
/****************************************************
 *         autohide : new version                   *
 ****************************************************/
static uint timeout_func(ValaPanelToplevel *self)
{
	if (self->autohide && self->ah_state == AH_WAITING)
	{
		vala_panel_style_class_toggle(GTK_WIDGET(self), "-panel-transparent", true);
		gtk_revealer_set_reveal_child(self->ah_rev, false);
		vala_panel_toplevel_update_geometry_no_orient(self);
		self->ah_state = AH_HIDDEN;
		return G_SOURCE_REMOVE;
	}
	return G_SOURCE_CONTINUE;
}

static void ah_show(ValaPanelToplevel *self)
{
	if (self->ah_state >= AH_GRAB)
		return;
	vala_panel_style_class_toggle(GTK_WIDGET(self), "-panel-transparent", false);
	gtk_revealer_set_reveal_child(self->ah_rev, true);
	vala_panel_toplevel_update_geometry_no_orient(self);
	self->ah_state = AH_VISIBLE;
}

static void ah_hide(ValaPanelToplevel *self)
{
	if (self->ah_state <= AH_GRAB)
		return;
	self->ah_state = AH_WAITING;
	g_timeout_add_full(G_PRIORITY_HIGH, PERIOD, (GSourceFunc)timeout_func, self, NULL);
}

static int enter_notify_event(GtkWidget *w, G_GNUC_UNUSED GdkEventCrossing *event)
{
	ValaPanelToplevel *self = VALA_PANEL_TOPLEVEL(w);
	ah_show(self);
	return false;
}

static int leave_notify_event(GtkWidget *w, GdkEventCrossing *event)
{
	ValaPanelToplevel *self = VALA_PANEL_TOPLEVEL(w);
	if (self->autohide && (event->detail != GDK_NOTIFY_INFERIOR))
		ah_hide(self);
	return false;
}

static void grab_notify(GtkWidget *w, int was_grabbed)
{
	ValaPanelToplevel *self = VALA_PANEL_TOPLEVEL(w);
	if (!was_grabbed)
		self->ah_state = AH_GRAB;
	else if (self->autohide)
	{
		self->ah_state = AH_VISIBLE;
		ah_hide(self);
	}
	else
		self->ah_state = AH_VISIBLE;
}

/*
 * Setters and getters
 */

G_GNUC_INTERNAL const char *vp_toplevel_get_uuid(ValaPanelToplevel *self)
{
	return self->uuid;
}

static void vala_panel_toplevel_get_property(GObject *object, guint property_id, GValue *value,
                                             GParamSpec *pspec)
{
	ValaPanelToplevel *self    = VALA_PANEL_TOPLEVEL(object);
	PangoFontDescription *desc = pango_font_description_from_string(self->font);
	char *str = NULL, *str2 = NULL;

	switch (property_id)
	{
	case TOP_UUID:
		g_value_set_string(value, self->uuid);
		break;
	case TOP_HEIGHT:
		g_value_set_int(value, self->height);
		break;
	case TOP_WIDTH:
		g_value_set_int(value, self->width);
		break;
	case TOP_USE_FONT:
		g_value_set_boolean(value, self->use_font);
		break;
	case TOP_USE_BG_COLOR:
		g_value_set_boolean(value, self->use_background_color);
		break;
	case TOP_USE_FG_COLOR:
		g_value_set_boolean(value, self->use_foreground_color);
		break;
	case TOP_USE_BG_FILE:
		g_value_set_boolean(value, self->use_background_file);
		break;
	case TOP_FONT_SIZE_ONLY:
		g_value_set_boolean(value, self->font_size_only);
		break;
	case TOP_TB_LOOK:
		g_value_set_boolean(value, self->use_toolbar_appearance);
		break;
	case TOP_FONT_SIZE:
		g_value_set_int(value, pango_font_description_get_size(desc));
		break;
	case TOP_CORNER_RAD:
		g_value_set_uint(value, self->corner_radius);
		break;
	case TOP_FONT:
		g_value_set_string(value, self->font);
		break;
	case TOP_BG_COLOR:
		str = gdk_rgba_to_string(&self->background_color);
		g_value_take_string(value, str);
		break;
	case TOP_FG_COLOR:
		str2 = gdk_rgba_to_string(&self->foreground_color);
		g_value_take_string(value, str2);
		break;
	case TOP_ICON_SIZE:
		g_value_set_uint(value, self->icon_size_hints);
		break;
	case TOP_BG_FILE:
		g_value_set_string(value, self->background_file);
		break;
	case TOP_GRAVITY:
		g_value_set_enum(value, (int)self->gravity);
		break;
	case TOP_ORIENTATION:
		g_value_set_enum(value, vala_panel_orient_from_gravity(self->gravity));
		break;
	case TOP_MONITOR:
		g_value_set_int(value, self->mon);
		break;
	case TOP_DOCK:
		g_value_set_boolean(value, self->dock);
		break;
	case TOP_STRUT:
		g_value_set_boolean(value, self->strut);
		break;
	case TOP_IS_DYNAMIC:
		g_value_set_boolean(value, self->is_dynamic);
		break;
	case TOP_AUTOHIDE:
		g_value_set_boolean(value, self->autohide);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
	g_clear_pointer(&desc, pango_font_description_free);
}

static void vala_panel_toplevel_set_property(GObject *object, guint property_id,
                                             const GValue *value, GParamSpec *pspec)
{
	ValaPanelToplevel *self       = VALA_PANEL_TOPLEVEL(object);
	bool geometry_update_required = false, appearance_update_required = false;
	uint icon_size_value       = 24;
	int mons                   = 1;
	PangoFontDescription *desc = pango_font_description_from_string(self->font);
	bool realized              = gtk_widget_get_realized(GTK_WIDGET(self));

	switch (property_id)
	{
	case TOP_UUID:
		g_clear_pointer(&self->uuid, g_free);
		self->uuid = g_value_dup_string(value);
		break;
	case TOP_HEIGHT:
		self->height             = g_value_get_int(value);
		geometry_update_required = true;
		break;
	case TOP_WIDTH:
		self->width              = g_value_get_int(value);
		geometry_update_required = true;
		break;
	case TOP_USE_FONT:
		self->use_font             = g_value_get_boolean(value);
		appearance_update_required = true;
		break;
	case TOP_USE_BG_COLOR:
		self->use_background_color = g_value_get_boolean(value);
		appearance_update_required = true;
		break;
	case TOP_USE_FG_COLOR:
		self->use_foreground_color = g_value_get_boolean(value);
		appearance_update_required = true;
		break;
	case TOP_USE_BG_FILE:
		self->use_background_file  = g_value_get_boolean(value);
		appearance_update_required = true;
		break;
	case TOP_FONT_SIZE_ONLY:
		self->font_size_only       = g_value_get_boolean(value);
		appearance_update_required = true;
		break;
	case TOP_TB_LOOK:
		self->use_toolbar_appearance = g_value_get_boolean(value);
		appearance_update_required   = true;
		break;
	case TOP_FONT_SIZE:
		pango_font_description_set_size(desc, g_value_get_int(value));
		appearance_update_required = true;
		break;
	case TOP_CORNER_RAD:
		self->corner_radius        = g_value_get_uint(value);
		appearance_update_required = true;
		break;
	case TOP_FONT:
		g_clear_pointer(&self->font, g_free);
		self->font                 = g_value_dup_string(value);
		appearance_update_required = true;
		break;
	case TOP_BG_COLOR:
		gdk_rgba_parse(&self->background_color, g_value_get_string(value));
		appearance_update_required = true;
		break;
	case TOP_FG_COLOR:
		gdk_rgba_parse(&self->foreground_color, g_value_get_string(value));
		appearance_update_required = true;
		break;
	case TOP_ICON_SIZE:
		icon_size_value = g_value_get_uint(value);
		if (icon_size_value >= (uint)XXXL)
			self->icon_size_hints = XXL;
		else if (icon_size_value >= (uint)XXL)
			self->icon_size_hints = XXL;
		else if (icon_size_value >= (uint)XL)
			self->icon_size_hints = XL;
		else if (icon_size_value >= (uint)L)
			self->icon_size_hints = L;
		else if (icon_size_value >= (uint)M)
			self->icon_size_hints = M;
		else if (icon_size_value >= (uint)S)
			self->icon_size_hints = S;
		else if (icon_size_value >= (uint)XS)
			self->icon_size_hints = XS;
		else
			self->icon_size_hints = XXS;
		appearance_update_required = true;
		break;
	case TOP_BG_FILE:
		g_clear_pointer(&self->background_file, g_free);
		self->background_file      = g_value_dup_string(value);
		appearance_update_required = true;
		break;
	case TOP_GRAVITY:
		self->gravity            = (ValaPanelGravity)g_value_get_enum(value);
		geometry_update_required = true;
		break;
	case TOP_MONITOR:
		if (gdk_display_get_default() != NULL)
			mons = gdk_display_get_n_monitors(gdk_display_get_default());
		g_assert(mons >= 1);
		if (-1 <= g_value_get_int(value))
			self->mon = g_value_get_int(value);
		geometry_update_required = true;
		break;
	case TOP_DOCK:
		self->dock               = g_value_get_boolean(value);
		geometry_update_required = true;
		break;
	case TOP_STRUT:
		self->strut                = g_value_get_boolean(value);
		geometry_update_required   = true;
		appearance_update_required = true;
		break;
	case TOP_IS_DYNAMIC:
		self->is_dynamic         = g_value_get_boolean(value);
		geometry_update_required = true;
		break;
	case TOP_AUTOHIDE:
		self->autohide = g_value_get_boolean(value);
		if (self->ah_rev != NULL)
		{
			if (self->autohide)
				ah_hide(self);
			else
				ah_show(self);
		}
		geometry_update_required = true;
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
	if (geometry_update_required && realized)
		vala_panel_toplevel_update_geometry(self);
	if (appearance_update_required)
		update_appearance(self);
	g_clear_pointer(&desc, pango_font_description_free);
}

void vala_panel_toplevel_init(ValaPanelToplevel *self)
{
	GtkWidget *w      = GTK_WIDGET(self);
	GdkVisual *visual = gdk_screen_get_rgba_visual(gtk_widget_get_screen(w));
	if (visual != NULL)
		gtk_widget_set_visual(w, visual);
	self->font            = g_strdup("");
	self->background_file = g_strdup("");
	self->context_menu    = NULL;
	self->ah_state        = AH_VISIBLE; // We starts as Visible to init autohide chain properly
}

void vala_panel_toplevel_class_init(ValaPanelToplevelClass *klass)
{
	GObjectClass *oclass                                    = G_OBJECT_CLASS(klass);
	GTK_WIDGET_CLASS(klass)->enter_notify_event             = enter_notify_event;
	GTK_WIDGET_CLASS(klass)->leave_notify_event             = leave_notify_event;
	GTK_WIDGET_CLASS(klass)->button_release_event           = button_release_event;
	GTK_WIDGET_CLASS(klass)->get_preferred_height           = get_preferred_height;
	GTK_WIDGET_CLASS(klass)->get_preferred_width            = get_preferred_width;
	GTK_WIDGET_CLASS(klass)->get_preferred_height_for_width = get_preferred_height_for_width;
	GTK_WIDGET_CLASS(klass)->get_preferred_width_for_height = get_preferred_width_for_height;
	GTK_WIDGET_CLASS(klass)->get_request_mode               = get_request_mode;
	GTK_WIDGET_CLASS(klass)->grab_notify                    = grab_notify;
	oclass->set_property                                    = vala_panel_toplevel_set_property;
	oclass->get_property                                    = vala_panel_toplevel_get_property;
	oclass->dispose                                         = vala_panel_toplevel_destroy;
	oclass->finalize                                        = vala_panel_toplevel_finalize;
	top_specs[TOP_UUID] =
	    g_param_spec_string(VALA_PANEL_KEY_UUID,
	                        VALA_PANEL_KEY_UUID,
	                        VALA_PANEL_KEY_UUID,
	                        NULL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE |
	                                      G_PARAM_CONSTRUCT_ONLY));
	top_specs[TOP_HEIGHT] =
	    g_param_spec_int(VALA_PANEL_KEY_HEIGHT,
	                     VALA_PANEL_KEY_HEIGHT,
	                     VALA_PANEL_KEY_HEIGHT,
	                     G_MININT,
	                     G_MAXINT,
	                     0,
	                     (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_WIDTH] =
	    g_param_spec_int(VALA_PANEL_KEY_WIDTH,
	                     VALA_PANEL_KEY_WIDTH,
	                     VALA_PANEL_KEY_WIDTH,
	                     G_MININT,
	                     G_MAXINT,
	                     0,
	                     (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));

	top_specs[TOP_USE_FONT] =
	    g_param_spec_boolean(VALA_PANEL_KEY_USE_FONT,
	                         VALA_PANEL_KEY_USE_FONT,
	                         VALA_PANEL_KEY_USE_FONT,
	                         FALSE,
	                         (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_USE_BG_COLOR] =
	    g_param_spec_boolean(VALA_PANEL_KEY_USE_BACKGROUND_COLOR,
	                         VALA_PANEL_KEY_USE_BACKGROUND_COLOR,
	                         VALA_PANEL_KEY_USE_BACKGROUND_COLOR,
	                         FALSE,
	                         (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_USE_FG_COLOR] =
	    g_param_spec_boolean(VALA_PANEL_KEY_USE_FOREGROUND_COLOR,
	                         VALA_PANEL_KEY_USE_FOREGROUND_COLOR,
	                         VALA_PANEL_KEY_USE_FOREGROUND_COLOR,
	                         FALSE,
	                         (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_USE_BG_FILE] =
	    g_param_spec_boolean(VALA_PANEL_KEY_USE_BACKGROUND_FILE,
	                         VALA_PANEL_KEY_USE_BACKGROUND_FILE,
	                         VALA_PANEL_KEY_USE_BACKGROUND_FILE,
	                         FALSE,
	                         (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_FONT_SIZE_ONLY] =
	    g_param_spec_boolean(VALA_PANEL_KEY_FONT_SIZE_ONLY,
	                         VALA_PANEL_KEY_FONT_SIZE_ONLY,
	                         VALA_PANEL_KEY_FONT_SIZE_ONLY,
	                         FALSE,
	                         (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_TB_LOOK] =
	    g_param_spec_boolean(VALA_PANEL_KEY_USE_TOOLBAR_APPEARANCE,
	                         VALA_PANEL_KEY_USE_TOOLBAR_APPEARANCE,
	                         VALA_PANEL_KEY_USE_TOOLBAR_APPEARANCE,
	                         FALSE,
	                         (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_FONT_SIZE] =
	    g_param_spec_uint(VALA_PANEL_KEY_FONT_SIZE,
	                      VALA_PANEL_KEY_FONT_SIZE,
	                      VALA_PANEL_KEY_FONT_SIZE,
	                      0,
	                      G_MAXUINT,
	                      0U,
	                      (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_CORNER_RAD] =
	    g_param_spec_uint(VALA_PANEL_KEY_CORNER_RADIUS,
	                      VALA_PANEL_KEY_CORNER_RADIUS,
	                      VALA_PANEL_KEY_CORNER_RADIUS,
	                      0,
	                      G_MAXUINT,
	                      0U,
	                      (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_FONT] =
	    g_param_spec_string(VALA_PANEL_KEY_FONT,
	                        VALA_PANEL_KEY_FONT,
	                        VALA_PANEL_KEY_FONT,
	                        NULL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_BG_COLOR] =
	    g_param_spec_string(VALA_PANEL_KEY_BACKGROUND_COLOR,
	                        VALA_PANEL_KEY_BACKGROUND_COLOR,
	                        VALA_PANEL_KEY_BACKGROUND_COLOR,
	                        NULL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_FG_COLOR] =
	    g_param_spec_string(VALA_PANEL_KEY_FOREGROUND_COLOR,
	                        VALA_PANEL_KEY_FOREGROUND_COLOR,
	                        VALA_PANEL_KEY_FOREGROUND_COLOR,
	                        NULL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_ICON_SIZE] =
	    g_param_spec_uint(VALA_PANEL_KEY_ICON_SIZE,
	                      VALA_PANEL_KEY_ICON_SIZE,
	                      VALA_PANEL_KEY_ICON_SIZE,
	                      0,
	                      G_MAXUINT,
	                      0U,
	                      (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_BG_FILE] =
	    g_param_spec_string(VALA_PANEL_KEY_BACKGROUND_FILE,
	                        VALA_PANEL_KEY_BACKGROUND_FILE,
	                        VALA_PANEL_KEY_BACKGROUND_FILE,
	                        NULL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_GRAVITY] =
	    g_param_spec_enum(VALA_PANEL_KEY_GRAVITY,
	                      VALA_PANEL_KEY_GRAVITY,
	                      VALA_PANEL_KEY_GRAVITY,
	                      VALA_PANEL_TYPE_PANEL_GRAVITY,
	                      0,
	                      (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE |
	                                    G_PARAM_CONSTRUCT));
	top_specs[TOP_ORIENTATION] =
	    g_param_spec_enum(VALA_PANEL_KEY_ORIENTATION,
	                      VALA_PANEL_KEY_ORIENTATION,
	                      VALA_PANEL_KEY_ORIENTATION,
	                      GTK_TYPE_ORIENTATION,
	                      0,
	                      (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READABLE));

	top_specs[TOP_MONITOR] =
	    g_param_spec_int(VALA_PANEL_KEY_MONITOR,
	                     VALA_PANEL_KEY_MONITOR,
	                     VALA_PANEL_KEY_MONITOR,
	                     G_MININT,
	                     G_MAXINT,
	                     0,
	                     (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE |
	                                   G_PARAM_CONSTRUCT));
	top_specs[TOP_DOCK] =
	    g_param_spec_boolean(VALA_PANEL_KEY_DOCK,
	                         VALA_PANEL_KEY_DOCK,
	                         VALA_PANEL_KEY_DOCK,
	                         FALSE,
	                         (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_STRUT] =
	    g_param_spec_boolean(VALA_PANEL_KEY_STRUT,
	                         VALA_PANEL_KEY_STRUT,
	                         VALA_PANEL_KEY_STRUT,
	                         FALSE,
	                         (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_IS_DYNAMIC] =
	    g_param_spec_boolean(VALA_PANEL_KEY_DYNAMIC,
	                         VALA_PANEL_KEY_DYNAMIC,
	                         VALA_PANEL_KEY_DYNAMIC,
	                         FALSE,
	                         (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	top_specs[TOP_AUTOHIDE] =
	    g_param_spec_boolean(VALA_PANEL_KEY_AUTOHIDE,
	                         VALA_PANEL_KEY_AUTOHIDE,
	                         VALA_PANEL_KEY_AUTOHIDE,
	                         FALSE,
	                         (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));

	g_object_class_install_properties(oclass, TOP_LAST, top_specs);
}

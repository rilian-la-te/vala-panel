#include "panel-layout.h"
#include "lib/applets-new/applet-api.h"
#include "lib/settings-manager.h"

struct _ValaPanelAppletLayout
{
	GtkBox __parent__;
	GtkWidget *center;
	int width;
	int height;
	bool is_dynamic_height;
	bool is_dynamic_width;
};

G_DEFINE_TYPE(ValaPanelAppletLayout, vala_panel_applet_layout, GTK_TYPE_BOX)

#define vala_panel_applet_layout_place_applet_widget(self, applet, pack, pos)                      \
	{                                                                                          \
		if (pack == PACK_START)                                                            \
		{                                                                                  \
			gtk_box_pack_start(GTK_BOX(self), applet, false, true, 0);                 \
			gtk_box_reorder_child(GTK_BOX(self), applet, pos);                         \
		}                                                                                  \
		else if (pack == PACK_END)                                                         \
		{                                                                                  \
			gtk_box_pack_end(GTK_BOX(self), applet, false, true, 0);                   \
			gtk_box_reorder_child(GTK_BOX(self), applet, pos);                         \
		}                                                                                  \
		else if (pack == PACK_CENTER)                                                      \
		{                                                                                  \
			gtk_box_pack_start(GTK_BOX(self->center), applet, false, true, 0);         \
			gtk_box_reorder_child(GTK_BOX(self->center), applet, pos);                 \
		}                                                                                  \
	}

static void vala_panel_applet_layout_get_preferred_height(GtkWidget *widget, gint *minimum_height,
                                                          gint *natural_height)
{
	ValaPanelAppletLayout *self = VALA_PANEL_APPLET_LAYOUT(widget);
	GtkOrientation orient       = gtk_orientable_get_orientation(GTK_ORIENTABLE(self));
	if (self->is_dynamic_height)
		GTK_WIDGET_CLASS(vala_panel_applet_layout_parent_class)
		    ->get_preferred_height(widget, minimum_height, natural_height);
	else
		*minimum_height = *natural_height =
		    (orient == GTK_ORIENTATION_VERTICAL) ? self->width : self->height;
}

static void vala_panel_applet_layout_get_preferred_width(GtkWidget *widget, gint *minimum_height,
                                                         gint *natural_height)
{
	ValaPanelAppletLayout *self = VALA_PANEL_APPLET_LAYOUT(widget);
	GtkOrientation orient       = gtk_orientable_get_orientation(GTK_ORIENTABLE(self));
	if (self->is_dynamic_width)
		GTK_WIDGET_CLASS(vala_panel_applet_layout_parent_class)
		    ->get_preferred_width(widget, minimum_height, natural_height);
	else
		*minimum_height = *natural_height =
		    (orient == GTK_ORIENTATION_HORIZONTAL) ? self->width : self->height;
}

static void vala_panel_applet_layout_init(ValaPanelAppletLayout *self)
{
	self->center = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	g_object_bind_property(self, "orientation", self->center, "orientation", (GBindingFlags)0);
	gtk_box_set_center_widget(GTK_BOX(self), self->center);
}

static void vala_panel_applet_layout_class_init(ValaPanelAppletLayoutClass *klass)
{
	GTK_WIDGET_CLASS(klass)->get_preferred_height =
	    vala_panel_applet_layout_get_preferred_height;
	GTK_WIDGET_CLASS(klass)->get_preferred_width = vala_panel_applet_layout_get_preferred_width;
}

static void update_applet_positions(ValaPanelAppletLayout *self)
{
	g_autoptr(GList) c_c = gtk_container_get_children(GTK_CONTAINER(self));
	for (GList *l = c_c; l != NULL; l = g_list_next(l))
	{
		int idx = vala_panel_applet_get_position_metadata(G_OBJECT(l->data));
		gtk_box_reorder_child(GTK_BOX(self), GTK_WIDGET(l->data), idx);
	}
}

void vala_panel_applet_layout_update_views(ValaPanelAppletLayout *self)
{
	g_autoptr(GList) ch = gtk_container_get_children(GTK_CONTAINER(self->center));
	if (g_list_length(ch) <= 0)
		gtk_widget_hide(GTK_WIDGET(self->center));
	else
		gtk_widget_show(GTK_WIDGET(self->center));
}

void vala_panel_applet_layout_place_applet(ValaPanelAppletLayout *self, ValaPanelPlatform *gmgr,
                                           GSettings *toplevel_settings,
                                           ValaPanelAppletManager *mgr, const char *applet_type,
                                           PanelAppletPackType pack, int pos)
{
	g_autofree char *uid  = vala_panel_generate_new_hash();
	g_autofree char *path = NULL;
	g_object_get(toplevel_settings, "path", &path, NULL);
	g_autofree char *cpath = g_strconcat(path, uid, "/", NULL);
	GtkWidget *applet      = GTK_WIDGET(
	    vala_panel_applet_manager_get_applet_widget_for_type(mgr, path, applet_type, uid));
	GSettings *csettings =
	    vala_panel_platform_get_settings_for_scheme(gmgr, DEFAULT_PLUGIN_SETTINGS_ID, cpath);
	g_settings_set_int(csettings, VALA_PANEL_KEY_POSITION, pos);
	g_settings_set_string(csettings, VALA_PANEL_KEY_NAME, applet_type);
	g_settings_set_enum(csettings, VALA_PANEL_KEY_PACK, pack);
	vala_panel_applet_set_position_metadata(applet, pos);
	vala_panel_applet_layout_place_applet_widget(self, applet, pack, pos);
	vala_panel_applet_layout_update_views(self);
}

void vala_panel_applet_layout_load_applets(ValaPanelAppletLayout *self, ValaPanelAppletManager *mgr,
                                           GSettings *settings)
{
	g_auto(GStrv) children = g_settings_list_children(settings);
	for (int i = 0; children[i] != NULL; i++)
	{
		g_autoptr(GSettings) csettings = g_settings_get_child(settings, children[i]);
		g_autofree char *applet_type =
		    g_settings_get_string(csettings, VALA_PANEL_KEY_NAME);
		g_autofree char *path = NULL;
		g_object_get(settings, "path", &path, NULL);
		GtkWidget *applet =
		    GTK_WIDGET(vala_panel_applet_manager_get_applet_widget_for_type(mgr,
		                                                                    path,
		                                                                    applet_type,
		                                                                    children[i]));
		PanelAppletPackType pack =
		    (PanelAppletPackType)g_settings_get_enum(csettings, VALA_PANEL_KEY_PACK);
		int pos = g_settings_get_int(csettings, VALA_PANEL_KEY_POSITION);
		vala_panel_applet_set_position_metadata(applet, pos);
		vala_panel_applet_layout_place_applet_widget(self, applet, pack, pos);
	}
	update_applet_positions(self);
	vala_panel_applet_layout_update_views(self);
}

ValaPanelAppletLayout *vala_panel_applet_layout_new(GtkOrientation orient, int spacing)
{
	return VALA_PANEL_APPLET_LAYOUT(g_object_new(vala_panel_applet_layout_get_type(),
	                                             "orientation",
	                                             orient,
	                                             "spacing",
	                                             spacing,
	                                             "baseline-position",
	                                             GTK_BASELINE_POSITION_CENTER,
	                                             "border-width",
	                                             0,
	                                             "hexpand",
	                                             true,
	                                             "vexpand",
	                                             true,
	                                             NULL));
}

#include "applet-widget.h"
#include "definitions.h"
#include "panel-layout.h"
#include "toplevel.h"
#include "util-gtk.h"

#include "private.h"

typedef struct
{
	GtkDialog *dialog;
	GtkWidget *background;
	ValaPanelToplevel *toplevel;
	GSettings *settings;
	char *uuid;
	GSimpleActionGroup *grp;

} ValaPanelAppletPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(ValaPanelApplet, vala_panel_applet, GTK_TYPE_BIN)

static void activate_configure(GSimpleAction *act, GVariant *param, gpointer self);
static void activate_remote(GSimpleAction *act, GVariant *param, gpointer self);
static void activate_remove(GSimpleAction *act, GVariant *param, gpointer self);
static void activate_about(GSimpleAction *act, GVariant *param, gpointer self);

static const GActionEntry entries[] = {
	{ VALA_PANEL_APPLET_ACTION_REMOTE, activate_remote, "s", NULL, NULL, { 0 } },
	{ VALA_PANEL_APPLET_ACTION_CONFIGURE, activate_configure, NULL, NULL, NULL, { 0 } },
	{ VALA_PANEL_APPLET_ACTION_ABOUT, activate_about, NULL, NULL, NULL, { 0 } },
	{ VALA_PANEL_APPLET_ACTION_REMOVE, activate_remove, NULL, NULL, NULL, { 0 } }
};

enum
{
	VALA_PANEL_APPLET_DUMMY_PROPERTY,
	VALA_PANEL_APPLET_BACKGROUND_WIDGET,
	VALA_PANEL_APPLET_TOPLEVEL,
	VALA_PANEL_APPLET_SETTINGS,
	VALA_PANEL_APPLET_UUID,
	VALA_PANEL_APPLET_GRP,
	VALA_PANEL_APPLET_ALL
};

static GParamSpec *applet_specs[VALA_PANEL_APPLET_ALL];

static bool release_event_helper(GtkWidget *_sender, GdkEventButton *b, gpointer obj)
{
	return vp_toplevel_release_event_helper(_sender, b, obj);
}

gpointer vala_panel_applet_construct(GType ex, ValaPanelToplevel *top, GSettings *settings,
                                     const char *uuid)
{
	return g_object_new(ex, "toplevel", top, "settings", settings, "uuid", uuid, NULL);
}

static GObject *vala_panel_applet_constructor(GType type, guint n_construct_properties,
                                              GObjectConstructParam *construct_properties)
{
	GObjectClass *parent_class = G_OBJECT_CLASS(vala_panel_applet_parent_class);
	GObject *obj =
	    parent_class->constructor(type, n_construct_properties, construct_properties);
	ValaPanelApplet *self     = VALA_PANEL_APPLET(obj);
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	g_signal_connect(self,
	                 "button-release-event",
	                 G_CALLBACK(release_event_helper),
	                 p->toplevel);
	return G_OBJECT(self);
}

void vala_panel_applet_init_background(ValaPanelApplet *self)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	GdkRGBA color;
	gdk_rgba_parse(&color, "rgba(0,0,0,0)");
	g_autofree char *css = css_generate_background(NULL, &color);
	css_apply_with_class(p->background, css, "-vala-panel-background", false);
}

GtkWidget *vala_panel_applet_get_settings_ui(ValaPanelApplet *self)
{
	GtkWidget *ui = VALA_PANEL_APPLET_GET_CLASS(self)->get_settings_ui(self);
	gtk_widget_show(ui);
	return ui;
}

bool vala_panel_applet_remote_command(ValaPanelApplet *self, const char *command)
{
	if (VALA_PANEL_APPLET_GET_CLASS(self)->remote_command)
		return VALA_PANEL_APPLET_GET_CLASS(self)->remote_command(self, command);
	return false;
}
bool vala_panel_applet_is_configurable(ValaPanelApplet *self)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	return g_action_group_get_action_enabled(G_ACTION_GROUP(p->grp), "configure");
}
static void activate_configure(G_GNUC_UNUSED GSimpleAction *act, G_GNUC_UNUSED GVariant *param,
                               gpointer data)
{
	ValaPanelApplet *self = VALA_PANEL_APPLET(data);
	vala_panel_toplevel_configure_applet(vala_panel_applet_get_toplevel(self),
	                                     vala_panel_applet_get_uuid(self));
}
static void activate_remote(G_GNUC_UNUSED GSimpleAction *act, G_GNUC_UNUSED GVariant *param,
                            gpointer obj)
{
	const char *command = g_variant_get_string(param, NULL);
	vala_panel_applet_remote_command(VALA_PANEL_APPLET(obj), command);
}
static void activate_about(G_GNUC_UNUSED GSimpleAction *act, G_GNUC_UNUSED GVariant *param,
                           gpointer obj)
{
	ValaPanelApplet *self = VALA_PANEL_APPLET(obj);
	ValaPanelAppletInfo *pl_info =
	    vp_applet_manager_get_applet_info(vp_layout_get_manager(),
	                                      self,
	                                      vp_toplevel_get_core_settings());
	vala_panel_applet_info_show_about_dialog(pl_info);
}
static void activate_remove(G_GNUC_UNUSED GSimpleAction *act, G_GNUC_UNUSED GVariant *param,
                            gpointer obj)
{
	ValaPanelApplet *self     = VALA_PANEL_APPLET(obj);
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	/* If the configuration dialog is open, there will certainly be a crash if the
	 * user manipulates the Configured Plugins list, after we remove this entry.
	 * Close the configuration dialog if it is open. */
	vp_toplevel_destroy_pref_dialog(p->toplevel);
	vp_layout_remove_applet(vala_panel_toplevel_get_layout(p->toplevel), self);
}
static GtkWidget *vala_panel_applet_get_config_dialog(G_GNUC_UNUSED ValaPanelApplet *self)
{
	return NULL;
}

static void vala_panel_applet_measure(ValaPanelApplet *self, GtkOrientation orient, int for_size,
                                      int *min, int *nat, int *base_min, int *base_nat)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	GtkOrientation panel_ori;
	g_object_get(p->toplevel, VP_KEY_ORIENTATION, &panel_ori, NULL);
	int height, icon_size;
	g_object_get(p->toplevel, VP_KEY_HEIGHT, &height, VP_KEY_ICON_SIZE, &icon_size, NULL);
	if (panel_ori != orient)
	{
		*min = icon_size;
		*nat = height;
	}
	else
	{
		if (orient == GTK_ORIENTATION_HORIZONTAL)
			GTK_WIDGET_CLASS(vala_panel_applet_parent_class)
			    ->get_preferred_width_for_height(GTK_WIDGET(self), for_size, min, nat);
		else
			GTK_WIDGET_CLASS(vala_panel_applet_parent_class)
			    ->get_preferred_height_for_width(GTK_WIDGET(self), for_size, min, nat);
	}
	*base_min = *base_nat = -1;
}

static void vala_panel_applet_get_preferred_height_for_width(GtkWidget *obj, int width, int *min,
                                                             int *nat)
{
	ValaPanelApplet *self = VALA_PANEL_APPLET(obj);
	int x, y;
	vala_panel_applet_measure(self, GTK_ORIENTATION_VERTICAL, width, min, nat, &x, &y);
}
static void vala_panel_applet_get_preferred_width_for_height(GtkWidget *obj, int height, int *min,
                                                             int *nat)
{
	ValaPanelApplet *self = VALA_PANEL_APPLET(obj);
	int x, y;
	vala_panel_applet_measure(self, GTK_ORIENTATION_HORIZONTAL, height, min, nat, &x, &y);
}
static GtkSizeRequestMode vala_panel_applet_get_request_mode(GtkWidget *obj)
{
	ValaPanelApplet *self     = VALA_PANEL_APPLET(obj);
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(VALA_PANEL_APPLET(self));
	GtkOrientation pos;
	g_object_get(p->toplevel, VP_KEY_ORIENTATION, &pos, NULL);
	return (pos == GTK_ORIENTATION_HORIZONTAL) ? GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT
	                                           : GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}
static void vala_panel_applet_get_preferred_width(GtkWidget *obj, int *min, int *nat)
{
	ValaPanelApplet *self = VALA_PANEL_APPLET(obj);
	int x, y;
	vala_panel_applet_measure(self, GTK_ORIENTATION_HORIZONTAL, 0, min, nat, &x, &y);
}
static void vala_panel_applet_get_preferred_height(GtkWidget *obj, int *min, int *nat)
{
	ValaPanelApplet *self = VALA_PANEL_APPLET(obj);
	int x, y;
	vala_panel_applet_measure(self, GTK_ORIENTATION_VERTICAL, 0, min, nat, &x, &y);
}

static void vala_panel_applet_parent_set(GtkWidget *w, GtkWidget *prev_parent)
{
	ValaPanelApplet *self     = VALA_PANEL_APPLET(w);
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(VALA_PANEL_APPLET(self));
	if (prev_parent == NULL)
	{
		if (p->background == NULL)
			p->background = GTK_WIDGET(self);
		vala_panel_applet_init_background(self);
	}
}

void vala_panel_applet_update_context_menu(ValaPanelApplet *self, GMenu *parent_menu)
{
	VALA_PANEL_APPLET_GET_CLASS(self)->update_context_menu(self, parent_menu);
}

static void vala_panel_applet_update_context_menu_private(G_GNUC_UNUSED ValaPanelApplet *self,
                                                          G_GNUC_UNUSED GMenu *parent_menu)
{
}

static void vala_panel_applet_init(ValaPanelApplet *self)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	p->grp                    = g_simple_action_group_new();
	p->settings               = NULL;
	g_action_map_add_action_entries(G_ACTION_MAP(p->grp), entries, G_N_ELEMENTS(entries), self);
	GSimpleAction *cnf = G_SIMPLE_ACTION(
	    g_action_map_lookup_action(G_ACTION_MAP(p->grp), VALA_PANEL_APPLET_ACTION_CONFIGURE));
	g_simple_action_set_enabled(cnf, false);
	cnf = G_SIMPLE_ACTION(
	    g_action_map_lookup_action(G_ACTION_MAP(p->grp), VALA_PANEL_APPLET_ACTION_REMOTE));
	g_simple_action_set_enabled(cnf, false);
	gtk_widget_set_has_window(GTK_WIDGET(self), false);
	gtk_widget_insert_action_group(GTK_WIDGET(self), "applet", G_ACTION_GROUP(p->grp));
}
GtkWidget *vala_panel_applet_get_background_widget(ValaPanelApplet *self)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	return p->background;
}

void vala_panel_applet_set_background_widget(ValaPanelApplet *self, GtkWidget *w)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	p->background             = w;
}

ValaPanelToplevel *vala_panel_applet_get_toplevel(ValaPanelApplet *self)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	return p->toplevel;
}

GSettings *vala_panel_applet_get_settings(ValaPanelApplet *self)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	return p->settings;
}
const char *vala_panel_applet_get_uuid(ValaPanelApplet *self)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	return p->uuid;
}

GActionMap *vala_panel_applet_get_action_group(ValaPanelApplet *self)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	return G_ACTION_MAP(p->grp);
}
static void vala_panel_applet_get_property(GObject *object, guint property_id, GValue *value,
                                           GParamSpec *pspec)
{
	ValaPanelApplet *self     = VALA_PANEL_APPLET(object);
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(VALA_PANEL_APPLET(self));
	switch (property_id)
	{
	case VALA_PANEL_APPLET_BACKGROUND_WIDGET:
		g_value_set_object(value, p->background);
		break;
	case VALA_PANEL_APPLET_TOPLEVEL:
		g_value_set_object(value, p->toplevel);
		break;
	case VALA_PANEL_APPLET_SETTINGS:
		g_value_set_object(value, p->settings);
		break;
	case VALA_PANEL_APPLET_UUID:
		g_value_set_string(value, p->uuid);
		break;
	case VALA_PANEL_APPLET_GRP:
		g_value_set_object(value, p->grp);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void vala_panel_applet_set_property(GObject *object, guint property_id, const GValue *value,
                                           GParamSpec *pspec)
{
	ValaPanelApplet *self;
	self                      = VALA_PANEL_APPLET(object);
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(VALA_PANEL_APPLET(self));
	switch (property_id)
	{
	case VALA_PANEL_APPLET_BACKGROUND_WIDGET:
		p->background = GTK_WIDGET(g_value_get_object(value));
		g_object_notify_by_pspec(object, pspec);
		break;
	case VALA_PANEL_APPLET_TOPLEVEL:
		p->toplevel = VALA_PANEL_TOPLEVEL(g_value_get_object(value));
		g_object_notify_by_pspec(object, pspec);
		break;
	case VALA_PANEL_APPLET_SETTINGS:
		p->settings = G_SETTINGS(g_value_get_object(value));
		g_object_notify_by_pspec(object, pspec);
		break;
	case VALA_PANEL_APPLET_UUID:
		g_free0(p->uuid);
		p->uuid = g_value_dup_string(value);
		g_object_notify_by_pspec(object, pspec);
		break;
	case VALA_PANEL_APPLET_GRP:
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void vala_panel_applet_finalize(GObject *obj)
{
	ValaPanelApplet *self     = VALA_PANEL_APPLET(obj);
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(VALA_PANEL_APPLET(self));
	gtk_widget_insert_action_group(GTK_WIDGET(obj), "applet", NULL);
	g_clear_object(&p->grp);
	g_free0(p->uuid);
	G_OBJECT_CLASS(vala_panel_applet_parent_class)->finalize(obj);
}
static void vala_panel_applet_class_init(ValaPanelAppletClass *klass)
{
	VALA_PANEL_APPLET_CLASS(klass)->update_context_menu =
	    vala_panel_applet_update_context_menu_private;
	VALA_PANEL_APPLET_CLASS(klass)->remote_command  = NULL;
	VALA_PANEL_APPLET_CLASS(klass)->get_settings_ui = vala_panel_applet_get_config_dialog;
	GTK_WIDGET_CLASS(klass)->parent_set             = vala_panel_applet_parent_set;
	GTK_WIDGET_CLASS(klass)->get_preferred_height_for_width =
	    vala_panel_applet_get_preferred_height_for_width;
	GTK_WIDGET_CLASS(klass)->get_preferred_width_for_height =
	    vala_panel_applet_get_preferred_width_for_height;
	GTK_WIDGET_CLASS(klass)->get_request_mode     = vala_panel_applet_get_request_mode;
	GTK_WIDGET_CLASS(klass)->get_preferred_width  = vala_panel_applet_get_preferred_width;
	GTK_WIDGET_CLASS(klass)->get_preferred_height = vala_panel_applet_get_preferred_height;
	G_OBJECT_CLASS(klass)->constructor            = vala_panel_applet_constructor;
	G_OBJECT_CLASS(klass)->get_property           = vala_panel_applet_get_property;
	G_OBJECT_CLASS(klass)->set_property           = vala_panel_applet_set_property;
	G_OBJECT_CLASS(klass)->finalize               = vala_panel_applet_finalize;
	applet_specs[VALA_PANEL_APPLET_BACKGROUND_WIDGET] =
	    g_param_spec_object(VP_KEY_BACKGROUND_WIDGET,
	                        VP_KEY_BACKGROUND_WIDGET,
	                        VP_KEY_BACKGROUND_WIDGET,
	                        gtk_widget_get_type(),
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));
	applet_specs[VALA_PANEL_APPLET_TOPLEVEL] =
	    g_param_spec_object(VP_KEY_TOPLEVEL,
	                        VP_KEY_TOPLEVEL,
	                        VP_KEY_TOPLEVEL,
	                        VALA_PANEL_TYPE_TOPLEVEL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE |
	                                      G_PARAM_CONSTRUCT_ONLY));
	applet_specs[VALA_PANEL_APPLET_UUID] =
	    g_param_spec_string(VP_KEY_UUID,
	                        VP_KEY_UUID,
	                        VP_KEY_UUID,
	                        NULL,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE |
	                                      G_PARAM_CONSTRUCT_ONLY));
	applet_specs[VALA_PANEL_APPLET_SETTINGS] =
	    g_param_spec_object(VP_KEY_SETTINGS,
	                        VP_KEY_SETTINGS,
	                        VP_KEY_SETTINGS,
	                        G_TYPE_SETTINGS,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE |
	                                      G_PARAM_CONSTRUCT_ONLY));
	applet_specs[VALA_PANEL_APPLET_GRP] =
	    g_param_spec_object(VP_KEY_ACTION_GROUP,
	                        VP_KEY_ACTION_GROUP,
	                        VP_KEY_ACTION_GROUP,
	                        G_TYPE_SIMPLE_ACTION_GROUP,
	                        (GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READABLE));
	g_object_class_install_properties(G_OBJECT_CLASS(klass),
	                                  VALA_PANEL_APPLET_ALL,
	                                  applet_specs);
}

#include "applet-info.h"
#include "lib/definitions.h"

#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

#include <gtk/gtk.h>

struct _ValaPanelAppletInfo
{
	ValaPanelAppletWidget *applet;
	GSettings *settings;
	gchar *icon;
	gchar *applet_type;
	gchar *name;
	gchar *description;
	gchar *uuid;
	gchar *profile;
	GtkAlign alignment;
	GtkOrientation orientation;
	gint position;
	bool expand;
};

enum
{
	VALA_PANEL_APPLET_INFO_DUMMY_PROPERTY,
	VALA_PANEL_APPLET_INFO_APPLET,
	VALA_PANEL_APPLET_INFO_SETTINGS,
	VALA_PANEL_APPLET_INFO_ICON,
	VALA_PANEL_APPLET_INFO_APPLET_TYPE,
	VALA_PANEL_APPLET_INFO_NAME,
	VALA_PANEL_APPLET_INFO_DESCRIPTION,
	VALA_PANEL_APPLET_INFO_UUID,
	VALA_PANEL_APPLET_INFO_FILENAME,
	VALA_PANEL_APPLET_INFO_ALIGNMENT,
	VALA_PANEL_APPLET_INFO_ORIENTATION,
	VALA_PANEL_APPLET_INFO_POSITION,
	VALA_PANEL_APPLET_INFO_EXPAND
};

G_DEFINE_TYPE(ValaPanelAppletInfo, vala_panel_applet_info, G_TYPE_OBJECT)

// static ValaPanelAppletInfo*
// vala_panel_applet_info_new_libpeas(PeasPluginInfo* plugin_info, const char* uuid,
//                           const char* filename, ValaPanelAppletWidget* applet)
//{
//  return g_object_new(vala_panel_applet_info_get_type(), "icon",
//                      peas_plugin_info_get_icon_name(plugin_info), "name",
//                      peas_plugin_info_get_name(plugin_info), "description",
//                      peas_plugin_info_get_description(plugin_info), "uuid",
//                      uuid, "applet", applet, "filename", filename, NULL);
//}

void vala_panel_applet_info_init(ValaPanelAppletInfo *self)
{
}

static void vala_panel_applet_info_bind_settings(ValaPanelAppletInfo *self)
{
	g_settings_bind(self->settings,
	                "position",
	                (GObject *)self,
	                "position",
	                (G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET) | G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(self->settings,
	                "alignment",
	                (GObject *)self,
	                "alignment",
	                (G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET) | G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(self->settings,
	                "expand",
	                (GObject *)self,
	                "expand",
	                (G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET) | G_SETTINGS_BIND_DEFAULT);
}

static void vala_panel_applet_info_unbind_settings(ValaPanelAppletInfo *self)
{
	g_settings_unbind(self->settings, "position");
	g_settings_unbind(self->settings, "alignment");
	g_settings_unbind(self->settings, "expand");
}

static void vala_panel_applet_info_constructed(ValaPanelAppletInfo *self)
{
	// GSettings must not be accessed directly!!! Need a Manager-based access.
	//	g_autofree gchar *path    = g_build_path("/", DEFAULT_PLUGIN_PATH, self->uuid,
	// NULL);
	//	g_autofree char *filename = _user_config_file_name(path, self->profile, self->uuid);
	//	g_autoptr(GSettingsBackend) bck =
	//	    g_keyfile_settings_backend_new(filename, path, DEFAULT_PLUGIN_GROUP);
	//	self->settings = g_settings_new_with_backend(DEFAULT_PLUGIN_SETTINGS_ID, bck);
	//	gchar *str     = g_strdup(g_strstrip(g_strdelimit(self->name, " '", '_')));
	//	g_ascii_inplace_tolower(str);
	//	self->applet_type = str;
	G_OBJECT_CLASS(vala_panel_applet_info_parent_class)->constructed(G_OBJECT(self));
}

static void vala_panel_applet_info_get_property(GObject *object, guint property_id, GValue *value,
                                                GParamSpec *pspec)
{
	ValaPanelAppletInfo *self;
	self = G_TYPE_CHECK_INSTANCE_CAST(object,
	                                  vala_panel_applet_info_get_type(),
	                                  ValaPanelAppletInfo);
	switch (property_id)
	{
	case VALA_PANEL_APPLET_INFO_APPLET:
		g_value_set_object(value, self->applet);
		break;
	case VALA_PANEL_APPLET_INFO_SETTINGS:
		g_value_set_object(value, self->settings);
		break;
	case VALA_PANEL_APPLET_INFO_ICON:
		g_value_set_string(value, self->icon);
		break;
	case VALA_PANEL_APPLET_INFO_APPLET_TYPE:
		g_value_set_string(value, self->applet_type);
		break;
	case VALA_PANEL_APPLET_INFO_NAME:
		g_value_set_string(value, self->name);
		break;
	case VALA_PANEL_APPLET_INFO_DESCRIPTION:
		g_value_set_string(value, self->description);
		break;
	case VALA_PANEL_APPLET_INFO_UUID:
		g_value_set_string(value, self->uuid);
		break;
	case VALA_PANEL_APPLET_INFO_FILENAME:
		g_value_set_string(value, self->profile);
		break;
	case VALA_PANEL_APPLET_INFO_ALIGNMENT:
		g_value_set_enum(value, self->alignment);
		break;
	case VALA_PANEL_APPLET_INFO_ORIENTATION:
		g_value_set_enum(value, self->orientation);
		break;
	case VALA_PANEL_APPLET_INFO_POSITION:
		g_value_set_int(value, self->position);
		break;
	case VALA_PANEL_APPLET_INFO_EXPAND:
		g_value_set_boolean(value, self->expand);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void vala_panel_applet_info_set_property(GObject *object, guint property_id,
                                                const GValue *value, GParamSpec *pspec)
{
	ValaPanelAppletInfo *self;
	self = G_TYPE_CHECK_INSTANCE_CAST(object,
	                                  vala_panel_applet_info_get_type(),
	                                  ValaPanelAppletInfo);
	switch (property_id)
	{
	case VALA_PANEL_APPLET_INFO_APPLET:
		self->applet = g_value_get_object(value);
		break;
	case VALA_PANEL_APPLET_INFO_ICON:
		g_value_replace_string(self->icon, value);
		break;
	case VALA_PANEL_APPLET_INFO_APPLET_TYPE:
		g_value_replace_string(self->applet_type, value);
		break;
	case VALA_PANEL_APPLET_INFO_NAME:
		g_value_replace_string(self->name, value);
		break;
	case VALA_PANEL_APPLET_INFO_DESCRIPTION:
		g_value_replace_string(self->description, value);
		break;
	case VALA_PANEL_APPLET_INFO_UUID:
		g_value_replace_string(self->uuid, value);
		break;
	case VALA_PANEL_APPLET_INFO_FILENAME:
		g_value_replace_string(self->profile, value);
		break;
	case VALA_PANEL_APPLET_INFO_ALIGNMENT:
		self->alignment = g_value_get_enum(value);
		break;
	case VALA_PANEL_APPLET_INFO_ORIENTATION:
		self->orientation = g_value_get_enum(value);
		break;
	case VALA_PANEL_APPLET_INFO_POSITION:
		self->position = g_value_get_int(value);
		break;
	case VALA_PANEL_APPLET_INFO_EXPAND:
		self->expand = g_value_get_boolean(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void vala_panel_applet_info_finalize(GObject *obj)
{
	ValaPanelAppletInfo *self;
	self =
	    G_TYPE_CHECK_INSTANCE_CAST(obj, vala_panel_applet_info_get_type(), ValaPanelAppletInfo);
	g_object_unref(self->settings);
	g_free0(self->icon);
	g_free0(self->applet_type);
	g_free0(self->name);
	g_free0(self->description);
	g_free0(self->uuid);
	g_free0(self->profile);
	G_OBJECT_CLASS(vala_panel_applet_info_parent_class)->finalize(obj);
}

void vala_panel_applet_info_class_init(ValaPanelAppletInfoClass *klass)
{
	vala_panel_applet_info_parent_class = g_type_class_peek_parent(klass);
	G_OBJECT_CLASS(klass)->constructed  = vala_panel_applet_info_constructed;
	G_OBJECT_CLASS(klass)->get_property = vala_panel_applet_info_get_property;
	G_OBJECT_CLASS(klass)->set_property = vala_panel_applet_info_set_property;
	G_OBJECT_CLASS(klass)->finalize     = vala_panel_applet_info_finalize;
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APPLET_INFO_APPLET,
	                                g_param_spec_object("applet",
	                                                    "applet",
	                                                    "applet",
	                                                    vala_panel_applet_widget_get_type(),
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE |
	                                                        G_PARAM_WRITABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APPLET_INFO_SETTINGS,
	                                g_param_spec_object("settings",
	                                                    "settings",
	                                                    "settings",
	                                                    G_TYPE_SETTINGS,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APPLET_INFO_ICON,
	                                g_param_spec_string("icon",
	                                                    "icon",
	                                                    "icon",
	                                                    NULL,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APPLET_INFO_APPLET_TYPE,
	                                g_param_spec_string("applet-type",
	                                                    "applet-type",
	                                                    "applet-type",
	                                                    NULL,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APPLET_INFO_NAME,
	                                g_param_spec_string("name",
	                                                    "name",
	                                                    "name",
	                                                    NULL,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APPLET_INFO_DESCRIPTION,
	                                g_param_spec_string("description",
	                                                    "description",
	                                                    "description",
	                                                    NULL,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APPLET_INFO_UUID,
	                                g_param_spec_string("uuid",
	                                                    "uuid",
	                                                    "uuid",
	                                                    NULL,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE |
	                                                        G_PARAM_WRITABLE |
	                                                        G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APPLET_INFO_FILENAME,
	                                g_param_spec_string("filename",
	                                                    "filename",
	                                                    "filename",
	                                                    NULL,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE |
	                                                        G_PARAM_WRITABLE |
	                                                        G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APPLET_INFO_ALIGNMENT,
	                                g_param_spec_enum("alignment",
	                                                  "alignment",
	                                                  "alignment",
	                                                  GTK_TYPE_ALIGN,
	                                                  0,
	                                                  G_PARAM_STATIC_NAME |
	                                                      G_PARAM_STATIC_NICK |
	                                                      G_PARAM_STATIC_BLURB |
	                                                      G_PARAM_READABLE | G_PARAM_WRITABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APPLET_INFO_ORIENTATION,
	                                g_param_spec_enum("orientation",
	                                                  "orientation",
	                                                  "orientation",
	                                                  GTK_TYPE_ORIENTATION,
	                                                  0,
	                                                  G_PARAM_STATIC_NAME |
	                                                      G_PARAM_STATIC_NICK |
	                                                      G_PARAM_STATIC_BLURB |
	                                                      G_PARAM_READABLE | G_PARAM_WRITABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APPLET_INFO_POSITION,
	                                g_param_spec_int("position",
	                                                 "position",
	                                                 "position",
	                                                 G_MININT,
	                                                 G_MAXINT,
	                                                 0,
	                                                 G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
	                                                     G_PARAM_STATIC_BLURB |
	                                                     G_PARAM_READABLE | G_PARAM_WRITABLE));
	g_object_class_install_property(G_OBJECT_CLASS(klass),
	                                VALA_PANEL_APPLET_INFO_EXPAND,
	                                g_param_spec_boolean("expand",
	                                                     "expand",
	                                                     "expand",
	                                                     false,
	                                                     G_PARAM_STATIC_NAME |
	                                                         G_PARAM_STATIC_NICK |
	                                                         G_PARAM_STATIC_BLURB |
	                                                         G_PARAM_READABLE |
	                                                         G_PARAM_WRITABLE));
}

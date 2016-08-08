#include "applet-info.h"
#include "applet-widget.h"
#include "lib/definitions.h"

#include <gtk/gtk.h>

struct _ValaPanelAppletInfo
{
  ValaPanelAppletWidget* applet;
  GSettings* settings;
  gchar* icon;
  gchar* applet_type;
  gchar* name;
  gchar* description;
  gchar* uuid;
  GtkAlign alignment;
  gint position;
  gboolean expand;
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
  VALA_PANEL_APPLET_INFO_ALIGNMENT,
  VALA_PANEL_APPLET_INFO_POSITION,
  VALA_PANEL_APPLET_INFO_EXPAND
};

G_DEFINE_TYPE(ValaPanelAppletInfo, vala_panel_applet_info, G_TYPE_OBJECT)

void
vala_panel_applet_info_init(ValaPanelAppletInfo* self)
{
}

static void
vala_panel_applet_info_get_property(GObject* object, guint property_id,
                                    GValue* value, GParamSpec* pspec)
{
  ValaPanelAppletInfo* self;
  self = G_TYPE_CHECK_INSTANCE_CAST(object, vala_panel_applet_info_get_type(),
                                    ValaPanelAppletInfo);
  switch (property_id) {
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
    case VALA_PANEL_APPLET_INFO_ALIGNMENT:
      g_value_set_enum(value, self->alignment);
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

static void
vala_panel_applet_info_set_property(GObject* object, guint property_id,
                                           const GValue* value,
                                           GParamSpec* pspec)
{
  ValaPanelAppletInfo* self;
  self = G_TYPE_CHECK_INSTANCE_CAST(object, vala_panel_applet_info_get_type(),
                                    ValaPanelAppletInfo);
  switch (property_id) {
    case VALA_PANEL_APPLET_INFO_APPLET:
      self->applet = g_value_get_object(value);
      break;
    case VALA_PANEL_APPLET_INFO_SETTINGS:
      self->settings = g_value_get_object(value);
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
    case VALA_PANEL_APPLET_INFO_ALIGNMENT:
      self->alignment = g_value_get_enum(value);
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

static void
vala_panel_applet_info_finalize(GObject* obj)
{
    ValaPanelAppletInfo * self;
    self = G_TYPE_CHECK_INSTANCE_CAST (obj, vala_panel_applet_info_get_type(), ValaPanelAppletInfo);
    g_object_unref (self->settings);
    g_free0 (self->icon);
    g_free0 (self->applet_type);
    g_free0 (self->name);
    g_free0 (self->description);
    g_free0 (self->uuid);
    G_OBJECT_CLASS (vala_panel_applet_info_parent_class)->finalize (obj);
}

void
vala_panel_applet_info_class_init(ValaPanelAppletInfoClass* klass)
{
  vala_panel_applet_info_parent_class = g_type_class_peek_parent(klass);
  G_OBJECT_CLASS(klass)->get_property = vala_panel_applet_info_get_property;
  G_OBJECT_CLASS(klass)->set_property = vala_panel_applet_info_set_property;
  G_OBJECT_CLASS(klass)->finalize = vala_panel_applet_info_finalize;
  g_object_class_install_property(
    G_OBJECT_CLASS(klass), VALA_PANEL_APPLET_INFO_APPLET,
    g_param_spec_object("applet", "applet", "applet",
                        vala_panel_applet_widget_get_type(),
                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                          G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
  g_object_class_install_property(
    G_OBJECT_CLASS(klass), VALA_PANEL_APPLET_INFO_SETTINGS,
    g_param_spec_object("settings", "settings", "settings", G_TYPE_SETTINGS,
                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                          G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
  g_object_class_install_property(
    G_OBJECT_CLASS(klass), VALA_PANEL_APPLET_INFO_ICON,
    g_param_spec_string("icon", "icon", "icon", NULL,
                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                          G_PARAM_STATIC_BLURB | G_PARAM_READABLE));
  g_object_class_install_property(
    G_OBJECT_CLASS(klass), VALA_PANEL_APPLET_INFO_APPLET_TYPE,
    g_param_spec_string("applet-type", "applet-type", "applet-type", NULL,
                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                          G_PARAM_STATIC_BLURB | G_PARAM_READABLE));
  g_object_class_install_property(
    G_OBJECT_CLASS(klass), VALA_PANEL_APPLET_INFO_NAME,
    g_param_spec_string("name", "name", "name", NULL,
                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                          G_PARAM_STATIC_BLURB | G_PARAM_READABLE));
  g_object_class_install_property(
    G_OBJECT_CLASS(klass), VALA_PANEL_APPLET_INFO_DESCRIPTION,
    g_param_spec_string("description", "description", "description", NULL,
                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                          G_PARAM_STATIC_BLURB | G_PARAM_READABLE));
  g_object_class_install_property(
    G_OBJECT_CLASS(klass), VALA_PANEL_APPLET_INFO_UUID,
    g_param_spec_string("uuid", "uuid", "uuid", NULL,
                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                          G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property(
    G_OBJECT_CLASS(klass), VALA_PANEL_APPLET_INFO_ALIGNMENT,
    g_param_spec_enum("alignment", "alignment", "alignment", GTK_TYPE_ALIGN, 0,
                      G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                        G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
  g_object_class_install_property(
    G_OBJECT_CLASS(klass), VALA_PANEL_APPLET_INFO_POSITION,
    g_param_spec_int("position", "position", "position", G_MININT, G_MAXINT, 0,
                     G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                       G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
  g_object_class_install_property(
    G_OBJECT_CLASS(klass), VALA_PANEL_APPLET_INFO_EXPAND,
    g_param_spec_boolean("expand", "expand", "expand", FALSE,
                         G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                           G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
}

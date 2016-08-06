#include "applet-widget.h"
#include "applet-api.h"

typedef struct {
        char *uuid;
        char *path;
        char *filename;
        char *scheme;
        char *panel;
        GVariant *actions;
} ValaPanelAppletWidgetPrivate;

enum { VALA_PANEL_APPLET_WIDGET_DUMMY_PROPERTY,
       VALA_PANEL_APPLET_WIDGET_UUID,
       VALA_PANEL_APPLET_WIDGET_PATH,
       VALA_PANEL_APPLET_WIDGET_FILENAME,
       VALA_PANEL_APPLET_WIDGET_SCHEME,
       VALA_PANEL_APPLET_WIDGET_PANEL,
       VALA_PANEL_APPLET_WIDGET_ACTIONS };

G_DEFINE_TYPE_WITH_PRIVATE(ValaPanelAppletWidget, vala_panel_applet_widget, G_TYPE_OBJECT)

static void vala_panel_applet_widget_get_property(GObject *object, guint property_id, GValue *value,
                                                  GParamSpec *pspec)
{
        ValaPanelAppletWidget *self = VALA_PANEL_APPLET_WIDGET(object);
        self = G_TYPE_CHECK_INSTANCE_CAST(object,
                                          vala_panel_applet_widget_get_type(),
                                          ValaPanelAppletWidget);
        ValaPanelAppletWidgetPrivate *priv = vala_panel_applet_widget_get_instance_private(self);
        switch (property_id) {
        case VALA_PANEL_APPLET_WIDGET_UUID:
                g_value_set_string(value, priv->uuid);
                break;
        case VALA_PANEL_APPLET_WIDGET_PATH:
                g_value_set_string(value, priv->path);
                break;
        case VALA_PANEL_APPLET_WIDGET_FILENAME:
                g_value_set_string(value, priv->filename);
                break;
        case VALA_PANEL_APPLET_WIDGET_SCHEME:
                g_value_set_string(value, priv->scheme);
                break;
        case VALA_PANEL_APPLET_WIDGET_PANEL:
                g_value_set_string(value, priv->panel);
                break;
        case VALA_PANEL_APPLET_WIDGET_ACTIONS:
                g_value_set_variant(value, priv->actions);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
                break;
        }
}
static void vala_panel_applet_widget_set_property(GObject *object, guint property_id,
                                                  const GValue *value, GParamSpec *pspec)
{
        ValaPanelAppletWidget *self = VALA_PANEL_APPLET_WIDGET(object);
        self = G_TYPE_CHECK_INSTANCE_CAST(object,
                                          vala_panel_applet_widget_get_type(),
                                          ValaPanelAppletWidget);
        ValaPanelAppletWidgetPrivate *priv = vala_panel_applet_widget_get_instance_private(self);
        switch (property_id) {
        case VALA_PANEL_APPLET_WIDGET_UUID:
                priv->uuid = g_value_get_string(value);
                break;
        case VALA_PANEL_APPLET_WIDGET_PATH:
                priv->path = g_value_get_string(value);
                break;
        case VALA_PANEL_APPLET_WIDGET_FILENAME:
                priv->filename = g_value_get_string(value);
                break;
        case VALA_PANEL_APPLET_WIDGET_SCHEME:
                priv->scheme = g_value_get_string(value);
                break;
        case VALA_PANEL_APPLET_WIDGET_PANEL:
                priv->panel = g_value_get_string(value);
                break;
        case VALA_PANEL_APPLET_WIDGET_ACTIONS:
                priv->actions = g_value_get_variant(value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
                break;
        }
}
static void vala_panel_applet_widget_finalize(GObject *obj)
{
        ValaPanelAppletWidgetPrivate *priv = vala_panel_applet_widget_get_instance_private(obj);
        g_free(priv->uuid);
        g_free(priv->path);
        g_free(priv->filename);
        g_free(priv->scheme);
        g_free(priv->panel);
        g_variant_unref(priv->actions);
        G_OBJECT_CLASS(vala_panel_applet_widget_parent_class)->finalize(obj);
}

static void vala_panel_applet_widget_class_init(ValaPanelAppletWidgetClass *klass)
{
        vala_panel_applet_widget_parent_class = g_type_class_peek_parent(klass);
        g_type_class_add_private(klass, sizeof(ValaPanelAppletWidgetPrivate));
        G_OBJECT_CLASS(klass)->get_property = vala_panel_applet_widget_get_property;
        G_OBJECT_CLASS(klass)->set_property = vala_panel_applet_widget_set_property;
        G_OBJECT_CLASS(klass)->finalize = vala_panel_applet_widget_finalize;
        g_object_class_install_property(G_OBJECT_CLASS(klass),
                                        VALA_PANEL_APPLET_WIDGET_UUID,
                                        g_param_spec_string("uuid",
                                                            "uuid",
                                                            "uuid",
                                                            NULL,
                                                            G_PARAM_STATIC_NAME |
                                                                G_PARAM_STATIC_NICK |
                                                                G_PARAM_STATIC_BLURB |
                                                                G_PARAM_READABLE |
                                                                G_PARAM_WRITABLE));
        g_object_class_install_property(G_OBJECT_CLASS(klass),
                                        VALA_PANEL_APPLET_WIDGET_PATH,
                                        g_param_spec_string("path",
                                                            "path",
                                                            "path",
                                                            NULL,
                                                            G_PARAM_STATIC_NAME |
                                                                G_PARAM_STATIC_NICK |
                                                                G_PARAM_STATIC_BLURB |
                                                                G_PARAM_READABLE |
                                                                G_PARAM_WRITABLE));
        g_object_class_install_property(G_OBJECT_CLASS(klass),
                                        VALA_PANEL_APPLET_WIDGET_FILENAME,
                                        g_param_spec_string("filename",
                                                            "filename",
                                                            "filename",
                                                            NULL,
                                                            G_PARAM_STATIC_NAME |
                                                                G_PARAM_STATIC_NICK |
                                                                G_PARAM_STATIC_BLURB |
                                                                G_PARAM_READABLE |
                                                                G_PARAM_WRITABLE));
        g_object_class_install_property(G_OBJECT_CLASS(klass),
                                        VALA_PANEL_APPLET_WIDGET_SCHEME,
                                        g_param_spec_string("scheme",
                                                            "scheme",
                                                            "scheme",
                                                            NULL,
                                                            G_PARAM_STATIC_NAME |
                                                                G_PARAM_STATIC_NICK |
                                                                G_PARAM_STATIC_BLURB |
                                                                G_PARAM_READABLE |
                                                                G_PARAM_WRITABLE));
        g_object_class_install_property(G_OBJECT_CLASS(klass),
                                        VALA_PANEL_APPLET_WIDGET_PANEL,
                                        g_param_spec_string("panel",
                                                            "panel",
                                                            "panel",
                                                            NULL,
                                                            G_PARAM_STATIC_NAME |
                                                                G_PARAM_STATIC_NICK |
                                                                G_PARAM_STATIC_BLURB |
                                                                G_PARAM_READABLE |
                                                                G_PARAM_WRITABLE));
        g_object_class_install_property(G_OBJECT_CLASS(klass),
                                        VALA_PANEL_APPLET_WIDGET_ACTIONS,
                                        g_param_spec_variant("actions",
                                                             "actions",
                                                             "actions",
                                                             G_TYPE_STRV,
                                                             G_PARAM_STATIC_NAME |
                                                                 G_PARAM_STATIC_NICK |
                                                                 G_PARAM_STATIC_BLURB |
                                                                 G_PARAM_CONSTRUCT_ONLY |
                                                                 G_PARAM_READABLE |
                                                                 G_PARAM_WRITABLE));
        g_signal_new("panel_size_changed",
                     vala_panel_applet_widget_get_type(),
                     G_SIGNAL_RUN_LAST,
                     0,
                     NULL,
                     NULL,
                     g_cclosure_user_marshal_VOID__ENUM_INT_INT,
                     G_TYPE_NONE,
                     3,
                     GTK_TYPE_ORIENTATION,
                     G_TYPE_INT,
                     G_TYPE_INT);
}

static void vala_panel_applet_widget_init()
{
}

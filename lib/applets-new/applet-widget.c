#include "applet-widget.h"
#include "applet-api.h"

typedef struct {
        char *uuid;
        char *path;
        char *filename;
        char *scheme;
        char *panel;
        GVariant *options;
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
}
static void vala_panel_applet_widget_set_property(GObject *object, guint property_id,
                                                  const GValue *value, GParamSpec *pspec)
{
}
static void vala_panel_applet_widget_finalize(GObject *obj)
{
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
                                        g_param_spec_boxed("actions",
                                                           "actions",
                                                           "actions",
                                                           G_TYPE_STRV,
                                                           G_PARAM_STATIC_NAME |
                                                               G_PARAM_STATIC_NICK |
                                                               G_PARAM_STATIC_BLURB |
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

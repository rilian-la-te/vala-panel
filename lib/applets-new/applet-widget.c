#define G_SETTINGS_ENABLE_BACKEND 1
#include <gio/gsettingsbackend.h>

#include "applet-api-marshal.h"
#include "applet-api.h"
#include "applet-widget.h"
#include "lib/definitions.h"

typedef struct
{
	const char *uuid;
	GSettings *settings;
	const char *panel;
	GVariant *actions;
} ValaPanelAppletWidgetPrivate;

enum
{
	VALA_PANEL_APPLET_WIDGET_DUMMY_PROPERTY,
	VALA_PANEL_APPLET_WIDGET_UUID,
	VALA_PANEL_APPLET_WIDGET_SETTINGS,
	VALA_PANEL_APPLET_WIDGET_PANEL,
	VALA_PANEL_APPLET_WIDGET_ACTIONS
};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(ValaPanelAppletWidget, vala_panel_applet_widget, GTK_TYPE_BIN)

static void vala_panel_applet_widget_get_property(GObject *object, guint property_id, GValue *value,
                                                  GParamSpec *pspec)
{
	ValaPanelAppletWidget *self = VALA_PANEL_APPLET_WIDGET(object);
	self                        = G_TYPE_CHECK_INSTANCE_CAST(object,
	                                  vala_panel_applet_widget_get_type(),
	                                  ValaPanelAppletWidget);
	ValaPanelAppletWidgetPrivate *priv =
	    (ValaPanelAppletWidgetPrivate *)vala_panel_applet_widget_get_instance_private(self);
	switch (property_id)
	{
	case VALA_PANEL_APPLET_WIDGET_UUID:
		g_value_set_string(value, priv->uuid);
		break;
	case VALA_PANEL_APPLET_WIDGET_SETTINGS:
		g_value_set_object(value, priv->settings);
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
	self                        = G_TYPE_CHECK_INSTANCE_CAST(object,
	                                  vala_panel_applet_widget_get_type(),
	                                  ValaPanelAppletWidget);
	ValaPanelAppletWidgetPrivate *priv =
	    (ValaPanelAppletWidgetPrivate *)vala_panel_applet_widget_get_instance_private(self);
	switch (property_id)
	{
	case VALA_PANEL_APPLET_WIDGET_UUID:
		priv->uuid = g_value_get_string(value);
		break;
	case VALA_PANEL_APPLET_WIDGET_SETTINGS:
		priv->settings = G_SETTINGS(g_value_get_object(value));
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
	ValaPanelAppletWidgetPrivate *priv =
	    (ValaPanelAppletWidgetPrivate *)vala_panel_applet_widget_get_instance_private(
	        (ValaPanelAppletWidget *)obj);
	g_object_unref(priv->settings);
	g_variant_unref(priv->actions);
	G_OBJECT_CLASS(vala_panel_applet_widget_parent_class)->finalize(obj);
}

static void vala_panel_applet_widget_class_init(ValaPanelAppletWidgetClass *klass)
{
	vala_panel_applet_widget_parent_class = g_type_class_peek_parent(klass);
	g_type_class_add_private(klass, sizeof(ValaPanelAppletWidgetPrivate));
	G_OBJECT_CLASS(klass)->get_property = vala_panel_applet_widget_get_property;
	G_OBJECT_CLASS(klass)->set_property = vala_panel_applet_widget_set_property;
	G_OBJECT_CLASS(klass)->finalize     = vala_panel_applet_widget_finalize;
	//	klass->update_popup                 = NULL;
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
	                                VALA_PANEL_APPLET_WIDGET_SETTINGS,
	                                g_param_spec_object("settings",
	                                                    "settings",
	                                                    "settings",
	                                                    G_TYPE_SETTINGS,
	                                                    G_PARAM_STATIC_NAME |
	                                                        G_PARAM_STATIC_NICK |
	                                                        G_PARAM_STATIC_BLURB |
	                                                        G_PARAM_READABLE |
	                                                        G_PARAM_WRITABLE |
	                                                        G_PARAM_CONSTRUCT_ONLY));
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
	                                                     "as",
	                                                     NULL,
	                                                     G_PARAM_STATIC_NAME |
	                                                         G_PARAM_STATIC_NICK |
	                                                         G_PARAM_STATIC_BLURB |
	                                                         G_PARAM_CONSTRUCT_ONLY |
	                                                         G_PARAM_READABLE |
	                                                         G_PARAM_WRITABLE));
	g_signal_new("panel-size-changed",
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

static void vala_panel_applet_widget_init(ValaPanelAppletWidget *self)
{
	ValaPanelAppletWidgetPrivate *priv =
	    (ValaPanelAppletWidgetPrivate *)vala_panel_applet_widget_get_instance_private(self);
	gtk_widget_set_can_focus(GTK_WIDGET(self), false);
}

// void vala_panel_applet_widget_update_popup(ValaPanelAppletWidget *self, ValaPanelPopupManager
// *mgr)
//{
//	if (self)
//	{
//		ValaPanelAppletWidgetClass *klass = VALA_PANEL_APPLET_WIDGET_GET_CLASS(self);
//		if (klass->update_popup)
//			klass->update_popup(self, mgr);
//	}
//}

void vala_panel_applet_widget_invoke_applet_action(ValaPanelAppletWidget *self, const char *action,
                                                   GVariantDict *param)
{
	if (self)
	{
		ValaPanelAppletWidgetClass *klass = VALA_PANEL_APPLET_WIDGET_GET_CLASS(self);
		ValaPanelAppletWidgetPrivate *priv =
		    (ValaPanelAppletWidgetPrivate *)vala_panel_applet_widget_get_instance_private(
		        self);
		g_autofree const char **actions = g_variant_get_strv(priv->actions, NULL);
		if (klass->invoke_applet_action && g_strv_contains(actions, action))
			klass->invoke_applet_action(self, action, param);
	}
}

GtkWidget *vala_panel_applet_widget_get_settings_ui(ValaPanelAppletWidget *self)
{
	if (self)
	{
		ValaPanelAppletWidgetClass *klass = VALA_PANEL_APPLET_WIDGET_GET_CLASS(self);
		if (klass->get_settings_ui)
			klass->get_settings_ui(self);
	}
	return NULL;
}

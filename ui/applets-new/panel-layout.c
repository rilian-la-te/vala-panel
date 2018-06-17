#include "panel-layout.h"
#include "settings-manager.h"

ValaPanelCoreSettings *core_settings;
ValaPanelAppletManager *manager;

struct _ValaPanelAppletLayout
{
	GtkBox __parent__;
	const char *toplevel_id;
};

G_DEFINE_TYPE(ValaPanelAppletLayout, vala_panel_applet_layout, GTK_TYPE_BOX)

static void vala_panel_applet_layout_init(ValaPanelAppletLayout *self)
{
}

static void vala_panel_applet_layout_class_init(ValaPanelAppletLayoutClass *klass)
{
	manager       = vala_panel_applet_manager_new();
	core_settings = vala_panel_toplevel_get_core_settings();
}

ValaPanelAppletLayout *vala_panel_applet_layout_new(ValaPanelToplevel *top, GtkOrientation orient,
                                                    int spacing)
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
	                                             "toplevel-id",
	                                             vala_panel_toplevel_get_uuid(top),
	                                             NULL));
}

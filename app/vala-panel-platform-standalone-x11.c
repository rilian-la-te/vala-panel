#include "vala-panel-platform-standalone-x11.h"

struct _ValaPanelPlatformX11
{
	GObject __parent__;
};

static void vala_panel_platform_x11_default_init(ValaPanelPlatformInterface *iface);
G_DEFINE_TYPE_WITH_CODE(ValaPanelPlatformX11, vala_panel_platform_x11, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(vala_panel_platform_get_type(),
                                              vala_panel_platform_x11_default_init))

ValaPanelPlatformX11 *vala_panel_platform_x11_new()
{
	return VALA_PANEL_PLATFORM_X11(g_object_new(vala_panel_platform_x11_get_type(), NULL));
}

static void vala_panel_platform_x11_default_init(ValaPanelPlatformInterface *iface)
{
}

static void vala_panel_platform_x11_init(ValaPanelPlatformX11 *self)
{
}

static void vala_panel_platform_x11_class_init(ValaPanelPlatformX11Class *klass)
{
}

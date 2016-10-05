#ifndef VALAPANELPLATFORMSTANDALONEX11_H
#define VALAPANELPLATFORMSTANDALONEX11_H

#include "panel-platform.h"
#include <glib-object.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ValaPanelPlatformX11, vala_panel_platform_x11, VALA_PANEL, PLATFORM_X11,
                     GObject)

ValaPanelPlatformX11 *vala_panel_platform_x11_new();

G_END_DECLS

#endif // VALAPANELPLATFORMSTANDALONEX11_H

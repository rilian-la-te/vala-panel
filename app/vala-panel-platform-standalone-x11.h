/*
 * vala-panel
 * Copyright (C) 2015-2017 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VALAPANELPLATFORMSTANDALONEX11_H
#define VALAPANELPLATFORMSTANDALONEX11_H

#include "panel-platform.h"
#include <glib-object.h>

#define VALA_PANEL_APPLICATION_SETTINGS "org.valapanel"
#define VALA_PANEL_APPLICATION_PANELS "toplevels"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ValaPanelPlatformX11, vala_panel_platform_x11, VALA_PANEL, PLATFORM_X11,
                     GObject)

ValaPanelPlatformX11 *vala_panel_platform_x11_new(const char *profile);

G_END_DECLS

#endif // VALAPANELPLATFORMSTANDALONEX11_H

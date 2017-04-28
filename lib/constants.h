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

#ifndef CONSTANTS_H
#define CONSTANTS_H

/*
 * GSettings constants
 */
#define VALA_PANEL_CORE_SCHEMA "org.valapanel"
#define VALA_PANEL_TOPLEVEL_SCHEMA "org.valapanel.toplevel"
#define VALA_PANEL_TOPLEVEL_SCHEMA_ELEM "toplevel"
#define VALA_PANEL_OBJECT_PATH "/org/vala-panel/objects/"
#define VALA_PANEL_OBJECT_PATH_TEMPLATE "/org/vala-panel/objects/%s/"
#define VALA_PANEL_CONFIG_HEADER "global"
#define VALA_PANEL_PLUGIN_SCHEMA "org.valapanel.toplevel.plugin"

/*
 * Plugins keys
 */
#define VALA_PANEL_KEY_NAME "plugin-type"
#define VALA_PANEL_KEY_EXPAND "is-expanded"
#define VALA_PANEL_KEY_CAN_EXPAND "can-expand"
#define VALA_PANEL_KEY_PACK "pack-type"
#define VALA_PANEL_KEY_POSITION "position"
#define VALA_PANEL_KEY_APPLETS "applets"

/*
 * Autohide gap
 */
#define GAP 2

#define MAX_PANEL_HEIGHT 200
/*
 * Toplevel keys
 */
#define VALA_PANEL_KEY_EDGE "edge"
#define VALA_PANEL_KEY_ALIGNMENT "alignment"
#define VALA_PANEL_KEY_ORIENTATION "orientation"
#define VALA_PANEL_KEY_HEIGHT "height"
#define VALA_PANEL_KEY_WIDTH "width"
#define VALA_PANEL_KEY_DYNAMIC "is-dynamic"
#define VALA_PANEL_KEY_AUTOHIDE "autohide"
#define VALA_PANEL_KEY_SHOW_HIDDEN "show-hidden"
#define VALA_PANEL_KEY_STRUT "strut"
#define VALA_PANEL_KEY_DOCK "dock"
#define VALA_PANEL_KEY_MONITOR "monitor"
#define VALA_PANEL_KEY_MARGIN "panel-margin"
#define VALA_PANEL_KEY_ICON_SIZE "icon-size"
#define VALA_PANEL_KEY_BACKGROUND_COLOR "background-color"
#define VALA_PANEL_KEY_FOREGROUND_COLOR "foreground-color"
#define VALA_PANEL_KEY_BACKGROUND_FILE "background-file"
#define VALA_PANEL_KEY_FONT "font"
#define VALA_PANEL_KEY_CORNERS_SIZE "round-corners-size"
#define VALA_PANEL_KEY_USE_BACKGROUND_COLOR "use-background-color"
#define VALA_PANEL_KEY_USE_FOREGROUND_COLOR "use-foreground-color"
#define VALA_PANEL_KEY_USE_FONT "use-font"
#define VALA_PANEL_KEY_FONT_SIZE_ONLY "font-size-only"
#define VALA_PANEL_KEY_USE_BACKGROUND_FILE "use-background-file"

#endif // CONSTANTS_H

/*
 * vala-panel
 * Copyright (C) 2015 Konstantin Pugin <ria.freelander@gmail.com>
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

using GLib;
using Gtk;

namespace ValaPanel
{
    public static void setup_icon(Image img, Icon icon, Toplevel? top = null, int size = -1)
    {
        img.set_from_gicon(icon,IconSize.INVALID);
        if (top != null)
            top.bind_property(Key.ICON_SIZE,img,"pixel-size",BindingFlags.DEFAULT|BindingFlags.SYNC_CREATE);
        else if (size > 0)
            img.set_pixel_size(size);
    }
    public static void setup_icon_button(Button btn, Icon? icon = null, string? label = null, Toplevel? top = null)
    {
        PanelCSS.apply_from_resource(btn,"/org/vala-panel/lib/style.css","-panel-icon-button");
        PanelCSS.apply_with_class(btn,"",Gtk.STYLE_CLASS_BUTTON,true);
        Image? img = null;
        if (icon != null)
        {
            img = new Image();
            setup_icon(img,icon,top);
        }
        setup_button(btn, img, label);
        btn.set_border_width(0);
        btn.set_can_focus(false);
        btn.set_has_window(false);
    }
}

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

namespace MenuMaker
{
    public static const string ATTRIBUTE_DND_SOURCE = "x-dnd-source";
    public static const string ATTRIBUTE_TOOLTIP = "x-tooltip";
    public static const TargetEntry[] menu_targets = {
        { "text/uri-list", 0, 0},
        { "application/x-desktop", 0, 0},
        { "text/x-commandline", 0, 0}
    };
    public static void append_all_sections(GLib.Menu menu1, GLib.MenuModel menu2)
    {
        for (int i = 0; i< menu2.get_n_items(); i++)
        {
            var link = menu2.get_item_link(i,GLib.Menu.LINK_SECTION);
            var labelv = menu2.get_item_attribute_value(i,"label",
                                                        GLib.VariantType.STRING);
            var label = labelv != null ? labelv.get_string(): null;
            if (link != null)
                menu1.append_section(label,link);
        }
    }

    public static void activate_menu_launch_id(SimpleAction? action, Variant? param)
    {
        var id = param.get_string();
        var info = new DesktopAppInfo(id);
        try{
        info.launch(null,Gdk.Display.get_default().get_app_launch_context());
        } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
    }

    public static void activate_menu_launch_command(SimpleAction? action, Variant? param)
    {
        var command = param.get_string();
        try{
        GLib.AppInfo info = AppInfo.create_from_commandline(command,null,
                    AppInfoCreateFlags.SUPPORTS_STARTUP_NOTIFICATION);
        info.launch(null,Gdk.Display.get_default().get_app_launch_context());
        } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
    }

    public static void activate_menu_launch_uri(SimpleAction? action, Variant? param)
    {
        var uri = param.get_string();
        try{
        GLib.AppInfo.launch_default_for_uri(uri,
                    Gdk.Display.get_default().get_app_launch_context());
        } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
    }
    private static void apply_menu_dnd(Gtk.MenuItem item, MenuModel section, int model_item)
    {
        Icon? icon = null;
        string[]? uri_list = null;
        string? str = null;
        Variant? val = null;
        MenuAttributeIter attr_iter = section.iterate_item_attributes(model_item);
        while(attr_iter.get_next(out str,out val))
        {
            if (str == GLib.Menu.ATTRIBUTE_ICON)
                icon = Icon.deserialize(val);
            if (str == GLib.Menu.ATTRIBUTE_TARGET)
            {
                uri_list = new string[1];
                uri_list[0] = val.get_string();
            }
        }
        if (uri_list != null)
        {
            if (icon == null)
            {
                try{
                    icon = Icon.new_for_string("system-run-symbolic");
                } catch (Error e){}
            }
        // Make the this widget a DnD source.
            Gtk.drag_source_set (
                    item,                      // widget will be drag-able
                    Gdk.ModifierType.BUTTON1_MASK, // modifier that will start a drag
                    menu_targets,               // lists of target to support
                    Gdk.DragAction.COPY            // what to do with data after dropped
                );
            Gtk.drag_source_set_icon_gicon(item,icon);
            item.drag_data_get.connect((context,data,type,time)=>{
                data.set_uris(uri_list);
            });
        }
    }
    public static void apply_menu_properties(List<Widget> w, MenuModel menu)
    {
        unowned List<Widget> l = w;
        for(var i = 0; i < menu.get_n_items(); i++)
        {
            var jumplen = 1;
            if (l.data is SeparatorMenuItem) l = l.next;
            var shell = l.data as Gtk.MenuItem;
            string? str = null;
            Variant? val = null;
            MenuAttributeIter attr_iter = menu.iterate_item_attributes(i);
            while(attr_iter.get_next(out str,out val))
            {
                if (str == GLib.Menu.ATTRIBUTE_ICON)
                    shell.set("icon",Icon.deserialize(val));
                if (str == ATTRIBUTE_TOOLTIP)
                    shell.set_tooltip_text(val.get_string());
                if (str == ATTRIBUTE_DND_SOURCE && val.get_boolean())
                    apply_menu_dnd(l.data as Gtk.MenuItem, menu, i);
            }
            var menuw = shell.submenu;
            MenuLinkIter iter = menu.iterate_item_links(i);
            MenuModel? link_menu;
            while (iter.get_next(out str, out link_menu))
            {
                if (menuw != null && str == GLib.Menu.LINK_SUBMENU)
                    apply_menu_properties(menuw.get_children(),link_menu);
                else if (str == GLib.Menu.LINK_SECTION)
                {
                    jumplen += (link_menu.get_n_items() - 1);
                    apply_menu_properties(l,link_menu);
                }
            }
            l = l.nth(jumplen);
            if (l == null) break;
        }
    }
}

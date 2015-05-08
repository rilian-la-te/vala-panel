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
    [Compact,Immutable]
    public class SpawnData
    {
        internal Posix.pid_t pid; /* getpgid(getppid()); */
        public SpawnData()
        {
            pid = Posix.getpgid(Posix.getppid());
        }
        public void child_spawn_func()
        {
            Posix.setpgid(0,this.pid);
        }
    }
    [Compact,CCode (ref_function = "menu_maker_drag_data_ref", unref_function = "menu_maker_drag_data_unref")]
    private class DragData
    {
        internal unowned MenuModel section;
        internal unowned Gtk.MenuItem menuitem;
        internal int item_pos;
        internal Volatile ref_count;
        internal static void destroy (Widget w, DragData data)
        {
            SignalHandler.disconnect_by_data(data.menuitem,data);
            Gtk.drag_source_unset(data.menuitem);
            data.unref();
        }
        internal DragData(Gtk.MenuItem item, MenuModel section, int model_item)
        {
            this.section = section;
            this.menuitem = item;
            item_pos = model_item;
            this.ref_count = 1;
        }
        internal void @get (Gdk.DragContext context, SelectionData data, uint info, uint time_)
        {
            string[]? uri_list = null;
            string action,target;
            section.get_item_attribute(item_pos,GLib.Menu.ATTRIBUTE_ACTION,"s",out action);
            section.get_item_attribute(item_pos,GLib.Menu.ATTRIBUTE_TARGET,"s",out target);
            if (action == "app.launch-id")
            {
                try
                {
                    var appinfo = new DesktopAppInfo(target);
                    target = Filename.to_uri(appinfo.get_filename());
                } catch (GLib.Error e){}
            }
            uri_list = new string[1];
            uri_list[0] = target;
            data.set_uris(uri_list);
        }
        internal void begin(Gdk.DragContext context)
        {
            var val = section.get_item_attribute_value(item_pos,GLib.Menu.ATTRIBUTE_ICON,null);
            var icon = Icon.deserialize(val);
            if (icon != null)
                Gtk.drag_source_set_icon_gicon(menuitem,icon);
            else
                Gtk.drag_source_set_icon_name(menuitem,"system-run-symbolic");
        }
        internal unowned DragData @ref ()
        {
            GLib.AtomicInt.inc (ref this.ref_count);
            return this;
        }
        internal void unref ()
        {
            if (GLib.AtomicInt.dec_and_test (ref this.ref_count))
                this.free ();
        }
        private extern void free ();
    }
    public static const string ATTRIBUTE_DND_SOURCE = "x-valapanel-dnd-source";
    public static const string ATTRIBUTE_TOOLTIP = "x-valapanel-tooltip";
    public static const TargetEntry[] MENU_TARGETS = {
        { "text/uri-list", 0, 0},
        { "application/x-desktop", 0, 0},
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
    public static void launch_callback(DesktopAppInfo info, Pid pid)
    {

    }
    public static AppInfo? get_default_for_uri(string uri)
    {
        /* g_file_query_default_handler() calls
        * g_app_info_get_default_for_uri_scheme() too, but we have to do it
        * here anyway in case GFile can't parse @uri correctly.
        */
        AppInfo? app_info = null;
        var uri_scheme = Uri.parse_scheme (uri);
        if (uri_scheme != null && uri_scheme.length <= 0)
            app_info = AppInfo.get_default_for_uri_scheme (uri_scheme);
        if (app_info == null)
        {
            var file = File.new_for_uri (uri);
            try
            {
                app_info = file.query_default_handler (null);
            } catch (GLib.Error e){}
        }
        return app_info;
    }
    public static void activate_menu_launch_id(SimpleAction? action, Variant? param)
    {
        unowned string id = param.get_string();
        var info = new DesktopAppInfo(id);
        try{
            var data = new SpawnData();
            info.launch_uris_as_manager(null,
                                         Gdk.Display.get_default().get_app_launch_context(),
                                         SpawnFlags.SEARCH_PATH | SpawnFlags.DO_NOT_REAP_CHILD,
                                         data.child_spawn_func,launch_callback);
        } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
    }

    public static void activate_menu_launch_command(SimpleAction? action, Variant? param)
    {
        unowned string command = param.get_string();
        try{
            var data = new SpawnData();
            var info = AppInfo.create_from_commandline(command,null,
                            AppInfoCreateFlags.SUPPORTS_STARTUP_NOTIFICATION) as DesktopAppInfo;
            info.launch_uris_as_manager(null,
                                         Gdk.Display.get_default().get_app_launch_context(),
                                         SpawnFlags.SEARCH_PATH | SpawnFlags.DO_NOT_REAP_CHILD,
                                         data.child_spawn_func,launch_callback);
        } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
    }

    public static void activate_menu_launch_uri(SimpleAction? action, Variant? param)
    {
        unowned string uri = param.get_string();
        try{
            var data = new SpawnData();
            var info = get_default_for_uri(uri) as DesktopAppInfo;
            List<string> uri_l = new List<string>();
            uri_l.append(uri);
            info.launch_uris_as_manager(uri_l,
                                         Gdk.Display.get_default().get_app_launch_context(),
                                         SpawnFlags.SEARCH_PATH | SpawnFlags.DO_NOT_REAP_CHILD,
                                         data.child_spawn_func,launch_callback);
        } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
    }
    private static void apply_menu_dnd(Gtk.MenuItem item, MenuModel section, int model_item)
    {
        // Make the this widget a DnD source.
        Gtk.drag_source_set (
                item,                      // widget will be drag-able
                Gdk.ModifierType.BUTTON1_MASK, // modifier that will start a drag
                MENU_TARGETS,               // lists of target to support
                Gdk.DragAction.COPY            // what to do with data after dropped
            );
        var data = new DragData(item,section,model_item);
        data.ref();
        item.drag_begin.connect(data.begin);
        item.drag_data_get.connect(data.get);
        Signal.connect(item,"destroy",(GLib.Callback)DragData.destroy,data);
    }
    public static void apply_menu_properties(List<unowned Widget> w, MenuModel menu)
    {
        unowned List<unowned Widget> l = w;
        for(var i = 0; i < menu.get_n_items(); i++)
        {
            var jumplen = 1;
            if (l.data is SeparatorMenuItem) l = l.next;
            unowned Gtk.MenuItem shell = l.data as Gtk.MenuItem;
            unowned string? str = null;
            var has_section = false;
            var has_submenu = false;
            unowned MenuShell menuw = shell.submenu;
            MenuLinkIter iter = menu.iterate_item_links(i);
            MenuModel? link_menu;
            while (iter.get_next(out str, out link_menu))
            {
                has_section = has_section || (str == GLib.Menu.LINK_SECTION);
                has_submenu = has_submenu || (str == GLib.Menu.LINK_SUBMENU);
                if (menuw != null && has_submenu)
                    apply_menu_properties(menuw.get_children(),link_menu);
                else if (has_section)
                {
                    jumplen += (link_menu.get_n_items() - 1);
                    apply_menu_properties(l,link_menu);
                }
            }
            Variant? val = null;
            MenuAttributeIter attr_iter = menu.iterate_item_attributes(i);
            while(attr_iter.get_next(out str,out val))
            {
                if (str == GLib.Menu.ATTRIBUTE_ICON && (has_submenu || has_section))
                {
                    var icon = Icon.deserialize(val);
                    shell.set("icon",icon);
                }
                if (str == ATTRIBUTE_TOOLTIP)
                    shell.set_tooltip_text(val.get_string());
                if (str == ATTRIBUTE_DND_SOURCE && val.get_boolean())
                    apply_menu_dnd(l.data as Gtk.MenuItem, menu, i);
            }
            l = l.nth(jumplen);
            if (l == null) break;
        }
    }
}

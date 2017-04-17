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

using ValaPanel;
using Gtk;
using GLib;

public const TargetEntry[] MENU_TARGETS = {
    { "text/uri-list", 0, 0},
    { "application/x-desktop", 0, 0},
};

namespace LaunchBar
{
    public class AppletImpl : AppletPlugin, Peas.ExtensionBase
    {
        public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                        GLib.Settings? settings,
                                        uint number)
        {
            return new Bar(toplevel,settings,number);
        }
    }
    private const string BUTTONS = "launch-buttons";
    public class Bar: Applet, AppletConfigurable
    {
        internal string[]? ids;
        FlowBox layout;
        string[]? prev_ids;
        AppInfoMonitor? app_monitor;
        public Bar(ValaPanel.Toplevel toplevel,
                                        GLib.Settings? settings,
                                        uint number)
        {
            base(toplevel,settings,number);
        }
        private Dialog get_config_dialog()
        {
            return new ConfigDialog(this);
        }
        private void update_buttons_from_gsettings()
        {
            var loaded_ids = this.settings.get_strv(LaunchBar.BUTTONS);
            load_buttons(loaded_ids);
        }
        public override void create()
        {
            layout = new FlowBox();
            Gtk.drag_dest_set (
                    layout,                     // widget that will accept a drop
                    DestDefaults.MOTION       // default actions for dest on DnD
                    | DestDefaults.HIGHLIGHT,
                    MENU_TARGETS,              // lists of target to support
                    Gdk.DragAction.COPY|Gdk.DragAction.MOVE        // what to do with data after dropped
                );
            layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
            layout.activate_on_single_click = true;
            layout.selection_mode = SelectionMode.SINGLE;
            add(layout);
            toplevel.notify["edge"].connect((o,a)=> {
                layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
            });
            layout.drag_drop.connect(drag_drop_cb);
            layout.drag_data_received.connect(drag_data_received_cb);
            layout.set_sort_func(launchbar_layout_sort_func);
            settings.changed[LaunchBar.BUTTONS].connect(update_buttons_from_gsettings);
            app_monitor = AppInfoMonitor.get();
            app_monitor.changed.connect(update_buttons_from_gsettings);
            update_buttons_from_gsettings();
            layout.child_activated.connect((ch)=>{
                var lb = ch as LaunchBar.Button;
                lb.launch();
                layout.unselect_child(lb);
            });
            show_all();
        }
        internal void request_remove_id(string id)
        {
            int idx;
            for(idx = 0; idx< ids.length; idx++)
                if (ids[idx]==id) break;
            prev_ids = ids;
            ids = concat_strv_uniq(ids[0:idx],ids[idx+1:ids.length]);
        }
        internal void commit_ids()
        {
            settings.set_strv(LaunchBar.BUTTONS,ids);
        }
        internal void undo_removal_request()
        {
            ids = prev_ids;
            prev_ids = null;
        }
        private bool drag_drop_cb (Widget widget, Gdk.DragContext context,
                                   int x, int y, uint time)
        {
            bool is_valid_drop_site = true;
            if (context.list_targets() != null)
            {
                var target_type = (Gdk.Atom) context.list_targets().nth_data (0);
                Gtk.drag_get_data (widget, context, target_type, time);
            }
            else
                is_valid_drop_site = false;
            return is_valid_drop_site;
        }
        private void drag_data_received_cb (Widget widget, Gdk.DragContext context,
                                            int x, int y,
                                            SelectionData selection_data,
                                            uint target_type, uint time)
        {
            bool delete_selection_data = false;
            bool dnd_success = false;
            int index = 0;
            Gdk.Rectangle r = {x, y, 1, 1};
            var flowbox = widget as FlowBox;
            foreach(var w in flowbox.get_children())
            {
                if (w.intersect(r,null))
                    break;
                index++;
            }
            if (context.get_suggested_action() == Gdk.DragAction.MOVE) {
                delete_selection_data = true;
            }
            if ((selection_data != null) && (selection_data.get_length() >= 0)) {
                string []? new_ids = null;
                var loaded_ids = selection_data.get_uris();
                if (index>=0)
                    new_ids = concat_strv_uniq(ids[0:index],loaded_ids,ids[index:ids.length]);
                else
                    new_ids = concat_strv_uniq(loaded_ids,ids);
                settings.set_strv(BUTTONS,new_ids);
                dnd_success = true;
            }
            if (dnd_success == false) {
                stderr.printf ("Invalid DnD data.\n");
            }
            Gtk.drag_finish(context,dnd_success,delete_selection_data,time);
        }
        private void load_buttons(string[] loaded_ids)
        {
            string[] widget_ids = {};
            prev_ids = ids;
            ids = loaded_ids;
            foreach(var w in layout.get_children())
            {
                var lb = w as LaunchBar.Button;
                if (!(lb.id in ids))
                    layout.remove(w);
                else widget_ids += lb.id;
            }
            foreach(var id in ids)
            {
                if (!(id in widget_ids))
                {
                    AppInfo info;
                    string content_type;
                    ButtonType type = load_button_info(id,out info, out content_type);
                    if (type != ButtonType.NONE)
                    {
                        LaunchBar.Button? btn = null;
                        if (content_type != null)
                            btn = new LaunchBar.Button.with_content_type(info,id,type,content_type);
                        else
                            btn = new LaunchBar.Button(info,id,type);
                        toplevel.bind_property(Key.ICON_SIZE,btn,"icon-size",BindingFlags.SYNC_CREATE);
                        layout.add(btn);
                    }
                }
            }
            layout.invalidate_sort();
            if (ids.length == 0)
            {
                var btn = new LaunchBar.Button(null,null,ButtonType.BOOTSTRAP);
                toplevel.bind_property(Key.ICON_SIZE,btn,"icon-size",BindingFlags.SYNC_CREATE);
                layout.add(btn);
            }
            layout.show_all();
        }
        private ButtonType load_button_info(string str_pretend, out AppInfo info, out string content_type)
        {
            info = null;
            content_type = null;
            if (str_pretend == BOOTSTRAP)
                return ButtonType.BOOTSTRAP;
            info = new DesktopAppInfo(str_pretend) as AppInfo;
            if (info != null)
                return ButtonType.DESKTOP;
            try
            {
                var filename = Filename.from_uri(str_pretend);
                content_type = ContentType.guess(filename,null,null);
                if (content_type == "application/x-desktop")
                {
                    info = new DesktopAppInfo.from_filename(filename) as AppInfo;
                    if (info != null)
                        return ButtonType.DESKTOP;
                    return ButtonType.NONE;
                }
                else
                {
                    if (FileUtils.test(filename,FileTest.IS_EXECUTABLE) && !FileUtils.test(filename,FileTest.IS_DIR))
                    {
                        info = AppInfo.create_from_commandline(filename,null,0);
                        return ButtonType.EXECUTABLE;
                    }
                    else
                    {
                        info = MenuMaker.get_default_for_uri(str_pretend);
                        if (info != null)
                            return ButtonType.URI;
                        return ButtonType.NONE;
                    }
                }
            } catch (GLib.Error e){}
            return ButtonType.NONE;
        }
        private int launchbar_layout_sort_func(FlowBoxChild a, FlowBoxChild b)
        {
            var lb_1 = a as LaunchBar.Button;
            var lb_2 = b as LaunchBar.Button;
            var inta = -1;
            var intb = -1;
            for (int i=0; i < ids.length; i++) {
                if(lb_1.id == ids[i]) inta = i;
                if(lb_2.id == ids[i]) intb = i;
            }
            return inta - intb;
        }
        private string[] concat_strv_uniq(string[] arr1, string[]? arr2 = null, string[]? arr3 = null)
        {
            var res = arr1;
            if (arr2 != null)
                foreach(var s in arr2)
                    if(!(s in res)) res+=s;
            if (arr3 != null)
                foreach(var s in arr3)
                    if(!(s in res)) res+=s;
            return res;
        }
        internal Icon get_icon_from_id (string id)
        {
            foreach(var ch in layout.get_children())
            {
                var bt = ch as LaunchBar.Button;
                if (bt.id == id)
                    return bt.icon;
            }
            return new ThemedIcon.with_default_fallbacks("image-missing-symbolic");
        }
        internal string get_display_name_from_id(string id)
        {
            foreach(unowned Widget ch in layout.get_children())
            {
                var bt = ch as LaunchBar.Button;
                if (bt.id == id)
                    return bt.display_name;
            }
            return id;
        }
    } // End class
}

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(LaunchBar.AppletImpl));
}

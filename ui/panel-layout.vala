/*
 * vala-panel
 * Copyright (C) 2017 Konstantin Pugin <ria.freelander@gmail.com>
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
using Gdk;
using Config;

namespace ValaPanel
{
    public class Layout : Gtk.Box
    {
        private static unowned CoreSettings core_settings = null;
        public static AppletHolder holder = null;
        public string toplevel_id {get; construct;}
        public Layout(ValaPanel.Toplevel top, Gtk.Orientation orient, int spacing)
        {
            Object(orientation: orient,
                   spacing: spacing,
                   baseline_position: BaselinePosition.CENTER,
                   border_width: 0, hexpand: true, vexpand: true,
                   toplevel_id: top.uuid);
        }
        static construct
        {
            holder = new AppletHolder();
            core_settings = Toplevel.core_settings;
        }
        construct
        {
            holder.applet_ready_to_place.connect(on_applet_ready_to_place);
            holder.applet_loaded.connect(on_applet_loaded);
        }
        public void init_applets()
        {
            foreach(var unit in core_settings.core_settings.get_strv(ValaPanel.Settings.CORE_UNITS))
            {
                unowned UnitSettings pl = core_settings.get_by_uuid(unit);
                if (!pl.is_toplevel() && pl.default_settings.get_string(Settings.TOPLEVEL_ID) == this.toplevel_id)
                    holder.load_applet(pl);
            }
        }
        public void add_applet(string type)
        {
            unowned UnitSettings s = core_settings.add_unit_settings(type,false);
            s.default_settings.set_string(Key.NAME,type);
            s.default_settings.set_string(Settings.TOPLEVEL_ID,this.toplevel_id);
            holder.load_applet(s);
        }
        private void on_applet_loaded(string type)
        {
            foreach (var unit in core_settings.core_settings.get_strv(Settings.CORE_UNITS))
            {
                unowned UnitSettings pl = core_settings.get_by_uuid(unit);
                if (!pl.is_toplevel() && pl.default_settings.get_string(Settings.TOPLEVEL_ID) == this.toplevel_id)
                {
                    if (pl.default_settings.get_string(Key.NAME) == type)
                    {
                        place_applet(holder.applet_ref(type),pl);
                        update_applet_positions();
                        return;
                    }
                }
            }
        }
        private void on_applet_ready_to_place(AppletPlugin applet_plugin, UnitSettings pl)
        {
            if (!pl.is_toplevel() && pl.default_settings.get_string(Settings.TOPLEVEL_ID) == this.toplevel_id)
                place_applet(applet_plugin,pl);
        }

        public void place_applet(AppletPlugin applet_plugin, UnitSettings s)
        {
            var aw = applet_plugin.get_applet_widget(this.get_parent().get_parent() as ValaPanel.Toplevel,s.custom_settings,s.uuid);
            unowned Applet applet = aw;
            var position = s.default_settings.get_uint(Key.POSITION);
            this.pack_start(applet,false, true);
            this.reorder_child(applet,(int)position);
            if (applet_plugin.plugin_info.get_external_data(Data.EXPANDABLE)!=null)
            {
                s.default_settings.bind(Key.EXPAND,applet,"hexpand",GLib.SettingsBindFlags.GET);
                applet.bind_property("hexpand",applet,"vexpand",BindingFlags.SYNC_CREATE);
            }
            applet.destroy.connect(()=>{
                    string uuid = applet.uuid;
                    applet_destroyed(uuid);
                    if (this.in_destruction())
                        core_settings.remove_unit_settings(uuid);
            });
        }
        public void remove_applet(Applet applet)
        {
            unowned UnitSettings s = core_settings.get_by_uuid(applet.uuid);
            applet.destroy();
            core_settings.remove_unit_settings_full(s.uuid, true);
        }
        public void applet_destroyed(string uuid)
        {
            unowned UnitSettings s = core_settings.get_by_uuid(uuid);
            var name = s.default_settings.get_string(Key.NAME);
            holder.applet_unref(name);
        }
        public void update_applet_positions()
        {
            var children = this.get_children();
            for (unowned List<unowned Widget> l = children; l != null; l = l.next)
            {
                var idx = get_applet_settings(l.data as Applet).default_settings.get_uint(Key.POSITION);
                this.reorder_child((l.data as Applet),(int)idx);
            }
        }
        public List<unowned Widget> get_applets_list()
        {
            return this.get_children();
        }
        public unowned UnitSettings get_applet_settings(Applet pl)
        {
            return core_settings.get_by_uuid(pl.uuid);
        }
        public uint get_applet_position(Applet pl)
        {
            int res;
            this.child_get(pl,"position",out res, null);
            return (uint)res;
        }
        public void set_applet_position(Applet pl, int pos)
        {
            this.reorder_child(pl,pos);
        }
    }
}

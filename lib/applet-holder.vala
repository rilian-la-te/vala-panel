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

using Gtk;
using Peas;
using Config;

namespace ValaPanel
{
    public class AppletHolder: Object
    {
        [Compact]
        private class PluginData
        {
            internal unowned AppletPlugin plugin = null;
            internal int count = 0;
        }
        private Peas.Engine engine;
        private Peas.ExtensionSet extset;
        private HashTable<string,PluginData?> loaded_types;
        public signal void applet_loaded(string type);
        internal signal void applet_ready_to_place(AppletPlugin plugin, UnitSettings s);
        construct
        {
            engine = new Peas.Engine.with_nonglobal_loaders();
            engine.add_search_path(PLUGINS_DIRECTORY,PLUGINS_DATA);
            loaded_types = new HashTable<string,PluginData?>(str_hash,str_equal);
            extset = new Peas.ExtensionSet(engine,typeof(AppletPlugin));
            extset.extension_added.connect(on_extension_added);
            extset.@foreach((s,i,ext)=>{
                on_extension_added(i,ext);
            });
        }
        private void on_extension_added(Peas.PluginInfo i, Object p)
        {
            unowned AppletPlugin plugin = p as AppletPlugin;
            unowned string type = i.get_module_name();
            if (!loaded_types.contains(type))
            {
                var data = new PluginData();
                data.plugin = plugin;
                data.count = 0;
                loaded_types.insert(type,(owned)data);
            }
            applet_loaded(type);
        }
        internal unowned AppletPlugin? applet_ref(string name)
        {
            if (loaded_types.contains(name))
            {
                unowned PluginData? data = loaded_types.lookup(name);
                if (data!=null)
                {
                    data.count +=1;
                    unowned AppletPlugin pl = data.plugin;
                    if (pl.get_plugin_info().is_loaded())
                        return pl;
                }
            }
            return null;
        }
        internal void load_applet(UnitSettings s)
        {
            string name = s.default_settings.get_string(Key.NAME);
            var pl = applet_ref(name);
            if (pl != null)
                applet_ready_to_place(pl,s);
            // Got this far we actually need to load the underlying plugin
            unowned Peas.PluginInfo? plugin = null;

            foreach(unowned Peas.PluginInfo plugini in engine.get_plugin_list())
            {
                if (plugini.get_module_name() == name)
                {
                    plugin = plugini;
                    break;
                }
            }
            if (plugin == null) {
                warning("Could not find plugin: %s", name);
                return;
            }
            engine.try_load_plugin(plugin);
        }
        public void applet_unref(string name)
        {
            unowned PluginData data = loaded_types.lookup(name);
            data.count -= 1;
            if (data.count <= 0)
            {
                unowned AppletPlugin pl = loaded_types.lookup(name).plugin;
                loaded_types.remove(name);
                unowned Peas.PluginInfo info = pl.plugin_info;
                engine.try_unload_plugin(info);
            }
        }
        internal unowned AppletPlugin get_plugin(Applet pl, CoreSettings core_settings)
        {
            return loaded_types.lookup((core_settings.get_by_uuid(pl.uuid)
                                        .default_settings.get_string(Key.NAME))).plugin;
        }
        internal unowned List<Peas.PluginInfo> get_all_types()
        {
            return engine.get_plugin_list();
        }
    }
}

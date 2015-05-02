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

namespace ValaPanel
{
    private static const string PLUGIN_SCHEMA = "org.valapanel.toplevel.plugin";
    private static const string PANEL_SCHEMA = "org.valapanel.toplevel";
    private static const string PANEL_PATH = "/org/vala-panel/toplevel/";
    private static const string ROOT_NAME = "toplevel-settings";

    namespace Key
    {
        internal static const string NAME = "plugin-type";
        private static const string SCHEMA = "has-schema";
        internal static const string EXPAND = "is-expanded";
        internal static const string CAN_EXPAND = "can-expand";
        internal static const string PACK = "pack-type";
        internal static const string POSITION = "position";
    }
    [Compact, CCode (ref_function = "vala_panel_plugin_settings_ref", unref_function = "vala_panel_plugin_settings_unref")]
    internal class PluginSettings
    {
        internal string path_append;
        internal GLib.Settings default_settings;
        internal GLib.Settings config_settings;
        internal uint number;
        internal Volatile ref_count;
        internal PluginSettings(ToplevelSettings settings, string name, uint num)
        {
            this.number = num;
            this.ref_count = 1;
            this.path_append = name;
            var path = "%s%u/".printf(settings.root_path,this.number);
            this.default_settings = new GLib.Settings.with_backend_and_path(
                                                PLUGIN_SCHEMA, settings.backend, path);
        }
        internal void init_configuration(ToplevelSettings settings, bool has_config)
        {
            default_settings.set_boolean(Key.SCHEMA,has_config);
            if (has_config)
            {
                var id = "%s.%s".printf(settings.root_schema, this.path_append);
                var path = "%s%u/".printf(settings.root_path,this.number);
                this.config_settings = new GLib.Settings.with_backend_and_path(
                                                id, settings.backend, path);
            }
        }
        public unowned PluginSettings @ref ()
        {
            GLib.AtomicInt.add (ref this.ref_count, 1);
            return this;
        }
        public void unref ()
        {
            if (GLib.AtomicInt.dec_and_test (ref this.ref_count))
                this.free ();
        }
        private extern void free ();
    }
    [Compact, CCode (ref_function = "vala_panel_toplevel_settings_ref", unref_function = "vala_panel_toplevel_settings_unref")]
    internal class ToplevelSettings
    {
        internal unowned GLib.SList<PluginSettings> plugins;
        internal GLib.Settings settings;
        internal GLib.SettingsBackend backend;
        internal string filename;
        internal string root_name;
        internal string root_schema;
        internal string root_path;
        internal Volatile ref_count;
        internal ToplevelSettings.full(string file, string schema, string path, string? root)
        {
            this.filename = file;
            this.root_name = root;
            this.root_path = path;
            this.root_schema = schema;
            this.ref_count = 1;
            backend = new KeyfileSettingsBackend(file,path,root);
            settings = new GLib.Settings.with_backend_and_path (schema,backend,path);
        }

        internal ToplevelSettings (string file)
        {
            ToplevelSettings.full(file,PANEL_SCHEMA,PANEL_PATH,ROOT_NAME);
        }

        internal uint find_free_num()
        {
            var f = new GLib.KeyFile();
            try{
                f.load_from_file(this.filename,GLib.KeyFileFlags.KEEP_COMMENTS);
            } catch (GLib.KeyFileError e) {} catch (GLib.FileError e) {}
            var numtable = new GLib.GenericSet<int>(direct_hash,direct_equal);
            var len = f.get_groups().length;
            foreach (var grp in f.get_groups())
            {
                if (grp == this.root_name)
                    continue;
                numtable.add(int.parse(grp));
            }
            for (var i = 0; i < len; i++)
                if (!numtable.contains(i))
                    return i;
            return len+1;
        }

        internal PluginSettings add_plugin_settings_full(string name, uint num)
        {
            var settings = new PluginSettings(this,name,num);
            plugins.append(settings);
            return settings;
        }
        internal PluginSettings add_plugin_settings(string name)
        {
            var num = find_free_num ();
            var settings = new PluginSettings(this,name,num);
            plugins.append(settings);
            return settings;
        }

        internal void remove_plugin_settings(uint num)
        {
            foreach (var tmp in plugins)
            {
                if (tmp.number == num)
                {
                    plugins.remove(tmp);
                    var f = new GLib.KeyFile();
                    try
                    {
                        f.load_from_file(this.filename,GLib.KeyFileFlags.KEEP_COMMENTS);
                        if (f.has_group(num.to_string()))
                        {
                            f.remove_group(num.to_string());
                            f.save_to_file(this.filename);
                        }
                    }
                    catch (GLib.KeyFileError e) {} catch (GLib.FileError e) {}
                    return;
                }
            }
        }
        internal bool init_plugin_list()
        {
            if (plugins != null)
                return false;
            var f = new GLib.KeyFile();
            try {
                f.load_from_file(this.filename,GLib.KeyFileFlags.KEEP_COMMENTS);
            }
            catch (Error e)
            {
                stderr.printf("Cannot load config file: %s\n",e.message);
                return false;
            }
            var groups = f.get_groups();
            foreach (var group in groups)
            {
                try
                {
                    var name = f.get_string(group,Key.NAME);
                    name = name._delimit("'",' ')._strip();
                    var config = f.get_boolean(group,Key.SCHEMA);
                    var s = add_plugin_settings_full(name,int.parse(group));
                    s.init_configuration(this,config);
                }
                catch (GLib.KeyFileError e)
                {
                    try{
                        f.remove_group(group);
                    } catch (GLib.KeyFileError e) {}
                }
            }
            return true;
        }
        internal PluginSettings? get_settings_by_num(uint num)
        {
            foreach (var pl in plugins)
                if (pl.number == num)
                    return pl;
            return null;
        }
        public unowned ToplevelSettings @ref ()
        {
            GLib.AtomicInt.add (ref this.ref_count, 1);
            return this;
        }
        public void unref ()
        {
            if (GLib.AtomicInt.dec_and_test (ref this.ref_count))
                this.free ();
        }
        private extern void free ();
    }
}

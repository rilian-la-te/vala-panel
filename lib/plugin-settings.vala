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
    private const string ROOT_NAME = "toplevel-settings";
    [Compact]
    internal class PluginSettings
    {
        internal string path_append;
        internal GLib.Settings default_settings;
        internal GLib.Settings config_settings;
        internal uint number;
        internal PluginSettings(ToplevelSettings settings, string name, uint num)
        {
            this.number = num;
            this.path_append = name;
            var id = "%s.%s".printf(settings.root_schema, this.path_append);
            var path = "%s%u/".printf(settings.root_path,this.number);
            this.default_settings = new GLib.Settings.with_backend_and_path(
                                                PLUGIN_SCHEMA, settings.backend, path);
            var source = SettingsSchemaSource.get_default();
            var schema = source.lookup(id,true);
            if (schema != null)
                this.config_settings = new GLib.Settings.with_backend_and_path(
                                                id, settings.backend, path);
        }
    }
    [Compact]
    internal class ToplevelSettings
    {
        internal GLib.SList<PluginSettings> plugins;
        internal GLib.Settings settings;
        internal GLib.SettingsBackend backend;
        internal string filename;
        internal string root_name;
        internal string root_schema;
        internal string root_path;
        internal ToplevelSettings.full(string file, string schema, string path, string? root)
        {
            this.filename = file;
            this.root_name = root;
            this.root_path = path;
            this.root_schema = schema;
            backend = new KeyfileSettingsBackend(file,path,root);
            settings = new GLib.Settings.with_backend_and_path (schema,backend,path);
        }

        internal ToplevelSettings (string file)
        {
            this.full(file,SETTINGS_SCHEMA,SETTINGS_PATH,ROOT_NAME);
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

        internal unowned PluginSettings add_plugin_settings_full(string name, uint num)
        {
            var settings = new PluginSettings(this,name,num);
            plugins.append((owned)settings);
            return get_settings_by_num(num);
        }
        internal unowned PluginSettings add_plugin_settings(string name)
        {
            var num = find_free_num ();
            return add_plugin_settings_full(name,num);
        }

        internal void remove_plugin_settings(uint num)
        {
            foreach (unowned PluginSettings tmp in plugins)
            {
                if (tmp.number == num)
                {
                    plugins.remove(tmp);
                    if (tmp.config_settings!= null)
                        reset_schema_with_children(tmp.config_settings);
                    reset_schema_with_children(tmp.default_settings);
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
                    add_plugin_settings_full(name,int.parse(group));
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
        internal unowned PluginSettings? get_settings_by_num(uint num)
        {
            foreach (unowned PluginSettings pl in plugins)
                if (pl.number == num)
                    return pl;
            return null;
        }
    }
}

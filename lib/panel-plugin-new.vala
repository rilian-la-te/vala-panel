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

using Gtk;
using Peas;
using Config;

namespace ValaPanel
{
    public interface PluginInterface
    {
        public abstract AppletWidget get_applet_widget(string uuid, string name, string path, string filename);
    }
    public abstract class AppletWidget : GLib.Object
    {
        public string uuid {get; set;}
        public string path {get; set;}
        public string filename {get; set;}
        public string scheme {get;set;}
        public string panel {get;set;}
        public string[] actions {get; set;}
        public signal void panel_size_changed(Gtk.Orientation orientation, int icon_size, int rows);
        public abstract void update_popup(PopupManager mgr);
        public abstract void invoke_applet_action(string action, VariantDict params);
        public abstract Widget get_settings_ui();
        public abstract GLib.Settings get_applet_settings();
    }
    public abstract class AppletInfo : GLib.Object
    {
        public unowned AppletWidget applet {get; private set;}
        public GLib.Settings settings {get; private set;}
        public string icon {get; private set;}
        public string applet_type {get; private set;}
        public string name {get; private set;}
        public string description {get; private set;}
        public string uuid {get; private set;}
        public Gtk.Align alignment {get; private set;}
        public int position {get; private set;}
        public bool expand {get; private set;}
        public void bind_plugin_settings()
        {
            settings.bind("position",this,"position",SettingsBindFlags.GET|SettingsBindFlags.SET|SettingsBindFlags.DEFAULT);
            settings.bind("alignment",this,"alignment",SettingsBindFlags.GET|SettingsBindFlags.SET|SettingsBindFlags.DEFAULT);
            settings.bind("expand",this,"expand",SettingsBindFlags.GET|SettingsBindFlags.SET|SettingsBindFlags.DEFAULT);
        }
        public void unbind_default_settings()
        {
            GLib.Settings.unbind(this,"position");
            GLib.Settings.unbind(this,"alignment");
            GLib.Settings.unbind(this,"expand");
        }
        public AppletInfo(Peas.PluginInfo info, string uuid, AppletWidget applet)
        {
            Object( icon: info.get_icon_name(),
                    name: info.get_name(),
                    description: info.get_description(),
                    applet_type: info.get_module_name(),
                    uuid: uuid,
                    applet: applet,
                    settings: applet.get_applet_settings()
            );
            bind_plugin_settings();
        }
    }
    public abstract class PopupManager
    {
        public abstract void register_popover(Widget widget, Popover popover);
        public abstract void register_menu(Widget widget, MenuModel menu);
        public abstract void unregister_popover(Widget widget);
        public abstract void unregister_menu(Widget widget);
        public abstract void show_popover(Widget widget);
        public abstract void show_menu(Widget widget);
        public abstract bool popover_is_registered(Widget widget);
        public abstract bool menu_is_registered(Widget widget);
    }
}

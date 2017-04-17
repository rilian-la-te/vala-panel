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

using ValaPanel;
public class ClockApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new Clock(toplevel,settings,number);
    }
}
public class DirmenuApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new Dirmenu(toplevel,settings,number);
    }
}
public class KbLEDApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new Kbled(toplevel,settings,number);
    }
}
public class LaunchbarApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new LaunchBar.Bar(toplevel,settings,number);
    }
}
public class MenuApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new Menu(toplevel,settings,number);
    }
}
public class SepApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new Sep(toplevel,settings,number);
    }
}

public class CoreEngine: AppletEngine, Peas.ExtensionBase
{
    private uint applet_ref_count = 0;
    private Peas.ExtensionSet extset;
    private Peas.Engine eng;
    construct
    {
        eng = Peas.Engine.get_default();
        eng.add_search_path("resource:/org/vala-panel/applets","resource:/org/vala-panel/applets");
        extset = new Peas.ExtensionSet(eng,typeof(AppletPlugin));
        foreach(unowned Peas.PluginInfo plugini in eng.get_plugin_list())
            eng.try_load_plugin(plugini);
    }
    public bool has_oafid(string oafid)
    {
        foreach(unowned Peas.PluginInfo plugini in eng.get_plugin_list())
            if (plugini.get_module_name() == oafid && plugini.is_loaded())
                return true;
        return false;
    }
    public uint applet_get_ref_count()
    {
        return applet_ref_count;
    }
    public void applet_unref()
    {
        applet_ref_count -= 1;
    }
    public ValaPanel.Applet get_applet_widget_by_oafid(Toplevel top, GLib.Settings? set,
                                                       string oafid, string uuid)
    {
        foreach(unowned Peas.PluginInfo plugini in eng.get_plugin_list())
            if (plugini.get_module_name() == oafid && plugini.is_loaded())
            {
                applet_ref_count+=1;
                AppletPlugin p = extset.get_extension(plugini) as AppletPlugin;
                return p.get_applet_widget(top,set,uuid);
            }
        assert_not_reached();
    }
}

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletEngine), typeof(CoreEngine));
}


public void peas_register_clock(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(ClockApplet));
}

public void peas_register_dirmenu(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(DirmenuApplet));
}
public void peas_register_kbled(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(KbLEDApplet));
}
public void peas_register_launchbar(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(LaunchbarApplet));
}
public void peas_register_menumodel(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(MenuApplet));
}
public void peas_register_separator(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(SepApplet));
}

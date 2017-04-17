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

public class ButtonsApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new Buttons(toplevel,settings,number);
    }
}
public class DesknoApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new Deskno(toplevel,settings,number);
    }
}
public class IconTasklist : ValaPanel.AppletPlugin, Peas.ExtensionBase
{
    public ValaPanel.Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new IconTasklistApplet(toplevel,settings,number);
    }
}
public class PagerApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new Pager(toplevel,settings,number);
    }
}
public class WincmdApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new Wincmd(toplevel,settings,number);
    }
}
public class TasklistApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new Tasklist(toplevel,settings,number);
    }
}

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(TasklistApplet));
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(WincmdApplet));
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(PagerApplet));
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(ButtonsApplet));
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(DesknoApplet));
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(IconTasklist));
}

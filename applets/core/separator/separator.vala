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
public class SepApplet : AppletPlugin
{
    public override Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new Sep(toplevel,settings,number);
    }
}
public class Sep: Applet
{
    Separator widget;
    private const string KEY_SIZE = "size";
    private const string KEY_SHOW_SEPARATOR = "show-separator";
    internal int size {get; set;}
    internal bool show_separator {get; set;}
    public Sep(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        base(toplevel,settings,number);
    }
    public override void constructed()
    {
        (this.action_group.lookup_action(AppletAction.CONFIGURE) as SimpleAction).set_enabled(true);
        widget = new Separator(toplevel.orientation == Orientation.HORIZONTAL ? Orientation.VERTICAL : Orientation.HORIZONTAL);
        this.add(widget);
        toplevel.notify["panel-gravity"].connect((pspec)=>{
            widget.orientation = toplevel.orientation == Orientation.HORIZONTAL ? Orientation.VERTICAL : Orientation.HORIZONTAL;
            if (toplevel.orientation == Orientation.HORIZONTAL)
                this.set_size_request(size,2);
            else this.set_size_request(2,size);
        });
        this.notify.connect((pspec)=>{
            if (toplevel.orientation == Orientation.HORIZONTAL)
                this.set_size_request(size,2);
            else this.set_size_request(2,size);
        });
        settings.bind(KEY_SIZE,this,KEY_SIZE,SettingsBindFlags.GET);
        settings.bind(KEY_SHOW_SEPARATOR,this,KEY_SHOW_SEPARATOR,SettingsBindFlags.GET);
        this.bind_property(KEY_SHOW_SEPARATOR,widget,"visible",BindingFlags.SYNC_CREATE);
        this.show();
    }
    public override Widget get_settings_ui()
    {
        return Configurator.generic_config_widget(this.settings,
                            _("Size"), KEY_SIZE, GenericConfigType.INT,
                            _("Visible separator"), KEY_SHOW_SEPARATOR, GenericConfigType.BOOL);
    }
} // End class

[ModuleInit]
public void g_io_separator_load(GLib.TypeModule module)
{
    // boilerplate - all modules need this
    module.use();
    GLib.IOExtensionPoint.implement(ValaPanel.Applet.EXTENSION_POINT,typeof(SepApplet),"org.valapanel.separator",10);
}

public void g_io_separator_unload(GLib.IOModule module)
{
    // boilerplate - all modules need this
}

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
public class SepApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        return new Sep(toplevel,settings,number);
    }
}
public class Sep: Applet, AppletConfigurable
{
    Separator widget;
    private const string KEY_SIZE = "size";
    private const string KEY_SHOW_SEPARATOR = "show-separator";
    internal int size {get; set;}
    internal bool show_separator {get; set;}
    public Sep(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        base(toplevel,settings,number);
    }
    public override void create()
    {
        widget = new Separator(toplevel.orientation == Orientation.HORIZONTAL ? Orientation.VERTICAL : Orientation.HORIZONTAL);
        this.add(widget);
        toplevel.notify["edge"].connect((pspec)=>{
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
        this.show_all();
    }
    public Dialog get_config_dialog()
    {
        return Configurator.generic_config_dlg(_("Separator Applet"),
                            toplevel, this.settings,
                            _("Size"), KEY_SIZE, GenericConfigType.INT,
                            _("Visible separator"), KEY_SHOW_SEPARATOR, GenericConfigType.BOOL);
    }
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(SepApplet));
}

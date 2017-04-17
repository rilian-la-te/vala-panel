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
public class KbLEDApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        return new Kbled(toplevel,settings,number);
    }
}
public class Kbled: Applet, AppletConfigurable
{
    private const string CAPS_ON = "capslock-on";
    private const string NUM_ON = "numlock-on";
    FlowBox widget;
    Gtk.Image caps;
    Gtk.Image num;
    Gdk.Keymap keymap;

    public Kbled(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        base(toplevel,settings,number);
    }
    public Dialog get_config_dialog()
    {
        Dialog dlg = Configurator.generic_config_dlg(_("Keyboard LED"),
                            toplevel, this.settings,
                            _("Show CapsLock"), CAPS_ON, GenericConfigType.BOOL,
                            _("Show NumLock"), NUM_ON, GenericConfigType.BOOL);
        dlg.set_size_request(200, -1);  /* Improve geometry */
        return dlg;
    }
    public override void create()
    {
        IconTheme.get_default().add_resource_path("/org/vala-panel/kbled/images/");
        widget = new FlowBox();
        widget.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        widget.selection_mode = SelectionMode.NONE;
        add(widget);
        caps = new Image();
        toplevel.bind_property(Key.ICON_SIZE,caps,"pixel-size",BindingFlags.DEFAULT|BindingFlags.SYNC_CREATE);
        settings.bind(CAPS_ON,caps,"visible",SettingsBindFlags.GET);
        caps.show();
        widget.add(caps);
        num = new Image();
        toplevel.bind_property(Key.ICON_SIZE,num,"pixel-size",BindingFlags.DEFAULT|BindingFlags.SYNC_CREATE);
        num.show();
        settings.bind(NUM_ON,num,"visible",SettingsBindFlags.GET);
        widget.add(num);
        widget.foreach((w)=>{w.get_style_context().remove_class("grid-child");});
        keymap = Gdk.Keymap.get_default();
        keymap.state_changed.connect(on_state_changed);
        on_state_changed();
        toplevel.notify["edge"].connect((o,a)=> {
            widget.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        });
        show_all();
    }

    /* Handle caps lock changes */
    protected void toggle_caps()
    {
        caps.set_sensitive(keymap.get_caps_lock_state());
        if (keymap.get_caps_lock_state()) {
            caps.set_tooltip_text("Caps lock is active");
            caps.set_from_icon_name("capslock-on",IconSize.INVALID);
        } else {
            caps.set_tooltip_text("Caps lock is not active");
            caps.set_from_icon_name("capslock-off",IconSize.INVALID);
        }
    }

    /* Handle num lock changes */
    protected void toggle_num()
    {
        num.set_sensitive(keymap.get_num_lock_state());
        if (keymap.get_num_lock_state()) {
            num.set_tooltip_text("Num lock is active");
            num.set_from_icon_name("numlock-on",IconSize.INVALID);
        } else {
            num.set_tooltip_text("Num lock is not active");
            num.set_from_icon_name("numlock-off",IconSize.INVALID);
        }
    }
    protected void on_state_changed()
    {
        toggle_caps();
        toggle_num();
    }
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(KbLEDApplet));
}

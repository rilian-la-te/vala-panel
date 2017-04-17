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
public class WincmdApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        return new Wincmd(toplevel,settings,number);
    }
}
public class Wincmd: Applet, AppletConfigurable
{
    private const string KEY_LEFT = "left-button-command";
    private const string KEY_MIDDLE = "middle-button-command";
    private const string KEY_TOGGLE = "toggle-iconify-and-shade";
    private const string KEY_ICON = "icon";
    internal enum Command
    {
        NONE = 0,
        ICONIFY = 1,
        SHADE = 2
    }
    private Button button;
    private Image image;
    private bool toggle_state;
    internal Command left_button_command
    {get; set;}
    internal Command middle_button_command
    {get; set;}
    internal bool toggle_iconify_and_shade
    {get; set;}
    internal string icon
    {get; set;}
    public Wincmd(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        base(toplevel,settings,number);
    }
    public Dialog get_config_dialog()
    {

       return Configurator.generic_config_dlg(_("Minimize All Windows"),
        toplevel, this.settings,
        _("Alternately iconify/shade and raise"), KEY_TOGGLE, GenericConfigType.BOOL
        /* FIXME: configure buttons 1 and 2 */);
    }
    public override void create()
    {
        button = new Button();
        image = new Image();
        settings.bind(KEY_LEFT,this,KEY_LEFT,SettingsBindFlags.GET);
        settings.bind(KEY_MIDDLE,this,KEY_MIDDLE,SettingsBindFlags.GET);
        settings.bind(KEY_TOGGLE,this,KEY_TOGGLE,SettingsBindFlags.GET);
        settings.bind(KEY_ICON,this,KEY_ICON,SettingsBindFlags.GET);
        Icon gicon = new ThemedIcon.with_default_fallbacks("preferences-desktop-wallpaper-symbolic");
        try {
            gicon = Icon.new_for_string(icon);
        } catch (Error e){
            stderr.printf("Default icon will be used\n");
            gicon = new ThemedIcon.with_default_fallbacks("preferences-desktop-wallpaper-symbolic");
        }
        setup_icon(image,gicon,toplevel);
        setup_button(button,image);
        button.set_image(image);
        button.clicked.connect(()=>{
            execute_command(left_button_command);
        });
        this.notify.connect((pspec)=>{
            update_icon();
        });
        this.add(button);
        this.show_all();
    }
    private void update_icon()
    {
        Icon gicon = new ThemedIcon.with_default_fallbacks("preferences-desktop-wallpaper-symbolic");
        try {
            gicon = Icon.new_for_string(icon);
        } catch (Error e){
            stderr.printf("Default icon will be used\n");
            gicon = new ThemedIcon.with_default_fallbacks("preferences-desktop-wallpaper-symbolic");
        }
        image.set_from_gicon(gicon,IconSize.INVALID);
    }
    private void execute_command(Command command)
    {
        Wnck.Screen scr = Wnck.Screen.get_default();
        /* Left-click to iconify. */
        if (command == Command.ICONIFY)
        {
            Gdk.X11.Screen screen = this.get_screen()  as Gdk.X11.Screen;
            Gdk.Atom atom = Gdk.Atom.intern("_NET_SHOWING_DESKTOP", false);

            /* If window manager supports _NET_SHOWING_DESKTOP, use it.
             * Otherwise, fall back to iconifying windows individually. */
            if (screen.supports_net_wm_hint(atom))
            {
                bool showing_desktop = (((!toggle_iconify_and_shade) || (!toggle_state)) ? true : false);
                scr.toggle_showing_desktop(showing_desktop);
                adjust_toggle_state();
                return;
            }
        }
        Wnck.Workspace desk = scr.get_active_workspace();
        foreach (var window in scr.get_windows())
        {
            if (window.is_visible_on_workspace(desk))
            {
                switch (command)
                {
                    case Command.NONE:
                        break;
                    case Command.ICONIFY:
                        if ((( !toggle_iconify_and_shade) || ( !toggle_state)))
                            window.minimize();
                        else
                            window.unminimize(get_current_event_time());
                        break;
                    case Command.SHADE:
                        if ((( !toggle_iconify_and_shade) || ( !toggle_state)))
                            window.shade();
                        else
                            window.unshade();
                        break;
                }
            }
        }
        adjust_toggle_state();
    }
    private void adjust_toggle_state()
    {
        if (toggle_iconify_and_shade)
            toggle_state = !toggle_state;
            else toggle_state = true;
    }
    protected override bool button_release_event(Gdk.EventButton e)
    {
        if (e.button == 2)
        {
            execute_command(middle_button_command);
            return true;
        }
        return false;
    }
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(WincmdApplet));
}

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

public class Wincmd: Applet
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

    public override void constructed()
    {
        (this.action_group.lookup_action(APPLET_ACTION_CONFIGURE) as SimpleAction).set_enabled(true);
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
        setup_icon(image,gicon,toplevel, -1);
        setup_button(button,image, null);
        button.set_image(image);
        button.clicked.connect(()=>{
            execute_command(left_button_command);
        });
        this.notify.connect((pspec)=>{
            update_icon();
        });
        this.add(button);
        image.show();
        button.show();
        this.show();
    }
    public override Widget get_settings_ui()
    {
        /* FIXME: configure buttons 1 and 2 */
        string[] names = {
            _("Alternately iconify/shade and raise")
        };
        string[] keys = {
            KEY_TOGGLE
        };
        ConfiguratorType[] types = {
            ConfiguratorType.BOOL
        };
        return generic_cfg_widget(settings, names, keys, types);
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
        if (e.button == 2 && middle_button_command != Command.NONE)
        {
            execute_command(middle_button_command);
            return true;
        }
        return false;
    }
} // End class

[ModuleInit]
public void g_io_wincmd_load(GLib.TypeModule module)
{
    // boilerplate - all modules need this
    GLib.IOExtensionPoint.implement(ValaPanel.APPLET_EXTENSION_POINT,typeof(Wincmd),"org.valapanel.wincmd",10);
}

public void g_io_wincmd_unload(GLib.IOModule module)
{
    // boilerplate - all modules need this
}

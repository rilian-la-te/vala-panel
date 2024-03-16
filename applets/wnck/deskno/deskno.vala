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

public class Deskno: Applet
{
    private const string KEY_LABELS = "wm-labels";
    private const string KEY_BOLD = "bold-font";
    internal bool wm_labels
    {get; set;}
    internal bool bold_font
    {get; set;}
    Label label;
    private ulong screen_handler;

    public override void constructed()
    {
        (this.action_group.lookup_action(APPLET_ACTION_CONFIGURE) as SimpleAction).set_enabled(true);
        label = new Label(null);
        settings.bind(KEY_LABELS,this,KEY_LABELS,SettingsBindFlags.GET);
        settings.bind(KEY_BOLD,this,KEY_BOLD,SettingsBindFlags.GET);
        toplevel.notify.connect((pspec)=>{
            if (pspec.name == "panel-gravity" || pspec.name == "monitor")
                name_update();
        });
        this.notify.connect((pspec)=>{
            name_update();
        });
        screen_handler = Wnck.Screen.get_default().active_workspace_changed.connect(name_update);
        screen_handler = Wnck.Screen.get_default().window_manager_changed.connect(name_update);
        name_update();
        this.add(label);
        label.show();
        this.show();
    }
    public override Widget get_settings_ui()
    {
        string[] names = {
            _("Bold font"),
            _("Display desktop names")
        };
        string[] keys = {
            KEY_BOLD,
            KEY_LABELS
        };
        ConfiguratorType[] types = {
            ConfiguratorType.BOOL,
            ConfiguratorType.BOOL
        };
        return generic_cfg_widget(settings, names, keys, types);
    }
    private void name_update()
    {
        var workspace = Wnck.Screen.get_default().get_active_workspace();
        if (workspace == null)
            return;
        string? name = null;
        if (wm_labels)
            name = workspace.get_name();
        else
            name = "%d".printf(workspace.get_number()+1);
        setup_label(label, name, bold_font, 1);
    }
    protected override bool button_release_event(Gdk.EventButton e)
    {
        if (e.button == 1)
        {
            /* Left-click goes to next desktop, wrapping around to first. */
            var screen = Gdk.Screen.get_default() as Gdk.X11.Screen;
            var desknum = screen.get_current_desktop();
            var desks = screen.get_number_of_desktops();
            var newdesk = desknum + 1;
            if (newdesk >= desks)
                newdesk = 0;

            /* Ask the window manager to make the new desktop current. */
            Wnck.Screen.get_default().get_workspace((int)newdesk).activate(get_current_event_time());
            return true;
        }
        return false;
    }
    ~Deskno()
    {
        Wnck.Screen.get_default().disconnect(screen_handler);
    }
} // End class

[ModuleInit]
public void g_io_deskno_load(GLib.TypeModule module)
{
    // boilerplate - all modules need this
    GLib.IOExtensionPoint.implement(ValaPanel.APPLET_EXTENSION_POINT,typeof(Deskno),"org.valapanel.deskno",10);
}

public void g_io_deskno_unload(GLib.IOModule module)
{
    // boilerplate - all modules need this
}

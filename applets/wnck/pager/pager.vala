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
private const int SIZE_GAP = 4;
public class PagerApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new Pager(toplevel,settings,number);
    }
}
public class Pager: Applet
{
    Wnck.Pager widget;
    int border;
    public Pager(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        base(toplevel,settings,number);
        widget = new Wnck.Pager();
        /* FIXME: use some global setting for border */
        this.set_border_width(0);
        toplevel.notify.connect((pspec)=>{
            if (pspec.name == "edge" || pspec.name == "height" || pspec.name == "width")
                on_params_change_callback();
        });
        widget.set_show_all(true);
        widget.set_display_mode(Wnck.PagerDisplayMode.CONTENT);
        widget.set_shadow_type(Gtk.ShadowType.IN);
        widget.set_size_request(0,0);
        this.add(widget);
        on_params_change_callback();
        this.show_all();
    }
    private void on_params_change_callback()
    {
        var h = toplevel.height;
        h -= 2 * border;
        /* set geometry */
        widget.set_orientation(toplevel.orientation);
        var rows = h / (toplevel.icon_size * 2) + 1; /* min */
        var r = (h - 2) / toplevel.icon_size; /* max */
        rows = uint.max(rows, r);
        widget.set_n_rows((int)rows);
        widget.queue_resize();
    }
    public override void update_context_menu(ref GLib.Menu parent)
    {
        Gdk.X11.Screen screen = Gdk.Screen.get_default() as Gdk.X11.Screen;
        string wm_name = screen.get_window_manager_name();
        string? path = null;
        string? config_command = null;

        /* update configure_command */
        config_command = null;
        if (wm_name == "Openbox")
        {
            path = Environment.find_program_in_path("obconf-qt");
            if (path != null)
                config_command = "obconf-qt";
            path = Environment.find_program_in_path("obconf");
            if (path != null)
                config_command = "obconf --tab 6";
        }
        else if (wm_name == "compiz")
        {
            path = Environment.find_program_in_path("simple-ccsm");
            if (path != null)
                  config_command = "simple-ccsm";
            path = Environment.find_program_in_path("ccsm");
            if (path != null)
                config_command = "ccsm";
        }
        /* FIXME: support other WMs */
        if (config_command != null)
            parent.prepend(_("Workspaces..."),"app.launch-command('%s')".printf(config_command));
    }
    private void measure(Orientation orient, int for_size, out int min, out int nat, out int base_min, out int base_nat)
    {
        if(toplevel.orientation != orient)
        {
            min = (int)toplevel.icon_size;
            nat = toplevel.height;
        }
        else
        {
            if (orient == Gtk.Orientation.HORIZONTAL)
                widget.get_preferred_width_for_height(for_size-SIZE_GAP, out min, out nat);
            else
                widget.get_preferred_height_for_width(for_size-SIZE_GAP, out min, out nat);
        }
        base_min=base_nat = -1;
    }

    public override void get_preferred_height_for_width(int width,out int min, out int nat)
    {
        int x,y;
        measure(Orientation.VERTICAL,width,out min,out nat,out x, out y);
    }
    public override void get_preferred_width_for_height(int height, out int min, out int nat)
    {
        int x,y;
        measure(Orientation.HORIZONTAL,height,out min,out nat,out x, out y);
    }
    protected override SizeRequestMode get_request_mode()
    {
        return (toplevel.orientation == Orientation.HORIZONTAL) ? SizeRequestMode.WIDTH_FOR_HEIGHT : SizeRequestMode.HEIGHT_FOR_WIDTH;
    }
    protected override void get_preferred_width(out int min, out int nat)
    {
        min = (int)toplevel.icon_size;
        nat = toplevel.height;
    }
    protected override void get_preferred_height(out int min, out int nat)
    {
        min = (int)toplevel.icon_size;
        nat = toplevel.height;
    }

} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(PagerApplet));
}

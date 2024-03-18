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
public class Clock: Applet
{
    private const string TIP_FORMAT = "tooltip-format";
    private const string LABEL_FORMAT = "clock-format";
    private const string BOLD = "bold-font";
    private ToggleButton clock;
    private enum Interval
    {
        AWAITING_FIRST_CHANGE = 0,          /* Experimenting to determine interval, waiting for first change */
        AWAITING_SECOND_CHANGE = 1,         /* Experimenting to determine interval, waiting for second change */
        ONE_SECOND = 2,         /* Determined that one second interval is necessary */
        ONE_MINUTE = 3          /* Determined that one minute interval is sufficient */
    }
    private Interval exp_interval;
    private int exp_count;
    private string? prev_clock_val;
    private uint timer;
    private Window calendar;
    internal string clock_format {get; set;}
    internal string tooltip_format {get; set;}
    internal bool bold_font {get; set;}

    public Clock(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        base(toplevel,settings,number);
    }
    public override void constructed()
    {
        (this.action_group.lookup_action(AppletAction.CONFIGURE) as SimpleAction).set_enabled(true);
        settings.bind(LABEL_FORMAT,this,LABEL_FORMAT,SettingsBindFlags.GET);
        settings.bind(TIP_FORMAT,this,TIP_FORMAT,SettingsBindFlags.GET);
        settings.bind(BOLD,this,BOLD,SettingsBindFlags.GET);
        clock = new ToggleButton();
        ValaPanel.setup_button(clock as Button,null,null);
        clock.toggled.connect(()=>{
            if (clock.get_active())
            {
                calendar = create_calendar();
                calendar.show();
            }
            else
            {
                calendar.destroy();
                calendar = null;
            }
        });
        this.notify.connect((pspec)=>{
            if (pspec.name == BOLD)
                PanelCSS.set_class(clock,get_css(),"-vala-panel-font-weight",false);
            else
            {
                if (timer != 0) Source.remove(timer);
                prev_clock_val = null;
                exp_count = 0;
                exp_interval = Interval.AWAITING_FIRST_CHANGE;
                DateTime now = new DateTime.now_local();
                timer_set(now);
                if (calendar != null)
                {
                    calendar.destroy();
                    calendar = null;
                }
            }
        });
        clock.show();
        this.add(clock);
        this.show();
    }
    public override Widget get_settings_ui()
    {
        return Configurator.generic_config_widget(this.settings,
        _("Clock Format"), LABEL_FORMAT, GenericConfigType.STR,
        _("Tooltip Format"), TIP_FORMAT, GenericConfigType.STR,
        _("Format codes: man 3 strftime; %n for line break"), null, GenericConfigType.TRIM,
        _("Bold font"), BOLD, GenericConfigType.BOOL);
    }
    private void popup_position_helper(Gtk.Widget popup,
                                      out int x, out int y)
    {
        Gtk.Allocation pa;
        Gtk.Allocation a;
        popup.realize();
        popup.get_allocation(out pa);
        if (popup.is_toplevel())
        {
            Gdk.Rectangle ext;
            popup.get_window().get_frame_extents(out ext);
            pa.width = ext.width;
            pa.height = ext.height;
        }
        if (popup is Gtk.Menu)
        {
            int min, nat, new_height = 0;
            foreach (var item in (popup as Gtk.Menu).get_children())
            {
                item.get_preferred_height(out min, out nat);
                new_height += nat;
            }
            pa.height = int.max(pa.height,new_height);
        }
        get_allocation(out a);
        get_window().get_origin(out x, out y);
        if (!get_has_window())
        {
            x += a.x;
            y += a.y;
        }
        var edge = edge_from_gravity(toplevel.panel_gravity);
        switch (edge)
        {
            case Gtk.PositionType.TOP:
                y+=a.height;
                break;
            case Gtk.PositionType.BOTTOM:
                y-=pa.height;
                break;
            case Gtk.PositionType.LEFT:
                x+=a.width;
                break;
            case Gtk.PositionType.RIGHT:
                x-=pa.width;
                break;
        }
        unowned Gdk.Monitor monitor = get_display().get_monitor_at_point(x,y);
        a = (Gtk.Allocation)monitor.get_workarea();
        x = x.clamp(a.x,a.x + a.width - pa.width);
        y = y.clamp(a.y,a.y + a.height - pa.height);
    }
    private void set_popup_position(Gtk.Widget popup)
    {
        int x,y;
        popup_position_helper(popup,out x, out y);
        if (popup is Gtk.Window)
            (popup as Window).move(x,y);
        else
            popup.get_window().move(x,y);
    }
    private string get_css()
    {
        return ".-vala-panel-font-weight{\n"
                    + " font-weight: %s;\n".printf(bold_font ? "bold" : "normal")
                    + "}";
    }
    private Window create_calendar()
    {
        /* Create a new window. */
        var win = new Window(WindowType.POPUP);
        win.set_default_size(180, 180);
        win.set_border_width(5);
        /* Create a standard calendar widget as a child of the window. */
        var calendar = new Calendar();
        var now = new DateTime.now_local();
        calendar.set_display_options(CalendarDisplayOptions.SHOW_WEEK_NUMBERS
                                    | CalendarDisplayOptions.SHOW_DAY_NAMES
                                    | CalendarDisplayOptions.SHOW_HEADING);
        calendar.mark_day(now.get_day_of_month());
        win.add(calendar);
        /* Preset the widget position right now to not move it across the screen */
        win.set_type_hint(Gdk.WindowTypeHint.UTILITY);
        win.set_transient_for(this.toplevel);
        win.set_attached_to(this.toplevel);
        calendar.show();
        set_popup_position(win);
        /* Return the widget. */
        return win;
    }
    /* Periodic timer callback.
     * Also used during initialization and configuration change to do a redraw. */
    private bool update_display()
    {
        /* Determine the current time. */
        var now = new DateTime.now_local();

        if (MainContext.current_source().is_destroyed())
            return false;

        timer_set(now);

        /* Determine the content of the clock label and tooltip. */
        string label = now.format(clock_format);
        string tooltip = now.format(tooltip_format);
        /* When we write the clock value, it causes the panel to do a full relayout.
         * Since this function may be called too often while the timing experiment is underway,
         * we take the trouble to check if the string actually changed first. */
        clock.set_label(label);
        clock.set_tooltip_text(tooltip);

        /* Conduct an experiment to see how often the value changes.
         * Use this to decide whether we update the value every second or every minute.
         * We need to account for the possibility that the experiment is being run when we cross a minute boundary. */
        if (exp_interval < Interval.ONE_SECOND)
        {
            if (prev_clock_val == null)
                /* Initiate the experiment. */
                prev_clock_val = label;
            else
            {
                if (prev_clock_val == label)
                {
                    exp_count += 1;
                    if (exp_count > 3)
                    {
                        /* No change within 3 seconds.  Assume change no more often than once per minute. */
                        exp_interval = Interval.ONE_MINUTE;
                        prev_clock_val = null;
                    }
                }
                else if (exp_interval == Interval.AWAITING_FIRST_CHANGE)
                {
                    /* We have a change at the beginning of the experiment, but we do not know when the next change might occur.
                     * Continue the experiment for 3 more seconds. */
                    exp_interval = Interval.AWAITING_SECOND_CHANGE;
                    exp_count = 0;
                    prev_clock_val = label;
                }
                else
                {
                    /* We have a second change.  End the experiment. */
                    exp_interval = ((exp_count > 3) ? Interval.ONE_MINUTE : Interval.ONE_SECOND);
                    prev_clock_val = null;
                }
            }
        }

        /* Reset the timer and return. */
        return false;
    }
    /* Set the timer. */
    private void timer_set(DateTime current_time)
    {
        uint microseconds = 1000000;
        /* Compute number of microseconds until next second boundary. */
        microseconds = 1000000 - (current_time.get_microsecond());
        /* If the expiration interval is the minute boundary,
        * add number of milliseconds after that until next minute boundary. */
        if (exp_interval == Interval.ONE_MINUTE)
        {
            uint seconds = 60 - current_time.get_second();
            microseconds += seconds * 1000000;
        }
        /* Be defensive, and set the timer. */
        if (microseconds <= 0)
            microseconds = 1000000;
        timer = Timeout.add(microseconds/1000,update_display);
    }
} // End class

[ModuleInit]
public void g_io_clock_load(GLib.TypeModule module)
{
    // boilerplate - all modules need this
    GLib.IOExtensionPoint.implement(ValaPanel.Applet.EXTENSION_POINT,typeof(Clock),"org.valapanel.clock",10);
}

public void g_io_clock_unload(GLib.IOModule module)
{
    // boilerplate - all modules need this
}

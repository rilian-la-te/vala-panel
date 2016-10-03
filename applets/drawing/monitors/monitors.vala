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
using Cairo;
public class MonitorsApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        return new Monitors(toplevel,settings,number);
    }
}

internal enum MonitorType
{
    CPU = 0,
    RAM = 1,
    N_MONS = 2
}

[Compact, CCode (ref_function = "monitor_ref", unref_function = "monitor_unref")]
internal class Monitor
{
    [CCode (has_target = false)]
    internal delegate bool UpdateMonitorFunc(Monitor m);
    [CCode (has_target = false)]
    internal delegate void UpdateTooltipFunc(Monitor m);
    private const int DEFAULT_WIDTH = 40;                 /* Pixels               */
    private const int BORDER_SIZE = 2;                  /* Pixels               */
    internal Gdk.RGBA foreground_color;  /* Foreground color for drawing area      */
    internal Gtk.DrawingArea da;               /* Drawing area                           */
    internal Cairo.Surface pixmap;     /* Pixmap to be drawn on drawing area     */
    internal int pixmap_width;      /* Width and size of the buffer           */
    internal int pixmap_height;     /* Does not include border size           */
    internal double[] stats;            /* Circular buffer of values              */
    internal double total;             /* Maximum possible value, as in mem_total*/
    internal int ring_cursor;       /* Cursor for ring/circular buffer        */
    internal int position;
    internal Volatile ref_count;
    internal unowned UpdateMonitorFunc update_monitor;
    internal unowned UpdateTooltipFunc update_tooltip;
    internal bool update()
    {
        if (update_tooltip != null && this.da != null)
            update_tooltip(this);
        return update_monitor(this);
    }
    internal Monitor(Monitors plugin, string color)
    {
        this.ref_count = 1;
        da = new DrawingArea();
        da.set_size_request(DEFAULT_WIDTH, plugin.toplevel.height);
        da.add_events(Gdk.EventMask.BUTTON_PRESS_MASK);
        foreground_color.parse(color);
        da.configure_event.connect((e)=>{
            Gtk.Allocation allocation;
            int new_pixmap_width, new_pixmap_height;
            da.get_allocation(out allocation);
            new_pixmap_width = allocation.width - BORDER_SIZE * 2;
            new_pixmap_height = allocation.height - BORDER_SIZE *2;
            if (new_pixmap_width > 0 && new_pixmap_height > 0)
            {
                /*
                 * If the stats buffer does not exist (first time we get inside this
                 * function) or its size changed, reallocate the buffer and preserve
                 * existing data.
                 */
                if (stats == null || (new_pixmap_width != pixmap_width))
                {
                    double[] new_stats = new double[new_pixmap_width];
                    if (new_stats == null)
                        return true;

                    if (stats != null)
                    {
                        /* New allocation is larger.
                         * Add new "oldest" samples of zero following the cursor*/
                        if (new_pixmap_width > pixmap_width)
                        {
                            /* Number of values between the ring cursor and the end of
                             * the buffer */
                            int nvalues = pixmap_width - ring_cursor;
                            Posix.memcpy((void*)new_stats, (void*)stats, ring_cursor * sizeof (double));
                            Posix.memcpy((void*)new_stats[nvalues:new_stats.length], (void*)stats[ring_cursor:stats.length], nvalues * sizeof(double));
                        }
                        /* New allocation is smaller, but still larger than the ring
                         * buffer cursor */
                        else if (ring_cursor <= new_pixmap_width)
                        {
                            /* Numver of values that can be stored between the end of
                             * the new buffer and the ring cursor */
                            int nvalues = new_pixmap_width - ring_cursor;
                            Posix.memcpy((void*)new_stats, (void*)stats, ring_cursor * sizeof (double));
                            Posix.memcpy((void*)new_stats[ring_cursor:new_stats.length],(void*)stats[pixmap_width-nvalues:stats.length], nvalues * sizeof(double));
                        }
                        /* New allocation is smaller, and also smaller than the ring
                         * buffer cursor.  Discard all oldest samples following the ring
                         * buffer cursor and additional samples at the beginning of the
                         * buffer. */
                        else
                        {
                            Posix.memcpy((void*)new_stats,(void*)stats[ring_cursor - new_pixmap_width:stats.length],new_pixmap_width * sizeof(double));
                        }
                    }
                    stats = new_stats;
                }

                pixmap_width = new_pixmap_width;
                pixmap_height = new_pixmap_height;
                pixmap = new ImageSurface(Format.ARGB32, (int)pixmap_width, (int)pixmap_height);
                pixmap.status();
                redraw_pixmap();
            }
            return true;
        });
        da.draw.connect((cr)=>{
            /* Draw the requested part of the pixmap onto the drawing area.
             * Translate it in both x and y by the border size. */
            if (pixmap != null)
            {
                cr.set_source_surface(pixmap, BORDER_SIZE, BORDER_SIZE);
                cr.paint();
                cr.status();
            }
            return false;
        });
        da.button_release_event.connect((e)=>{
            if (e.button == 1)
            {
                MenuMaker.activate_menu_launch_command(null,plugin.settings.get_value(Monitors.ACTION),plugin.toplevel.application);
                return true;
            }
            return false;
        });
    }
    ~Monitor()
    {
        this.da.destroy();
        this.da = null;
    }
    protected void redraw_pixmap ()
    {
        Cairo.Context cr = new Cairo.Context(pixmap);
        cr.set_line_width(1.0);
        /* Erase pixmap */
        cr.set_source_rgba(0,0,0,0);
        cr.paint();
        Gdk.cairo_set_source_rgba(cr, foreground_color);
        for (int i = 0; i < pixmap_width; i++)
        {
            uint drawing_cursor = (ring_cursor + i) % pixmap_width;
            /* Draw one bar of the graph */
            cr.move_to(i + 0.5, pixmap_height);
            cr.line_to(i + 0.5, (1.0 - stats[drawing_cursor]) * pixmap_height);
            cr.stroke();
        }
        cr.status();
        /* Redraw pixmap */
        da.queue_draw();
    }
    public unowned Monitor @ref ()
    {
        GLib.AtomicInt.add (ref this.ref_count, 1);
        return this;
    }
    public void unref ()
    {
        if (GLib.AtomicInt.dec_and_test (ref this.ref_count))
            this.free ();
    }
    private extern void free ();
}

internal class CpuMonitor : Monitor
{
    private struct cpu_stat{
        int64 u;
        int64 n;
        int64 s;
        int64 i;
    }
    internal CpuMonitor(Monitors plugin, string color)
    {
        base(plugin,color);
        position = 0;
        update_monitor = update_cpu;
        update_tooltip = tooltip_update_cpu;
    }
    internal static bool update_cpu(Monitor c)
    {
        cpu_stat previous_cpu_stat = { 0, 0, 0, 0 };
        if ((c.stats != null) && (c.pixmap != null))
        {
            /* Open statistics file and scan out CPU usage. */
            cpu_stat cpu = cpu_stat();
            Posix.FILE stat = Posix.FILE.open("/proc/stat", "r");
            if (stat == null)
                return true;
            int fscanf_result = stat.scanf("cpu %li %li %li %li",
                                        out cpu.u, out cpu.n, out cpu.s, out cpu.i);
            /* Ensure that fscanf succeeded. */
            if (fscanf_result == 4)
            {
                /* Comcolors delta from previous statistics. */
                cpu_stat cpu_delta = cpu_stat();
                cpu_delta.u = cpu.u - previous_cpu_stat.u;
                cpu_delta.n = cpu.n - previous_cpu_stat.n;
                cpu_delta.s = cpu.s - previous_cpu_stat.s;
                cpu_delta.i = cpu.i - previous_cpu_stat.i;
                /* Copy current to previous. */
                previous_cpu_stat = cpu;
                /* Comcolors user+nice+system as a fraction of total.
                 * Introduce this sample to ring buffer, increment and wrap ring
                 * buffer cursor. */
                double cpu_uns = cpu_delta.u + cpu_delta.n + cpu_delta.s;
                c.stats[c.ring_cursor] = cpu_uns / (cpu_uns + cpu_delta.i);
                c.ring_cursor += 1;
                if (c.ring_cursor >= c.pixmap_width)
                    c.ring_cursor = 0;
                /* Redraw with the new sample. */
                c.redraw_pixmap();
            }
        }
        return true;
    }
    internal static void tooltip_update_cpu(Monitor? m)
    {
        if (m!= null && m.stats != null)
        {
            int ring_pos = (m.ring_cursor == 0) ? m.pixmap_width - 1 : m.ring_cursor - 1;
            if (m.da != null)
                m.da.tooltip_text = _("CPU usage: %.2f%%").printf(m.stats[ring_pos] * 100);;
        }
    }
}

internal class MemMonitor : Monitor
{
    internal MemMonitor(Monitors plugin, string color)
    {
        base(plugin,color);
        position = 1;
        update_monitor = update_mem;
        update_tooltip = tooltip_update_mem;
    }
    internal static bool update_mem(Monitor m)
    {
        char buf[80];
        long mem_total = 0;
        long mem_free  = 0;
        long mem_buffers = 0;
        long mem_cached = 0;
        uint readmask = 0x8 | 0x4 | 0x2 | 0x1;
        if (m.stats == null || m.pixmap == null)
            return true;
        Posix.FILE meminfo = Posix.FILE.open("/proc/meminfo", "r");
        if (meminfo == null)
        {
            warning("monitors: Could not open /proc/meminfo: %d, %s",
                      Posix.errno, Posix.strerror(Posix.errno));
            return false;
        }

        while (readmask != 0 && meminfo.gets(buf) != null)
        {
            unowned string buf_str = (string)buf;
            if (buf_str.scanf("MemTotal: %ld kB\n", out mem_total) == 1)
            {
                readmask ^= 0x1;
                continue;
            }
            if (buf_str.scanf("MemFree: %ld kB\n", out mem_free) == 1)
            {
                readmask ^= 0x2;
                continue;
            }
            if (buf_str.scanf("Buffers: %ld kB\n", out mem_buffers) == 1)
            {
                readmask ^= 0x4;
                continue;
            }
            if (buf_str.scanf( "Cached: %ld kB\n", out mem_cached) == 1)
            {
                readmask ^= 0x8;
                continue;
            }
        }
        if (readmask != 0)
        {
            warning("""monitors: Could not read all values from /proc/meminfo:
                      readmask %x""", readmask);
            return false;
        }
        m.total = mem_total;
        /* Adding stats to the buffer:
         * It is debatable if 'mem_buffers' counts as free or not. I'll go with
         * 'free', because it can be flushed fairly quickly, and generally
         * isn't necessary to keep in memory.
         * It is hard to draw the line, which caches should be counted as free,
         * and which not. Pagecaches, dentry, and inode caches are quickly
         * filled up again for almost any use case. Hence I would not count
         * them as 'free'.
         * 'mem_cached' definitely counts as 'free' because it is immediately
         * released should any application need it. */
        m.stats[m.ring_cursor] = (mem_total - mem_buffers - mem_free -
                mem_cached) / (double)mem_total;

        m.ring_cursor+=1;
        if (m.ring_cursor >= m.pixmap_width)
            m.ring_cursor = 0;
        /* Redraw the pixmap, with the new sample */
        m.redraw_pixmap();
        return true;
    }
    internal static void tooltip_update_mem(Monitor? m)
    {
        if (m!= null && m.stats != null)
        {
            int ring_pos = (m.ring_cursor == 0) ? m.pixmap_width - 1 : m.ring_cursor - 1;
            if (m.da != null)
                m.da.tooltip_text = _("RAM usage: %.1fMB (%.2f%%)").printf(
                                            m.stats[ring_pos] * m.total / 1024,
                                            m.stats[ring_pos] * 100);
        }
    }
}

public class Monitors: Applet, AppletConfigurable
{
    private const uint UPDATE_PERIOD = 1; /* Seconds              */
    private const uint N_MONITORS = 2;
    private const string DISPLAY_CPU = "display-cpu-monitor";
    private const string DISPLAY_RAM = "display-ram-monitor";
    internal const string ACTION = "click-action";
    private const string CPU_CL = "cpu-color";
    private const string RAM_CL = "ram-color";
    private Monitor[] monitors;
    private bool[] displayed_mons = {true, true};
    private Gtk.Box box;
    private uint timer;
    public Monitors(ValaPanel.Toplevel toplevel,
                                  GLib.Settings? settings,
                                  uint number)
    {
        base(toplevel,settings,number);
    }
    public Dialog get_config_dialog()
    {
        return Configurator.generic_config_dlg(_("Resource monitors"),
            toplevel, this.settings,
            _("Display CPU usage"), DISPLAY_CPU, GenericConfigType.BOOL,
            _("CPU color"), CPU_CL, GenericConfigType.STR,
            _("Display RAM usage"), DISPLAY_RAM, GenericConfigType.BOOL,
            _("RAM color"), RAM_CL, GenericConfigType.STR,
            _("Action when clicked"), ACTION, GenericConfigType.STR);
    }
    public override void create()
    {
        monitors = new Monitor[2];
        box = new Gtk.Box(Orientation.HORIZONTAL,2);
        box.set_homogeneous(true);
        displayed_mons[(int)MonitorType.CPU] = settings.get_boolean(DISPLAY_CPU);
        displayed_mons[(int)MonitorType.RAM] = settings.get_boolean(DISPLAY_RAM);
        if (displayed_mons[(int)MonitorType.CPU])
            monitors[(int)MonitorType.CPU] = create_monitor(MonitorType.CPU);
        if (displayed_mons[(int)MonitorType.RAM])
            monitors[(int)MonitorType.RAM] = create_monitor(MonitorType.RAM);
        timer = Timeout.add_seconds(UPDATE_PERIOD,monitors_update);
        settings.changed.connect((key)=>{
            if (key == DISPLAY_CPU)
            {
                displayed_mons[(int)MonitorType.CPU] = settings.get_boolean(DISPLAY_CPU);
                rebuild_mons();
            }
            else if (key == DISPLAY_RAM)
            {
                displayed_mons[(int)MonitorType.RAM] = settings.get_boolean(DISPLAY_RAM);
                rebuild_mons();
            }
            else if (key == CPU_CL && monitors[(int)MonitorType.CPU] != null)
                monitors[(int)MonitorType.CPU].foreground_color.parse(settings.get_string(CPU_CL));
            else if (key == RAM_CL && monitors[(int)MonitorType.RAM] != null)
                monitors[(int)MonitorType.RAM].foreground_color.parse(settings.get_string(RAM_CL));
        });
        this.destroy.connect(()=>{Source.remove(timer);});
        this.add(box);
        this.show_all();
    }
    private void rebuild_mons()
    {
        for(int i = 0; i < (int)MonitorType.N_MONS; i++)
        {
            if (displayed_mons[i] && monitors[i] == null)
            {
                monitors[i] = create_monitor((MonitorType)i);
                box.reorder_child(monitors[i].da,monitors[i].position);
            }
            else if (!displayed_mons[i] && monitors[i] != null)
            {
                box.remove(monitors[i].da);
                monitors[i] = null;
            }
        }
    }
    private bool monitors_update()
    {
        if (MainContext.current_source().is_destroyed())
            return false;
        for (int i = 0; i < N_MONITORS; i++)
        {
            if (monitors[i] != null)
                monitors[i].update();
        }
        return true;
    }
    private Monitor? create_monitor(MonitorType type)
    {
        Monitor? new_mon = null;
        if (type == MonitorType.CPU)
            new_mon = new CpuMonitor(this,settings.get_string(CPU_CL));
        if (type == MonitorType.RAM)
            new_mon = new MemMonitor(this,settings.get_string(RAM_CL));
        if (new_mon != null)
        {
            box.pack_start(new_mon.da, false,false,0);
            new_mon.da.show();
        }
        return new_mon;
    }
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(MonitorsApplet));
}
